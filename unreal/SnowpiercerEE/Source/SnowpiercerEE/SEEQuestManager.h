#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SEEQuestManager.generated.h"

UENUM(BlueprintType)
enum class ESEEQuestState : uint8
{
	Available	UMETA(DisplayName = "Available"),
	Active		UMETA(DisplayName = "Active"),
	Completed	UMETA(DisplayName = "Completed"),
	Failed		UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class ESEEObjectiveType : uint8
{
	GoTo		UMETA(DisplayName = "Go To"),
	Interact	UMETA(DisplayName = "Interact"),
	Kill		UMETA(DisplayName = "Kill"),
	Collect		UMETA(DisplayName = "Collect"),
	Escort		UMETA(DisplayName = "Escort"),
	Dialogue	UMETA(DisplayName = "Dialogue"),
	Custom		UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct FSEEQuestObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ObjectiveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEObjectiveType Type = ESEEObjectiveType::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOptional = false;
};

USTRUCT(BlueprintType)
struct FSEEQuest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEQuestState State = ESEEQuestState::Available;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMainQuest = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSEEQuestObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 XPReward = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> ItemRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> FactionRepRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> PrerequisiteQuests;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStarted, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, FName, QuestID, FName, ObjectiveID);

UCLASS()
class SNOWPIERCEREE_API USEEQuestManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RegisterQuest(const FSEEQuest& Quest);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool StartQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void UpdateObjective(FName QuestID, FName ObjectiveID, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void FailQuest(FName QuestID);

	UFUNCTION(BlueprintPure, Category = "Quest")
	ESEEQuestState GetQuestState(FName QuestID) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	FSEEQuest GetQuest(FName QuestID) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FSEEQuest> GetActiveQuests() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FSEEQuest> GetCompletedQuests() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	FName GetTrackedQuestID() const { return TrackedQuestID; }

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetTrackedQuest(FName QuestID) { TrackedQuestID = QuestID; }

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestStarted OnQuestStarted;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestCompleted OnQuestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestFailed OnQuestFailed;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnObjectiveUpdated OnObjectiveUpdated;

private:
	TMap<FName, FSEEQuest> Quests;
	FName TrackedQuestID;

	bool ArePrerequisitesMet(const FSEEQuest& Quest) const;
	void CheckQuestCompletion(FName QuestID);
};
