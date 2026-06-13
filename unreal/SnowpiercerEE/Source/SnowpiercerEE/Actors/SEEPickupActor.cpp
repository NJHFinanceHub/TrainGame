// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEEPickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"
#include "SnowpiercerEE/SEEInventoryComponent.h"

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

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	SetRootComponent(VisualMesh);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Default visible shape so spawned/rescued pickups are never invisible.
	// Blueprints that assign their own mesh simply override this.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultShape(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (DefaultShape.Succeeded())
	{
		VisualMesh->SetStaticMesh(DefaultShape.Object);
		VisualMesh->SetRelativeScale3D(FVector(0.3f));
	}

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(VisualMesh);
	OverlapSphere->InitSphereRadius(100.0f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ASEEPickupActor::HandleOverlapBegin);
}

void ASEEPickupActor::BeginPlay()
{
	Super::BeginPlay();

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
