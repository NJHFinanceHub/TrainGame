// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SEENetTravel.generated.h"

/**
 * USEENetTravel
 *
 * Thin host/join travel helpers for iteration-1 co-op. Uses the engine's built-in
 * IP-based travel (listen server + ClientTravel) which works over LAN / direct IP
 * with the OnlineSubsystemNull driver — no session-search UI yet. A later iteration
 * can swap these bodies for an IOnlineSession create/find/join flow without changing
 * call sites (the main menu only knows about HostListenServer / JoinByAddress).
 */
UCLASS()
class SNOWPIERCEREE_API USEENetTravel : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Default map all co-op sessions open into. */
	static const TCHAR* GetCoopMapPath() { return TEXT("/Game/Maps/Zone1_Tail"); }

	/**
	 * Open the co-op map as a listen server (host is player 0 + authority).
	 * Equivalent to "open Zone1_Tail?listen". Returns false if the world is missing.
	 */
	UFUNCTION(BlueprintCallable, Category = "Net", meta = (WorldContext = "WorldContextObject"))
	static bool HostListenServer(const UObject* WorldContextObject);

	/**
	 * Client-travel to a host by IP (defaults to 127.0.0.1 for local testing).
	 * Equivalent to "open <Address>". Returns false if no local player controller.
	 */
	UFUNCTION(BlueprintCallable, Category = "Net", meta = (WorldContext = "WorldContextObject"))
	static bool JoinByAddress(const UObject* WorldContextObject, const FString& Address);
};
