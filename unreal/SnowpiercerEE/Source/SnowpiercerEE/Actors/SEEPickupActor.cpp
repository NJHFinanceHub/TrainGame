// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEEPickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "SnowpiercerEE/SEEInventoryComponent.h"
#include "SnowpiercerEE/SEEItemBase.h"

namespace SEEPickupPrivate
{
	// Short pickup IDs (and BP_Pickup_* class-name suffixes) that do not become
	// valid DT_Items rows by simply prefixing "Item_". Kept in one place so the
	// pickup actor and the level fixup subsystem agree.
	const TMap<FName, FName>& GetItemIDAliases()
	{
		static const TMap<FName, FName> Aliases =
		{
			{ FName(TEXT("Pipe")),       FName(TEXT("Item_RustyPipe")) },
			{ FName(TEXT("Cloth")),      FName(TEXT("Item_ClothStrips")) },
			{ FName(TEXT("Medicine")),   FName(TEXT("Item_Painkillers")) },
			{ FName(TEXT("FoodRation")), FName(TEXT("Item_RatMeat")) },
			{ FName(TEXT("Ration")),     FName(TEXT("Item_RatMeat")) },
			{ FName(TEXT("KeyCard")),    FName(TEXT("Item_GateKey")) },
			// "Lockpick" and "Coal" have no DT_Items row; they fall through
			// unchanged and are carried as data-less curios.
		};
		return Aliases;
	}
}

ASEEPickupActor::ASEEPickupActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// CO-OP REPLICATION: the pickup is server-authoritative. Replicating the actor
	// means the server spawns it (via the fixup subsystem / LOOT_ scatter) and it
	// appears on every client at the replicated transform; when the server grants
	// it and Destroy()s it, that destruction replicates so all clients see it
	// vanish at once. Standalone is authority, so single-player is unchanged.
	bReplicates = true;
	SetReplicateMovement(false); // static loot — transform set once, never moves

	// Clean scene root so the visible mesh sits ON the floor (mesh base at Z=0)
	// while the actor origin is the contact point used by the snap-to-floor trace.
	USceneComponent* PickupRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PickupRoot"));
	SetRootComponent(PickupRoot);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(PickupRoot);
	// The new static mesh must never block the player — it is purely cosmetic; the
	// OverlapSphere remains the only collision-enabled component (pickup trigger).
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Secondary accent piece (bottle neck, blade, lid...). Hidden until BuildPickupVisual uses it.
	AccentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AccentMesh"));
	AccentMesh->SetupAttachment(PickupRoot);
	AccentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AccentMesh->SetVisibility(false);

	// Engine basic shapes + material — guaranteed present, no project asset deps
	// (same idiom as ASEEWeaponBase and the train-dressing passes).
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	CubeShape = CubeFinder.Object;
	CylinderShape = CylinderFinder.Object;
	SphereShape = SphereFinder.Object;
	BasicShapeMaterial = MaterialFinder.Object;

	// Default visible shape so an un-configured pickup (ItemID unset, or built in a
	// no-table context) is never invisible. BuildPickupVisual overrides this once an
	// ItemID is known. Cube base sits on Z=0 (cube asset is 100cm, pivot centre).
	if (CubeShape)
	{
		VisualMesh->SetStaticMesh(CubeShape);
		VisualMesh->SetRelativeScale3D(FVector(0.18f));
		VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 9.0f)); // half of 18cm
	}

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(PickupRoot);
	OverlapSphere->InitSphereRadius(100.0f);
	// The actor origin now rests on the floor, so lift the trigger sphere to roughly
	// knee/waist height. This keeps the same generous pickup radius the player walks
	// into while making sure the sphere isn't buried half-under the floor.
	OverlapSphere->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ASEEPickupActor::HandleOverlapBegin);
}

