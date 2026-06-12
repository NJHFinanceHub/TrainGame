#include "SEEWeaponBase.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogSEEWeapon, Log, All);

namespace
{
	/** Static factory mapping: known DT_Items weapon ItemIDs -> stats + silhouette. */
	struct FSEEWeaponDef
	{
		const TCHAR* DisplayName;
		float Damage;
		float Speed;
		ESEEDamageType Type;
		ESEEWeaponShape Shape;
		float Range;
		float WeightKg;
		float Durability;
		ESEEWeaponTier Tier;
	};

	const TMap<FName, FSEEWeaponDef>& GetWeaponDefs()
	{
		static const TMap<FName, FSEEWeaponDef> Defs = []()
		{
			TMap<FName, FSEEWeaponDef> M;
			//                                          Name                Dmg   Speed  Type                        Shape                      Range  Wt    Dur    Tier
			M.Add("Item_Shiv",          FSEEWeaponDef{ TEXT("Shiv"),           12.0f, 1.3f, ESEEDamageType::Bladed,  ESEEWeaponShape::Shiv,   110.0f, 0.3f,  40.0f, ESEEWeaponTier::Improvised });
			M.Add("Item_RustyPipe",     FSEEWeaponDef{ TEXT("Rusty Pipe"),     18.0f, 0.8f, ESEEDamageType::Blunt,   ESEEWeaponShape::Pipe,   150.0f, 2.5f,  60.0f, ESEEWeaponTier::Improvised });
			M.Add("Item_Wrench",        FSEEWeaponDef{ TEXT("Heavy Wrench"),   15.0f, 1.0f, ESEEDamageType::Blunt,   ESEEWeaponShape::Wrench, 140.0f, 1.8f, 100.0f, ESEEWeaponTier::Functional });
			M.Add("Item_Machete",       FSEEWeaponDef{ TEXT("Machete"),        20.0f, 1.1f, ESEEDamageType::Bladed,  ESEEWeaponShape::Blade,  150.0f, 1.2f,  90.0f, ESEEWeaponTier::Functional });
			M.Add("Item_ReinforcedBat", FSEEWeaponDef{ TEXT("Reinforced Bat"), 22.0f, 0.7f, ESEEDamageType::Blunt,   ESEEWeaponShape::Bat,    160.0f, 3.0f,  80.0f, ESEEWeaponTier::Functional });
			M.Add("Item_JackbootBaton", FSEEWeaponDef{ TEXT("Jackboot Baton"), 16.0f, 1.2f, ESEEDamageType::Blunt,   ESEEWeaponShape::Baton,  130.0f, 1.0f, 150.0f, ESEEWeaponTier::Military });
			return M;
		}();
		return Defs;
	}
}

ASEEWeaponBase::ASEEWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Scene root so the visible mesh can be offset relative to the attach point (grip at origin)
	USceneComponent* WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
	RootComponent = WeaponRoot;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(WeaponRoot);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponHeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponHeadMesh"));
	WeaponHeadMesh->SetupAttachment(WeaponMesh);
	WeaponHeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponHeadMesh->SetVisibility(false);

	// Engine basic shapes — guaranteed present, no project asset dependencies.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	CylinderShape = CylinderFinder.Object;
	CubeShape = CubeFinder.Object;
	BasicShapeMaterial = MaterialFinder.Object;

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

bool ASEEWeaponBase::IsWeaponItemID(FName ItemID)
{
	return GetWeaponDefs().Contains(ItemID);
}

