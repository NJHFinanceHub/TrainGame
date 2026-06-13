#include "SEEHungerComponent.h"
#include "SEEHealthComponent.h"
#include "Net/UnrealNetwork.h"

USEEHungerComponent::USEEHungerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentHunger = MaxHunger;

	// Hunger is server-authoritative and replicates to the owning client.
	SetIsReplicatedByDefault(true);
}

void USEEHungerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Per-player hunger is private to its own player's client (COND_OwnerOnly).
	DOREPLIFETIME_CONDITION(USEEHungerComponent, CurrentHunger, COND_OwnerOnly);
}

void USEEHungerComponent::OnRep_CurrentHunger()
{
	// Mirror the server's broadcast so the owning client's hunger UI updates.
	OnHungerChanged.Broadcast(GetHungerPercent());
}

void USEEHungerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// CO-OP: hunger drain + starving damage are authoritative. Clients receive the
	// resulting CurrentHunger via replication + OnRep_CurrentHunger. Standalone is
	// authority, so this runs exactly as before (single-player unchanged).
	if (GetOwner() && !GetOwner()->HasAuthority()) return;

	float PrevHunger = CurrentHunger;
	CurrentHunger = FMath::Max(0.0f, CurrentHunger - DrainRate * DeltaTime);

	// Notify on threshold crossings
	if (PrevHunger > 0.0f && CurrentHunger <= 0.0f)
	{
		OnStarving.Broadcast();
	}

	// Broadcast changes at meaningful intervals
	float PrevPercent = FMath::FloorToFloat(PrevHunger / MaxHunger * 100.0f);
	float CurrPercent = FMath::FloorToFloat(CurrentHunger / MaxHunger * 100.0f);
	if (PrevPercent != CurrPercent)
	{
		OnHungerChanged.Broadcast(GetHungerPercent());
	}

	// Starving = HP drain (TakeDamage is itself server-gated)
	if (CurrentHunger <= 0.0f)
	{
		if (AActor* Owner = GetOwner())
		{
			if (USEEHealthComponent* Health = Owner->FindComponentByClass<USEEHealthComponent>())
			{
				Health->TakeDamage(StarvingHealthDrain * DeltaTime, ESEEDamageType::Environmental, nullptr);
			}
		}
	}
}

void USEEHungerComponent::Eat(float Amount)
{
	// CO-OP: authority-gated. Eat reaches the server through the inventory's
	// ServerUseItem path (consumables call Eat there); a stray client-side call is
	// a no-op so hunger can't be mutated non-authoritatively. The authoritative
	// CurrentHunger then replicates down + OnRep refreshes the owner's UI.
	// Standalone is authority — runs directly, single-player unchanged.
	if (GetOwner() && !GetOwner()->HasAuthority()) return;

	CurrentHunger = FMath::Min(MaxHunger, CurrentHunger + Amount);
	OnHungerChanged.Broadcast(GetHungerPercent());
}

float USEEHungerComponent::GetStaminaRegenModifier() const
{
	if (CurrentHunger / MaxHunger <= 0.5f) return 0.5f;
	return 1.0f;
}

float USEEHungerComponent::GetMaxStaminaModifier() const
{
	if (CurrentHunger / MaxHunger <= 0.25f) return 0.7f;
	return 1.0f;
}
