#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClimbingComponent.generated.h"

class ACharacter;

// What type of climbable surface was detected
UENUM(BlueprintType)
enum class EClimbableType : uint8
{
	None		UMETA(DisplayName = "None"),
	Ladder		UMETA(DisplayName = "Ladder"),
	Pipe		UMETA(DisplayName = "Pipe"),
	BunkBed		UMETA(DisplayName = "Stacked Bunk"),
	LedgeLow	UMETA(DisplayName = "Low Ledge (Mantle)"),
	LedgeHigh	UMETA(DisplayName = "High Ledge (Climb)")
};

UENUM(BlueprintType)
enum class EClimbState : uint8
{
	NotClimbing		UMETA(DisplayName = "Not Climbing"),
	Mounting		UMETA(DisplayName = "Mounting"),
	Climbing		UMETA(DisplayName = "Climbing"),
	Dismounting		UMETA(DisplayName = "Dismounting"),
	Mantling		UMETA(DisplayName = "Mantling")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClimbStateChanged, EClimbState, NewState, EClimbableType, SurfaceType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMantleComplete);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API UClimbingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UClimbingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Attempt to start climbing (called when player presses interact near climbable)
	UFUNCTION(BlueprintCallable, Category = "Climbing")
	bool TryStartClimb();

	// Release from climbable surface
	UFUNCTION(BlueprintCallable, Category = "Climbing")
	void StopClimb();

	// Attempt to mantle a ledge (contextual, auto-detected)
	UFUNCTION(BlueprintCallable, Category = "Climbing")
	bool TryMantle();

	// Input while climbing
	UFUNCTION(BlueprintCallable, Category = "Climbing")
	void ClimbInput(float VerticalAxis, float HorizontalAxis);

	UFUNCTION(BlueprintPure, Category = "Climbing")
	bool IsClimbing() const { return ClimbState != EClimbState::NotClimbing; }

	UFUNCTION(BlueprintPure, Category = "Climbing")
	EClimbState GetClimbState() const { return ClimbState; }

	UFUNCTION(BlueprintPure, Category = "Climbing")
	EClimbableType GetCurrentSurfaceType() const { return CurrentSurfaceType; }

	// Detect what's climbable in front of the player
	UFUNCTION(BlueprintCallable, Category = "Climbing")
	EClimbableType DetectClimbable() const;

	// Whether pipe climbing is unlocked (via skill tree)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing")
	bool bPipeClimbUnlocked = false;

	// Climb speed multiplier (modified by skill tree)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing")
	float ClimbSpeedMultiplier = 1.0f;

	UPROPERTY(BlueprintAssignable, Category = "Climbing")
	FOnClimbStateChanged OnClimbStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Climbing")
	FOnMantleComplete OnMantleComplete;

protected:
	virtual void BeginPlay() override;

	// Speeds
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing|Config")
	float LadderClimbSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing|Config")
	float PipeClimbSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing|Config")
	float BunkClimbSpeed = 180.0f;

	// Mantle settings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing|Config")
	float MantleLowHeight = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing|Config")
	float MantleHighHeight = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing|Config")
	float MantleDuration = 0.4f;

	// Detection
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing|Config")
	float DetectionRange = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing|Config")
	float DetectionRadius = 30.0f;

	// Stamina cost per second while climbing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing|Config")
	float ClimbStaminaCost = 10.0f;

private:
	UPROPERTY()
	TWeakObjectPtr<ACharacter> OwnerCharacter;

	EClimbState ClimbState = EClimbState::NotClimbing;
	EClimbableType CurrentSurfaceType = EClimbableType::None;

	// Mantle interpolation
	FVector MantleStartLocation;
	FVector MantleEndLocation;
	float MantleTimer = 0.0f;

	// Climbing surface info
	FVector ClimbSurfaceNormal;
	FVector ClimbSurfaceLocation;

	void SetClimbState(EClimbState NewState);
	void UpdateClimbing(float DeltaTime);
	void UpdateMantling(float DeltaTime);
	float GetClimbSpeedForSurface() const;
	bool TraceForClimbable(FHitResult& OutHit, EClimbableType& OutType) const;
	bool TraceForMantleLedge(FVector& OutLedgeTop, float& OutHeight) const;
};
