# SNOWPIERCER: ETERNAL ENGINE — GasTown Master Prompt

## PROJECT OVERVIEW

Build a **Snowpiercer-inspired first/third-person action-RPG** in Unreal Engine 5 (C++) where the player starts in the Tail Section and must fight, negotiate, deceive, and survive their way through **100+ unique train cars** to reach the Engine at the front. The game is linear but deeply branching — each car is a self-contained encounter/level with its own mechanics, factions, resources, and moral choices.

**Source Material:** Draw from the 2013 Bong Joon-ho film, the TNT/Netflix TV series (all seasons), and the original French graphic novel *Le Transperceneige*. Synthesize the best worldbuilding from all three — the class warfare, the sacred Engine mythology, the drug culture (Kronole), the frozen wasteland, the political intrigue, and the brutal survival.

**Engine:** Unreal Engine 5, C++
**Target:** PC (Steam), controller + keyboard/mouse support
**Tone:** Gritty survival with moments of surreal opulence. Dark, claustrophobic, morally gray.

---

## AGENT DELEGATION STRUCTURE

Mayor: Decompose this project into the following workstreams and assign agents accordingly. Agents should work in parallel where possible, with integration checkpoints.

### Agent Roles Needed

| Agent Role | Responsibility |
|---|---|
| **Game Architect** | Core game loop, progression systems, save/load, UE5 project structure, car-to-car transition system |
| **Narrative Designer** | 100+ car descriptions, dialogue trees, faction lore, multiple endings, moral choice system |
| **Combat Designer** | Melee-heavy combat system, weapon crafting, stealth mechanics, environmental combat |
| **Level Designer(s)** | Car layouts, environmental storytelling, puzzle design, verticality within cars |
| **Systems Designer** | Resource management, hunger/cold/morale systems, inventory, crafting, trading |
| **AI Programmer** | Enemy AI behaviors, NPC schedules, faction reputation system, companion AI |
| **UI/UX Designer** | HUD, inventory screens, dialogue UI, map/train diagram, resource meters |
| **Art Director** | Visual style guide, lighting per car zone, material palettes, atmosphere specs |
| **Audio Designer** | Sound design direction, ambient train sounds, music cues per zone, combat audio |
| **QA/Integration** | Cross-system testing specs, performance budgets, save system validation |

---

## CORE GAME DESIGN

### The Train — "Snowpiercer, 1,034 Cars Long"

The train is divided into **7 major zones**, each containing 12-18 cars. The player progresses forward but can backtrack. Each car has a unique identity.

### Zone Breakdown

#### ZONE 1: THE TAIL (Cars 1–15)
The slum. No windows. Rationed protein blocks. This is home — the player's origin.

| Car # | Name | Description | Mechanics |
|---|---|---|---|
| 1 | The Dead End | Sealed rear wall, makeshift memorials, cramped bunks stacked 4-high | Tutorial area — basic movement, interaction, inventory |
| 2 | The Protein Kitchen | Where the infamous protein blocks are processed from insects | First crafting station — learn to make improvised tools |
| 3 | The Pit | Open area where Tailies fight for extra rations | Combat tutorial — unarmed fighting, learn blocking/dodging |
| 4 | The Nursery | Where Tail children are kept, some go missing (taken forward) | Moral choice: steal children's rations or protect them |
| 5 | The Elders' Car | Council of Tail leaders, planning the revolution | Quest hub — receive main quest, recruit first companions |
| 6 | The Sickbay | Makeshift infirmary, disease spreading, limited medicine | Resource management intro — medicine is currency here |
| 7 | The Workshop | Hidden car where Tailies forge weapons from scrap | Weapon crafting system unlock — pipes, shivs, shields |
| 8 | The Listening Post | Tailies who press ears to the walls to learn train secrets | Lore dumps, optional intel that affects later choices |
| 9 | The Blockade | First major combat encounter — Jackboots guard the gate | First real fight — 8-10 enemies, use environment |
| 10 | The Dark Car | Pitch black, no power, things live in here | Horror/stealth section — limited visibility, sound-based |
| 11 | The Freezer Breach | Hull breach, sub-zero temps, must move fast | Environmental hazard — cold damage timer, find path through |
| 12 | The Kronole Den | First encounter with the drug made from industrial waste | Choice: use Kronole (temporary buffs, long-term debuffs) |
| 13 | The Smuggler's Cache | Hidden compartment car run by a black market dealer | Trading system unlock — barter economy |
| 14 | The Martyr's Gate | Where previous revolution attempts ended in massacre | Story beat — find evidence of what happened to past revolts |
| 15 | The Threshold | Massive reinforced door to Third Class | BOSS ENCOUNTER — Jackboot Commander + squad |

