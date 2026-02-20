# SNOWPIERCER: ETERNAL ENGINE — Next Level Implementation Prompt

> Use this prompt to take the game from prototype to production-quality.
> Feed this to any AI agent, coding team, or development partner.

---

## CURRENT STATE

You are continuing development on **Snowpiercer: Eternal Engine**, a first/third-person action-RPG built in **Unreal Engine 5.7.3 (C++)**.

### What Exists Now
- Full UE5 C++ project that compiles clean
- `ASEECharacter` — player character with WASD movement, sprint (Shift), run (R), crouch (Ctrl), jump (Space), first/third person toggle (V), stamina system, interaction system
- `ASEENPCCharacter` — base NPC class with patrol waypoints, sight + hearing detection, detection meter (unaware → suspicious → combat), ambient dialogue, state machine (Idle/Patrol/Suspicious/Alerted/Combat/Flee/Dead)
- `USEECarStreamingSubsystem` — loads/unloads car sublevels (current + adjacent)
- `USEESaveGameSubsystem` — save/load framework
- Exploration components: Collectible, Journal, Crawlspace, Exterior Traversal, Window View, Secret Car Manager
- `USEETrainMovementComponent` — constant train sway + ambient audio
- 200 imported art assets (FBX meshes, PBR textures, audio)
- Demo level: 100 connected train cars with themed lighting and materials
- Complete design documents (100+ pages covering all systems)

### What's Missing (Priority Order)

---

## TIER 1: MAKE IT PLAYABLE (Weeks 1-4)

### 1A. Combat System
The game is melee-focused in tight train corridors. Build:

```
SEECombatComponent (UActorComponent on both player and NPCs):
- Light attack (Left Click) — fast, low damage, short recovery
- Heavy attack (Hold Left Click) — slow, high damage, staggers
- Block (Right Click) — reduces incoming damage, drains stamina
- Parry (Right Click timed) — perfect block = free counter-attack window
- Dodge (Alt + direction) — i-frames, costs stamina
- Hit detection via weapon traces (not projectiles)
- Damage = base_weapon_damage × strength_modifier × (1 - target_armor_reduction)
- Weapon durability — degrades per hit, breaks at 0
- Stamina drain on all combat actions

SEEWeaponBase (AActor):
- 5 tiers: Improvised → Functional → Military → Specialized → Legendary
- 6 damage types: Blunt, Bladed, Piercing, Fire, Cold, Electric
- Stats: Damage, Speed, Durability, Weight, Range
- Attach to character hand socket

Environmental Combat:
- Steam vents (activate to burn nearby enemies)
- Electrical panels (shock enemies in water/metal)
- Window breach (cold damage zone, enemies stagger toward breach)
- Exploding pipes (area damage)
```

### 1B. Health & Survival Systems
```
SEEHealthComponent:
- HP pool (100 base, modified by Endurance)
- Damage types have armor interaction (bladed ignores some armor, blunt staggers)
- Downed state before death (companion can revive)
- Injury system: broken arm (-30% melee), leg wound (-40% speed), concussion (blur)
- Injuries persist until healed at medical station

SEEHungerComponent:
- Drains over real-time (~1 point per 2 minutes of gameplay)
- At 50%: stamina regen halved
- At 25%: max stamina reduced
- At 0%: HP slowly drains
- Food items restore hunger (protein blocks = 20, fresh food = 60)

SEEColdComponent:
- Only active in cold zones (freezer breach, exterior, hull damage)
- Temperature bar drains toward frostbite
- Frostbite stages: shivers (screen shake), numbness (-move speed), blackout (death)
- Cold suit extends time, fire sources pause drain
```

### 1C. Inventory System
```
SEEInventoryComponent (on player character):
- Slot-based with weight limit (carry capacity = 50 + Strength × 5)
- Item categories: Weapons, Armor, Consumables, Crafting Materials, Quest Items, Junk
- Stack sizes per item type
- Quick slots (1-4 keys) for consumables
- Loot containers in the world (crates, lockers, bodies)
- Drop/discard items

SEEItemBase (UObject):
- Name, Description, Icon, Weight, StackSize, Rarity
- Subclasses: SEEWeaponItem, SEEArmorItem, SEEConsumableItem, SEECraftingMaterial
```

### 1D. Basic HUD
```
Minimum viable HUD (UMG widgets):
- Health bar (top left)
- Stamina bar (below health)
- Cold indicator (only when in cold zone)
- Hunger indicator (only below 75%)
- Weapon/durability indicator (bottom right)
- Interaction prompt ("Press E to ___")
- Detection indicator (eye icon, fills with enemy awareness)
- Crosshair (minimal, context-sensitive)
- Damage direction indicators (red vignette from damage direction)
```

---

