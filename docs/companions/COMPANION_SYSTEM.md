# Companion System Design

## Overview

Companions are recruitable NPCs who fight alongside the player, provide narrative depth, and serve as the emotional anchor of the Snowpiercer: Eternal Engine experience. Each companion has a unique backstory, combat style, personal quest chain, and loyalty arc that responds to player choices.

## Party System

### Party Size

- **Maximum active party:** 2 companions + player = 3 total
- **Why 3:** Train corridors are 3.2m wide. Three combatants abreast is the physical maximum without blocking each other. Larger parties would trivialize the tight-corridor combat design.
- **Roster:** All recruited companions persist in the **Companion Roster** even when not in the active party
- **Swap points:** Companions can be swapped at rest points (braziers, safe rooms, beds) — same locations where perks are swapped
- **Emergency swap:** If a companion is downed in combat, you cannot swap mid-fight. You fight short-handed until you reach a rest point.

### Roster Management

- Recruited companions not in the active party wait at the **last visited rest point**
- Companions waiting at rest points may have idle dialogue that changes based on story progress
- If a rest point is in a zone the player has moved past, companions auto-relocate to the nearest accessible rest point ahead of the player
- Maximum roster size: 12 (one per companion in the game)

## Companion AI Behaviors

Companions operate on a **behavior mode** system with four settings the player can toggle at any time via a quick menu (not a pause menu — real-time toggle):

### Aggressive

- **Combat:** Companion rushes enemies, prioritizes damage output over self-preservation
- **Positioning:** Pushes to the front of the formation
- **Target priority:** Nearest enemy, then lowest HP enemy
- **Risk:** Takes more damage, may overextend in corridors
- **Stamina management:** Burns stamina aggressively, may fatigue faster
- **Best for:** Overwhelming small groups, boss DPS phases

### Defensive

- **Combat:** Companion prioritizes blocking and counter-attacks
- **Positioning:** Stays beside or slightly behind the player
- **Target priority:** Enemies targeting the player first
- **Risk:** Lower damage output, fights last longer
- **Stamina management:** Conservative, maintains stamina reserve for blocks
- **Best for:** Protecting low-HP player, sustained fights, unknown encounters

### Passive

- **Combat:** Companion does not initiate combat. Only fights back if directly attacked.
- **Positioning:** Stays behind the player
- **Target priority:** Only enemies actively hitting them
- **Risk:** Companion may take hits without retaliating until threshold
- **Stamina management:** Minimal expenditure
- **Best for:** Stealth sections, social encounters where violence would cause reputation loss

### Hold Position

- **Combat:** Companion stays at their current location and fights anything that approaches
- **Positioning:** Locked to the spot where the command was given
- **Target priority:** Anything within melee range
- **Risk:** Can be flanked if enemies bypass them
- **Stamina management:** Normal
- **Best for:** Holding chokepoints, guarding retreats, corridor control

### Corridor-Specific AI Rules

Train corridors impose special constraints on companion AI:

1. **Single-file mode:** In corridors narrower than 2m, companions automatically fall into single-file behind the player. Only the front combatant can engage in melee.
2. **Swap mechanic:** Player can command "swap" to rotate formation order. The rear companion moves to front. 2-second animation during which neither companion attacks.
3. **Ranged override:** Companions with ranged abilities can attack over the player's shoulder in single-file (reduced accuracy, friendly fire possible on missed shots).
4. **Doorway behavior:** When entering a new car through a door, companions stack on the near side of the door until the player advances 3m into the new car, then follow.
5. **Environmental awareness:** Companions will not walk into known hazards (steam vents, electrical panels) unless on Aggressive and in pursuit.

## Loyalty & Relationship System

### Overview

Each companion has a **Loyalty Meter** that tracks their relationship with the player. This is a **hidden system** — the player never sees a numerical value. Instead, loyalty manifests through observable behavioral changes.

### Loyalty Scale

Internal range: **-100 to +100**

| Range | Internal State | Observable Behavior |
|-------|---------------|---------------------|
| -100 to -60 | **Hostile** | Companion abandons the party. May become an enemy later. |
| -59 to -30 | **Resentful** | Snide dialogue, refuses some commands, reduced combat effectiveness (-20% damage) |
| -29 to 0 | **Cold** | Minimal dialogue, no personal stories shared, follows orders mechanically |
| 1 to 30 | **Neutral** | Standard companion behavior, occasional banter |
| 31 to 60 | **Friendly** | Shares personal stories, offers advice, combat synergy bonus (+10% when near player) |
| 61 to 85 | **Devoted** | Unique dialogue, reveals secrets, unlocks personal quest line, +15% combat synergy |
| 86 to 100 | **Bonded** | Maximum loyalty. Unique combat ability unlocked. Will sacrifice themselves for the player if permadeath triggers. |

### Loyalty-Affecting Events

