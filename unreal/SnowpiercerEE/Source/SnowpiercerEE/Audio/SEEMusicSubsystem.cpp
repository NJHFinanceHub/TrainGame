#include "Audio/SEEMusicSubsystem.h"
#include "AI/SEENPCAIController.h"
#include "SEEHealthComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogSEEMusic, Log, All);

namespace
{
	const TCHAR* ExploreTrackPath = TEXT("/Game/Audio/Music/MUS_Zone1_Explore.MUS_Zone1_Explore");
	const TCHAR* CombatTrackPath = TEXT("/Game/Audio/Music/MUS_Zone1_Combat.MUS_Zone1_Combat");

	USoundBase* LoadMusicTrack(const TCHAR* Path)
	{
		USoundBase* Sound = LoadObject<USoundBase>(nullptr, Path);
		if (USoundWave* Wave = Cast<USoundWave>(Sound))
		{
			// Imported wavs default to one-shot; force looping for music beds
			// (in-memory only — does not dirty the asset on disk in a build).
			Wave->bLooping = true;
		}
		return Sound;
	}
}

bool USEEMusicSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	// Game worlds only (PIE included via the base class world-type filter)
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void USEEMusicSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	USoundBase* ExploreSound = LoadMusicTrack(ExploreTrackPath);
	USoundBase* CombatSound = LoadMusicTrack(CombatTrackPath);

	if (!ExploreSound && !CombatSound)
	{
		// Wavs exist on disk but may not be imported as uassets yet — stay silent.
		UE_LOG(LogSEEMusic, Warning,
			TEXT("Music tracks not found (%s / %s) — soundtrack disabled for this session"),
			ExploreTrackPath, CombatTrackPath);
		return;
	}
	if (!ExploreSound || !CombatSound)
	{
		UE_LOG(LogSEEMusic, Warning, TEXT("Missing %s track — combat crossfade disabled"),
			!ExploreSound ? TEXT("explore") : TEXT("combat"));
	}

	// Both layers run continuously; the inactive one is held at zero volume so
	// crossfades stay phase-aligned and neither component is ever destroyed.
	ExploreComp = SpawnMusicComponent(ExploreSound, MusicVolume);
	CombatComp = SpawnMusicComponent(CombatSound, 0.0f);

	if (ExploreComp || CombatComp)
	{
		UE_LOG(LogSEEMusic, Log, TEXT("Zone soundtrack started (explore layer at %.2f volume)"), MusicVolume);
		InWorld.GetTimerManager().SetTimer(
			PollTimerHandle, this, &USEEMusicSubsystem::PollCombatState, PollInterval, true);
	}
}

void USEEMusicSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PollTimerHandle);
	}

	if (IsValid(ExploreComp))
	{
		ExploreComp->Stop();
	}
	if (IsValid(CombatComp))
	{
		CombatComp->Stop();
	}
	ExploreComp = nullptr;
	CombatComp = nullptr;

	Super::Deinitialize();
}

UAudioComponent* USEEMusicSubsystem::SpawnMusicComponent(USoundBase* Sound, float StartVolume)
{
	UWorld* World = GetWorld();
	if (!World || !Sound)
	{
		return nullptr;
	}

	UAudioComponent* Comp = UGameplayStatics::SpawnSound2D(
		World, Sound, StartVolume, 1.0f, 0.0f,
		/*ConcurrencySettings*/ nullptr,
		/*bPersistAcrossLevelTransition*/ false,
		/*bAutoDestroy*/ false);

	if (Comp)
	{
		Comp->bIsUISound = false; // respect game pause
	}
	return Comp;
}

void USEEMusicSubsystem::PollCombatState()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!PlayerPawn)
	{
		return;
	}

	UpdateDeathDuck(PlayerPawn);

	const bool bThreat = IsAnyHostileAggroNearPlayer(PlayerPawn);
	if (bThreat)
	{
		CombatLingerRemaining = CombatLingerTime;
		if (!bCombatActive)
		{
			SetCombatMusic(true);
		}
	}
	else if (bCombatActive)
	{
		CombatLingerRemaining -= PollInterval;
		if (CombatLingerRemaining <= 0.0f)
		{
			SetCombatMusic(false);
		}
	}
}

bool USEEMusicSubsystem::IsAnyHostileAggroNearPlayer(const APawn* PlayerPawn) const
{
	UWorld* World = GetWorld();
	if (!World || !PlayerPawn)
	{
		return false;
	}

	const float RangeSq = CombatDetectRange * CombatDetectRange;
	const FVector PlayerLoc = PlayerPawn->GetActorLocation();

	for (TActorIterator<APawn> It(World); It; ++It)
	{
		const APawn* Pawn = *It;
		if (!Pawn || Pawn == PlayerPawn)
		{
			continue;
		}

		const ASEENPCAIController* NPC = Cast<ASEENPCAIController>(Pawn->GetController());
		if (!NPC || !NPC->bHostile || !NPC->IsAggroed())
		{
			continue;
		}

		if (FVector::DistSquared(PlayerLoc, Pawn->GetActorLocation()) <= RangeSq)
		{
			return true;
		}
	}
	return false;
}

void USEEMusicSubsystem::SetCombatMusic(bool bCombat)
{
	bCombatActive = bCombat;

	UAudioComponent* FadeOutComp = bCombat ? ExploreComp : CombatComp;
	UAudioComponent* FadeInComp = bCombat ? CombatComp : ExploreComp;

	// AdjustVolume keeps both components playing (a true FadeOut would stop
	// the component); the layers just swap audibility.
	if (IsValid(FadeOutComp))
	{
		FadeOutComp->AdjustVolume(CrossfadeTime, 0.0f);
	}
	if (IsValid(FadeInComp))
	{
		FadeInComp->AdjustVolume(CrossfadeTime, ActiveLayerTargetVolume());
	}

	UE_LOG(LogSEEMusic, Log, TEXT("Music -> %s (crossfade %.1fs)"),
		bCombat ? TEXT("COMBAT") : TEXT("explore"), CrossfadeTime);
}

void USEEMusicSubsystem::UpdateDeathDuck(const APawn* PlayerPawn)
{
	const USEEHealthComponent* Health = PlayerPawn ? PlayerPawn->FindComponentByClass<USEEHealthComponent>() : nullptr;
	const bool bDead = Health && Health->IsDead();
	if (bDead == bDucked)
	{
		return;
	}

	bDucked = bDead;
	UAudioComponent* ActiveComp = bCombatActive ? CombatComp : ExploreComp;
	if (IsValid(ActiveComp))
	{
		ActiveComp->AdjustVolume(1.0f, ActiveLayerTargetVolume());
	}
	UE_LOG(LogSEEMusic, Verbose, TEXT("Music duck %s (player %s)"),
		bDucked ? TEXT("on") : TEXT("off"), bDead ? TEXT("dead") : TEXT("alive"));
}
