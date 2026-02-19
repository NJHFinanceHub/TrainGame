# Non-Lethal Options

## Overview

The player can complete the game without killing anyone. This isn't just a
constraint — it's a distinct playstyle with its own tools, challenges, and
rewards. Non-lethal approaches are harder but often yield better narrative
outcomes: spared NPCs can become allies, provide intel, or show up later to
return the favor.

---

## Stealth Takedowns

### Mechanics

Stealth takedowns are available when the player is behind an NPC who is in
UNAWARE or SUSPICIOUS state. The takedown is a brief animation that
incapacitates the target without killing them.

| Parameter | Value | Notes |
|-----------|-------|-------|
| Approach requirement | Behind target, within 1.5m | Must not be in target's vision cone |
| Required state | UNAWARE or SUSPICIOUS | ALERTED+ NPCs cannot be taken down silently |
| Animation duration | 2.0s | Interruptible if another NPC spots you |
| Noise radius | 3m | Muffled struggle |
| Target state after | Unconscious (120s) | Wakes up and alerts if not restrained |
| Lethal option | Hold action key for 3.5s | Kills instead — player choice |

### Takedown Types

The takedown animation varies based on context:

| Context | Animation | Special Effect |
|---------|-----------|----------------|
| Standing (behind) | Chokehold | Standard takedown |
| Standing (from above) | Drop takedown | From ceiling vents, ledges |
| Seated | Chair takedown | NPCs at desks, dining |
| From hiding spot | Ambush grab | Pull target into locker/alcove |
| In water/puddle | Slip takedown | Target slips, quieter |

### Takedown Modifiers

| Factor | Effect |
|--------|--------|
| Cunning ≥ 12 | Takedown is 0.5s faster |
| Strength ≥ 14 | Can takedown Brute-type NPCs (normally immune) |
| Target is distracted | No approach direction requirement |
| Target is in conversation | Both NPCs must be taken down within 3s or the second alerts |
| Player is injured (< 30% HP) | Takedown takes 1s longer (struggling) |

---

## Stun Weapons

### Available Stun Weapons

| Weapon | Type | Stun Duration | Range | Ammo/Uses | Noise | Notes |
|--------|------|--------------|-------|-----------|-------|-------|
| Improvised taser | Melee | 60s | 1.5m | 5 charges | 4m | Crafted, rechargeable at power outlets |
| Chloroform rag | Melee | 180s | 1m | 3 uses | 2m | Requires 2s hold, very quiet |
| Tranq dart (crossbow) | Ranged | 120s | 15m | Per dart | 8m | Rare ammo, takes 5s to take effect |
| Flash grenade | Thrown | 10s (blind) | 5m AoE | Single use | 20m | Loud, affects all in radius including player |
| Sonic emitter | Placed | 15s (stun) | 4m AoE | 2 uses | 12m | Craftable, can be used as trap |
| Sleep gas canister | Thrown | 90s | 6m AoE | Single use | 8m | Rare, affects player if no gas mask |

### Stun vs Kill Trade-offs

| Aspect | Lethal | Non-Lethal |
|--------|--------|------------|
| Permanence | Target eliminated forever | Target wakes up (must restrain or move) |
| Speed | Instant (melee kill) | Slower (stun duration, restraint time) |
| Noise | Varies by weapon | Generally quieter |
| Resources | Weapon durability | Stun weapon charges/ammo (scarcer) |
| Body discovery | Dead body → instant COMBAT alert | Unconscious body → ALERTED only |
| Narrative | Closes NPC quest lines | Preserves NPC quest lines |
| Karma | Negative faction impact | Neutral or positive faction impact |

---

## Restraints

Unconscious NPCs wake up after their stun duration. To prevent this, the player
can restrain them.

| Restraint Type | Application Time | Escape Difficulty | Where Found |
|---------------|-----------------|-------------------|-------------|
| Zip ties | 3s | Hard (DC 16) | Security stations, crafted |
| Rope | 5s | Medium (DC 12) | Storage cars, maintenance |
| Duct tape | 4s | Medium-Hard (DC 14) | Workshop, maintenance |
| Handcuffs | 2s | Very Hard (DC 20) | Jackboots, security office |
| Cable/wire | 6s | Easy (DC 8) | Engineering, everywhere |

