// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SaveGameSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/SecureHash.h"
#include "Async/Async.h"
#include "TimerManager.h"
#include "Engine/GameInstance.h"

USEESaveGameSubsystem::USEESaveGameSubsystem()
{
}

void USEESaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Ensure save directory exists
	IFileManager::Get().MakeDirectory(*GetSaveDirectory(), true);
}

void USEESaveGameSubsystem::Deinitialize()
{
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimedAutosaveHandle);
	}
	Super::Deinitialize();
}

// ============================================================================
// Save Operations
// ============================================================================

void USEESaveGameSubsystem::SaveToManualSlot(int32 SlotIndex, const FString& SlotName)
{
	if (SlotIndex < 0 || SlotIndex >= SlotConfig.ManualSlotCount)
	{
		OnSaveComplete.Broadcast(ESaveResult::FailedInvalid, SlotIndex);
		return;
	}

	FSaveGameData Data = GatherCurrentState();
	Data.Header.SlotType = ESaveSlotType::Manual;
	Data.Header.SlotName = SlotName;

	const FString FilePath = GetSlotFilePath(SlotIndex);
	PerformAsyncSave(MoveTemp(Data), FilePath, SlotIndex);
}

void USEESaveGameSubsystem::QuickSave()
{
	const int32 AbsoluteIndex = SlotConfig.ManualSlotCount + NextQuickSaveSlot;

	FSaveGameData Data = GatherCurrentState();
	Data.Header.SlotType = ESaveSlotType::QuickSave;
	Data.Header.SlotName = FString::Printf(TEXT("Quick Save %d"), NextQuickSaveSlot + 1);

	const FString FilePath = GetSlotFilePath(AbsoluteIndex);
	PerformAsyncSave(MoveTemp(Data), FilePath, AbsoluteIndex);

	NextQuickSaveSlot = (NextQuickSaveSlot + 1) % SlotConfig.QuickSaveSlotCount;
}

void USEESaveGameSubsystem::TriggerAutosave(EAutosaveTrigger Trigger)
{
	// Respect cooldown
	if (AutosaveCooldownRemaining > 0.f)
	{
		return;
	}

	// Mr. Wilford: restricted save points
	if (bIsPermadeathSave)
	{
		// Only allow autosave at rest points (not car transitions or timed)
		if (Trigger == EAutosaveTrigger::TimedInterval)
		{
			return;
		}
	}

	const int32 AbsoluteIndex = SlotConfig.ManualSlotCount + SlotConfig.QuickSaveSlotCount + NextAutosaveSlot;

	FSaveGameData Data = GatherCurrentState();
	Data.Header.SlotType = ESaveSlotType::Autosave;
	Data.Header.SlotName = FString::Printf(TEXT("Autosave %d"), NextAutosaveSlot + 1);

	const FString FilePath = GetSlotFilePath(AbsoluteIndex);
	PerformAsyncSave(MoveTemp(Data), FilePath, AbsoluteIndex);

	NextAutosaveSlot = (NextAutosaveSlot + 1) % SlotConfig.AutosaveSlotCount;
	AutosaveCooldownRemaining = SlotConfig.AutosaveCooldownSeconds;

	OnAutosaveTriggered.Broadcast(Trigger);
}

// ============================================================================
// Load Operations
// ============================================================================

void USEESaveGameSubsystem::LoadFromSlot(int32 AbsoluteSlotIndex)
{
	const FString FilePath = GetSlotFilePath(AbsoluteSlotIndex);

	FSaveGameData Data;
	ELoadResult Result = ReadSaveFile(FilePath, Data);

	if (Result == ELoadResult::Success || Result == ELoadResult::SuccessModded)
	{
		ApplyLoadedState(Data);
		ActiveSaveData = MoveTemp(Data);
		bIsPermadeathSave = (ActiveSaveData->GlobalState.DifficultyTier == 3);
	}

	OnLoadComplete.Broadcast(Result, AbsoluteSlotIndex);
}