ASEEWeaponBase* ASEEWeaponBase::SpawnWeaponForItem(UWorld* World, FName ItemID, AActor* OwnerActor)
{
	if (!World)
	{
		return nullptr;
	}

	const FSEEWeaponDef* Def = GetWeaponDefs().Find(ItemID);
	if (!Def)
	{
		UE_LOG(LogSEEWeapon, Warning, TEXT("SpawnWeaponForItem: '%s' is not a known weapon ItemID"), *ItemID.ToString());
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.Owner = OwnerActor;
	Params.Instigator = Cast<APawn>(OwnerActor);
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector SpawnLoc = OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
	ASEEWeaponBase* Weapon = World->SpawnActor<ASEEWeaponBase>(ASEEWeaponBase::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
	if (!Weapon)
	{
		return nullptr;
	}

	Weapon->WeaponName = FText::FromString(Def->DisplayName);
	Weapon->BaseDamage = Def->Damage;
	Weapon->AttackSpeed = Def->Speed;
	Weapon->DamageType = Def->Type;
	Weapon->Range = Def->Range;
	Weapon->Weight = Def->WeightKg;
	Weapon->MaxDurability = Def->Durability;
	Weapon->CurrentDurability = Def->Durability;
	Weapon->Tier = Def->Tier;
	Weapon->SourceItemID = ItemID;
	Weapon->ConfigureShape(Def->Shape);

	return Weapon;
}

void ASEEWeaponBase::ConfigureShape(ESEEWeaponShape Shape)
{
	if (!WeaponMesh || !CylinderShape || !CubeShape)
	{
		UE_LOG(LogSEEWeapon, Warning, TEXT("ConfigureShape: basic shape assets unavailable, weapon will be invisible"));
		return;
	}

	// Engine basic shapes are 100cm; scale Z = length, then pitch the piece 90deg
	// so the long axis runs along local +X (forward out of the grip at origin).
	const FRotator AlongX(90.0f, 0.0f, 0.0f);
	WeaponHeadMesh->SetVisibility(false);

	switch (Shape)
	{
	case ESEEWeaponShape::Pipe: // thin 60cm cylinder
		WeaponMesh->SetStaticMesh(CylinderShape);
		WeaponMesh->SetRelativeScale3D(FVector(0.045f, 0.045f, 0.6f));
		WeaponMesh->SetRelativeRotation(AlongX);
		break;

	case ESEEWeaponShape::Shiv: // small flat wedge, ~40cm overall
		WeaponMesh->SetStaticMesh(CubeShape);
		WeaponMesh->SetRelativeScale3D(FVector(0.02f, 0.05f, 0.4f));
		WeaponMesh->SetRelativeRotation(AlongX);
		break;

	case ESEEWeaponShape::Wrench: // 50cm cylinder handle + box head
		WeaponMesh->SetStaticMesh(CylinderShape);
		WeaponMesh->SetRelativeScale3D(FVector(0.035f, 0.035f, 0.5f));
		WeaponMesh->SetRelativeRotation(AlongX);
		WeaponHeadMesh->SetStaticMesh(CubeShape);
		// Head sits at the far end of the handle. Parent scale is compensated
		// (relative transform is in parent space: X/Y thin axes, Z = length).
		WeaponHeadMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 48.0f));
		WeaponHeadMesh->SetRelativeScale3D(FVector(12.0f / 3.5f, 16.0f / 3.5f, 0.14f / 0.5f));
		WeaponHeadMesh->SetVisibility(true);
		break;

	case ESEEWeaponShape::Blade: // long flat 55cm cube (machete)
		WeaponMesh->SetStaticMesh(CubeShape);
		WeaponMesh->SetRelativeScale3D(FVector(0.015f, 0.09f, 0.55f));
		WeaponMesh->SetRelativeRotation(AlongX);
		break;

	case ESEEWeaponShape::Bat: // thick 70cm cylinder
		WeaponMesh->SetStaticMesh(CylinderShape);
		WeaponMesh->SetRelativeScale3D(FVector(0.06f, 0.06f, 0.7f));
		WeaponMesh->SetRelativeRotation(AlongX);
		break;

	case ESEEWeaponShape::Baton: // slim 55cm cylinder
	default:
		WeaponMesh->SetStaticMesh(CylinderShape);
		WeaponMesh->SetRelativeScale3D(FVector(0.032f, 0.032f, 0.55f));
		WeaponMesh->SetRelativeRotation(AlongX);
		break;
	}

	// Shift so the grip end sits near the actor origin (attach point / hand)
	const float HalfLength = WeaponMesh->GetRelativeScale3D().Z * 100.0f * 0.5f;
	WeaponMesh->SetRelativeLocation(FVector(FMath::Max(HalfLength - 10.0f, 0.0f), 0.0f, 0.0f));

	ApplyMetalTint(WeaponMesh);
	if (WeaponHeadMesh->IsVisible())
	{
		ApplyMetalTint(WeaponHeadMesh);
	}
}

void ASEEWeaponBase::ApplyMetalTint(UStaticMeshComponent* MeshComp)
{
	if (!MeshComp)
	{
		return;
	}

	if (BasicShapeMaterial)
	{
		MeshComp->SetMaterial(0, BasicShapeMaterial);
	}

	if (UMaterialInstanceDynamic* MID = MeshComp->CreateAndSetMaterialInstanceDynamic(0))
	{
		// Engine BasicShapeMaterial exposes a "Color" vector parameter
		MID->SetVectorParameterValue(TEXT("Color"), MetalTint);
	}
}
