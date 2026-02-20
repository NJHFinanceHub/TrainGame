#include "SEEHungerComponent.h"
#include "SEEHealthComponent.h"

USEEHungerComponent::USEEHungerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentHunger = MaxHunger;
}

void USEEHungerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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

	// Starving = HP drain
	if (CurrentHunger <= 0.0f)
	{
		if (USEEHealthComponent* Health = GetOwner()->FindComponentByClass<USEEHealthComponent>())
		{
			Health->TakeDamage(StarvingHealthDrain * DeltaTime, ESEEDamageType::Environmental, nullptr);
		}
	}
}

void USEEHungerComponent::Eat(float Amount)
{
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