void USEESaveGameSubsystem::LoadMostRecentAutosave()
{
	FDateTime MostRecentTime = FDateTime::MinValue();
	int32 MostRecentSlot = -1;

	const int32 AutosaveStart = SlotConfig.ManualSlotCount + SlotConfig.QuickSaveSlotCount;
	for (int32 i = 0; i < SlotConfig.AutosaveSlotCount; ++i)
	{
		const int32 AbsIndex = AutosaveStart + i;
		const FString FilePath = GetSlotFilePath(AbsIndex);

		if (FPaths::FileExists(FilePath))
		{
			FSaveGameData Data;
			if (ReadSaveFile(FilePath, Data) == ELoadResult::Success)
			{
				if (Data.Header.Timestamp > MostRecentTime)
				{
					MostRecentTime = Data.Header.Timestamp;
					MostRecentSlot = AbsIndex;
				}
			}
		}
	}

	if (MostRecentSlot >= 0)
	{
		LoadFromSlot(MostRecentSlot);
	}
	else
	{
		OnLoadComplete.Broadcast(ELoadResult::FailedNotFound, -1);
	}
}

// ============================================================================
// Slot Queries
// ============================================================================

TArray<FSaveSlotInfo> USEESaveGameSubsystem::GetAllSlotInfo() const
{
	TArray<FSaveSlotInfo> Slots;
	const int32 Total = SlotConfig.GetTotalSlotCount();
	Slots.Reserve(Total);

	for (int32 i = 0; i < Total; ++i)
	{
		Slots.Add(GetSlotInfo(i));
	}
	return Slots;
}

FSaveSlotInfo USEESaveGameSubsystem::GetSlotInfo(int32 AbsoluteSlotIndex) const
{
	FSaveSlotInfo Info;
	Info.SlotIndex = AbsoluteSlotIndex;
	Info.SlotType = GetSlotType(AbsoluteSlotIndex);
	Info.bIsEmpty = true;

	const FString FilePath = GetSlotFilePath(AbsoluteSlotIndex);
	if (FPaths::FileExists(FilePath))
	{
		// Read only the JSON header without deserializing the full payload
		TArray<uint8> FileData;
		if (FFileHelper::LoadFileToArray(FileData, *FilePath))
		{
			if (FileData.Num() >= 12)
			{
				// Validate magic bytes
				if (FileData[0] == SAVE_MAGIC[0] && FileData[1] == SAVE_MAGIC[1] &&
					FileData[2] == SAVE_MAGIC[2] && FileData[3] == SAVE_MAGIC[3])
				{
					// Read JSON header length
					int32 HeaderLength = 0;
					FMemory::Memcpy(&HeaderLength, &FileData[8], sizeof(int32));

					if (HeaderLength > 0 && 12 + HeaderLength <= FileData.Num())
					{
						FString JsonStr;
						const auto Converter = FUTF8ToTCHAR(
							reinterpret_cast<const ANSICHAR*>(&FileData[12]), HeaderLength);
						JsonStr = FString(Converter.Length(), Converter.Get());

						if (DeserializeHeader(JsonStr, Info.Header))
						{
							Info.bIsEmpty = false;
						}
					}
				}
			}
		}
	}

	return Info;
}

bool USEESaveGameSubsystem::IsSlotOccupied(int32 AbsoluteSlotIndex) const
{
	return FPaths::FileExists(GetSlotFilePath(AbsoluteSlotIndex));
}

void USEESaveGameSubsystem::DeleteSlot(int32 AbsoluteSlotIndex)
{
	const FString FilePath = GetSlotFilePath(AbsoluteSlotIndex);
	IFileManager::Get().Delete(*FilePath);
}

// ============================================================================
// State Gathering / Application
// ============================================================================

FSaveGameData USEESaveGameSubsystem::GatherCurrentState() const
{
	FSaveGameData Data;

	// Header
	Data.Header.FormatVersion = SAVE_FORMAT_VERSION;
	Data.Header.GameVersion = FApp::GetBuildVersion();
	Data.Header.Timestamp = FDateTime::UtcNow();

	// Per-car state from cache
	CarStateCache.GenerateValueArray(Data.ModifiedCars);

	// Global state, player state, NPC state, quest state, companion state
	// are gathered from their respective subsystems/components at save time.
	// Blueprint implementers or game mode should populate these via
	// delegates or direct calls before the save completes.

	return Data;
}

