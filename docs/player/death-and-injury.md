# Death, Respawn & Injury System

## Save System: Checkpoint + Manual Save Hybrid

### Checkpoints (Automatic)
- **Auto-save on car transition** — Every time you pass through a connecting door
- **Auto-save at quest milestones** — Key story beats and quest stage completions
- **Auto-save at rest points** — Braziers, beds, safe rooms
- **3 rotating auto-save slots** — Can roll back to previous checkpoint if current state is undesirable

### Manual Save
- **Save anywhere** outside of combat and scripted sequences
- **Limited manual save slots: 10** — Forces some discipline, prevents save-scumming every 30 seconds
- **Save is instantaneous** — No "please wait" screens
- **Cannot save during:** Active combat, scripted events, environmental hazard sequences (hull breach, gas leak), boss fights

### Save-on-Quit
- Quitting mid-play creates a resume save that loads once and deletes itself
- Prevents progress loss from crashes or interruptions

### Difficulty-Based Save Restrictions

| Difficulty | Auto-Saves | Manual Saves |
|---|---|---|
| **Passenger** (Easy) | All checkpoints + frequent mid-car | Unlimited |
| **Tailie** (Normal) | Car transitions + quest milestones | 10 slots |
| **Breachman** (Hard) | Car transitions only | 5 slots |
| **Eternal Engine** (Permadeath) | Car transitions only | 0 — auto-save only, death is permanent |

## Death

### What Kills You
- **HP reaches 0** from combat damage, environmental hazards, or traps
- **Cold meter maxes out** — Hypothermia death in hull breach/exterior sections
- **Scripted deaths** — Rare, clearly telegraphed fail-states in boss encounters or timed sequences

### Death Screen
- Screen fades to white (not black — the cold takes you)
- Brief text: a line from a Tail elder's saying, randomized from a pool of ~20
- Options: **Load Last Checkpoint** / **Load Save** / **Main Menu**
- No death counter or shame mechanics. The train is hard enough.

### Death Consequences (on reload)
- **Enemy positions reset** in the current car only
- **Consumables used since last save are restored** (no potion chugging penalty)
- **Quest state preserved** up to the save point
- **NPC dialogue doesn't repeat** — if you've heard it, it stays heard

## Injury System

Injuries are a middle state between "fine" and "dead." They add texture to survival without being punishing enough to feel unfair.

### Injury Tiers

| Tier | Name | How Acquired | Duration | Effect |
|---|---|---|---|---|
| **0** | Healthy | Default state | — | No effects |
| **1** | Scratched | Minor combat hits, small falls | Heals naturally in 5 min | Cosmetic blood. No mechanical effect. |
| **2** | Wounded | Significant combat damage, medium falls, traps | Requires bandage or 30 min natural heal | -10% max stamina, slight limp animation |
| **3** | Badly Hurt | Heavy combat damage, explosions, boss attacks | Requires medicine | -25% max stamina, -15% movement speed, limited sprint |
| **4** | Critical | Near-death events, lethal damage survived via perks/skills | Requires surgery (rest point + medicine + time) | -50% max stamina, -30% movement speed, no sprint, screen distortion |

### Injury Accumulation
- Injuries **stack** — you can be Wounded (arm) AND Scratched (leg) simultaneously
- Each body region tracks independently: **head**, **torso**, **arms**, **legs**
- Region-specific effects:
  - **Head:** Visual distortion, impaired PER checks
  - **Torso:** Reduced max HP
  - **Arms:** Reduced melee damage, slower crafting
  - **Legs:** Reduced movement speed, impaired climbing

### Injury Treatment

| Treatment | Heals Up To | Availability | Cost |
|---|---|---|---|
| **Natural healing** | Tier 1-2 | Always (time) | Free (5-30 min) |
| **Bandage** | Tier 2 | Craftable, lootable | 1 cloth scrap |
| **Medicine** | Tier 3 | Rare loot, vendors | Scarce resource |
| **Surgery** | Tier 4 | Rest points only | Medicine + 1 hour rest + SRV 8 (self) or ally with medical skill |
| **Kronole** | All tiers (temporary) | Kronole supply | Masks injury effects for 10 min. Injury remains. Withdrawal makes it worse. |

### Injury Prevention
- **END stat** reduces injury severity (high END downgrades incoming injuries by 1 tier)
- **Armor** prevents injuries from minor hits entirely
- **Blocking/dodging** avoids injury-causing hits
- **Perks** (Iron Skin, Unbreakable) provide injury resistance

## Cold Death

Cold is the train's ever-present threat. It's not just an environmental hazard — it's the world trying to kill everyone, always.

### Cold Exposure

**Where cold is dangerous:**
- Hull breach cars (Car 11, various later cars)
- Exterior sections (maintenance, rooftop traversal, between-car gaps)
- Damaged/unheated cars (varies by train state and player actions)
- Deliberate cold traps (enemies flooding areas with outside air)

### Cold Meter

- **100-point meter** (hidden by default, appears when cold exposure begins)
- **Fills based on:** Exposure intensity × (1 - cold resistance)
- **Drains when:** In heated areas, near braziers/heat sources, using warming items

| Cold Level | Threshold | Effect |
|---|---|---|
| **Comfortable** | 0-20 | No effects |
| **Chilled** | 21-40 | Slight screen frost at edges. Stamina regen -20% |
| **Cold** | 41-60 | Visible breath. Stamina regen -50%. Movement speed -10%. Melee damage -10% |
| **Freezing** | 61-80 | Heavy screen frost. Stamina regen stopped. Movement -25%. Can't sprint. Hands shake (reduced aim/lockpick) |
| **Hypothermia** | 81-99 | Screen nearly whited out. Movement crawl speed only. HP drains 2%/sec. 30 seconds to find warmth. |
| **Frozen** | 100 | Death. Screen whites out. |

### Cold Resistance Sources
- **END stat** — Each point above 5 adds 8 seconds of cold tolerance
- **Clothing** — Insulated gear reduces cold meter fill rate (up to -60%)
- **Survival skills** — Thermal Mastery, Hardy, Frostblood perk
- **Consumables** — Alcohol (temporary warmth, impairs aim), hot food, warming chemicals
- **Environmental** — Stand near heat sources to drain cold meter

### Cold Death Specifics
- Cold death triggers at meter 100 — no save, no second chance (unless Cockroach skill or Frostblood perk)
- Cold death has its own animation: the player slows, stumbles, and collapses
- On reload, cold meter resets to 0
- Some cold sequences are timed puzzles — find the path through before you freeze

**Design intent:** Cold should feel relentless and terrifying. The player should dread hull breaches. When they reach First Class and feel warm air for the first time, it should feel like an oasis — then they should feel guilty, because the Tail never has this.
