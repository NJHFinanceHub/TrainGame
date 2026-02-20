#include "SEEWeaponBase.h"
#include "Components/StaticMeshComponent.h"

ASEEWeaponBase::ASEEWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CurrentDurability = MaxDurability;
}

void ASEEWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentDurability = MaxDurability;
}

void ASEEWeaponBase::DegradeDurability(float Amount)
{
	CurrentDurability = FMath::Max(0.0f, CurrentDurability - Amount);
}

void ASEEWeaponBase::Repair(float Amount)
{
	CurrentDurability = FMath::Min(MaxDurability, CurrentDurability + Amount);
}
