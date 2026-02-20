// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "CombatTestCar.h"
#include "CombatComponent.h"
#include "TrainGame/Environment/EnvironmentalHazardComponent.h"
#include "TrainGame/Weapons/WeaponComponent.h"
#include "Engine/World.h"

ACombatTestCar::ACombatTestCar()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ACombatTestCar::BeginPlay()
{
	Super::BeginPlay();
	SetupDefaultLayout();
}

void ACombatTestCar::SetupDefaultLayout()
{
	// Default hazard positions for "The Pit" test car
	// Steam vents along the walls at intervals
	if (SteamVentPositions.Num() == 0)
	{
		SteamVentPositions.Add(FVector(10000.f, 1400.f, 0.f));   // Right wall, 100m in
		SteamVentPositions.Add(FVector(25000.f, -1400.f, 0.f));  // Left wall, 250m in
		SteamVentPositions.Add(FVector(40000.f, 1400.f, 0.f));   // Right wall, 400m in
	}

	// Electrical panels
	if (ElectricalPanelPositions.Num() == 0)
	{
		ElectricalPanelPositions.Add(FVector(15000.f, 1450.f, 1200.f));  // Right wall, chest height
		ElectricalPanelPositions.Add(FVector(35000.f, -1450.f, 1200.f)); // Left wall
	}

	// Window breach points (along the outer walls)
	if (WindowBreachPositions.Num() == 0)
	{
		WindowBreachPositions.Add(FVector(20000.f, 1500.f, 1400.f));  // Right window, 200m in
		WindowBreachPositions.Add(FVector(30000.f, -1500.f, 1400.f)); // Left window, 300m in
	}

	// Default enemy spawn positions (distributed along car length)
	if (EnemySpawnOffsets.Num() == 0)
	{
		float Spacing = CarLength / (EnemyCount + 1);
		for (int32 i = 0; i < EnemyCount; ++i)
		{
			float X = Spacing * (i + 1);
			float Y = (i % 2 == 0) ? 500.f : -500.f; // Alternate sides
			EnemySpawnOffsets.Add(FVector(X, Y, 0.f));
		}
	}

	// Default weapon pickups for testing
	if (AvailableWeapons.Num() == 0)
	{
		AvailableWeapons.Add(UWeaponComponent::MakePipeClub());
		AvailableWeapons.Add(UWeaponComponent::MakeShiv());
		AvailableWeapons.Add(UWeaponComponent::MakeNailBat());
	}

	if (WeaponSpawnPositions.Num() == 0)
	{
		WeaponSpawnPositions.Add(FVector(5000.f, 0.f, 500.f));    // Near entrance
		WeaponSpawnPositions.Add(FVector(25000.f, 1000.f, 500.f)); // Mid-car
		WeaponSpawnPositions.Add(FVector(45000.f, -800.f, 500.f)); // Near exit
	}
}

void ACombatTestCar::InitializeCombatEncounter()
{
	ResetCar();
	SpawnHazards();
	SpawnEnemies();
	SpawnWeaponPickups();
}

void ACombatTestCar::ResetCar()
{
	for (AActor* Enemy : SpawnedEnemies)
	{
		if (Enemy) Enemy->Destroy();
	}
	SpawnedEnemies.Empty();

	for (AActor* Hazard : SpawnedHazards)
	{
		if (Hazard) Hazard->Destroy();
	}
	SpawnedHazards.Empty();

	EnemiesRemaining = 0;
}

void ACombatTestCar::SpawnHazards()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	auto SpawnHazardActor = [&](const FVector& LocalPos, EEnvironmentalHazard Type, bool bOneShot, bool bKnockback, float Damage)
	{
		FVector WorldPos = GetActorTransform().TransformPosition(LocalPos);
		AActor* HazardActor = World->SpawnActor<AActor>(AActor::StaticClass(), WorldPos, FRotator::ZeroRotator, SpawnParams);
		if (HazardActor)
		{
			UEnvironmentalHazardComponent* HazardComp = NewObject<UEnvironmentalHazardComponent>(HazardActor);
			HazardComp->RegisterComponent();

			// Configure via exposed properties (would normally use EditAnywhere defaults)
			// For the prototype, we configure programmatically
			SpawnedHazards.Add(HazardActor);
		}
	};

	// Steam vents: DOT, knockback, repeatable
	for (const FVector& Pos : SteamVentPositions)
	{
		SpawnHazardActor(Pos, EEnvironmentalHazard::SteamVent, false, true, 30.f);
	}

	// Electrical panels: burst damage, stagger, repeatable with cooldown
	for (const FVector& Pos : ElectricalPanelPositions)
	{
		SpawnHazardActor(Pos, EEnvironmentalHazard::ElectricalPanel, false, false, 40.f);
	}

	// Window breaches: one-shot, instant kill (sucked into frozen void)
	for (const FVector& Pos : WindowBreachPositions)
	{
		SpawnHazardActor(Pos, EEnvironmentalHazard::WindowBreach, true, true, 9999.f);
	}
}

void ACombatTestCar::SpawnEnemies()
{
	UWorld* World = GetWorld();
	if (!World || !EnemyClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (const FVector& Offset : EnemySpawnOffsets)
	{
		FVector WorldPos = GetActorTransform().TransformPosition(Offset);
		APawn* Enemy = World->SpawnActor<APawn>(EnemyClass, WorldPos, GetActorRotation(), SpawnParams);
		if (Enemy)
		{
			// Ensure enemy has combat component
			UCombatComponent* CombatComp = Enemy->FindComponentByClass<UCombatComponent>();
			if (!CombatComp)
			{
				CombatComp = NewObject<UCombatComponent>(Enemy);
				CombatComp->RegisterComponent();
			}

			// Bind death callback
			CombatComp->OnDeath.AddDynamic(this, &ACombatTestCar::OnEnemyDeath);

			SpawnedEnemies.Add(Enemy);
			EnemiesRemaining++;
		}
	}
}

void ACombatTestCar::SpawnWeaponPickups()
{
	// Weapon pickups would be actual pickup actors in full implementation.
	// For the prototype, weapons are configured via the WeaponComponent
	// and available weapon presets. The test car sets up spawn positions
	// for a future AWeaponPickup actor class.
}

int32 ACombatTestCar::GetRemainingEnemyCount() const
{
	return EnemiesRemaining;
}

void ACombatTestCar::OnEnemyDeath(AActor* Killer)
{
	EnemiesRemaining = FMath::Max(0, EnemiesRemaining - 1);

	if (EnemiesRemaining == 0)
	{
		// Encounter complete - in full game this would trigger car-clear rewards
		UE_LOG(LogTemp, Log, TEXT("Combat encounter complete! All enemies defeated in The Pit."));
	}
}