void USEESaveGameSubsystem::ApplyLoadedState(const FSaveGameData& Data)
{
	// Rebuild per-car state cache
	CarStateCache.Empty();
	for (const FSaveCarState& CarState : Data.ModifiedCars)
	{
		CarStateCache.Add(CarState.CarIndex, CarState);
	}

	// Other subsystems listen to OnLoadComplete to restore their own state.
}

// ============================================================================
// Per-Car State
// ============================================================================

void USEESaveGameSubsystem::MarkCarModified(int32 CarIndex)
{
	if (!CarStateCache.Contains(CarIndex))
	{
		FSaveCarState NewState;
		NewState.CarIndex = CarIndex;
		CarStateCache.Add(CarIndex, NewState);
	}
}

bool USEESaveGameSubsystem::GetCarState(int32 CarIndex, FSaveCarState& OutState) const
{
	if (const FSaveCarState* Found = CarStateCache.Find(CarIndex))
	{
		OutState = *Found;
		return true;
	}
	return false;
}

void USEESaveGameSubsystem::UpdateCarState(const FSaveCarState& CarState)
{
	CarStateCache.Add(CarState.CarIndex, CarState);
}

// ============================================================================
// Autosave Configuration
// ============================================================================

void USEESaveGameSubsystem::SetTimedAutosaveEnabled(bool bEnabled)
{
	bTimedAutosaveEnabled = bEnabled;

	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		if (bEnabled)
		{
			World->GetTimerManager().SetTimer(
				TimedAutosaveHandle,
				this,
				&USEESaveGameSubsystem::OnTimedAutosaveTick,
				SlotConfig.TimedAutosaveIntervalSeconds,
				true
			);
		}
		else
		{
			World->GetTimerManager().ClearTimer(TimedAutosaveHandle);
		}
	}
}

void USEESaveGameSubsystem::OnTimedAutosaveTick()
{
	TriggerAutosave(EAutosaveTrigger::TimedInterval);
}

// ============================================================================
// Permadeath (Mr. Wilford)
// ============================================================================

void USEESaveGameSubsystem::DeletePermadeathSave()
{
	if (bIsPermadeathSave && ActiveSaveData.IsSet())
	{
		// Delete all save files for this permadeath run
		const int32 Total = SlotConfig.GetTotalSlotCount();
		for (int32 i = 0; i < Total; ++i)
		{
			DeleteSlot(i);
		}
		ActiveSaveData.Reset();
	}
}

bool USEESaveGameSubsystem::IsPermadeathSave() const
{
	return bIsPermadeathSave;
}

// ============================================================================
// Internal File I/O
// ============================================================================

