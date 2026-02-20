# Weapons & Armor System

## Weapon Tier System

5 tiers from desperate improvisation to legendary one-of-a-kind pieces.

### Tier Overview

| Tier | Name | Source | Base Damage | Max Durability | Dur. Loss/Hit | Repair Cost |
|------|------|--------|-------------|----------------|---------------|-------------|
| T1 | Improvised | Tail crafting, scavenging | 8-15 | 40-60 | 3-5 | Basic Kit |
| T2 | Functional | Third Class workshops, quest rewards | 18-28 | 80-120 | 2-3 | Standard Kit |
| T3 | Military | Armory, Jackboot loot, quest rewards | 30-45 | 150-200 | 1-2 | Military Kit |
| T4 | Specialized | Blueprint crafting, boss drops | 40-60 | 200-300 | 1 | Military Kit + Parts |
| T5 | Legendary | Unique quest rewards only (5 total in game) | 50-75 | 500 | 0.5 | Irreplaceable (self-repair) |

### Weapon Categories

#### Blunt Weapons
Close-range, stamina-drain focused. Good against armored targets (ignores 25% DR).

| Weapon | Tier | Damage | Speed | Range | Special |
|--------|------|--------|-------|-------|---------|
| Fists | — | 5 | 1.5 | 100 | Always available, no durability |
| Pipe Club | T1 | 10 | 1.0 | 140 | Stagger chance 10% |
| Nail Bat | T1 | 13 | 0.9 | 150 | Bleed on hit (2 DPS, 5s) |
| Spiked Knuckles | T2 | 18 | 1.4 | 110 | Counter-attack bonus +25% |
| Reinforced Mace | T2 | 24 | 0.8 | 150 | Stagger chance 20% |
| Jackboot Baton | T3 | 30 | 1.1 | 140 | Stun on charged hit (2s) |
| Crowd Control Maul | T3 | 40 | 0.6 | 160 | Knockback, stagger 30% |
| The Last Spike | T5 | 55 | 1.0 | 150 | Railroad spike weapon. Stagger 40%, armor piercing |

#### Bladed Weapons
High damage, durability-hungry. Causes bleeding.

| Weapon | Tier | Damage | Speed | Range | Special |
|--------|------|--------|-------|-------|---------|
| Shiv | T1 | 8 | 1.6 | 100 | Stealth kill capable, bleed 3 DPS |
| Improvised Spear | T1 | 12 | 0.9 | 200 | Long reach, thrust attacks |
| Reinforced Axe | T2 | 25 | 0.8 | 140 | Bleed 5 DPS, can breach doors |
| Tactical Blade | T3 | 35 | 1.2 | 130 | Stealth kill capable, bleed 7 DPS |
| Wilford's Saber | T4 | 50 | 1.1 | 150 | Bleed 10 DPS, morale bonus to allies |
| Icebreaker | T5 | 65 | 0.9 | 160 | Legendary katana. Cold damage aura, bleed 12 DPS |

#### Piercing Weapons
Armor-penetrating. Effective against heavy targets.

| Weapon | Tier | Damage | Speed | Range | Special |
|--------|------|--------|-------|-------|---------|
| Bone Shiv | T1 | 7 | 1.8 | 90 | Ignores 15% armor |
| Throwing Knife | T1 | 10 | — | 800 (thrown) | Thrown, retrievable, stealth capable |
| Reinforced Spear | T2 | 22 | 0.9 | 220 | Ignores 25% armor, thrust attacks |
| Military Bayonet | T3 | 32 | 1.1 | 160 | Ignores 35% armor, attached to rifle |
| Mr. Wilford's Corkscrew | T5 | 45 | 1.5 | 120 | Ignores 60% armor, poison on crit |

#### Ranged Weapons
Distance engagement. Limited by ammo.

| Weapon | Tier | Damage | Speed | Range | Special |
|--------|------|--------|-------|-------|---------|
| Thrown Object | T1 | 6 | — | 600 | Uses misc items, improvised |
| Crossbow | T2 | 20/bolt | 0.5 | 1500 | Silent, retrievable bolts |
| Improvised Firearm | T2 | 22/shot | 0.6 | 1000 | Loud, attracts guards |
| Military Rifle | T3 | 38/shot | 0.8 | 2000 | 5-round magazine, loud |
| EMP Rifle | T4 | 15/shot | 0.7 | 1200 | Disables electronics, stuns cyborg enemies |