**Gagging**: Restraints alone don't prevent the NPC from yelling when they
wake. The player must also gag the NPC (+2s application time) to prevent
vocal alerts. An ungagged, restrained NPC will shout for help when they
wake (15m noise radius).

---

## Environmental Non-Lethal Options

### Distractions

| Distraction | Noise Radius | Duration | Effect |
|-------------|-------------|----------|--------|
| Thrown object (bottle, tool) | 8m | 5s | NPCs investigate sound source |
| Triggered alarm (pull station) | Entire car | 30s | All NPCs move to alarm location |
| Sabotaged equipment | 6m | 15s | NPCs investigate malfunction |
| Radio lure (stolen radio) | 10m | 10s | Mimics a call for help |
| Power outage (fuse box) | 0m | 45s | Lights go out in section, NPCs patrol with flashlights |
| Released steam vent | 5m | 20s | Visual + audio cover, NPCs avoid area |

### Environmental Incapacitation

| Method | Effect | Lethal? | Notes |
|--------|--------|---------|-------|
| Steam vent (overload) | 8s stun + knockback | No (unless sustained) | Short burst is non-lethal |
| Electrical panel (overload) | 10s stun | No (controlled) | Requires Engineer skill |
| Trapdoor/chute | Removes from area | No | NPC ends up in lower deck, confused |
| Lock in room | Contained indefinitely | No | Must find lockable room with NPC inside |
| Sleeping pills in food/drink | 300s unconscious | No | Requires kitchen access + Cunning check |

---

## Pacifist Route Viability

### Can You Complete the Game Without Killing Anyone?

**Yes**, but it requires:

1. **High investment in Cunning and Engineer stats**: Most non-lethal solutions
   require skill checks that combat players can ignore.

2. **Resource management**: Stun weapon charges and restraints are scarce. The
   player must plan routes and engagements carefully.

3. **Disguise mastery**: Avoiding combat entirely requires strong disguise play
   to move through hostile zones without engagement.

4. **Knowledge of patrol routes**: Non-lethal play demands understanding of NPC
   schedules and movement patterns to find windows of opportunity.

5. **Patience**: Non-lethal approaches take longer. Stealth takedowns are slower
   than kills. Restraining adds extra time. Distractions require setup.

### Pacifist Rewards

| Reward | Description |
|--------|-------------|
| "Ghost of the Eternal Engine" achievement | Complete the game with zero kills |
| Faction reputation bonuses | All factions respect mercy (+15% disposition) |
| Unique dialogue options | "I didn't kill anyone to get here" opens special paths |
| Alternate endings | Two endings are only accessible with zero/minimal kills |
| NPC callbacks | Spared NPCs appear later with help, intel, or gifts |
| Moral authority | Late-game persuasion checks are easier with a clean record |

### Hard Choices

Some encounters are designed to pressure the pacifist player:

- **Timed scenarios**: A non-lethal approach may take too long, forcing a moral
  choice between killing one guard or letting hostages die.
- **Boss encounters**: Major antagonists can be subdued non-lethally but require
  specific preparation (acquiring the right tools/intel beforehand).
- **Escort missions**: Protecting a vulnerable NPC while maintaining non-lethal
  play requires creative crowd control.
- **Betrayals**: An NPC you spared may betray you later. Do you spare them again?

These are design constraints, not bugs. The pacifist path should feel earned,
not trivial.

---

## Integration with Other Systems

### Combat System
- Stun weapons use the existing `WeaponComponent` with `EDamageType::NonLethal`.
- `CombatComponent::ReceiveAttack()` checks damage type and applies stun instead
  of damage for non-lethal hits.
- Takedowns use `EDamageType::StealthTakedown` with a non-lethal flag.

### Stealth System
- Non-lethal takedowns generate less noise than kills (3m vs 5m+).
- Unconscious bodies trigger ALERTED instead of COMBAT when found.
- Restrained NPCs trigger ALERTED but don't call for radio backup.

### Dialogue System
- Kill count is tracked and affects dialogue options throughout the game.
- NPCs in the same faction as spared enemies may have warmer dispositions.
- Pacifist dialogue options are marked with a dove icon in the dialogue UI.

### Player Stats
- **Cunning**: Core stat for non-lethal play (takedown speed, disguise checks).
- **Engineer**: Enables environmental non-lethal options (sabotage, traps).
- **Strength**: Required for takedowns on Brute-type enemies.
- **Charisma**: Talking your way out instead of fighting.