ESaveResult USEESaveGameSubsystem::WriteSaveFile(const FSaveGameData& Data, const FString& FilePath)
{
	// Serialize binary payload
	TArray<uint8> PayloadBytes;
	FMemoryWriter PayloadWriter(PayloadBytes);

	// Write all payload sections via FArchive
	// Global state
	FSaveGlobalState GlobalCopy = Data.GlobalState;
	PayloadWriter << GlobalCopy.WorldTimeSeconds;
	PayloadWriter << GlobalCopy.DifficultyTier;
	PayloadWriter << GlobalCopy.RevolutionFlags;
	PayloadWriter << GlobalCopy.FactionReputations;
	PayloadWriter << GlobalCopy.DiscoveredCars;

	// Player state
	FSavePlayerState PlayerCopy = Data.PlayerState;
	PayloadWriter << PlayerCopy.Position;
	PayloadWriter << PlayerCopy.Rotation;
	PayloadWriter << PlayerCopy.CurrentCarIndex;
	PayloadWriter << PlayerCopy.Stats;
	PayloadWriter << PlayerCopy.Level;
	PayloadWriter << PlayerCopy.XP;

	// Car states
	int32 NumCars = Data.ModifiedCars.Num();
	PayloadWriter << NumCars;
	for (const FSaveCarState& Car : Data.ModifiedCars)
	{
		FSaveCarState CarCopy = Car;
		PayloadWriter << CarCopy.CarIndex;
		PayloadWriter << CarCopy.LootedContainers;
		PayloadWriter << CarCopy.DoorStates;
		PayloadWriter << CarCopy.DestroyedDestructibles;
		PayloadWriter << CarCopy.ClutterSeed;
	}

	// Compute checksum of uncompressed payload
	const FString Checksum = ComputePayloadChecksum(PayloadBytes);

	// LZ4 compression
	TArray<uint8> CompressedPayload;
	const int32 UncompressedSize = PayloadBytes.Num();
	CompressedPayload.SetNum(FCompression::CompressMemoryBound(NAME_LZ4, UncompressedSize));

	int32 CompressedSize = CompressedPayload.Num();
	if (!FCompression::CompressMemory(
		NAME_LZ4,
		CompressedPayload.GetData(),
		CompressedSize,
		PayloadBytes.GetData(),
		UncompressedSize))
	{
		return ESaveResult::FailedCompress;
	}
	CompressedPayload.SetNum(CompressedSize);

	// Build header JSON
	FSaveHeaderData HeaderCopy = Data.Header;
	HeaderCopy.Checksum = Checksum;
	const FString HeaderJson = SerializeHeader(HeaderCopy);
	const FTCHARToUTF8 HeaderUtf8(*HeaderJson);
	const int32 HeaderLength = HeaderUtf8.Length();

	// Assemble final file
	TArray<uint8> FileData;
	FMemoryWriter FileWriter(FileData);

	// Magic bytes
	FileWriter.Serialize(const_cast<uint8*>(SAVE_MAGIC), 4);

	// Format version
	int32 Version = SAVE_FORMAT_VERSION;
	FileWriter << Version;

	// Header length
	FileWriter << const_cast<int32&>(HeaderLength);

	// JSON header
	FileData.Append(reinterpret_cast<const uint8*>(HeaderUtf8.Get()), HeaderLength);

	// Uncompressed size (for decompression)
	FileData.Append(reinterpret_cast<const uint8*>(&UncompressedSize), sizeof(int32));

	// Compressed payload
	FileData.Append(CompressedPayload);

	// Write to disk
	if (!FFileHelper::SaveArrayToFile(FileData, *FilePath))
	{
		return ESaveResult::FailedIO;
	}

	return ESaveResult::Success;
}

