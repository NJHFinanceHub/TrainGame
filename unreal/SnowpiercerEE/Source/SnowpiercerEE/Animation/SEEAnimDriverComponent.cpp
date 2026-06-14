// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEEAnimDriverComponent.h"

#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "UObject/SoftObjectPath.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"

USEEAnimDriverComponent::USEEAnimDriverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Tick after movement so GetVelocity() reflects this frame's locomotion.
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void USEEAnimDriverComponent::BeginPlay()
{
	Super::BeginPlay();

	// Player path: the mesh is assigned by the Blueprint CDO before BeginPlay,
	// so it is ready here. NPC path: the AI controller calls InitDriver()
	// explicitly after possess (the mesh is already set on the placed pawn).
	if (!bInitialized)
	{
		InitDriver();
	}
}

void USEEAnimDriverComponent::InitDriver()
{
	MeshComp = ResolveMeshComponent();
	if (!MeshComp)
	{
		if (!bWarnedNoMesh)
		{
			bWarnedNoMesh = true;
			UE_LOG(LogTemp, Warning,
				TEXT("SEEAnimDriver: %s has no skeletal mesh component — animation disabled"),
				*GetNameSafe(GetOwner()));
		}
		return;
	}

	// Guarantee a sane, upright, non-ragdolling pose BEFORE we start driving anim.
	// This is the single runtime chokepoint that covers the player AND every
	// runtime-possessed NPC, so a model that imported lying on its side (or a BP
	// CDO left simulating physics) is corrected uniformly.
	FixupMeshForStanding();

	// Take ownership of the mesh: single-node mode replaces the static idle the
	// import set (and any AnimBlueprint mode the player ctor set).
	MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);

	ResolveAnimSet();

	bInitialized = true;

	// Kick off the resting clip immediately so we never sit on a T-pose.
	CurrentLoopingAnim = nullptr;
	PlayLooping(IdleAnim ? IdleAnim : WalkAnim);
}

void USEEAnimDriverComponent::FixupMeshForStanding()
{
	if (!MeshComp)
	{
		return;
	}

	// A dead pawn whose mesh is intentionally ragdolling must be left alone — the
	// death path owns the mesh then. (InitDriver only runs on spawn/possess of a
	// live pawn, but guard anyway so a re-init never stands a corpse back up.)
	if (bDeathPlayed)
	{
		return;
	}

	// A live pawn must NOT be simulating physics at spawn. If a BP CDO left the
	// mesh ragdolling, the character collapses on the floor at spawn — turn it off
	// and restore the standard walking collision profile before standing it up.
	if (MeshComp->IsSimulatingPhysics())
	{
		MeshComp->SetSimulatePhysics(false);
		MeshComp->SetCollisionProfileName(TEXT("CharacterMesh"));
	}

	// Re-attach to the owning Character's capsule so the mesh rides the pawn
	// (a BP that detached it, or a stray world-attach, would otherwise float).
	// Orient upright: Quaternius GLBs import facing +Y, so Yaw -90 turns them to
	// face the capsule's +X forward; feet sit at the capsule base (Z = -half height).
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		if (UCapsuleComponent* Capsule = Char->GetCapsuleComponent())
		{
			if (MeshComp->GetAttachParent() != Capsule)
			{
				MeshComp->AttachToComponent(
					Capsule, FAttachmentTransformRules::KeepRelativeTransform);
			}

			const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
			MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -HalfHeight));
		}
	}

	// The import script rolled NPC meshes onto their side (Roll instead of Yaw);
	// force the canonical standing orientation regardless of the baked CDO value.
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
}

USkeletalMeshComponent* USEEAnimDriverComponent::ResolveMeshComponent() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Prefer the Character's canonical mesh, fall back to any skeletal mesh.
	if (const ACharacter* Char = Cast<ACharacter>(Owner))
	{
		if (USkeletalMeshComponent* CharMesh = Char->GetMesh())
		{
			return CharMesh;
		}
	}

	return Owner->FindComponentByClass<USkeletalMeshComponent>();
}

