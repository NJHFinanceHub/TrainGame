# Zone 1: The Tail — Level Design Overview

## Zone Identity

**Cars:** 1-15
**Theme:** Desperate survival, revolution's birthplace
**Tone:** Claustrophobic, dark, oppressive. Rare moments of warmth between people who have nothing.
**Palette:** Blue-gray steel, rust orange, condensation whites, harsh fluorescent flicker
**Lighting:** Minimal. Overhead fluorescents at 30-40% power, many broken. Emergency LEDs. Firelight from improvised braziers.
**Sound:** Constant train rhythm (thunk-thunk), distant engine hum, dripping water, coughing, whispered conversations.

## Physical Constants

All Tail cars share these baseline specifications:

| Property | Value |
|---|---|
| Car length | ~50m (interior playable space) |
| Car width | 3.2m (standard gauge interior) |
| Ceiling height | 2.6m (low, oppressive) |
| Floor material | Bare corrugated steel, some rubber matting |
| Wall material | Riveted steel panels, exposed conduit, insulation tears |
| Temperature | 8-12C (cold but survivable) |
| Air quality | Poor — recycled, humid, smells of bodies and protein processing |

## Architectural Language

### Recurring Elements
- **Makeshift partitions** — Hung blankets, scrap metal sheets, stacked crates creating semi-private zones
- **Overhead pipes** — Exposed plumbing and electrical conduit running along ceiling. Some leak. Some can be used as weapons.
- **Bunk stacks** — 4-high sleeping shelves bolted to walls. ~0.6m per bunk. Claustrophobic.
- **Braziers** — Improvised fire pits made from cut barrels. Gathering points. Light sources.
- **Condensation** — Every surface has a slight wet sheen. Drips from ceiling. Puddles on floor.
- **Graffiti** — Tally marks (days since last revolt), names of the dead, crude maps, revolutionary slogans

### Connecting Doors
Each car connects via heavy steel doors with manual wheel locks:
- **Standard door:** Takes 3 seconds to open (animation mask for level streaming)
- **Locked door:** Requires key, lockpick skill, or force (scripted)
- **Barricaded door:** Must clear debris / solve environmental puzzle
- **Sealed door (Car 15 → Zone 2):** Boss encounter gate

## Progression Structure

```
Car 1  [Tutorial: Movement/Interaction]
  ↓
Car 2  [Tutorial: Crafting]
  ↓
Car 3  [Tutorial: Combat]
  ↓
Car 4  [First Moral Choice]
  ↓
Car 5  [Quest Hub — Main quest begins]
  ↓
Car 6  [Resource Management intro]
  ↓
Car 7  [Weapon Crafting unlock]
  ↓
Car 8  [Lore/Intel gathering]
  ↓
Car 9  [First Real Combat — Blockade]
  ↓
Car 10 [Horror/Stealth — Dark Car]
  ↓
Car 11 [Environmental Hazard — Cold]
  ↓
Car 12 [Drug Choice — Kronole]
  ↓
Car 13 [Trading System unlock]
  ↓
Car 14 [Story Beat — History of revolts]
  ↓
Car 15 [BOSS — Jackboot Commander]
  → Zone 2: Third Class
```

## Estimated Play Time
- **First playthrough:** 2.5-3.5 hours
- **Speedrun (experienced):** 45-60 minutes
- **100% completion:** 4-5 hours

## NPC Population
Zone 1 contains approximately 80-100 NPCs total:
- ~60 ambient Tailies (non-interactive or single-line barks)
- ~15 named NPCs with dialogue trees
- ~10 Jackboot enemies (Cars 9, 15)
- ~5 potential companion candidates (2 recruitable in this zone)

## Resource Economy (Zone 1)
Resources are scarce by design. The player should feel desperate.

| Resource | Availability | Sources |
|---|---|---|
| Protein blocks | Common | Kitchen (Car 2), ration handouts |
| Scrap metal | Moderate | Workshop (Car 7), environmental |
| Medicine | Very rare | Sickbay (Car 6) — 2-3 doses total |
| Kronole | Rare | Den (Car 12) — 1-2 doses |
| Clean water | Moderate | Condensation collectors, pipes |
| Cloth/rags | Common | Bunks, clothing |
| Wire/cord | Moderate | Stripped from walls, pipes |

## Key Characters Introduced in Zone 1

| Name | Car | Role | Notes |
|---|---|---|---|
| **Old Pike** | 1 | Elder, mentor figure | Tutorial guide. Can die in Car 9 if player fails. |
| **Marta** | 2 | Kitchen boss | Controls protein distribution. Potential ally or antagonist. |
| **Kez** | 3 | Pit fighter champion | Recruitable companion. Melee specialist. |
| **Nani** | 4 | Nursery caretaker | Moral compass NPC. Judges player choices. |
| **Gilliam** | 5 | Tail Elder leader | Quest giver. Architect of the revolution. |
| **Dr. Asha** | 6 | Tail's only doctor | Medicine trader. Has secrets about missing children. |
| **Forge** | 7 | Blacksmith | Weapon crafting vendor. Former Third Class engineer. |
| **Whisper** | 8 | Intelligence gatherer | Provides lore and intel. Paranoid. |
| **Cmdr. Grey** | 9/15 | Jackboot Commander | Zone 1 antagonist. Appears at Blockade, boss at Threshold. |
| **Kronole Kim** | 12 | Drug dealer | First Kronole Network contact. |
| **The Trader** | 13 | Black market dealer | Trading system NPC. Morally ambiguous. |