#### ZONE 2: THIRD CLASS (Cars 16–30)
Marginally better. Workers who maintain the train. Bunks with thin curtains. Shared bathrooms. Resentment.

- **Car 16: Processing Hall** — Third Class orientation, ID papers, tracking bracelets
- **Car 17: The Laundry** — Industrial washers, steam vents (combat arena with environmental kills)
- **Car 18: Worker Barracks A** — Recruit potential allies, faction introductions
- **Car 19: Worker Barracks B** — Rival worker faction, choose who to align with
- **Car 20: The Mess Hall** — Better food than Tail, but rationed. Social hub.
- **Car 21: The Shift Office** — Work assignments. Can forge documents to access restricted cars.
- **Car 22: Sanitation Systems** — Sewage/water processing. Stealth route to bypass cars.
- **Car 23: The Chapel of the Engine** — Wilford worship. First encounter with Engine theology.
- **Car 24: Machine Shop** — Better crafting materials. Upgrade weapons.
- **Car 25: The Brig** — Train prison. Free prisoners for allies or leave them.
- **Car 26: Medical Bay** — Real medicine. Bribe, steal, or trade for supplies.
- **Car 27: The School Car** — Children being indoctrinated. Propaganda mechanic introduced.
- **Car 28: Hydroponics Bay** — Food production. Sabotage or protect?
- **Car 29: The Union Hall** — Worker strike brewing. Major faction quest.
- **Car 30: The Checkpoint** — Second Class border. Papers or combat.

#### ZONE 3: SECOND CLASS (Cars 31–48)
The middle. Professionals, technicians, bureaucrats. Small private rooms. Relative comfort. Fear of falling to Third.

- Economy cars, a dentist, a tailor, a small market
- **The Bureaucracy Car** — Administrative center, access codes, political manipulation
- **The Surveillance Hub** — Hack cameras to scout ahead or erase evidence
- **The Second Class Lounge** — Alcohol, gossip, blackmail opportunities
- **The Greenhouse** — Exotic plants, poisons available for stealth players
- **The Print Shop** — Propaganda production, can create counter-propaganda
- **The Clinic** — Surgeon who performs "treatments" (lobotomies for dissidents)
- **The Children's Academy** — More indoctrination, can radicalize or protect kids
- **The Voting Car** — Second Class elects representatives. Rig elections?
- **The Night Car** — Drug lounge, psychedelic Kronole variants, vision quests
- **The Archive** — Train history, Wilford's real origins, critical lore
- Major quest: Second Class coup attempt — support or betray?

#### ZONE 4: THE WORKING SPINE (Cars 49–62)
The mechanical guts of the train. Engine rooms, electrical systems, water treatment, HVAC.

- **The Generator Cars** — Power systems, can cause blackouts in specific zones
- **The Water Treatment Plant** — Control the water, control the train
- **The HVAC Nexus** — Temperature control for the whole train
- **The Comms Center** — Radio equipment, possibly contact the outside?
- **The Armory** — Jackboot weapons storage. Heist opportunity.
- **The Maintenance Tunnels** — Connecting passages between car undersides (stealth routes)
- **The Cold Lab** — Scientists studying the frozen world outside
- **The Drawbridge Cars** — Retractable sections, dramatic set-piece battles
- Boss: Chief Engineer — controls critical infrastructure

#### ZONE 5: FIRST CLASS (Cars 63–82)
Opulence. Sushi bars, aquariums, saunas, tailors, nightclubs. Grotesque luxury while the Tail starves.

- **The Aquarium Car** — Living fish, glass walls, beautiful and obscene
- **The Sushi Bar** — Fresh fish in a frozen world. The contrast is the point.
- **The Sauna/Spa** — Relaxation. Can overhear First Class secrets.
- **The Tailor** — Disguise system. Dress as First Class to infiltrate.
- **The Ballroom** — Grand parties. Assassination opportunity or intelligence gathering.
- **The Art Gallery** — Stolen/preserved art. Pretentious patrons. Hidden passages.
- **The Steak House** — Real meat (how?). Dark secret about the source.
- **The Casino** — Gambling mini-game. Win resources or lose everything.
- **The Opera House** — Performance in progress. Stealth through the rafters.
- **The Penthouse Suites** — First Class living quarters. Each suite is a mini-dungeon.
- **The Boardroom** — First Class council. Political endgame begins.
- **Mr. Wilford's Private Cars** — His personal domain before the Engine.
- Boss: Head of Hospitality — controls First Class security, social manipulation fight

