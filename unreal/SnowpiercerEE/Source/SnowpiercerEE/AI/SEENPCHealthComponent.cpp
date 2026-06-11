// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEENPCHealthComponent.h"

USEENPCHealthComponent::USEENPCHealthComponent()
{
	// The base component goes "downed" at zero health and only dies after
	// DownedDuration (a player revive window). NPCs just die: zero duration
	// promotes downed -> dead (OnDeath) on the next component tick, and the
	// AI controller also reacts to OnDowned immediately.
	DownedDuration = 0.0f;
}

void USEENPCHealthComponent::InitHealth(float InMaxHealth)
{
	MaxHealth = FMath::Max(1.0f, InMaxHealth);
	CurrentHealth = MaxHealth;
}
