// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEENetTravel.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

bool USEENetTravel::HostListenServer(const UObject* WorldContextObject)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World)
	{
		return false;
	}

	// ServerTravel with ?listen makes this instance the authoritative listen server.
	// Absolute travel (true) so we cleanly leave the boot/menu map.
	const FString TravelURL = FString::Printf(TEXT("%s?listen"), GetCoopMapPath());
	UE_LOG(LogTemp, Log, TEXT("[SEENet] Hosting co-op listen server: %s"), *TravelURL);
	World->ServerTravel(TravelURL, /*bAbsolute=*/true);
	return true;
}

bool USEENetTravel::JoinByAddress(const UObject* WorldContextObject, const FString& Address)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World)
	{
		return false;
	}

	FString CleanAddress = Address.TrimStartAndEnd();
	if (CleanAddress.IsEmpty())
	{
		CleanAddress = TEXT("127.0.0.1");
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SEENet] JoinByAddress: no local player controller"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[SEENet] Joining co-op host at %s"), *CleanAddress);
	// ClientTravel(Absolute) is the engine equivalent of the "open <ip>" console command.
	PC->ClientTravel(CleanAddress, ETravelType::TRAVEL_Absolute);
	return true;
}
