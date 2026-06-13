// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEEGameState.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

ASEEGameState::ASEEGameState()
{
	// GameStateBase already sets bReplicates = true; nothing extra needed here.
}

void ASEEGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASEEGameState, ConnectedPlayerCount);
}

void ASEEGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	RefreshConnectedPlayerCount();
}

void ASEEGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	RefreshConnectedPlayerCount();
}

void ASEEGameState::RefreshConnectedPlayerCount()
{
	// Authority-only mutation; the value replicates down to clients.
	if (!HasAuthority()) return;

	// Exclude inactive (disconnecting) player states so the count reflects live co-op players.
	int32 Count = 0;
	for (const APlayerState* PS : PlayerArray)
	{
		if (PS && !PS->IsInactive())
		{
			++Count;
		}
	}

	if (Count != ConnectedPlayerCount)
	{
		ConnectedPlayerCount = Count;
		// OnRep doesn't fire on the server — broadcast locally so listen-server UI updates too.
		OnConnectedPlayerCountChanged.Broadcast(ConnectedPlayerCount);
	}
}

void ASEEGameState::OnRep_ConnectedPlayerCount()
{
	OnConnectedPlayerCountChanged.Broadcast(ConnectedPlayerCount);
}
