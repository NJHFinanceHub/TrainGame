#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SEEHealthComponent.h"
#include "SEEWeaponBase.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

UENUM(BlueprintType)
enum class ESEEWeaponTier : uint8
{
	Improvised		UMETA(DisplayName = "Improvised"),
	Functional		UMETA(DisplayName = "Functional"),
	Military		UMETA(DisplayName = "Military"),
	Specialized		UMETA(DisplayName = "Specialized"),
	Legendary		UMETA(DisplayName = "Legendary")
};

/** Code-built visual silhouette for a weapon (engine basic shapes, no asset deps). */
UENUM(BlueprintType)
enum class ESEEWeaponShape : uint8
{
	Pipe		UMETA(DisplayName = "Pipe"),		// thin cylinder
	Shiv		UMETA(DisplayName = "Shiv"),		// short grip + small blade wedge
	Wrench		UMETA(DisplayName = "Wrench"),		// cylinder + box head
	Blade		UMETA(DisplayName = "Blade"),		// short grip + long flat blade (machete)
	Bat			UMETA(DisplayName = "Bat"),			// thick tapered club
	Baton		UMETA(DisplayName = "Baton"),		// short slim cylinder
	Axe			UMETA(DisplayName = "Axe"),			// long haft + box axe head (fire axe)
	Rebar		UMETA(DisplayName = "Rebar"),		// thin ribbed bar, sharpened end
	ChainWhip	UMETA(DisplayName = "ChainWhip")	// short grip + thin trailing length
};

UCLASS()
class SNOWPIERCEREE_API ASEEWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ASEEWeaponBase();

	/**
	 * Spawn a fully configured, visible weapon for a known weapon inventory item.
	 * Maps DT_Items weapon ItemIDs (Item_Shiv, Item_RustyPipe, Item_Wrench,
	 * Item_Machete, Item_ReinforcedBat, Item_JackbootBaton) to stats + shape.
	 * Returns nullptr (with a log) for unknown ItemIDs.
	 */
	static ASEEWeaponBase* SpawnWeaponForItem(UWorld* World, FName ItemID, AActor* OwnerActor);

	/** True if ItemID is one of the known weapon inventory items the factory can build. */
	static bool IsWeaponItemID(FName ItemID);

	/** Build the visible mesh for the given silhouette (engine basic shapes, dark metal tint). */
	void ConfigureShape(ESEEWeaponShape Shape);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetBaseDamage() const { return BaseDamage; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	ESEEDamageType GetDamageType() const { return DamageType; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetAttackSpeed() const { return AttackSpeed; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetWeaponRange() const { return Range; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetDurabilityPercent() const { return MaxDurability > 0.0f ? CurrentDurability / MaxDurability : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsBroken() const { return CurrentDurability <= 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FName GetSourceItemID() const { return SourceItemID; }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DegradeDurability(float Amount = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Repair(float Amount);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	ESEEWeaponTier Tier = ESEEWeaponTier::Improvised;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	ESEEDamageType DamageType = ESEEDamageType::Blunt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float Range = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float Weight = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float MaxDurability = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	float CurrentDurability = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float StaminaCostLight = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float StaminaCostHeavy = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float HeavyDamageMultiplier = 2.5f;

	/** Inventory ItemID this weapon was spawned from (NAME_None for hand-placed weapons). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName SourceItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	UStaticMeshComponent* WeaponMesh;

	/** Secondary piece (wrench head). Hidden for single-piece shapes. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	UStaticMeshComponent* WeaponHeadMesh;

protected:
	virtual void BeginPlay() override;

	/** Tint applied via dynamic material instance on the engine BasicShapeMaterial. */
	UPROPERTY(EditAnywhere, Category = "Weapon|Visual")
	FLinearColor MetalTint = FLinearColor(0.06f, 0.06f, 0.07f, 1.0f);

	/** Darker tint for grips/handles/hafts (wood, tape, rubber) so blades read as metal. */
	UPROPERTY(EditAnywhere, Category = "Weapon|Visual")
	FLinearColor GripTint = FLinearColor(0.10f, 0.07f, 0.04f, 1.0f);

	/** Brighter steel tint for sharpened edges (shiv/machete/axe/rebar). */
	UPROPERTY(EditAnywhere, Category = "Weapon|Visual")
	FLinearColor BladeTint = FLinearColor(0.55f, 0.57f, 0.60f, 1.0f);

private:
	// Engine basic shape assets resolved in the constructor (no project asset deps)
	UPROPERTY()
	TObjectPtr<UStaticMesh> CylinderShape;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeShape;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> BasicShapeMaterial;

	void ApplyTint(UStaticMeshComponent* MeshComp, const FLinearColor& Color);
};
