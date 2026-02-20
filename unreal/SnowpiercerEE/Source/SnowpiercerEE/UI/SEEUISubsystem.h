// SEEUISubsystem.h - Game UI manager: screen stack, input mode switching
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SEEUISubsystem.generated.h"

class UUserWidget;
class ASEEPlayerController;

UENUM(BlueprintType)
enum class ESEEUIScreen : uint8
{
	None		UMETA(DisplayName = "None"),
	Inventory	UMETA(DisplayName = "Inventory"),
	Character	UMETA(DisplayName = "Character"),
	QuestLog	UMETA(DisplayName = "Quest Log"),
	TrainMap	UMETA(DisplayName = "Train Map"),
	Factions	UMETA(DisplayName = "Factions"),
	Companions	UMETA(DisplayName = "Companions"),
	Crafting	UMETA(DisplayName = "Crafting"),
	Codex		UMETA(DisplayName = "Codex"),
	PauseMenu	UMETA(DisplayName = "Pause Menu"),
	MainMenu	UMETA(DisplayName = "Main Menu"),
	DeathScreen	UMETA(DisplayName = "Death Screen")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScreenChanged, ESEEUIScreen, NewScreen);

UCLASS()
class SNOWPIERCEREE_API USEEUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenScreen(ESEEUIScreen Screen);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseCurrentScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleScreen(ESEEUIScreen Screen);

	UFUNCTION(BlueprintPure, Category = "UI")
	ESEEUIScreen GetCurrentScreen() const { return CurrentScreen; }

	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsScreenOpen() const { return CurrentScreen != ESEEUIScreen::None; }

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnScreenChanged OnScreenChanged;

private:
	ESEEUIScreen CurrentScreen = ESEEUIScreen::None;
};