ELoadResult USEESaveGameSubsystem::ReadSaveFile(const FString& FilePath, FSaveGameData& OutData)
{
	if (!FPaths::FileExists(FilePath))
	{
		return ELoadResult::FailedNotFound;
	}

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		return ELoadResult::FailedCorrupt;
	}

	if (FileData.Num() < 12)
	{
		return ELoadResult::FailedCorrupt;
	}

	// Validate magic
	if (FileData[0] != SAVE_MAGIC[0] || FileData[1] != SAVE_MAGIC[1] ||
		FileData[2] != SAVE_MAGIC[2] || FileData[3] != SAVE_MAGIC[3])
	{
		return ELoadResult::FailedCorrupt;
	}

	// Read version
	int32 Version = 0;
	FMemory::Memcpy(&Version, &FileData[4], sizeof(int32));
	if (Version > SAVE_FORMAT_VERSION)
	{
		return ELoadResult::FailedVersion;
	}

	// Read header length
	int32 HeaderLength = 0;
	FMemory::Memcpy(&HeaderLength, &FileData[8], sizeof(int32));

	const int32 PayloadOffset = 12 + HeaderLength;
	if (PayloadOffset + static_cast<int32>(sizeof(int32)) > FileData.Num())
	{
		return ELoadResult::FailedCorrupt;
	}

	// Parse JSON header
	FString HeaderJson;
	const auto Converter = FUTF8ToTCHAR(
		reinterpret_cast<const ANSICHAR*>(&FileData[12]), HeaderLength);
	HeaderJson = FString(Converter.Length(), Converter.Get());

	if (!DeserializeHeader(HeaderJson, OutData.Header))
	{
		return ELoadResult::FailedCorrupt;
	}

	// Read uncompressed size
	int32 UncompressedSize = 0;
	FMemory::Memcpy(&UncompressedSize, &FileData[PayloadOffset], sizeof(int32));

	// Decompress payload
	const int32 CompressedDataOffset = PayloadOffset + sizeof(int32);
	const int32 CompressedSize = FileData.Num() - CompressedDataOffset;

	TArray<uint8> PayloadBytes;
	PayloadBytes.SetNum(UncompressedSize);

	if (!FCompression::UncompressMemory(
		NAME_LZ4,
		PayloadBytes.GetData(),
		UncompressedSize,
		&FileData[CompressedDataOffset],
		CompressedSize))
	{
		return ELoadResult::FailedCorrupt;
	}

	// Validate checksum
	bool bChecksumValid = ValidateChecksum(OutData.Header.Checksum, PayloadBytes);

	// Deserialize payload
	FMemoryReader PayloadReader(PayloadBytes);

	// Global state
	PayloadReader << OutData.GlobalState.WorldTimeSeconds;
	PayloadReader << OutData.GlobalState.DifficultyTier;
	PayloadReader << OutData.GlobalState.RevolutionFlags;
	PayloadReader << OutData.GlobalState.FactionReputations;
	PayloadReader << OutData.GlobalState.DiscoveredCars;

	// Player state
	PayloadReader << OutData.PlayerState.Position;
	PayloadReader << OutData.PlayerState.Rotation;
	PayloadReader << OutData.PlayerState.CurrentCarIndex;
	PayloadReader << OutData.PlayerState.Stats;
	PayloadReader << OutData.PlayerState.Level;
	PayloadReader << OutData.PlayerState.XP;

	// Car states
	int32 NumCars = 0;
	PayloadReader << NumCars;
	OutData.ModifiedCars.SetNum(NumCars);
	for (int32 i = 0; i < NumCars; ++i)
	{
		PayloadReader << OutData.ModifiedCars[i].CarIndex;
		PayloadReader << OutData.ModifiedCars[i].LootedContainers;
		PayloadReader << OutData.ModifiedCars[i].DoorStates;
		PayloadReader << OutData.ModifiedCars[i].DestroyedDestructibles;
		PayloadReader << OutData.ModifiedCars[i].ClutterSeed;
	}

	return bChecksumValid ? ELoadResult::Success : ELoadResult::SuccessModded;
}

void USEESaveGameSubsystem::PerformAsyncSave(FSaveGameData Data, FString FilePath, int32 SlotIndex)
{
	// Run save on background thread
	Async(EAsyncExecution::TaskGraph, [this, Data = MoveTemp(Data), FilePath = MoveTemp(FilePath), SlotIndex]()
	{
		ESaveResult Result = WriteSaveFile(Data, FilePath);

		// Broadcast result on game thread
		AsyncTask(ENamedThreads::GameThread, [this, Result, SlotIndex]()
		{
			OnSaveComplete.Broadcast(Result, SlotIndex);
		});
	});
}

// ============================================================================
// Path Helpers
// ============================================================================

FString USEESaveGameSubsystem::GetSaveDirectory() const
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SaveGames"), TEXT("SnowpiercerEE"));
}

FString USEESaveGameSubsystem::GetSlotFilePath(int32 AbsoluteSlotIndex) const
{
	const ESaveSlotType Type = GetSlotType(AbsoluteSlotIndex);
	const int32 SubIndex = GetSlotSubIndex(AbsoluteSlotIndex);

	FString FileName;
	switch (Type)
	{
	case ESaveSlotType::Manual:
		FileName = FString::Printf(TEXT("slot_%d.sav"), SubIndex);
		break;
	case ESaveSlotType::QuickSave:
		FileName = FString::Printf(TEXT("quick_%d.sav"), SubIndex);
		break;
	case ESaveSlotType::Autosave:
		FileName = FString::Printf(TEXT("auto_%d.sav"), SubIndex);
		break;
	}

	return FPaths::Combine(GetSaveDirectory(), FileName);
}

ESaveSlotType USEESaveGameSubsystem::GetSlotType(int32 AbsoluteSlotIndex) const
{
	if (AbsoluteSlotIndex < SlotConfig.ManualSlotCount)
	{
		return ESaveSlotType::Manual;
	}
	if (AbsoluteSlotIndex < SlotConfig.ManualSlotCount + SlotConfig.QuickSaveSlotCount)
	{
		return ESaveSlotType::QuickSave;
	}
	return ESaveSlotType::Autosave;
}

