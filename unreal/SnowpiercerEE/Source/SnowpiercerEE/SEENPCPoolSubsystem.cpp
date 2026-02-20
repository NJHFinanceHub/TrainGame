#include "SEENPCPoolSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"

DECLARE_STATS_GROUP(TEXT("NPCPool"), STATGROUP_NPCPool, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("NPC Pool Tick"), STAT_NPCPoolTick, STATGROUP_NPCPool);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active NPCs"), STAT_ActiveNPCs, STATGROUP_NPCPool);
DECLARE_DWORD_COUNTER_STAT(TEXT("Pooled NPCs"), STAT_PooledNPCs, STATGROUP_NPCPool);
DECLARE_DWORD_COUNTER_STAT(TEXT("Full Tick NPCs"), STAT_FullTickNPCs, STATGROUP_NPCPool);

TStatId USEENPCPoolSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(USEENPCPoolSubsystem, STATGROUP_NPCPool);
}

void USEENPCPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Subscribe to streaming state changes
    UWorld* World = GetWorld();
    if (World)
    {
        USEECarStreamingSubsystem* StreamingSub = World->GetSubsystem<USEECarStreamingSubsystem>();
        if (StreamingSub)
        {
            StreamingSub->OnCarStreamingStateChanged.AddDynamic(this, &USEENPCPoolSubsystem::OnCarStreamingStateChanged);
        }
    }
}

void USEENPCPoolSubsystem::Deinitialize()
{
    PooledNPCs.Empty();
    Super::Deinitialize();
}

void USEENPCPoolSubsystem::Tick(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_NPCPoolTick);

    // Stagger AI ticking across frames
    UpdateStaggeredTicking();

    // Periodic cleanup of stale entries
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= CleanupInterval)
    {
        CleanupTimer = 0.0f;
        CleanupStaleEntries();
    }

    // Update stats
    int32 ActiveCount = 0;
    int32 FullTickCount = 0;
    for (const FPooledNPCInfo& Info : PooledNPCs)
    {
        if (Info.bActive) ActiveCount++;
        if (Info.bFullTickThisFrame) FullTickCount++;
    }
    SET_DWORD_STAT(STAT_ActiveNPCs, ActiveCount);
    SET_DWORD_STAT(STAT_PooledNPCs, PooledNPCs.Num());
    SET_DWORD_STAT(STAT_FullTickNPCs, FullTickCount);
}

void USEENPCPoolSubsystem::RegisterNPC(ASEENPCCharacter* NPC, int32 CarIndex)
{
    if (!NPC || CarIndex < 0)
    {
        return;
    }

    // Check per-car limit
    int32 CurrentCount = GetActiveNPCCount(CarIndex);
    if (CurrentCount >= Config.MaxNPCsPerCar)
    {
        OnNPCBudgetExceeded.Broadcast(CarIndex, CurrentCount + 1);
        UE_LOG(LogTemp, Warning, TEXT("NPC Pool: Car %d exceeds max NPCs (%d/%d). NPC %s not registered."),
               CarIndex, CurrentCount + 1, Config.MaxNPCsPerCar, *NPC->GetDisplayName().ToString());
        return;
    }

    // Check total pool limit
    if (PooledNPCs.Num() >= Config.MaxPoolSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPC Pool: Max pool size (%d) reached. NPC %s not registered."),
               Config.MaxPoolSize, *NPC->GetDisplayName().ToString());
        return;
    }

    // Check for duplicate
    for (const FPooledNPCInfo& Existing : PooledNPCs)
    {
        if (Existing.NPC.Get() == NPC)
        {
            return;
        }
    }

    FPooledNPCInfo Info;
    Info.NPC = NPC;
    Info.AssignedCarIndex = CarIndex;
    Info.bActive = true;
    Info.SavedState = NPC->GetCurrentState();
    PooledNPCs.Add(Info);

    OnNPCPoolChanged.Broadcast(CarIndex, GetActiveNPCCount(CarIndex));
}

void USEENPCPoolSubsystem::UnregisterNPC(ASEENPCCharacter* NPC)
{
    if (!NPC) return;

    for (int32 i = PooledNPCs.Num() - 1; i >= 0; --i)
    {
        if (PooledNPCs[i].NPC.Get() == NPC)
        {
            int32 CarIndex = PooledNPCs[i].AssignedCarIndex;
            PooledNPCs.RemoveAt(i);
            OnNPCPoolChanged.Broadcast(CarIndex, GetActiveNPCCount(CarIndex));
            return;
        }
    }
}

int32 USEENPCPoolSubsystem::GetActiveNPCCount(int32 CarIndex) const
{
    int32 Count = 0;
    for (const FPooledNPCInfo& Info : PooledNPCs)
    {
        if (Info.AssignedCarIndex == CarIndex && Info.bActive)
        {
            Count++;
        }
    }
    return Count;
}

TArray<ASEENPCCharacter*> USEENPCPoolSubsystem::GetActiveNPCsInCar(int32 CarIndex) const
{
    TArray<ASEENPCCharacter*> Result;
    for (const FPooledNPCInfo& Info : PooledNPCs)
    {
        if (Info.AssignedCarIndex == CarIndex && Info.bActive && Info.NPC.IsValid())
        {
            Result.Add(Info.NPC.Get());
        }
    }
    return Result;
}

void USEENPCPoolSubsystem::SetPoolConfig(const FNPCPoolConfig& NewConfig)
{
    Config = NewConfig;
}

