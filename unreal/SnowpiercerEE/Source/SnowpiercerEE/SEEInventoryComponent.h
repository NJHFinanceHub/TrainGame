#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEEItemBase.h"
#include "SEEInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, FName, ItemID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, FName, ItemID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

/**
 * One serialized inventory stack for save games. Mirrors FSEEInventorySlot
 * (ItemID + Count); Durability is reserved for item types that track it
 * (0 == not tracked). Lives here so both the inventory and the save subsystem
 * can share it without a dependency cycle.
 */
USTRUCT(BlueprintType)
struct FSEEItemSaveEntry
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	int32 Count = 0;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	float Durability = 0.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEEInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEEInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(FName ItemID, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FName ItemID, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DropItem(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItem(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetQuickSlot(int32 QuickSlotIndex, int32 InventorySlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseQuickSlot(int32 QuickSlotIndex);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemID, int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetMaxWeight() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsOverweight() const { return GetCurrentWeight() > GetMaxWeight(); }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<FSEEInventorySlot> GetAllSlots() const { return Slots; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetSlotCount() const { return Slots.Num(); }

	const FSEEItemData* GetItemDataPtr(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool GetItemData(FName ItemID, FSEEItemData& OutData) const;

	// --- Save / restore (additive; used by USEESaveGameSubsystem) ---

	/** Snapshot every non-empty slot into save entries (ItemID + Count). */
	UFUNCTION(BlueprintPure, Category = "Inventory|Save")
	TArray<FSEEItemSaveEntry> GetSaveState() const;

	/** Replace the inventory contents from a saved snapshot. Resizes slots as needed and broadcasts a change. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
	void SetSaveState(const TArray<FSEEItemSaveEntry>& Entries);

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 MaxSlots = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	float BaseCarryCapacity = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	float StrengthWeightBonus = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UDataTable> ItemDataTable;

private:
	// CO-OP REPLICATION: inventory contents are server-authoritative and replicate
	// to the OWNING client only (COND_OwnerOnly) — each player sees just their own
	// bag, never the other co-op players'. AddItem (pickups) now runs on the
	// server and writes here; the change replicates down and OnRep_Slots fires
	// OnInventoryChanged so the owning client's inventory UI refreshes. Standalone
	// is authority, so the local single-player path is unchanged (OnRep never runs
	// on authority; the authority path broadcasts OnInventoryChanged inline).
	UPROPERTY(ReplicatedUsing = OnRep_Slots)
	TArray<FSEEInventorySlot> Slots;

	UFUNCTION()
	void OnRep_Slots();

	TArray<int32> QuickSlots; // Maps quick slot 0-3 -> inventory slot index

	int32 FindSlotWithItem(FName ItemID) const;
	int32 FindEmptySlot() const;

	/** Sizes the slot array (idempotent) - AddItem may run before BeginPlay. */
	void EnsureSlots();

	virtual void BeginPlay() override;
};