void ASEEPickupActor::BeginPlay()
{
	Super::BeginPlay();

	// MISSING MODELS FIX: build a category-representative mesh for whatever ItemID
	// this pickup carries. Runs on server AND clients (purely cosmetic, no asset
	// streaming / no authority needed) so every machine sees the right silhouette
	// even though only the server grants the item.
	BuildPickupVisual();

	// FLOATING FIX: drop the pickup onto the floor. Authority-only because the
	// resulting transform replicates down to clients (SetReplicateMovement is off,
	// but the spawned/initial transform replicates and we move before that settles
	// on standalone/host); clients keep whatever transform they received. Standalone
	// is authority, so single-player snaps exactly here.
	if (bSnapToFloorOnBeginPlay && HasAuthority())
	{
		SnapToFloor();
	}

	// BUG FIX: a pawn standing inside the sphere when the pickup spawns (or
	// when the level streams in around the player) never received a fresh
	// BeginOverlap - sweep the initial overlaps once.
	if (bAutoPickupOnOverlap && OverlapSphere)
	{
		TArray<AActor*> Overlapping;
		OverlapSphere->GetOverlappingActors(Overlapping, APawn::StaticClass());
		for (AActor* Other : Overlapping)
		{
			if (Other && Other != this && TryPickup(Other))
			{
				break;
			}
		}
	}
}

void ASEEPickupActor::InitPickup(FName InItemID, int32 InQuantity)
{
	ItemID = InItemID;
	Quantity = FMath::Max(1, InQuantity);
	bConsumed = false;

	// The fixup subsystem calls InitPickup right after SpawnActor (pre-BeginPlay),
	// but a Blueprint/script could call it later too — rebuild the visual now so the
	// mesh always matches the assigned item rather than the placeholder cube.
	if (HasActorBegunPlay())
	{
		BuildPickupVisual();
	}
}

FName ASEEPickupActor::ResolveItemRowName(FName RawItemID, const USEEInventoryComponent* Inventory)
{
	if (RawItemID.IsNone())
	{
		return RawItemID;
	}

	// No table to validate against: prefer the alias, else keep the raw ID.
	if (!Inventory)
	{
		const FName* Alias = SEEPickupPrivate::GetItemIDAliases().Find(RawItemID);
		return Alias ? *Alias : RawItemID;
	}

	// 1. Raw ID already resolves (also covers the inventory's own
	//    "Item_<ID>" prefix fallback) - but prefer returning the exact form
	//    only when the table truly knows the raw row.
	if (Inventory->GetItemDataPtr(RawItemID))
	{
		// GetItemDataPtr accepts both raw and prefixed forms; normalise to the
		// canonical prefixed row name when that is what actually matched, so
		// quest Collect objectives (which target "Item_*" IDs) count the item.
		const FString RawString = RawItemID.ToString();
		if (!RawString.StartsWith(TEXT("Item_")))
		{
			const FName Prefixed(*(TEXT("Item_") + RawString));
			if (Inventory->GetItemDataPtr(Prefixed))
			{
				return Prefixed;
			}
		}
		return RawItemID;
	}

	// 2. Explicit alias (Pipe -> Item_RustyPipe, ...)
	if (const FName* Alias = SEEPickupPrivate::GetItemIDAliases().Find(RawItemID))
	{
		if (Inventory->GetItemDataPtr(*Alias))
		{
			return *Alias;
		}
	}

	// 3/4. Nothing matched - keep the raw ID (item still stows, data-less).
	return RawItemID;
}

ESEEItemCategory ASEEPickupActor::ResolveItemCategory(FName InItemID)
{
	if (InItemID.IsNone())
	{
		return ESEEItemCategory::Junk;
	}

	// Load DT_Items directly (no inventory needed at visual-build time). Same table
	// and idiom USEEInventoryComponent resolves by default.
	const UDataTable* ItemTable = LoadObject<UDataTable>(nullptr,
		TEXT("/Game/DataTables/DT_Items.DT_Items"));
	if (!ItemTable)
	{
		return ESEEItemCategory::Junk;
	}

	static const FString Context(TEXT("SEEPickupActor::ResolveItemCategory"));

	// Try the raw row name, then the "Item_" prefixed form (BP_Pickup_* stubs carry
	// short IDs like "Bandage"/"Pipe" that map to "Item_Bandage"/"Item_RustyPipe").
	if (const FSEEItemData* Row = ItemTable->FindRow<FSEEItemData>(InItemID, Context, /*bWarnIfMissing*/false))
	{
		return Row->Category;
	}

	const FString Raw = InItemID.ToString();
	if (!Raw.StartsWith(TEXT("Item_")))
	{
		const FName Prefixed(*(TEXT("Item_") + Raw));
		if (const FSEEItemData* Row = ItemTable->FindRow<FSEEItemData>(Prefixed, Context, false))
		{
			return Row->Category;
		}
	}

	// Honour the same short-ID aliases the grant path uses (Pipe -> Item_RustyPipe...).
	if (const FName* Alias = SEEPickupPrivate::GetItemIDAliases().Find(InItemID))
	{
		if (const FSEEItemData* Row = ItemTable->FindRow<FSEEItemData>(*Alias, Context, false))
		{
			return Row->Category;
		}
	}

	return ESEEItemCategory::Junk;
}