#### ZONE 6: THE SANCTUM (Cars 83–95)
Wilford's inner circle. Religious zealots, personal guard, experimental labs.

- **The Cathedral Car** — Full church dedicated to Engine worship
- **The Confession Booth** — Interrogation disguised as faith
- **The Eugenics Lab** — Breeding program for "ideal" passengers
- **The Cryogenics Bay** — Frozen people. Who are they? Why?
- **The Propaganda Studio** — Where all train messaging originates
- **The War Room** — Military command center for Jackboots
- **The Experimental Car** — Human testing. Kronole weaponization.
- **The Garden of Eden** — Wilford's private paradise. Forests inside a train car.
- **The Vault** — Train's historical records, original manifest, dark secrets
- **The Betrayal Car** — Major plot twist location. Ally becomes enemy (or vice versa).
- **The Gauntlet** — Wilford's elite guard, wave-based combat challenge
- Boss: The High Priest of the Engine — fanatical, uses Kronole-enhanced abilities

#### ZONE 7: THE ENGINE (Cars 96–103+)
The sacred Engine. The truth. The end.

- **The Antechamber** — Final preparation. Point of no return.
- **The Child Labor Car** — Children maintaining the Engine (from the film). Horrifying reveal.
- **The Observation Deck** — First clear view of the frozen world. Is it thawing?
- **The Memory Car** — Holographic records of the pre-freeze world
- **The Failsafe** — Self-destruct mechanism. Nuclear option ending.
- **The Engine Room** — The Eternal Engine itself. Final confrontation.
- **The Conductor's Cabin** — Wilford (or his successor). Final dialogue. Final choice.
- **The Front Door** — The sealed exit. To the outside. The ultimate choice.

*Additional cars (103–110+) can be secret/hidden cars accessible through special routes, adding replayability.*

---

## KEY GAME SYSTEMS

### 1. Combat System
- **Melee-focused** with limited ranged options (crossbows, improvised firearms)
- **Tight corridors** force positional combat — can't be surrounded if you control the width
- **Environmental kills** — steam vents, electrical panels, window breaches (sucks enemies into cold)
- **Weapon degradation** — weapons break, must be repaired or replaced
- **Fatigue system** — extended combat drains stamina, forces tactical retreats
- **Kronole combat mode** — temporary slow-motion perception, costs long-term health

### 2. Resource Management
- **Hunger** — affects stamina regen and combat effectiveness
- **Cold** — increases in damaged cars, hull breaches, outer maintenance sections
- **Morale** — party morale affects companion effectiveness and available dialogue
- **Medicine** — heals injuries, cures disease, extremely scarce in the Tail
- **Kronole** — powerful but addictive. Withdrawal penalties escalate.
- **Scrap** — crafting material for weapons and tools
- **Influence** — social currency earned through quests and choices

### 3. Faction Reputation System

| Faction | Description | Benefits |
|---|---|---|
| **Tailies** | Your people. Revolutionary. | Companions, safe houses, shared resources |
| **Third Class Union** | Organized labor. | Access to workshops, insider info |
| **The Jackboots** | Train security/military. | Can be infiltrated. Access to armory. |
| **The Bureaucracy** | Second Class administrators. | Forged documents, political access |
| **First Class Elite** | The wealthy. | Luxury resources, blackmail material |
| **The Order of the Engine** | Religious zealots. | Access to Sanctum, Engine lore |
| **The Kronole Network** | Drug runners spanning all classes. | Kronole supply, smuggling routes, intel |
| **The Thaw** | Secret group who believe outside is survivable. | Endgame faction, alternate ending path |

### 4. Moral Choice System
Every 3-5 cars, a **major moral decision** that permanently affects:
- Which companions survive
- Which factions are allied/hostile
- Which cars are accessible or locked
- Which of **5 endings** the player is tracking toward

### 5. Endings (Minimum 5)

