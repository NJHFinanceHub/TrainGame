# Combat System Prototype - Design Notes

## Architecture Overview

The combat system is built as modular UE5 components that can be attached to any Actor/Pawn:

```
ATrainGameCombatCharacter (Character base)
├── UCombatComponent       (attacks, blocks, dodges, stamina, Kronole mode)
├── UWeaponComponent       (equipped weapon, degradation, presets)
└── [Animation/Mesh]       (to be added)

ACombatTestCar (Level container)
├── UEnvironmentalHazardComponent[] (steam vents, panels, windows)
├── Enemy spawn points
└── Weapon pickup locations

ACombatAIController (Enemy brain)
├── Profile-based behavior (Desperate, Disciplined, Cunning, Brute)
├── Corridor-aware engagement limits
└── Environmental hazard awareness
```

## Core Combat Loop

1. **Positional Combat**: In a 3m-wide train corridor, at most 2-3 combatants can
   engage simultaneously. Control the width = control the fight.

2. **Directional Attack/Block**: Attacks come from High/Mid/Low. Blocks must match
   direction for full mitigation. Wrong direction = partial block. This creates a
   reading/prediction game.

3. **Stamina Economy**: Every action costs stamina. Blocking drains stamina over time.
   Running out of stamina while blocking = guard break (stagger). Fatigue accumulates
   over extended fights, reducing max stamina. Forces tactical retreats.

4. **Weapon Degradation**: All weapons degrade. Improvised weapons (Tail) break fast.
   Military weapons (Jackboot) last longer. Broken weapons deal minimal damage. Players
   must scavenge, repair, or go unarmed.

5. **Environmental Kills**: Steam vents (DOT + knockback), electrical panels (burst damage),
   window breaches (one-shot instant kill, sucked into frozen void). Players and AI can
   trigger these.

6. **Kronole Mode**: Slow-motion perception at the cost of health. Limited duration,
   long cooldown. High risk/reward for skilled players. Represents the drug's dangerous
   enhancement effect from the source material.

## Weapon Presets (Prototype)

| Weapon | Category | Tier | Damage | Speed | Durability | Notes |
|--------|----------|------|--------|-------|------------|-------|
| Pipe Club | Blunt | Improvised | 12 | 0.9x | 40 | Workhorse Tail weapon |
| Shiv | Bladed | Improvised | 15 | 1.4x | 25 | Fast but fragile |
| Nail Bat | Blunt | Improvised | 16 | 0.8x | 35 | High damage, slow |
| Reinforced Axe | Bladed | Functional | 22 | 0.7x | 80 | Third Class upgrade |
| Jackboot Baton | Blunt | Military | 18 | 1.1x | 120 | Durable, reliable |
| Crossbow | Ranged | Functional | 35 | 0.3x | 60 | Slow but powerful |
| Pipe Gun | Ranged | Improvised | 45 | 0.2x | 15 | Devastating but breaks fast |

## AI Profiles

- **Desperate** (Tailies): Wild swings, poor blocking, panicked. Easy individually but
  dangerous in groups due to unpredictability.
- **Disciplined** (Jackboots): Formation fighting, coordinated attacks, good blocking.
  The standard challenge. Up to 3 can coordinate simultaneously.
- **Cunning** (Smugglers): Dirty fighters. Favor low attacks, high dodge chance, very
  likely to use environmental hazards against you.
- **Brute** (Heavies): Slow but devastating. Hard to stagger. Overhead strikes.
  One-on-one challenges.

## Test Car: "The Pit" (Car 3)

The Pit is the Tail's fighting arena where Tailies fight for extra rations.
- 50m long, 3m wide standard gauge corridor
- 6 enemies (configurable)
- 3 steam vents, 2 electrical panels, 2 window breach points
- 3 weapon pickup locations (pipe club, shiv, nail bat)

## Integration Points (for future systems)

- **Hunger System**: Affects stamina regen rate
- **Cold System**: Increases stamina costs in cold zones
- **Companion AI**: Companions use same CombatComponent
- **Crafting**: Weapon creation feeds into WeaponComponent
- **Save System**: CombatComponent state serializable via FWeaponStats/FStaminaState