void ASEEPickupActor::TintMesh(UStaticMeshComponent* MeshComp, const FLinearColor& Color)
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
		// Engine BasicShapeMaterial exposes a "Color" vector parameter.
		MID->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

void ASEEPickupActor::BuildPickupVisual()
{
	if (!VisualMesh || !CubeShape || !CylinderShape || !SphereShape)
	{
		return; // basic shapes unavailable — keep whatever the constructor set
	}

	const ESEEItemCategory Category = ResolveItemCategory(ItemID);

	// Helper: configure a basic-shape mesh component so its BOUNDS BOTTOM rests at
	// Z=0 (actor origin). Engine basic shapes are 100cm: Cube/Cylinder pivots
	// are centred, so a shape of height H (= 100 * ScaleZ) is lifted by H/2; the
	// Sphere pivot is its centre too. This keeps the snap-to-floor contact correct.
	auto SetShape = [](UStaticMeshComponent* M, UStaticMesh* Mesh,
		const FVector& Scale, const FRotator& Rot, float ExtraZ)
	{
		M->SetStaticMesh(Mesh);
		M->SetRelativeRotation(Rot);
		M->SetRelativeScale3D(Scale);
		const float HalfHeight = 50.0f * Scale.Z; // 100cm asset, half = 50 * scaleZ
		M->SetRelativeLocation(FVector(0.0f, 0.0f, HalfHeight + ExtraZ));
		M->SetVisibility(true);
	};

	// Reset accent each rebuild; categories that need it re-enable below.
	AccentMesh->SetVisibility(false);

	// Tints chosen to read at a glance in the dim train interior.
	const FLinearColor TinCan   (0.62f, 0.63f, 0.60f, 1.0f); // dull tin
	const FLinearColor GlassGreen(0.18f, 0.42f, 0.22f, 1.0f); // med/bottle glass
	const FLinearColor Steel    (0.55f, 0.57f, 0.60f, 1.0f); // bright steel
	const FLinearColor RustWood (0.18f, 0.12f, 0.07f, 1.0f); // rust / wood haft
	const FLinearColor Crate    (0.30f, 0.20f, 0.10f, 1.0f); // scrap crate
	const FLinearColor Cloth    (0.32f, 0.30f, 0.27f, 1.0f); // folded cloth/plate
	const FLinearColor QuestGlow(0.95f, 0.80f, 0.25f, 1.0f); // small glowing curio

	switch (Category)
	{
	case ESEEItemCategory::Consumable:
	{
		// Can / bottle: a short tin cylinder body with a small cone/cap accent so it
		// reads as a ration tin or medicine bottle.
		SetShape(VisualMesh, CylinderShape, FVector(0.12f, 0.12f, 0.16f), FRotator::ZeroRotator, 0.0f);
		TintMesh(VisualMesh, TinCan);
		// Bottle-neck cap on top.
		AccentMesh->SetStaticMesh(CylinderShape);
		AccentMesh->SetRelativeRotation(FRotator::ZeroRotator);
		AccentMesh->SetRelativeScale3D(FVector(0.05f, 0.05f, 0.05f));
		AccentMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 16.0f + 2.5f));
		AccentMesh->SetVisibility(true);
		TintMesh(AccentMesh, GlassGreen);
		break;
	}

	case ESEEItemCategory::Weapon:
	{
		// Improvised melee silhouette: a thin bar laid forward + flat steel edge,
		// echoing ASEEWeaponBase's grip+blade look without spawning a weapon actor.
		// Cylinder pitched 90deg so its length runs along +X, then lifted to lie flat.
		VisualMesh->SetStaticMesh(CylinderShape);
		VisualMesh->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f)); // long axis -> +X
		VisualMesh->SetRelativeScale3D(FVector(0.04f, 0.04f, 0.55f));  // ~4cm dia, ~55cm long
		VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 5.0f));    // rests on floor
		VisualMesh->SetVisibility(true);
		TintMesh(VisualMesh, RustWood);
		// Flat steel edge near the forward end.
		AccentMesh->SetStaticMesh(CubeShape);
		AccentMesh->SetRelativeRotation(FRotator::ZeroRotator);
		AccentMesh->SetRelativeScale3D(FVector(0.30f, 0.05f, 0.015f));
		AccentMesh->SetRelativeLocation(FVector(18.0f, 0.0f, 5.0f));
		AccentMesh->SetVisibility(true);
		TintMesh(AccentMesh, Steel);
		break;
	}

	case ESEEItemCategory::Armor:
	{
		// Folded cloth / plate: a flat wide box (a stack of armor laid down).
		SetShape(VisualMesh, CubeShape, FVector(0.32f, 0.24f, 0.10f), FRotator::ZeroRotator, 0.0f);
		TintMesh(VisualMesh, Cloth);
		// A second thinner layer on top for the "folded" read.
		AccentMesh->SetStaticMesh(CubeShape);
		AccentMesh->SetRelativeRotation(FRotator::ZeroRotator);
		AccentMesh->SetRelativeScale3D(FVector(0.24f, 0.18f, 0.05f));
		AccentMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f + 2.5f));
		AccentMesh->SetVisibility(true);
		TintMesh(AccentMesh, Steel);
		break;
	}

	case ESEEItemCategory::Crafting:
	case ESEEItemCategory::Junk:
	{
		// Scrap crate: a small cube, the universal "stuff" prop used by the dressing
		// passes. Slightly squat so it reads as a box of materials, not a die.
		SetShape(VisualMesh, CubeShape, FVector(0.26f, 0.26f, 0.20f), FRotator(0.0f, 35.0f, 0.0f), 0.0f);
		TintMesh(VisualMesh, Crate);
		break;
	}

	case ESEEItemCategory::Quest:
	default:
	{
		// Small glowing curio: a compact sphere on a tiny cube base, brightly tinted.
		SetShape(VisualMesh, SphereShape, FVector(0.14f, 0.14f, 0.14f), FRotator::ZeroRotator, 4.0f);
		TintMesh(VisualMesh, QuestGlow);
		AccentMesh->SetStaticMesh(CubeShape);
		AccentMesh->SetRelativeRotation(FRotator::ZeroRotator);
		AccentMesh->SetRelativeScale3D(FVector(0.08f, 0.08f, 0.04f));
		AccentMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 2.0f));
		AccentMesh->SetVisibility(true);
		TintMesh(AccentMesh, RustWood);
		break;
	}
	}
}