bool USEEAnimDriverComponent::ResolveAnimSet()
{
	if (!MeshComp)
	{
		return false;
	}

	USkeletalMesh* SkelMesh = MeshComp->GetSkeletalMeshAsset();
	if (!SkelMesh)
	{
		if (!bWarnedNoAnims)
		{
			bWarnedNoAnims = true;
			UE_LOG(LogTemp, Warning,
				TEXT("SEEAnimDriver: %s skeletal mesh component has no mesh asset — animation disabled"),
				*GetNameSafe(GetOwner()));
		}
		return false;
	}

	// Package path looks like /Game/Characters/QuaterniusZombieApocalypse/Characters_Sam/<MeshName>
	// The model folder is the package's containing folder; the model name is that folder's leaf.
	const FString PackagePath = SkelMesh->GetPackage()->GetName(); // e.g. /Game/.../Characters_Sam/SK_Characters_Sam
	FString FolderPath = FPackageName::GetLongPackagePath(PackagePath); // strips the asset leaf
	FString ModelName = FPackageName::GetShortName(FolderPath); // leaf folder name, e.g. Characters_Sam

	// Defensive: if the mesh sits directly in the model folder (no extra subfolder),
	// GetLongPackagePath already yields the model folder. If the import nested the
	// mesh one level deeper, the model name still matches the FBX import name.
	if (ModelName.IsEmpty())
	{
		ModelName = SkelMesh->GetName();
	}

	// Resolve each clip. Locomotion is essential; action clips are best-effort.
	IdleAnim     = LoadSequenceFromFolder(FolderPath, ModelName, TEXT("Idle"));
	WalkAnim     = LoadSequenceFromFolder(FolderPath, ModelName, TEXT("Walk"));
	RunAnim      = LoadSequenceFromFolder(FolderPath, ModelName, TEXT("Run"));
	SlashAnim    = LoadSequenceFromFolder(FolderPath, ModelName, TEXT("Slash"));
	PunchAnim    = LoadSequenceFromFolder(FolderPath, ModelName, TEXT("Punch"));
	StabAnim     = LoadSequenceFromFolder(FolderPath, ModelName, TEXT("Stab"));
	HitReactAnim = LoadSequenceFromFolder(FolderPath, ModelName, TEXT("HitReact"));
	DeathAnim    = LoadSequenceFromFolder(FolderPath, ModelName, TEXT("Death"));

	if (!IdleAnim && !WalkAnim && !RunAnim)
	{
		if (!bWarnedNoAnims)
		{
			bWarnedNoAnims = true;
			UE_LOG(LogTemp, Warning,
				TEXT("SEEAnimDriver: %s found no locomotion anims under '%s' (model '%s') — character will be static"),
				*GetNameSafe(GetOwner()), *FolderPath, *ModelName);
		}
		return false;
	}

	return true;
}

UAnimSequence* USEEAnimDriverComponent::LoadSequenceFromFolder(
	const FString& FolderPath, const FString& ModelName, const FString& BaseName) const
{
	if (FolderPath.IsEmpty() || BaseName.IsEmpty())
	{
		return nullptr;
	}

	// 1) Try the most likely exact object paths first (cheap, no registry scan).
	//    Object path form: /Game/.../Folder/AssetName.AssetName
	const TArray<FString> NameVariants = {
		FString::Printf(TEXT("%s_%s"), *ModelName, *BaseName), // Characters_Sam_Idle
		BaseName,                                              // Idle
		FString::Printf(TEXT("A_%s_%s"), *ModelName, *BaseName),
		FString::Printf(TEXT("%s_Anim"), *BaseName)
	};

	for (const FString& AssetName : NameVariants)
	{
		const FString ObjectPath = FString::Printf(
			TEXT("%s/%s.%s"), *FolderPath, *AssetName, *AssetName);
		// Speculative path probes — quiet so missing variants don't spam the log.
		if (UAnimSequence* Seq = LoadObject<UAnimSequence>(
				nullptr, *ObjectPath, nullptr, LOAD_NoWarn | LOAD_Quiet))
		{
			return Seq;
		}
	}

	// 2) Fallback: scan the model folder via the asset registry for any
	//    AnimSequence whose name contains BaseName (handles import-time
	//    prefixes/suffixes and nested subfolders).
	const FAssetRegistryModule& ARM =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AR = ARM.Get();

	TArray<FAssetData> Assets;
	AR.GetAssetsByPath(FName(*FolderPath), Assets, /*bRecursive=*/true,
		/*bIncludeOnlyOnDiskAssets=*/false);

	const FString BaseLower = BaseName.ToLower();

	// Prefer the most specific match: exact name, then "_BaseName" suffix, then substring.
	FAssetData ExactMatch;
	FAssetData SuffixMatch;
	FAssetData LooseMatch;

	for (const FAssetData& Data : Assets)
	{
		if (Data.AssetClassPath != UAnimSequence::StaticClass()->GetClassPathName())
		{
			continue;
		}

		const FString AssetNameLower = Data.AssetName.ToString().ToLower();

		if (AssetNameLower == BaseLower)
		{
			ExactMatch = Data;
			break;
		}
		if (!SuffixMatch.IsValid() &&
			(AssetNameLower.EndsWith(FString::Printf(TEXT("_%s"), *BaseLower))))
		{
			SuffixMatch = Data;
		}
		else if (!LooseMatch.IsValid() && AssetNameLower.Contains(BaseLower))
		{
			// Guard against "Run" matching "Run_Slash" when we wanted plain Run:
			// only accept loose substring if no better candidate appears.
			LooseMatch = Data;
		}
	}

	const FAssetData& Chosen = ExactMatch.IsValid() ? ExactMatch
		: (SuffixMatch.IsValid() ? SuffixMatch : LooseMatch);

	if (Chosen.IsValid())
	{
		return Cast<UAnimSequence>(Chosen.GetAsset());
	}

	return nullptr;
}

void USEEAnimDriverComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInitialized || !MeshComp)
	{
		return;
	}

	// Death is terminal: hold the last frame, never resume locomotion.
	if (bDeathPlayed)
	{
		return;
	}

	// If something started simulating physics on the mesh (death ragdoll from
	// the AI controller / player), stop driving so anim and physics don't fight.
	if (IsMeshSimulatingPhysics())
	{
		return;
	}

	// A one-shot action owns the mesh until its timer elapses.
	if (ActionLockTimer > 0.0f)
	{
		ActionLockTimer -= DeltaTime;
		return;
	}

	// Locomotion selection from ground speed.
	const FVector Velocity = GetOwner() ? GetOwner()->GetVelocity() : FVector::ZeroVector;
	const float GroundSpeed = Velocity.Size2D();
	PlayLooping(SelectLocomotionAnim(GroundSpeed));
}

UAnimSequence* USEEAnimDriverComponent::SelectLocomotionAnim(float GroundSpeed) const
{
	if (GroundSpeed < IdleSpeedThreshold)
	{
		// Idle preferred; degrade to whatever exists so we never go static.
		return IdleAnim ? IdleAnim : (WalkAnim ? WalkAnim : RunAnim);
	}

	// Resolve the run threshold: explicit override, else a fraction of MaxWalkSpeed.
	float RunThreshold = RunSpeedThreshold;
	if (RunThreshold <= 0.0f)
	{
		float MaxSpeed = 400.0f;
		if (const ACharacter* Char = Cast<ACharacter>(GetOwner()))
		{
			if (const UCharacterMovementComponent* Move = Char->GetCharacterMovement())
			{
				if (Move->GetMaxSpeed() > 0.0f)
				{
					MaxSpeed = Move->GetMaxSpeed();
				}
			}
		}
		RunThreshold = MaxSpeed * RunThresholdSpeedFraction;
	}

	if (GroundSpeed >= RunThreshold)
	{
		return RunAnim ? RunAnim : (WalkAnim ? WalkAnim : IdleAnim);
	}

	return WalkAnim ? WalkAnim : (RunAnim ? RunAnim : IdleAnim);
}

void USEEAnimDriverComponent::PlayLooping(UAnimSequence* Seq)
{
	if (!MeshComp || !Seq)
	{
		return;
	}

	// Only restart when the desired clip actually changes (avoid per-frame reset).
	if (Seq == CurrentLoopingAnim)
	{
		return;
	}

	CurrentLoopingAnim = Seq;
	MeshComp->PlayAnimation(Seq, /*bLooping=*/true);
}

void USEEAnimDriverComponent::PlayAction(ESEEAnimAction Action)
{
	if (!bInitialized || !MeshComp || bDeathPlayed)
	{
		return;
	}

	// Don't drive anim onto a ragdolling mesh.
	if (IsMeshSimulatingPhysics())
	{
		// Death is still meaningful to record so locomotion stays off if the
		// mesh stops simulating later.
		if (Action == ESEEAnimAction::Death)
		{
			bDeathPlayed = true;
		}
		return;
	}

	UAnimSequence* Seq = nullptr;
	switch (Action)
	{
	case ESEEAnimAction::Attack:
		// Alternate Slash/Punch for variety; fall back across the melee set.
		if (bAttackUsePunch)
		{
			Seq = PunchAnim ? PunchAnim : (SlashAnim ? SlashAnim : StabAnim);
		}
		else
		{
			Seq = SlashAnim ? SlashAnim : (PunchAnim ? PunchAnim : StabAnim);
		}
		bAttackUsePunch = !bAttackUsePunch;
		break;

	case ESEEAnimAction::HitReact:
		Seq = HitReactAnim;
		break;

	case ESEEAnimAction::Death:
		Seq = DeathAnim;
		bDeathPlayed = true; // terminal regardless of whether a clip exists
		break;

	default:
		break;
	}

	if (!Seq)
	{
		// No clip for this action: for Death, leave locomotion off (bDeathPlayed
		// set); for others just ignore so we keep animating.
		return;
	}

	// Non-looping one-shot; lock out locomotion for its length.
	MeshComp->PlayAnimation(Seq, /*bLooping=*/false);
	CurrentLoopingAnim = nullptr; // force a fresh locomotion pick after the lock

	const float Length = Seq->GetPlayLength();
	if (Action == ESEEAnimAction::Death)
	{
		// Hold the last frame indefinitely (TickComponent early-outs on bDeathPlayed).
		ActionLockTimer = 0.0f;
	}
	else
	{
		ActionLockTimer = (Length > 0.0f) ? Length : 0.4f;
	}
}

bool USEEAnimDriverComponent::IsMeshSimulatingPhysics() const
{
	return MeshComp && MeshComp->IsSimulatingPhysics();
}
