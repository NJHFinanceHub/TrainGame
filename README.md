# Snowpiercer: Eternal Engine

**A single-player action RPG set aboard the last train on Earth.**

---

## Overview

Snowpiercer: Eternal Engine is a UE5 action RPG inspired by the Snowpiercer universe. Players fight, sneak, and negotiate their way through 100+ train cars spanning 7 distinct zones, from the squalid Tail to the mythical Engine. Every car is a self-contained encounter space with its own NPCs, loot, hazards, and moral dilemmas.

The game blends melee-focused corridor combat, stealth infiltration, deep crafting and survival systems, faction politics, and branching narrative choices that shape the fate of every passenger aboard.

## Key Features

- **100+ unique train cars** across 7 zones, each with distinct art direction, gameplay, and atmosphere
- **Melee-focused combat** designed for tight corridor encounters with environmental hazards
- **Stealth system** with line-of-sight detection, disguises, and full non-lethal playthrough support
- **Survival mechanics** -- cold exposure, hunger, Kronole addiction, injury persistence
- **Barter economy** with zone-specific currencies, 50+ crafting recipes, and 5-tier weapon progression
- **9 factions** with independent reputation tracks that gate access, quests, and endings
- **12+ recruitable companions** with loyalty systems, personal quests, and permadeath
- **Branching dialogue** with skill checks, NPC memory, and 24+ major moral choices
- **Multiple endings** determined by cumulative faction alignment, moral choices, and companion survival
- **Car streaming architecture** -- max 3 cars loaded at once, door animations mask loading
- **Per-car save state** -- every container opened, NPC killed, and door unlocked is tracked

## Zones

| # | Zone | Cars | Theme |
|---|------|------|-------|
| 1 | **The Tail** | 1-15 | Poverty, desperation, improvised survival |
| 2 | **Third Class** | 16-35 | Labor, industry, blue-collar resistance |
| 3 | **Second Class** | 36-55 | Bureaucracy, faded luxury, political intrigue |
| 4 | **Working Spine** | 56-70 | Machinery, infrastructure, the train's guts |
| 5 | **First Class** | 71-90 | Opulence, decadence, power games |
| 6 | **The Sanctum** | 91-98 | Religious order, Engine worship, fanaticism |
| 7 | **The Engine** | 99-100+ | The truth. Wilford. The choice. |

## Project Structure

```
unreal/SnowpiercerEE/          UE5.5 project scaffold
  Source/SnowpiercerEE/         Core module (types, subsystems, components)
    Exploration/                Collectible, secret car, exterior traversal components

Source/TrainGame/               Gameplay module
  Combat/                      Combat system, AI controllers, test car
  Companions/                  Companion roster, AI, loyalty, data assets
  Dialogue/                    Branching dialogue, NPC memory, rumor propagation
  Economy/                     Barter, merchants, crafting, armor, degradation
  Stealth/                     Detection, disguises, non-lethal options
  UI/                          HUD widgets, inventory, dialogue, crafting, train map

Source/SnowyEngine/             Core systems module
  Survival/                    Cold, hunger, Kronole, survival component
  Crafting/                    Crafting stations, recipes, types
  Faction/                     9-faction reputation system
  Inventory/                   Weight-based inventory, item types

narrative/                     Story content
  zone1/                       Zone 1 quest outlines, dialogue trees, moral choices

design/                        Level design documents
  zone1/cars/                  Per-car layout docs (Cars 1-15)

docs/                          Design documentation
  audio/                       Music direction, adaptive system, voice acting scope
  characters/                  Wilford, antagonists, zone bosses, traitor
  companions/                  Companion roster and system design
  dialogue/                    Dialogue system architecture
  economy/                     Resources, crafting, trading, weapons, degradation
  exploration/                 Collectibles, secret cars, exterior traversal, journal
  lore/                        World history, timeline, demographics, revolutions
  moral-choices/               24+ moral choice catalog with consequences
  npcs/                        36 named NPC catalog
  player/                      Protagonist, stats, leveling, perks, movement, death
  publishing/                  Steam page, achievements, content rating, mod support
  replayability/               NG+, completion tracking, procedural elements
  settings/                    Difficulty tiers, accessibility options
  stealth/                     Stealth system, disguises, non-lethal design
  technical/                   Engine workflow, save format, car streaming, perf budgets
  tutorial/                    Cars 1-5 onboarding, hint system, later zone mechanics
```

## Technical Stack

- **Engine:** Unreal Engine 5.5
- **Language:** C++ (no Blueprints in source -- all gameplay logic in code)
- **Target:** PC (Steam), 60fps at minimum spec
- **Architecture:** Car streaming (max 3 loaded), sublevel-per-car, door-animation load masking
- **Save System:** Per-car state persistence via `USaveGame` subsystem

## Building

### Prerequisites
- Unreal Engine 5.5 (source or Epic Games Launcher)
- Visual Studio 2022 or Rider (Windows), Xcode (macOS), clang (Linux)

### Steps
1. Clone the repository
2. Open `unreal/SnowpiercerEE/SnowpiercerEE.uproject` in the UE5 Editor
3. Accept module rebuild if prompted
4. Press Play in the Editor to launch

### Command Line (Linux)
```bash
/path/to/UE_5.5/Engine/Binaries/Linux/UnrealEditor \
  "$(pwd)/unreal/SnowpiercerEE/SnowpiercerEE.uproject"
```

## Development Status

### Completed
- UE5 project scaffold with core types, enums, and subsystems
- Combat system (melee, AI, environmental, test car)
- Stealth system (detection, disguises, non-lethal, alert propagation)
- Survival system (cold, hunger, Kronole addiction, injury)
- Economy and crafting (resources, 50+ recipes, barter, merchants, degradation)
- Faction reputation (9 factions, gated access)
- Companion system (12+ companions, loyalty, permadeath, personal quests)
- Dialogue system (branching, skill checks, NPC memory, rumor propagation)
- UI/HUD (survival bars, inventory, dialogue, crafting, train map, faction panel)
- Car streaming subsystem and save system
- Player character (protagonist, stats, leveling, perks, movement, death/injury)
- Zone 1 narrative (quest outline, dialogue trees, moral choices, faction lore)
- Zone 1 level design (15 car layouts)
- 36 named NPCs and 24 moral choices cataloged
- Audio and music direction (adaptive system, zone identity, voice acting scope)
- Technical specifications (save format, car streaming, performance budgets)
- Difficulty settings and accessibility options
- Tutorial and onboarding design (Cars 1-5)
- Exploration (collectibles, secret cars, exterior traversal, journal/codex)
- Replayability (NG+, branching tracking, procedural elements)
- Steam integration (achievements, store page, content rating, mod support)
- Antagonist design (Wilford, mid-game antagonist, zone bosses, traitor)
- World lore (history, timeline, demographics, revolutions, route)

### In Progress
- Exterior environment and EVA/cold suit mechanics
- Remaining zone narratives (Zones 2-7)
- Asset production and level blockouts

## Repository

- **GitHub:** [NJHFinanceHub/TrainGame](https://github.com/NJHFinanceHub/TrainGame)

## License

All rights reserved. This project is proprietary and not open source.