void ASEEPickupActor::SnapToFloor()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Trace straight down from a little above the actor origin to find the floor,
	// then place the actor origin (= mesh base) exactly on the hit point. The drop
	// is clamped to MaxFloorSnapDistance so a pickup spawned over a pit/void doesn't
	// teleport far away.
	const FVector Origin = GetActorLocation();
	const FVector Start = Origin + FVector(0.0f, 0.0f, 50.0f);
	const FVector End = Origin - FVector(0.0f, 0.0f, MaxFloorSnapDistance);

	FCollisionQueryParams Params(FName(TEXT("SEEPickupFloorSnap")), /*bTraceComplex*/false, this);
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	// Trace against world static/dynamic geometry (the train floor is WorldStatic).
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params)
		|| World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldDynamic, Params))
	{
		FVector NewLoc = GetActorLocation();
		NewLoc.Z = Hit.ImpactPoint.Z; // mesh base sits at the actor origin
		SetActorLocation(NewLoc, /*bSweep*/false);
	}
}

bool ASEEPickupActor::TryPickup(AActor* Picker)
{
	if (bConsumed || !Picker || ItemID.IsNone() || Quantity <= 0)
	{
		return false;
	}

	// SERVER-AUTHORITATIVE GRANT: only the authority grants items and destroys the
	// pickup. On a listen server the overlap fires on both host and clients, but
	// clients must NOT mutate inventory or destroy the actor — they receive the
	// granted item via the owning player's replicated inventory and see the actor
	// vanish via replicated destruction. Standalone is authority, so the
	// single-player overlap path runs exactly as before.
	if (!HasAuthority())
	{
		return false;
	}

	// Resolve the grant to the OVERLAPPING player's own inventory: AddItem now runs
	// on the server but writes into the specific picker's component, and the
	// component replicates that change down to that player's owning client only.
	USEEInventoryComponent* Inventory = Picker->FindComponentByClass<USEEInventoryComponent>();
	if (!Inventory)
	{
		return false;
	}

	// BUG FIX: pickups carried short IDs ("Bandage", "Pipe", "Cloth") that
	// failed every DT_Items lookup ("Item_Bandage", "Item_RustyPipe", ...).
	// Resolve to the canonical row name before storing, so the inventory UI,
	// UseItem and quest Collect objectives all see real item data.
	const FName ResolvedID = ResolveItemRowName(ItemID, Inventory);

	if (!Inventory->AddItem(ResolvedID, Quantity))
	{
		return false;
	}

	bConsumed = true;
	OnPickedUp(Picker);

	if (bDestroyOnPickup)
	{
		Destroy();
	}
	else
	{
		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);
	}

	return true;
}