void USEENPCPoolSubsystem::OnCarStreamingStateChanged(int32 CarIndex, ECarStreamingState NewState)
{
    switch (NewState)
    {
    case ECarStreamingState::Active:
        ActivateNPCsForCar(CarIndex);
        break;
    case ECarStreamingState::Loaded:
        FreezeNPCsForCar(CarIndex);
        break;
    case ECarStreamingState::Unloaded:
        DeactivateNPCsForCar(CarIndex);
        break;
    default:
        break;
    }
}

void USEENPCPoolSubsystem::ActivateNPCsForCar(int32 CarIndex)
{
    for (FPooledNPCInfo& Info : PooledNPCs)
    {
        if (Info.AssignedCarIndex != CarIndex || !Info.NPC.IsValid())
        {
            continue;
        }

        ASEENPCCharacter* NPC = Info.NPC.Get();
        Info.bActive = true;

        // Restore from frozen state
        NPC->SetActorTickEnabled(true);
        NPC->SetActorHiddenInGame(false);
        NPC->SetActorEnableCollision(true);

        // Restore saved state
        NPC->SetState(Info.SavedState);

        if (UCharacterMovementComponent* MoveComp = NPC->GetCharacterMovement())
        {
            MoveComp->Activate();
        }
    }

    OnNPCPoolChanged.Broadcast(CarIndex, GetActiveNPCCount(CarIndex));
}

void USEENPCPoolSubsystem::FreezeNPCsForCar(int32 CarIndex)
{
    for (FPooledNPCInfo& Info : PooledNPCs)
    {
        if (Info.AssignedCarIndex != CarIndex || !Info.NPC.IsValid())
        {
            continue;
        }

        ASEENPCCharacter* NPC = Info.NPC.Get();

        // Save current state before freezing
        Info.SavedState = NPC->GetCurrentState();
        Info.SavedLocation = NPC->GetActorLocation();
        Info.SavedRotation = NPC->GetActorRotation();
        Info.bActive = false;

        // Freeze: disable tick but keep visible (visible through doors)
        NPC->SetActorTickEnabled(false);
        NPC->SetActorEnableCollision(true);

        if (UCharacterMovementComponent* MoveComp = NPC->GetCharacterMovement())
        {
            MoveComp->StopMovementImmediately();
            MoveComp->Deactivate();
        }
    }
}

void USEENPCPoolSubsystem::DeactivateNPCsForCar(int32 CarIndex)
{
    for (FPooledNPCInfo& Info : PooledNPCs)
    {
        if (Info.AssignedCarIndex != CarIndex || !Info.NPC.IsValid())
        {
            continue;
        }

        ASEENPCCharacter* NPC = Info.NPC.Get();

        // Save state
        if (Info.bActive)
        {
            Info.SavedState = NPC->GetCurrentState();
            Info.SavedLocation = NPC->GetActorLocation();
            Info.SavedRotation = NPC->GetActorRotation();
        }
        Info.bActive = false;

        // Fully deactivate: hidden, no tick, no collision
        NPC->SetActorTickEnabled(false);
        NPC->SetActorHiddenInGame(true);
        NPC->SetActorEnableCollision(false);

        if (UCharacterMovementComponent* MoveComp = NPC->GetCharacterMovement())
        {
            MoveComp->StopMovementImmediately();
            MoveComp->Deactivate();
        }
    }
}

void USEENPCPoolSubsystem::UpdateStaggeredTicking()
{
    // Collect all active NPCs in the current active car
    UWorld* World = GetWorld();
    if (!World) return;

    USEECarStreamingSubsystem* StreamingSub = World->GetSubsystem<USEECarStreamingSubsystem>();
    if (!StreamingSub) return;

    int32 ActiveCar = StreamingSub->GetCurrentCarIndex();
    if (ActiveCar == INDEX_NONE) return;

    // Gather active NPCs in the current car
    TArray<FPooledNPCInfo*> ActiveNPCs;
    for (FPooledNPCInfo& Info : PooledNPCs)
    {
        if (Info.AssignedCarIndex == ActiveCar && Info.bActive && Info.NPC.IsValid())
        {
            ActiveNPCs.Add(&Info);
        }
    }

    if (ActiveNPCs.Num() == 0) return;

    // Reset all full-tick flags
    for (FPooledNPCInfo* Info : ActiveNPCs)
    {
        Info->bFullTickThisFrame = false;
    }

    // Assign full tick to MaxFullTickPerFrame NPCs (round-robin)
    int32 NumFullTick = FMath::Min(Config.MaxFullTickPerFrame, ActiveNPCs.Num());
    for (int32 i = 0; i < NumFullTick; i++)
    {
        int32 Index = (FullTickRoundRobinIndex + i) % ActiveNPCs.Num();
        ActiveNPCs[Index]->bFullTickThisFrame = true;
    }
    FullTickRoundRobinIndex = (FullTickRoundRobinIndex + NumFullTick) % FMath::Max(1, ActiveNPCs.Num());

    // Apply tick intervals: full-tick NPCs get every-frame, others get reduced rate
    float ReducedTickInterval = 1.0f / FMath::Max(1.0f, Config.ReducedTickRateHz);
    for (FPooledNPCInfo* Info : ActiveNPCs)
    {
        ASEENPCCharacter* NPC = Info->NPC.Get();
        if (Info->bFullTickThisFrame)
        {
            NPC->PrimaryActorTick.TickInterval = 0.0f; // Every frame
        }
        else
        {
            NPC->PrimaryActorTick.TickInterval = ReducedTickInterval;
        }
    }
}

void USEENPCPoolSubsystem::CleanupStaleEntries()
{
    for (int32 i = PooledNPCs.Num() - 1; i >= 0; --i)
    {
        if (!PooledNPCs[i].NPC.IsValid())
        {
            PooledNPCs.RemoveAt(i);
        }
    }
}