## TIER 2: MAKE IT A GAME (Weeks 5-10)

### 2A. Dialogue System
```
SEEDialogueManager (UGameInstanceSubsystem):
- Data-driven dialogue trees (UDataTable or custom UDataAsset)
- Node types: NPC line, Player choice (up to 4), Skill check, Branch, Set flag, End
- Skill check display: "[Charisma 6] Convince them to let you pass"
- Failed checks: different response, not a dead end
- NPC memory: TMap<FName, bool> of conversation flags
- Bark system: context-sensitive one-liners triggered by proximity/events

UI:
- Full-screen dialogue with speaker portrait + name
- Response options with skill indicators
- Timed responses (optional per node, silence = default choice)
```

### 2B. Quest System
```
SEEQuestManager (UGameInstanceSubsystem):
- Quest states: Available, Active, Completed, Failed
- Objective types: GoTo, Interact, Kill, Collect, Escort, Dialogue, Custom
- Main quest (7 zones, ~20 main objectives)
- Side quests (50+ total, 7-8 per zone)
- Companion personal quests (3-5 per companion, ~48 total)
- Quest UI: log with active/completed/failed tabs, map markers, tracking
- Quest rewards: XP, items, faction rep, companion loyalty
```

### 2C. Faction Reputation
```
SEEFactionManager (UGameInstanceSubsystem):
- 8 factions: Tailies, Union, Jackboots, Bureaucracy, FirstClass, Order, Kronole, Thaw
- Rep range: -100 (hostile) to +100 (devoted)
- Rep affects: NPC aggression, prices, dialogue options, area access
- Faction-specific quests unlock at rep thresholds
- Some factions are mutually exclusive (Jackboots vs Tailies, Order vs Thaw)
```

### 2D. AI Behavior Trees
```
Replace simple state machine with UE5 behavior trees:
- Jackboot: Patrol → Investigate sound → Alert squad → Combat formation
- Civilian: Daily schedule → Flee on danger → Report to Jackboots
- Companion: Follow player → Combat support → Use abilities → Banter
- Kronole Dealer: Wander → Approach player → Offer trade → Flee if hostile
- Breachman: Work exterior → Come inside → Share intel → Return to work

Use Blackboard for:
- Target actor, last known position, alert level
- Home car, patrol route, schedule phase
- Player reputation with NPC's faction
```

### 2E. Crafting
```
SEECraftingSubsystem:
- 6 station types (Improvised Workbench, Machine Shop, Chemistry, Electronics, Armorer, Tailor)
- Recipes as UDataTable rows: required materials, station type, skill requirements
- Crafting time (1-5 seconds with progress bar)
- Failure chance based on skill (0% at max skill, 30% at minimum)
- 50+ recipes across weapons, armor, tools, consumables, disguises
```

---

## TIER 3: MAKE IT BEAUTIFUL (Weeks 11-16)

### 3A. Art & Materials
```
Per-zone material palettes (create in UE Material Editor):
- Tail: Rust, condensation, grime, blue-gray with harsh fluorescent
- Third Class: Industrial yellow-green, worn metal, functional
- Second Class: Warm wood, soft fabric, aging comfort, amber
- First Class: Gold leaf, crystal, marble, art deco luxury
- Sanctum: White marble, religious gold, sterile unsettling
- Engine: Brass, copper, steam, mechanical cathedral

Material instances for variation:
- Damage states (clean → scratched → dented → destroyed)
- Wetness overlay (leaking pipes, condensation)
- Frost overlay (cold zones, exterior-adjacent)
```

### 3B. Modular Level Kit
```
Build reusable BSP/mesh pieces:
- Wall panels (standard, windowed, damaged, reinforced)
- Floor tiles (metal grate, solid plate, carpet, tile)
- Ceiling tiles (pipe-run, light fixture, vent grate, structural)
- Door frames (standard, reinforced, broken, locked)
- Furniture: bunks (stacked 2-4), tables, chairs, workbenches, lockers
- Pipes: straight, elbow, T-junction, valve (interactive)
- Decorative: posters, graffiti, blood splatter, frost, condensation decals

Each car built from ~20-30 modular pieces with unique arrangement
```

### 3C. Lighting
```
Per-zone lighting rigs (reusable):
- Tail: Single fluorescent strips (flickering), zero natural light
- Third Class: Industrial strip lights, occasional colored bulb
- Second Class: Warm sconces, overhead panels, window light
- First Class: Crystal chandeliers, accent lighting, dramatic shadows
- Sanctum: Candles, stained glass projections, ethereal glow
- Engine: Furnace glow, instrument panels, blue-white reactor light

Dynamic lighting events:
- Power outages (per-car, story triggered)
- Combat damage breaks lights
- Kronole vision (chromatic aberration, light bleeding)
```

