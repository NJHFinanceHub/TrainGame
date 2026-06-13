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
			// Expanded improvised melee (also added as DT_Items rows).
			M.Add("Item_FireAxe",       FSEEWeaponDef{ TEXT("Fire Axe"),       26.0f, 0.7f, ESEEDamageType::Bladed,  ESEEWeaponShape::Axe,    160.0f, 3.2f, 110.0f, ESEEWeaponTier::Functional });
			M.Add("Item_SharpenedRebar",FSEEWeaponDef{ TEXT("Sharpened Rebar"), 17.0f, 1.0f, ESEEDamageType::Piercing,ESEEWeaponShape::Rebar, 170.0f, 1.6f,  70.0f, ESEEWeaponTier::Improvised });
			M.Add("Item_ChainWhip",     FSEEWeaponDef{ TEXT("Chain Whip"),     14.0f, 1.4f, ESEEDamageType::Bladed,  ESEEWeaponShape::ChainWhip, 200.0f, 1.1f, 90.0f, ESEEWeaponTier::Improvised });
			return M;
		}();
		return Defs;
	}
}

ASEEWeaponBase::ASEEWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// CO-OP REPLICATION: held weapons are server-authoritative actors. The server
	// spawns + attaches them (player quickslot equip and NPC EnsureHeldWeapon); a
	// replicated actor attached on the server replicates its attach-parent + socket
	// to clients automatically, so every client sees both player- and NPC-held
	// weapons at the right hand. ReplicateMovement stays OFF: the weapon never moves
	// independently — it rides its attach parent (the holder's mesh), and enabling
	// movement replication would fight the attachment. Standalone is authority, so
	// single-player spawns/attaches exactly as before (replication is a no-op there).
	bReplicates = true;
	SetReplicateMovement(false);

	// Scene root so the visible mesh can be offset relative to the attach point (grip at origin)
	USceneComponent* WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
	RootComponent = WeaponRoot;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(WeaponRoot);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Secondary piece (blade/head/tip) lives on the clean scene root, not on the
	// thin non-uniformly-scaled WeaponMesh, so its own scale/rotation aren't
	// sheared by the parent. Each shape positions it in root-local space.
	WeaponHeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponHeadMesh"));
	WeaponHeadMesh->SetupAttachment(WeaponRoot);
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

	// Both meshes hang off the clean WeaponRoot, so layout is in one consistent
	// root-local space: the weapon's long axis runs along +X (forward out of the
	// grip at origin). Engine basic shapes are 100cm; a cylinder is scaled on Z
	// then pitched 90deg (AlongX) so its length lies on +X. A cube's X scale is
	// its length directly. Helper lambdas keep each shape declarative.
	const FRotator AlongX(90.0f, 0.0f, 0.0f);
	WeaponHeadMesh->SetVisibility(false);
	WeaponHeadMesh->SetRelativeRotation(FRotator::ZeroRotator);
	WeaponHeadMesh->SetRelativeLocation(FVector::ZeroVector);

	// Per-shape tints: grips/hafts read as wood/tape, edges as bright steel, so a
	// held weapon doesn't look like one uniform bar. Default metal; override below.
	FLinearColor PrimaryTint = MetalTint;
	FLinearColor HeadTint = MetalTint;

	// Place a cylinder along +X: Radius (cm) and Length (cm), centre at X=CenterX.
	auto MakeBarX = [&](UStaticMeshComponent* M, float Radius, float Length, float CenterX)
	{
		M->SetStaticMesh(CylinderShape);
		M->SetRelativeRotation(AlongX);
		M->SetRelativeScale3D(FVector(Radius / 50.0f, Radius / 50.0f, Length / 100.0f));
		M->SetRelativeLocation(FVector(CenterX, 0.0f, 0.0f));
	};
	// Place a cube of full size (cm) on each axis (cube asset is 100cm, so
	// scale = Size/100). Centre at (CenterX, CenterY, CenterZ).
	auto MakeBoxX = [&](UStaticMeshComponent* M, float SizeX, float SizeY, float SizeZ,
		float CenterX, float CenterY, float CenterZ)
	{
		M->SetStaticMesh(CubeShape);
		M->SetRelativeRotation(FRotator::ZeroRotator);
		M->SetRelativeScale3D(FVector(SizeX / 100.0f, SizeY / 100.0f, SizeZ / 100.0f));
		M->SetRelativeLocation(FVector(CenterX, CenterY, CenterZ));
	};

	switch (Shape)
	{
	case ESEEWeaponShape::Pipe: // thin 60cm corroded cylinder
		MakeBarX(WeaponMesh, 2.3f, 60.0f, 25.0f);
		PrimaryTint = FLinearColor(0.18f, 0.10f, 0.06f, 1.0f); // rust
		break;

	case ESEEWeaponShape::Shiv: // short taped grip + small steel blade (~30cm)
		MakeBarX(WeaponMesh, 1.6f, 12.0f, 4.0f);          // grip at the hand
		PrimaryTint = GripTint;
		MakeBoxX(WeaponHeadMesh, 20.0f, 4.0f, 1.2f, 20.0f, 0.0f, 0.0f); // flat blade
		WeaponHeadMesh->SetVisibility(true);
		HeadTint = BladeTint;
		break;

	case ESEEWeaponShape::Wrench: // cylinder handle + box head at the far end
		MakeBarX(WeaponMesh, 1.8f, 50.0f, 20.0f);
		MakeBoxX(WeaponHeadMesh, 14.0f, 16.0f, 4.0f, 47.0f, 0.0f, 0.0f);
		WeaponHeadMesh->SetVisibility(true);
		HeadTint = BladeTint;
		break;

	case ESEEWeaponShape::Blade: // short grip + long flat steel blade (machete)
		MakeBarX(WeaponMesh, 1.8f, 14.0f, 4.0f);          // grip
		PrimaryTint = GripTint;
		MakeBoxX(WeaponHeadMesh, 46.0f, 7.0f, 1.4f, 36.0f, 0.0f, 0.0f); // blade
		WeaponHeadMesh->SetVisibility(true);
		HeadTint = BladeTint;
		break;

	case ESEEWeaponShape::Bat: // thick 70cm club, scrap-metal band near the end
		MakeBarX(WeaponMesh, 3.2f, 70.0f, 28.0f);
		PrimaryTint = FLinearColor(0.14f, 0.10f, 0.06f, 1.0f); // wood
		MakeBarX(WeaponHeadMesh, 3.9f, 14.0f, 56.0f);     // reinforcing band
		WeaponHeadMesh->SetVisibility(true);
		HeadTint = MetalTint;
		break;

	case ESEEWeaponShape::Axe: // long haft + box axe head near the top (fire axe)
		MakeBarX(WeaponMesh, 1.8f, 70.0f, 28.0f);
		PrimaryTint = FLinearColor(0.12f, 0.08f, 0.05f, 1.0f); // wood haft
		MakeBoxX(WeaponHeadMesh, 8.0f, 16.0f, 12.0f, 58.0f, 5.0f, 0.0f); // wedge head, offset
		WeaponHeadMesh->SetVisibility(true);
		HeadTint = BladeTint;
		break;

	case ESEEWeaponShape::Rebar: // thin ribbed bar, bright sharpened tip
		MakeBarX(WeaponMesh, 1.4f, 65.0f, 26.0f);
		PrimaryTint = FLinearColor(0.20f, 0.13f, 0.08f, 1.0f); // weathered steel
		MakeBarX(WeaponHeadMesh, 1.0f, 14.0f, 58.0f);     // sharpened tip
		WeaponHeadMesh->SetVisibility(true);
		HeadTint = BladeTint;
		break;

	case ESEEWeaponShape::ChainWhip: // short grip + thin trailing length
		MakeBarX(WeaponMesh, 1.6f, 16.0f, 5.0f);          // grip
		PrimaryTint = GripTint;
		MakeBarX(WeaponHeadMesh, 0.9f, 40.0f, 33.0f);     // chain stand-in
		WeaponHeadMesh->SetVisibility(true);
		HeadTint = MetalTint;
		break;

	case ESEEWeaponShape::Baton: // slim 55cm cylinder
	default:
		MakeBarX(WeaponMesh, 1.6f, 55.0f, 22.0f);
		break;
	}

	ApplyTint(WeaponMesh, PrimaryTint);
	if (WeaponHeadMesh->IsVisible())
	{
		ApplyTint(WeaponHeadMesh, HeadTint);
	}
}

void ASEEWeaponBase::ApplyTint(UStaticMeshComponent* MeshComp, const FLinearColor& Color)
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
		MID->SetVectorParameterValue(TEXT("Color"), Color);
	}
}
