# Car 03: The Pit

**Type:** Tutorial — Combat (Unarmed Fighting)
**Difficulty:** Easy (guided combat)
**Estimated Play Time:** 15-20 minutes

## Narrative Context

An open area where Tailies settle disputes and fight for extra rations. The Jackboots allow it — even encourage it — because it keeps the Tail's aggression directed inward. A crude fighting ring made from rope and crates dominates the center. The current champion is Kez, a scarred woman who fights with brutal efficiency.

## Layout

```
[DOOR ← Car 02]
|                                    |
|  [Spectator benches — crate seats]|  Section A: Entrance/Viewing (8m)
|  [Betting table — the bookmaker]  |
|                                    |
|======= Rope ring boundary ========|
|                                    |
|                                    |  Section B: THE PIT (20m)
|        [Fighting Ring]             |
|   [Sand/grit floor covering]      |
|   [4 corner posts w/ rope]        |
|                                    |
|                                    |
|======= Rope ring boundary ========|
|                                    |
|  [Kez's corner — gear, wraps]    |  Section C: Fighter's Area (12m)
|  [Water bucket] [Rags]            |
|  [Trophy shelf — taken items]     |
|                                    |
|-------- Open passage -------------|
|                                    |
|  [Recovery area — crude mats]     |  Section D: Exit/Recovery (10m)
|  [First aid — bandage wraps]      |
|                                    |
[DOOR → Car 04]
```

## Environmental Details

### Section A: Entrance/Viewing
- Crate seats arranged in tiers along both walls. Standing room in between.
- A bookmaker runs a betting operation from a plank-and-crate table. Bets in protein blocks and favors.
- Crowd is excited, hostile, hungry. This is entertainment in the Tail.

### Section B: The Pit
- The ring is ~6m x 6m, marked by rope strung between pipe-posts welded to the floor.
- Floor is covered in sand/grit for traction and blood absorption.
- Overhead, a single strong work light illuminates the ring — everything outside is shadow.
- Bloodstains in the sand. Some old, some fresh.

### Section C: Fighter's Area
- Kez has a personal corner with hand wraps, a water bucket, and a shelf of "trophies" — items taken from defeated opponents.
- This area has slightly more space — fighters get respect.
- A crude mirror (polished metal sheet) mounted on the wall.

### Section D: Recovery
- Mats on the floor for fighters who lost badly.
- Basic first aid: cloth bandages, a bottle of improvised antiseptic (Kronole derivative).
- This is also where the player rests after the tutorial fight.

## Gameplay

### Combat Tutorial Beats

1. **Enter the Pit** — Crowd is watching a fight in progress. Two NPCs grappling. One goes down hard. The crowd roars.
2. **Meet Kez** — She notices you watching. "You want extra rations? You fight for them." Dialogue: accept or watch first.
3. **Watch First (Optional)** — Kez fights another Tailie. Player observes combat mechanics demonstrated: light attack, heavy attack, block, dodge. UI callouts appear.
4. **Your First Fight** — Player enters the ring against a weak opponent. Guided combat:
   - Round 1: "Attack him!" — Teaches light attack (jab, jab, cross combo)
   - Round 2: "Block when he swings!" — Teaches block/parry timing
   - Round 3: "Move! Don't stand there!" — Teaches dodge/sidestep
   - Opponent goes down after ~30 seconds of guided combat.
5. **Kez Challenges You** — Optional harder fight. She's fast, hits hard. Tests all three mechanics. Winnable but difficult. Losing is okay — no penalty, different dialogue.
   - **Win:** Kez respects you. Recruitable as companion later (Car 5 quest). Bonus: hand wraps (minor melee damage boost).
   - **Lose:** Kez still respects the attempt. Recruitable later but requires an additional favor quest.
6. **Claim Reward** — Extra protein block for the tutorial fight. If you beat Kez: her hand wraps + 2 protein blocks.

### Advanced Combat Elements (Foreshadowed)
- **Environmental awareness:** Ropes can be used to corner opponents (not interactive yet, but Kez does it to the player).
- **Stamina system:** After extended fighting, attacks slow. Teaches resource management in combat.
- **Crowd influence:** If player fights stylishly (varied combos), crowd cheers louder. Foreshadows morale system.

### Discoverable Content
- **The Bookmaker** — Can bet 1 protein block on the NPC fights. Win/lose is randomized. Introduces risk/reward.
- **Kez's trophies** — Examining them reveals stories of previous fighters. Some names appear on Car 1's memorial wall.
- **Recovery area NPC** — A fighter with a broken arm. Dialogue about how the Jackboots started the Pit to keep Tailies from uniting.

## Items

| Item | Location | Type |
|---|---|---|
| Protein Block (x1) | Fight reward | Food |
| Hand Wraps | Beat Kez reward | Accessory (+5% melee dmg) |
| Protein Block (x2) | Beat Kez reward | Food |
| Cloth Bandage (x2) | Recovery area | Medicine (minor heal) |
| Betting Token | Bookmaker (if you win a bet) | Currency/lore |

## NPCs

| Name | Location | Role |
|---|---|---|
| **Kez** | Section C / Ring | Pit champion. Potential companion. Combat tutorial partner. |
| The Bookmaker | Section A | Betting NPC. Flavor character. |
| Tutorial Opponent | Ring | Weak fighter for first bout. |
| Recovery Fighter | Section D | Lore NPC. Exposition about the Pit's origins. |
| ~15 ambient Tailies | Spectator area | Crowd. React to fights with cheers/boos. |

## Lighting

- **Ring:** Single bright overhead work light. Harsh white. Creates a spotlight effect.
- **Spectator area:** Dim. Faces lit from below by the ring light. Dramatic shadows.
- **Fighter's area:** Moderate. A caged bulb on the wall.
- **Recovery:** Low. Comfortable darkness for the injured.

## Audio

- **Ambient:** Crowd noise (cheering, jeering, stomping feet), impact sounds (flesh on flesh), grunting
- **During fights:** Music tempo increases — percussion-heavy, primal drums
- **Between fights:** Murmuring, the bookmaker calling odds, water splashing in buckets
- **Combat SFX:** Meaty punches, whooshing dodges, blocking thuds. Weighty and visceral.

## Ring Mechanics (Technical)

The Pit ring functions as a combat arena with specific rules:
- **Boundary:** Player can't leave the ring during a fight (invisible wall at rope).
- **NPC AI:** Tutorial opponent uses telegraphed attacks with long wind-ups. Kez uses the full combat AI.
- **Crowd system:** ~15 ambient NPCs face the ring and react to combat events (cheering on hits, gasping on knockdowns).
- **Camera:** Slightly pulled back to show both fighters. More cinematic than standard gameplay camera.

## Revisit Behavior

- Kez is available for sparring (practice combat with no stakes)
- The Pit hosts periodic fights — can bet with the Bookmaker for resources
- If Kez is recruited as companion: a replacement champion appears
- During revolution prep: The Pit becomes a training ground for fighters
