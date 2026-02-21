// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SEENPCActor.generated.h"

class UBehaviorTree;
class UDataTable;
class UPrimitiveComponent;
class USkeletalMeshComponent;
class USphereComponent;

UCLASS(Blueprintable)
class SNOWPIERCEREE_API ASEENPCActor : public ACharacter
{
	GENERATED_BODY()

public:
	ASEENPCActor();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "NPC|Interaction")
	void Interact(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category = "NPC|Interaction")
	void OnInteract(AActor* Interactor);

protected:
	UFUNCTION()
	void HandleDialogueTriggerBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> DialogueTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FName NPCId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
	TObjectPtr<UDataTable> DialogueDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
	TObjectPtr<UBehaviorTree> PatrolIdleBehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
	float DialogueTriggerRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
	bool bAutoInteractOnDialogueTrigger = false;
};