### 3D. Post-Processing
```
Zone-specific post-process volumes:
- Tail: Desaturated, high contrast, film grain, slight blue tint
- Third Class: Slightly warm, moderate contrast
- Second Class: Warm, soft bloom, moderate saturation
- First Class: High saturation, golden bloom, lens flare
- Sanctum: Cool, ethereal bloom, slight vignette
- Engine: High contrast, blue highlights, motion blur
- Exterior: Extreme brightness, snow glare, lens distortion
```

---

## TIER 4: MAKE IT COMPLETE (Weeks 17-24)

### 4A. All 100 Cars as Sublevels
Each car needs:
- Unique sublevel in `/Game/Maps/Cars/Car_XX_Name`
- Modular geometry from kit
- Zone-appropriate lighting and materials
- NPC placements (data table: which NPCs, where, what schedule)
- Loot placements (data table: which containers, what items, respawn rules)
- Interactive objects (doors, switches, vents, crafting stations)
- Quest triggers
- Ambient audio

### 4B. All 6 Endings
- Ending calculator checks: faction reps, companion survival, choice flags
- Each ending needs: unique final dialogue, cutscene/montage, credits music
- Some endings require specific companions alive
- Some require specific secret cars discovered

### 4C. Full Audio
- Commission or source: 7 zone ambient tracks, 7 boss themes, exploration/combat layers
- Implement adaptive music system (FMOD or Wwise, or UE MetaSounds)
- Record/generate PA announcements per zone
- Full SFX pass on all interactions

### 4D. Complete Testing
- Full playthrough: 20-30 hours main quest, 40-50 hours completionist
- All 6 endings achievable
- All skill checks reachable
- No soft locks
- Performance: 60fps at 1080p on GTX 1070 / RX 580

---

## DESIGN PRINCIPLES

1. **The train is a character.** It sways, creaks, groans. You feel it moving. The rhythm of the wheels is the heartbeat.

2. **Scarcity is tension.** In the Tail, everything is precious. By First Class, abundance is obscene. The contrast IS the game.

3. **No right answers.** Every moral choice should make the player pause. Steal food from children or let your people starve. Free prisoners who might be murderers. Sabotage the food supply to weaken the elite.

4. **Corridors are arenas.** Tight spaces force tactical combat. You can't be surrounded if you hold the width. Push enemies into hazards. Use the environment.

5. **Class is visual.** The player should FEEL the transition from Tail to First Class. The lighting changes, the materials change, the NPCs change, the music changes. It should be jarring.

6. **The Engine is earned.** 99 cars of struggle, sacrifice, and choice before you reach it. The payoff must be worth the journey.

---

## TECHNICAL NOTES

- **Engine:** Unreal Engine 5.7.3, C++ module `SnowpiercerEE`
- **Build:** `Build.bat SnowpiercerEEEditor Win64 Development`
- **Repo:** `git@github.com:NJHFinanceHub/TrainGame.git` (branch: main)
- **Source path:** `unreal/SnowpiercerEE/Source/SnowpiercerEE/`
- **Dependencies:** Core, CoreUObject, Engine, InputCore, EnhancedInput, UMG, Slate, SlateCore
- **Existing classes:**
  - `ASEECharacter` — player
  - `ASEENPCCharacter` — NPC base
  - `ASnowpiercerEEGameMode` — game mode
  - `ASEEPlayerController` — player controller with view toggle
  - `USEECarStreamingSubsystem` — car loading
  - `USEESaveGameSubsystem` — save/load
  - `USEETrainMovementComponent` — train sway
  - `UCollectibleComponent` — pickups
  - `UCollectibleJournalSubsystem` — collectible tracking
  - `UCrawlspaceComponent` — under-car traversal
  - `UExteriorTraversalComponent` — rooftop traversal
  - `UWindowViewComponent` — exterior views
  - `USecretCarManager` — hidden car tracking

---

## HOW TO USE THIS PROMPT

### For AI Agents (Claude, Codex, GPT)
1. Clone the repo
2. Read existing source files to understand patterns
3. Pick a Tier 1 item and implement it fully in C++
4. Compile, test, commit
5. Move to next item

### For Human Developers
1. Use Tier 1 as your sprint backlog
2. Each bullet is roughly 1-3 days of work
3. Tier 2 items depend on Tier 1 completion
4. Art/audio (Tier 3) can be parallelized

### For a Studio
1. Tiers map to milestones: Playable → Game → Beautiful → Complete
2. Each tier is ~4-6 weeks for a small team (3-5 devs)
3. Total estimate: 6-9 months for a polished vertical slice (Zones 1-3)
4. Full game: 12-18 months with dedicated team

---

*The train waits for no one. Start building.*
