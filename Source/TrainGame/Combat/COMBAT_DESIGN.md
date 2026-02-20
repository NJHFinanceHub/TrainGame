# Combat System Prototype - Design Notes

## Architecture Overview

The combat system is built as modular UE5 components that can be attached to any Actor/Pawn:

```
ATrainGameCombatCharacter (Character base)
├── UCombatComponent             (attacks, blocks, dodges, stamina, Kronole mode)
├── UWeaponComponent             (equipped weapon, degradation, presets)
├── URangedCombatComponent       (ranged weapons, projectiles, ammo)
├── UStealthTakedownComponent    (stealth kills, chokeholds, knockouts)
└── [Animation/Mesh]             (to be added)

AEnemyCharacterBase (Enemy base — extends CombatCharacter)
├── EEnemyType config            (7 enemy types with unique presets)
├── Alert/backup system          (Captains call reinforcements)
└── Stealth vulnerability        (per-type resistance values)

ABossCharacterBase (Boss base — extends EnemyCharacter)
├── UBossFightComponent          (phases, special attacks, add spawning)
├── EBossIdentity                (7 zone bosses with unique mechanics)
└── Phase transitions            (invulnerability window, add waves)

ACombatTestCar (Level container)
├── UEnvironmentalHazardComponent[] (steam vents, panels, windows)
├── Enemy spawn points
└── Weapon pickup locations

AProjectileBase (Ranged projectile)
├── Collision + movement         (sphere trace, gravity, ricochet)
├── Damage delivery              (on-hit combat integration)
└── Corridor bouncing            (ricochets off walls)

ACombatAIController (Enemy brain)
├── Profile-based behavior       (Desperate, Disciplined, Cunning, Brute)
├── Corridor-aware engagement    (max simultaneous attackers)
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

## Ranged Combat System

Ranged weapons are high-risk/high-reward in tight corridors:

- **Projectile actors** (AProjectileBase): Physical projectiles that travel through corridors
- **Corridor ricochets**: Bolts and thrown objects can bounce off walls
- **Ammo management**: Limited ammo, slow reloads leave you vulnerable
- **Thrown objects**: Improvised ranged attacks (lower damage, higher gravity)
- **Reload interruption**: Taking damage cancels reload

### Ranged Weapon Stats

| Weapon | Category | Damage | Speed | Range | Ammo | Notes |
|--------|----------|--------|-------|-------|------|-------|
| Crossbow | Ranged | 35 | 0.3x | 2000 | Limited | Slow reload, reliable |
| Pipe Gun | Ranged | 45 | 0.2x | 1500 | Limited | Devastating, breaks fast |
| Thrown Knife | Thrown | 15 | 1.0x | 800 | 1 | Quick, disposable |
| Thrown Object | Thrown | 15 | 0.5x | 600 | 1 | Improvised, ricochets |

## Stealth Takedown System

Stealth is viable in Snowpiercer's corridor layout — approaching from behind requires timing:

### Takedown Types

| Type | Duration | Effect | Noise |
|------|----------|--------|-------|
| Grab From Behind | 0.5s | Hold (no damage) | Low |
| Chokehold | 3.0s | Non-lethal down | Low |
| Knockout | 0.8s | Near-death damage | Low |
| Silent Kill | 1.5s | Instant kill | Very low |

### Requirements
- Must be behind target (within 60° arc of their back)
- Target must not be in combat stance (Neutral or Downed)
- Must be within 150cm range
- Failed takedowns alert nearby enemies within 800cm radius
- Some enemies have stealth resistance (Breachmen are immune)

## Enemy Types

7 distinct enemy types with unique behaviors and equipment:

| Type | HP | Speed | Damage | Stealth Resist | Weapon | Notes |
|------|-----|-------|--------|----------------|--------|-------|
| Jackboot Grunt | 80 | 450 | 12 | 10% | Shock Baton | Standard, disciplined |
| Jackboot Captain | 150 | 400 | 18 | 40% | Reinforced Axe | Calls reinforcements |
| Order Zealot | 120 | 500 | 20 | 30% | Ceremonial Blade | Fanatical, aggressive |
| First Class Guard | 100 | 550 | 16 | 50% | Fencing Sabre | Fast, precise |
| Tail Fighter | 60 | 420 | 10 | 0% | Random improvised | Desperate, scrappy |
| Kronole Addict | 70 | 480 | 14 | 20% | Shiv | Uses Kronole mode |
| Breachman | 200 | 300 | 25 | 100% | Ice Pick | Tank, cold immune |

### Unique Enemy Behaviors
- **Jackboot Captain**: Calls backup within 3000cm when alerted
- **Order Zealot**: Fights to the death, never retreats
- **Kronole Addict**: Can enter slow-motion mode (unpredictable)
- **Breachman**: Immune to stealth kills, heavy armor vs cold/physical

## Boss Fight Framework

7 zone bosses with health phases and special attacks:

### Boss Roster

| Zone | Boss | HP | Phases | Adds | Key Mechanic |
|------|------|-----|--------|------|--------------|
| 1 (Tail) | The Pit Champion | 400 | 3 | 2 | Brute force, crowd |
| 2 (Third) | Chef Pierrot | 350 | 3 | 3 | Kitchen fire, boiling oil |
| 3 (Security) | Jackboot Commander | 500 | 3 | 4 | Squad tactics, formations |
| 4 (Night) | Kronole Lord | 300 | 3 | 0 | Time dilation, speed |
| 5 (First) | First Class Duelist | 350 | 3 | 0 | Fencing, riposte, precision |
| 6 (Shrine) | Order High Priest | 450 | 3 | 6 | Zealot waves, rituals |
| 7 (Engine) | Mr. Wilford | 600 | 4 | 2 | Engine hazards, final boss |

### Phase System
- Bosses transition phases at health thresholds (e.g., 60%, 25%)
- Brief invulnerability (2s) during transitions
- Each phase modifies damage, speed, and attack patterns
- Adds spawn on phase transitions (boss-specific)
- Enraged phase: dramatic stat boost at low health

### Special Attacks (per boss)
- **The Pit Champion**: Ground Slam, Bull Rush, Crowd Call
- **Chef Pierrot**: Cleaver Combo, Boiling Oil Throw, Kitchen Fire Ignite
- **Jackboot Commander**: Formation Charge, Shield Bash, Call Reinforcements
- **Kronole Lord**: Time Fracture, Kronole Burst, Slow Field
- **First Class Duelist**: Flurry of Blades, Riposte, Perfect Parry
- **Order High Priest**: Divine Punishment, Summon Zealots, Faithful Shield
- **Mr. Wilford**: Engine Overload, Steam Blast, Train Brake, Final Order

## Damage Types

Full damage type system for armor/resistance calculations:

| Type | Source | Notes |
|------|--------|-------|
| Physical | Melee, thrown objects | Standard |
| Blunt | Clubs, batons | Stagger bonus |
| Bladed | Swords, axes, shivs | Armor penetration |
| Piercing | Crossbow, firearms | High single-target |
| Fire/Thermal | Kitchen hazards, flammable | DOT potential |
| Cold | Window breaches, exterior | Stamina penalty |
| Electric | Panels, shock batons | Stagger |
| Explosive | Improvised charges | AoE |
| Stealth Takedown | Stealth kills | Bypasses defense |
| Non-Lethal | Chokeholds, knockout | Downs without killing |

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
| Ceremonial Blade | Bladed | Specialized | 22 | 1.2x | 100 | Order zealot weapon |
| Fencing Sabre | Bladed | Specialized | 18 | 1.5x | 150 | First Class elegant |
| Breachman's Ice Pick | Piercing | Military | 30 | 0.5x | 200 | Heavy, durable |

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
- **Stealth/Detection**: NPC detection system integrates with StealthTakedownComponent
- **Boss Encounters**: BossFightComponent triggers zone completion events
- **Ranged Combat**: Projectile actors integrate with physics and collision channels