#### Explosive Weapons
Area damage. Destroys cover, breaches walls. Loud.

| Weapon | Tier | Damage | Speed | Range | Special |
|--------|------|--------|-------|-------|---------|
| Molotov Cocktail | T1 | 15 (area) | — | 600 (thrown) | Fire area (3m, 8s), panic effect |
| Explosive Charge | T3 | 60 (area) | — | Placed | 5m blast, breaches reinforced doors |
| Stun Grenade | T3 | 5 (area) | — | 800 (thrown) | Stun all in 5m for 4s |
| Proximity Mine | T3 | 50 (area) | — | Placed | Triggered by movement, 4m blast |

#### Electric Weapons
Stun-focused. Non-lethal option.

| Weapon | Tier | Damage | Speed | Range | Special |
|--------|------|--------|-------|-------|---------|
| Jury-Rigged Taser | T2 | 8 | 1.0 | 120 | Stun 3s, non-lethal takedown |
| Shock Trap | T2 | 10 | — | Placed | Stun 3s on trigger |
| Arc Welder | T4 | 30 | 0.8 | 150 | Chain lightning (hits 3 targets) |

## Armor System

Armor provides Damage Reduction (DR%) and optional special properties.

### Armor Slots

| Slot | Coverage | Weight Range |
|------|----------|-------------|
| Head | Head hits only | 1-3 kg |
| Torso | Body hits (most common) | 3-10 kg |
| Shield | Directional block (active) | 4-8 kg |

### Armor by Tier

| Armor | Tier | Slot | DR% | Cold Resist | Noise | Weight | Special |
|-------|------|------|-----|-------------|-------|--------|---------|
| Scrap Shield | T1 | Shield | +5% (block) | 0 | Loud | 4 kg | Breaks fast (30 dur) |
| Padded Vest | T1 | Torso | 8% | +5 | Quiet | 3 kg | No movement penalty |
| Chain Vest | T2 | Torso | 15% | 0 | Normal | 5 kg | Bladed resistance +5% |
| Metal Helmet | T2 | Head | 10% (head) | 0 | Normal | 2 kg | Reduces headshot crit |
| Reinforced Shield | T2 | Shield | +12% (block) | 0 | Normal | 5 kg | — |
| Ballistic Vest | T3 | Torso | 25% | 0 | Normal | 7 kg | Ranged DR +10% |
| Riot Shield | T3 | Shield | +20% (block) | 0 | Loud | 7 kg | Full frontal cover |
| Silk-Lined Armor | T3 | Torso | 20% | +10 | Silent | 4 kg | Stealth-compatible |
| Insulated Suit | T3 | Torso | 10% | +40 | Quiet | 5 kg | EVA-capable |
| Engineer's Exo-Frame | T4 | Torso | 35% | +15 | Loud | 10 kg | +15 carry weight |
| Phoenix Cloak | T4 | Torso | 25% | +30 | Quiet | 4 kg | Fire immune |

### Armor Properties

- **Noise Level**: Affects stealth detection radius. Silent < Quiet < Normal < Loud.
- **Cold Resistance**: Added to base cold resistance for EVA/exterior sections.
- **Movement Penalty**: Heavy armor (>7kg torso) reduces dodge speed by 15%.
- **Durability**: Armor degrades on hits received. Same tier system as weapons.

### Disguises (Special Armor)

Disguises occupy the torso slot. They provide no DR but enable social infiltration.

| Disguise | Access Granted | Detection Risk | Blown By |
|----------|---------------|----------------|----------|
| Worker Disguise | Spine service areas | Medium | Spine supervisors, running |
| Jackboot Uniform | Guard-only areas, barracks | High (requires behavior) | Officers, known Jackboots |
| First Class Attire | First Class cars, lounges | Low (if behaving) | Personal staff, tailors |
| Bureaucrat Robes | Bureaucracy offices, records | Medium | Senior bureaucrats |

Disguise detection accumulates. Suspicious actions (running, fighting, looting) increase
detection meter. When the meter fills, the disguise is blown for that zone permanently.