void ASEEPickupActor::HandleOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bAutoPickupOnOverlap || !OtherActor || OtherActor == this)
	{
		return;
	}

	TryPickup(OtherActor);
}

// ---------------------------------------------------------------------------
// USEEPickupFixupSubsystem
// ---------------------------------------------------------------------------

void USEEPickupFixupSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// SERVER-ONLY: spawning the real ASEEPickupActor replacements is authority
	// work. This WorldSubsystem is created on both the listen-server host and
	// every joined client; without this guard each client would spawn its own
	// (now-replicated) pickups on top of the server's, producing duplicate loot.
	// Clients receive the server's replicated pickup actors instead. Standalone
	// is authority, so single-player rescue runs exactly as before. The inert
	// BP_Pickup_* stubs are themselves level actors that exist on clients, but
	// only the server destroys them (below) and that destruction replicates.
	if (InWorld.GetNetMode() == NM_Client) return;

	int32 RescuedCount = 0;

	for (TActorIterator<AActor> It(&InWorld); It; ++It)
	{
		AActor* Stub = *It;
		if (!IsValid(Stub) || Stub->IsA<ASEEPickupActor>())
		{
			continue;
		}

		// Generated classes are named "BP_Pickup_<Item>_C".
		const UClass* StubClass = Stub->GetClass();
		FString ClassName = StubClass ? StubClass->GetName() : FString();
		if (!ClassName.StartsWith(TEXT("BP_Pickup_")))
		{
			continue;
		}

		ClassName.RemoveFromStart(TEXT("BP_Pickup_"));
		ClassName.RemoveFromEnd(TEXT("_C"));
		if (ClassName.IsEmpty())
		{
			continue;
		}

		const FTransform StubTransform = Stub->GetActorTransform();

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASEEPickupActor* Pickup = InWorld.SpawnActor<ASEEPickupActor>(
			ASEEPickupActor::StaticClass(), StubTransform, Params);
		if (!Pickup)
		{
			continue;
		}

		// Resolution against DT_Items happens at pickup time (TryPickup), once
		// the picker's inventory (and its data table) is in hand; here we just
		// carry the short ID derived from the blueprint name.
		Pickup->InitPickup(FName(*ClassName), 1);

#if WITH_EDITOR
		Pickup->SetActorLabel(FString::Printf(TEXT("%s_Rescued"), *Stub->GetActorNameOrLabel()));
#endif

		Stub->Destroy();
		++RescuedCount;
	}

	if (RescuedCount > 0)
	{
		UE_LOG(LogTemp, Log,
			TEXT("USEEPickupFixupSubsystem: replaced %d inert BP_Pickup_* stub actors with functional ASEEPickupActor instances."),
			RescuedCount);
	}
}