1. **The New Wilford** — Take the Engine. Become the tyrant. The train survives.
2. **The Revolution** — Destroy the class system. Democracy on rails. Fragile hope.
3. **The Derailment** — Destroy the Engine. Everyone dies or faces the cold together.
4. **The Exodus** — Open the front door. Lead survivors into the thawing world.
5. **The Eternal Loop** — Discover the train is going nowhere. Choose to tell the truth or maintain the lie.
6. *(Secret) The Bridge** — Contact another train. Civilization isn't dead.

---

## TECHNICAL REQUIREMENTS

### Architecture
- **Car Streaming System** — Each car is a sublevel, streamed in/out as player progresses. Max 3 cars loaded at once (current + adjacent).
- **Save System** — Per-car state persistence. Every car remembers what happened in it.
- **Procedural Variation** — While cars are hand-designed, minor randomization (loot, NPC placement, environmental details) increases replayability.
- **Train Movement** — Constant subtle motion, sway, sound. The player should never forget they're on a moving train.

### Performance Targets
- 60fps on mid-range hardware
- Loading masked by door-opening animations between cars
- Each car: ~50m long, variable width (standard gauge to double-wide for special cars)

### Art Direction
- **Tail:** Dark, rust, condensation, blue-gray palette, harsh fluorescent flickers
- **Third Class:** Industrial yellow-green, functional, worn but maintained
- **Second Class:** Warm wood tones, soft lighting, aging comfort
- **First Class:** Gold, crystal, excessive color, art deco meets modern luxury
- **The Sanctum:** White marble, religious iconography, sterile and unsettling
- **The Engine:** Brass, copper, steam, mechanical cathedral aesthetic

---

## AGENT INSTRUCTIONS

### Phase 1: Foundation (Parallel)
- **Game Architect:** Set up UE5 project, implement car streaming/transition system, save system, core player controller
- **Narrative Designer:** Write full car manifest (all 100+ cars with descriptions, encounters, choices), faction lore bible, main quest outline, all 5+ endings
- **Combat Designer:** Prototype melee combat in a test car — attacks, blocks, dodges, environmental interactions
- **Systems Designer:** Implement hunger/cold/morale/inventory systems, crafting blueprint

### Phase 2: Vertical Slice (Cars 1–15)
- Build the complete Tail experience as proof of concept
- All systems functional: combat, resources, crafting, dialogue, moral choices
- First boss fight (Car 15) polished
- Art direction established for Tail zone

### Phase 3: Full Production
- Build remaining zones in parallel (one agent team per zone)
- Integration checkpoints every 15 cars
- Faction reputation system online
- Companion system implemented

### Phase 4: Polish & Endings
- All 5+ endings implemented and tested
- Full playthrough testing (estimated 20-30 hour campaign)
- Performance optimization
- Steam integration, achievements, controller support

---

## REFERENCE MATERIAL TO STUDY

Agents should familiarize themselves with:
1. **Film (2013):** Axe fight in the dark, protein block reveal, Namgoong's Kronole explosives, the classroom car, the sushi bar, the Engine room, Curtis's speech about eating babies
2. **TV Series:** The Drawers (suspended animation punishment), the Night Car, the cattle car, Big Alice (second train), Layton's detective arc, the breachmen, Mr. Wilford's manipulation
3. **Graphic Novel:** The original bleaker tone, the religious mythology around the Engine, the concept of the "Explorers" who go outside
4. **Common Themes:** Class warfare, the trolley problem at scale, whether the system can be reformed or must be destroyed, the cost of revolution, what makes someone human

---

## SUCCESS CRITERIA

The game is complete when:
- [ ] 100+ unique, hand-designed cars are playable
- [ ] All 7 zones have distinct visual/gameplay identity
- [ ] Player can complete the game in 5+ meaningfully different ways
- [ ] Combat feels weighty and desperate, not power-fantasy
- [ ] Resource scarcity creates genuine tension
- [ ] Moral choices have visible, permanent consequences
- [ ] The train feels alive — NPCs have schedules, factions react, the world changes
- [ ] A player in the Tail should feel claustrophobic and desperate; a player in First Class should feel disgusted by the luxury
- [ ] The Engine reveal is earned and devastating

---

*Mayor: This is a massive project. Prioritize the vertical slice (Zone 1) to validate all systems before scaling. Assign agents to workstreams, establish communication protocols, and set integration milestones. The train waits for no one.*