**Major positive events (+10 to +25):**
- Completing a companion's personal quest step
- Making a story choice that aligns with their values
- Saving their life in combat (using a healing item on them when they're critical)
- Siding with them in a companion conflict
- Giving them a gift item that matches their interests

**Minor positive events (+2 to +8):**
- Choosing dialogue options they approve of
- Winning fights without them taking excessive damage
- Sharing food/resources when supplies are scarce
- Asking about their past (if loyalty is high enough for them to share)

**Major negative events (-10 to -25):**
- Making story choices that violate their core values
- Letting innocents die when they could have been saved (for moral companions)
- Killing unnecessarily (for pacifist companions)
- Siding against them in a companion conflict
- Using Kronole excessively (some companions disapprove)

**Minor negative events (-2 to -8):**
- Ignoring their dialogue or cutting them off
- Looting corpses excessively (some companions disapprove)
- Choosing violent solutions when peaceful ones exist (for diplomatic companions)
- Leaving them injured without healing when resources are available

### Loyalty Decay & Maintenance

- Loyalty does not decay naturally — it only changes from events
- Exception: If a companion witnesses something deeply against their values and the player doesn't address it within 24 in-game hours, loyalty decays by -5 per day until addressed
- The **Natural Leader** perk (CHA 12) increases all positive loyalty gains by 20% and reduces negative hits by 20%

## Companion Permadeath

### Design Philosophy

Companion death is permanent and meaningful. There are no resurrections, no magical recoveries. The train is lethal, and companions are mortal.

### Permadeath Triggers

A companion dies permanently when:

1. **HP reaches zero in combat and is not revived within 30 seconds.** The player can attempt a revive action (10 seconds, interruptible) to bring them to 10% HP. If the player doesn't revive them in time, they die.
2. **Story choice kills them.** Certain narrative branches result in companion death regardless of player combat skill. These are always clearly telegraphed (the companion will voice their fear, the situation will feel like a trap).
3. **Loyalty drops to -100 and they leave, then the player encounters them as an enemy.** If the player kills them in the resulting confrontation, they're gone forever. If the player spares them, they vanish from the game.
4. **Sacrifice trigger at Bonded loyalty.** In certain critical moments, a Bonded companion may choose to sacrifice themselves to save the player. The player gets a brief window (3 seconds) to stop them, but stopping them means accepting the consequence they were trying to prevent.
5. **Environmental death.** Companions can die from environmental hazards (hull breach cold, explosions, train derailment sequences) if the player doesn't protect them or warn them in time.

### Consequences of Permadeath

- The companion's personal quest line becomes permanently unavailable
- NPCs throughout the train will reference the companion's death
- Other companions react to the death (dialogue, loyalty shifts)
- The companion's unique combat ability is lost forever
- An empty slot opens in the roster (some companions are only recruitable if another has died)

### The Memorial

At the next rest point after a companion dies, the player finds a **memorial interaction** — a brief, non-interactive moment where the remaining party acknowledges the loss. This is a 15-second cutscene specific to each companion. It plays once and cannot be replayed.

## Companion Conflicts

### When Companions Disagree

At certain story junctions, two companions in your active party may disagree about the correct course of action. This creates a **Companion Conflict** — a structured argument where:

1. Both companions state their position (dialogue)
2. The player must choose whose side to take (or attempt a compromise)
3. The chosen path proceeds; the rejected companion takes a loyalty hit

### Conflict Types

**Moral conflicts:** "Should we share our supplies with these strangers?" One companion says yes (moral), the other says no (pragmatic).

**Strategic conflicts:** "Do we fight through or sneak past?" One companion favors combat, the other stealth. The player's choice determines the encounter approach.

**Personal conflicts:** Two companions have history or conflicting values. "I don't trust them. It's them or me." These are the most severe — the losing companion takes a -15 loyalty hit.

### Compromise Option

If the player's CHA is 10+, a compromise dialogue option appears in some conflicts. This reduces the loyalty hit to the "losing" companion from full to half. Not all conflicts can be compromised — some are binary by design.

### The **Oathkeeper** Perk

The Oathkeeper perk (complete 5 companion quests) prevents companions from ever abandoning the player, regardless of loyalty. However, a companion at -100 loyalty with Oathkeeper active becomes **Bitter** — they stay but are mechanically crippled: -30% combat effectiveness, no banter, no personal quest progress. This is intentionally worse than losing them in some ways, to make the perk a double-edged sword.

## Romance System

### Decision: Romance is Present, but Grounded

Romance exists in the companion system, but it is **not the focus**. The Snowpiercer setting is brutal — romance here is about connection in extremity, not wish fulfillment.

### Rules

1. **4 romanceable companions** out of 12 (2 male-presenting, 2 female-presenting). All romances are available regardless of player gender.
2. **Romance is not a separate loyalty track.** It emerges naturally from high loyalty (Devoted+) combined with specific dialogue choices. There is no "romance meter."
3. **No sex scenes.** Intimacy is implied through a fade-to-black and a unique morning-after dialogue. The game earns its M rating through violence, not sex.
4. **Romance has narrative consequences.** A romanced companion who dies triggers unique grief dialogue from the player character (the one exception to the silent protagonist — a single voiced line). A romanced companion at low loyalty triggers a breakup scene.
5. **Only one active romance.** Attempting to romance a second companion while the first romance is active triggers a unique conflict scene. The player must choose.
6. **Romance does not make companions stronger.** No mechanical bonuses for romance. This is a narrative-only system. The reward is emotional investment, not stat boosts.
7. **Romance can end.** If loyalty drops below Friendly (30) while in a romance, the romance ends permanently. There is no re-romancing a companion.

## Integration with Existing Systems

### Combat Integration
- Companions use the same `UCombatComponent` system as the player
- Companion AI controllers extend `UCombatAIController` with behavior mode support
- Companion weapons degrade the same way player weapons do
- Companions have the same stamina/fatigue system

### Dialogue Integration
- Companions use the `UDialogueComponent` for barks and conversations
- Companion dialogue is tracked by `UNPCMemoryComponent` — they remember what they've said
- Loyalty-gated dialogue uses the loyalty system to determine available lines

### Stat Integration
- Companions have fixed stats (they don't level up — the player does)
- Companion stats follow the same 1-20 scale as the player
- Companion-specific abilities are tied to unique perks, not stats
