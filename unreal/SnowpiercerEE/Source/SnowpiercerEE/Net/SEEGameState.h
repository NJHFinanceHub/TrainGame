// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SEEGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectedPlayerCountChanged, int32, NewCount);

/**
 * ASEEGameState
 *
 * Replicated, server-authoritative container for co-op session state that every
 * client needs to see. Iteration 1 only tracks the connected player count (used
 * by the join flow / future lobby UI), but this is the home for any shared
 * world state (objective progress, shared difficulty, run timer) that later
 * iterations replicate. AGameStateBase already replicates PlayerArray; this adds
 * a convenience count + change delegate clients can bind to.
 */
UCLASS()
class SNOWPIERCEREE_API ASEEGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ASEEGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Hard cap on co-op players for iteration 1 (host counts as one). */
	static constexpr int32 MaxCoopPlayers = 4;

	UFUNCTION(BlueprintPure, Category = "Net")
	int32 GetConnectedPlayerCount() const { return ConnectedPlayerCount; }

	/** True when the session is already full and should refuse another join. */
	UFUNCTION(BlueprintPure, Category = "Net")
	bool IsSessionFull() const { return ConnectedPlayerCount >= MaxCoopPlayers; }

	/** Server-only: recompute the replicated player count from the PlayerArray. */
	void RefreshConnectedPlayerCount();

	/** Fires on server and clients whenever the replicated count changes. */
	UPROPERTY(BlueprintAssignable, Category = "Net")
	FOnConnectedPlayerCountChanged OnConnectedPlayerCountChanged;

	// AGameStateBase hooks — keep the replicated count in sync as players come/go.
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_ConnectedPlayerCount, VisibleAnywhere, BlueprintReadOnly, Category = "Net")
	int32 ConnectedPlayerCount = 0;

	UFUNCTION()
	void OnRep_ConnectedPlayerCount();
};