int32 USEESaveGameSubsystem::GetSlotSubIndex(int32 AbsoluteSlotIndex) const
{
	if (AbsoluteSlotIndex < SlotConfig.ManualSlotCount)
	{
		return AbsoluteSlotIndex;
	}
	if (AbsoluteSlotIndex < SlotConfig.ManualSlotCount + SlotConfig.QuickSaveSlotCount)
	{
		return AbsoluteSlotIndex - SlotConfig.ManualSlotCount;
	}
	return AbsoluteSlotIndex - SlotConfig.ManualSlotCount - SlotConfig.QuickSaveSlotCount;
}

// ============================================================================
// JSON Header
// ============================================================================

FString USEESaveGameSubsystem::SerializeHeader(const FSaveHeaderData& Header) const
{
	TSharedRef<FJsonObject> JsonObj = MakeShared<FJsonObject>();

	JsonObj->SetNumberField(TEXT("version"), Header.FormatVersion);
	JsonObj->SetStringField(TEXT("gameVersion"), Header.GameVersion);
	JsonObj->SetStringField(TEXT("timestamp"), Header.Timestamp.ToIso8601());
	JsonObj->SetNumberField(TEXT("playTime"), Header.PlayTimeSeconds);
	JsonObj->SetStringField(TEXT("slotName"), Header.SlotName);
	JsonObj->SetStringField(TEXT("playerName"), Header.PlayerName);
	JsonObj->SetNumberField(TEXT("currentCar"), Header.CurrentCarIndex);
	JsonObj->SetStringField(TEXT("currentZone"), Header.CurrentZone);
	JsonObj->SetNumberField(TEXT("completionPct"), Header.CompletionPercent);
	JsonObj->SetStringField(TEXT("checksum"), Header.Checksum);
	JsonObj->SetNumberField(TEXT("slotType"), static_cast<int32>(Header.SlotType));

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObj, Writer);
	return OutputString;
}

bool USEESaveGameSubsystem::DeserializeHeader(const FString& JsonStr, FSaveHeaderData& OutHeader) const
{
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);

	if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
	{
		return false;
	}

	OutHeader.FormatVersion = JsonObj->GetIntegerField(TEXT("version"));
	OutHeader.GameVersion = JsonObj->GetStringField(TEXT("gameVersion"));
	FDateTime::ParseIso8601(*JsonObj->GetStringField(TEXT("timestamp")), OutHeader.Timestamp);
	OutHeader.PlayTimeSeconds = JsonObj->GetNumberField(TEXT("playTime"));
	OutHeader.SlotName = JsonObj->GetStringField(TEXT("slotName"));
	OutHeader.PlayerName = JsonObj->GetStringField(TEXT("playerName"));
	OutHeader.CurrentCarIndex = JsonObj->GetIntegerField(TEXT("currentCar"));
	OutHeader.CurrentZone = JsonObj->GetStringField(TEXT("currentZone"));
	OutHeader.CompletionPercent = JsonObj->GetNumberField(TEXT("completionPct"));
	OutHeader.Checksum = JsonObj->GetStringField(TEXT("checksum"));
	OutHeader.SlotType = static_cast<ESaveSlotType>(JsonObj->GetIntegerField(TEXT("slotType")));

	return true;
}

// ============================================================================
// Checksum
// ============================================================================

FString USEESaveGameSubsystem::ComputePayloadChecksum(const TArray<uint8>& PayloadBytes) const
{
	FSHA256Signature Signature;
	FSHA256::HashBuffer(PayloadBytes.GetData(), PayloadBytes.Num(), Signature.Signature);

	FString Result;
	for (int32 i = 0; i < 32; ++i)
	{
		Result += FString::Printf(TEXT("%02x"), Signature.Signature[i]);
	}
	return FString::Printf(TEXT("sha256:%s"), *Result);
}

bool USEESaveGameSubsystem::ValidateChecksum(const FString& Expected, const TArray<uint8>& PayloadBytes) const
{
	const FString Computed = ComputePayloadChecksum(PayloadBytes);
	return Expected == Computed;
}
