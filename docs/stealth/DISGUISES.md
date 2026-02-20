# Disguise System

## Overview

The disguise system allows the player to blend into different train zones by wearing
zone-appropriate uniforms. Snowpiercer is a rigidly stratified society — everyone
belongs somewhere, and being out of place gets noticed. Disguises shift the question
from "can the NPC see me?" to "does the NPC think I belong here?"

---

## Zone Uniforms

Each zone has expected attire. Wearing the wrong clothes is itself a detection trigger.

| Zone | Expected Attire | Where to Obtain | Notes |
|------|----------------|-----------------|-------|
| Tail | Rags, layered scraps | Starting gear | Tail residents ignore you |
| Third Class | Worker jumpsuit | Laundry room, unconscious workers | Basic labor uniform |
| Second Class | Service uniform | Staff quarters, locker rooms | Wait staff, technicians |
| First Class | Formal attire | First Class cabins, tailor | Suits, dresses |
| Engine Section | Engineer coveralls | Engineering bay | Requires tool belt prop |
| Security/Jackboot | Jackboot armor + helmet | Armory, defeated Jackboots | Helmet hides face |
| Medical | Lab coat + badge | Medical car | Badge is zone-specific |

### Obtaining Disguises

- **Loot**: Take from unconscious/dead NPCs (takes 5s, can be interrupted)
- **Find**: Located in specific containers (lockers, dressers, laundry)
- **Craft/modify**: Some disguises can be improved with the crafting system
- **Buy/trade**: Available from certain friendly NPCs

---

## Disguise Effectiveness

### Base Effectiveness

Disguises have a base effectiveness rating that determines how much they reduce
detection. This is NOT binary — even a good disguise can fail under scrutiny.

| Disguise Quality | Detection Rate Modifier | Notes |
|-----------------|------------------------|-------|
| Perfect (full uniform + props) | 0.1x | Near-invisible to casual NPCs |
| Good (correct uniform) | 0.3x | Passes casual inspection |
| Partial (some correct items) | 0.6x | Obviously something is off |
| Wrong zone attire | 1.5x | Sticks out, draws attention |
| No disguise (Tail rags in upper zones) | 2.0x | Immediately suspicious |

### Zone Familiarity

NPCs are more perceptive about their own zone:

- **Home zone NPCs**: +50% detection effectiveness against disguises
  (they know who belongs and who doesn't)
- **Visiting NPCs**: Standard detection (less familiar with zone regulars)
- **Security/Jackboots**: +100% detection against all disguises (trained observers)

### Disguise Degradation

Disguises degrade over time and through actions:

| Event | Degradation | Notes |
|-------|------------|-------|
| Combat (any) | Immediate downgrade to "Partial" | Blood, torn clothing |
| Running/sprinting | 2% per 10s | Out of character for most zones |
| Climbing/crawling | 5% per event | Dirties the uniform |
| Getting wet | 10% | Stains, discoloration |
| Time (passive) | 1% per 5 minutes | Gradual wear |
| Player injury (visible) | 15% | Blood, limping draws attention |

When degradation exceeds 50%, the disguise drops from its current quality tier
to the next lower one.

---

## NPC Challenges

When an NPC becomes SUSPICIOUS of a disguised player, they may issue a **challenge**
instead of immediately escalating. This is a confrontation where the NPC questions
whether the player belongs.

### Challenge Triggers

- NPC enters SUSPICIOUS state while player is disguised
- Player lingers in a restricted sub-area too long (30s)
- Player performs an action inconsistent with the disguise role
  (e.g., a "waiter" opening a security panel)
- Player is recognized by an NPC who has seen their real face

### Challenge Flow

```
NPC initiates challenge
│
├── Player has correct verbal response (dialogue skill check)
│   └── NPC returns to UNAWARE. Player gains 30s grace period.
│
├── Player fails verbal response
│   └── NPC escalates to ALERTED. Disguise flagged for this NPC.
│
├── Player flees
│   └── NPC immediately enters COMBAT.
│
└── Player attacks the challenger
    └── COMBAT. Adjacent NPCs alerted.
```

### Verbal Response Skill Checks

Challenges are resolved through dialogue with stat checks:

| Disguise Zone | Primary Check | Secondary Check | Difficulty |
|--------------|---------------|-----------------|------------|
| Third Class | Cunning (DC 8) | Strength (DC 10) | Easy |
| Second Class | Charisma (DC 12) | Cunning (DC 14) | Medium |
| First Class | Charisma (DC 16) | Perception (DC 18) | Hard |
| Engineering | Engineer (DC 14) | Cunning (DC 16) | Medium-Hard |
| Security | Strength (DC 16) | Cunning (DC 18) | Hard |
| Medical | Engineer (DC 15) | Charisma (DC 17) | Medium-Hard |

**Primary check**: The natural response for that zone (e.g., talking like an
engineer in Engineering).
**Secondary check**: An alternative approach (e.g., being cunning enough to
deflect the question).

### Challenge Frequency

- First challenge in a zone: after 60s of presence
- Subsequent challenges: every 120s (NPCs become more suspicious over time)
- After passing 3 challenges with the same NPC: that NPC stops challenging
  (they "know" you now — but only for this zone visit)
- Challenge cooldown resets when the player leaves and re-enters the zone

---

## Failure Conditions

A disguise is **blown** when:

1. Player fails a challenge check and the NPC escalates
2. Player is seen performing a violent action by any NPC
3. A blown NPC communicates your description via radio (all Jackboots in the
   car now know your face — disguise ineffective against them)
4. Player enters a checkpoint that requires ID/biometrics the disguise doesn't cover

### Blown Disguise Consequences

- The specific disguise type is **burned for the current car**. Wearing the same
  uniform in that car gives no benefit until the alert level resets.
- NPCs who personally witnessed the blow have your face flagged — they recognize
  you regardless of future disguises (persistent per zone visit).
- Changing to a DIFFERENT disguise type resets detection for NPCs who haven't
  personally seen you.

### Recovery

- **Leave the car**: Alert levels are per-car. Moving to a different car gives
  a fresh start (unless radio propagation spread your description).
- **Wait for reset**: Car alert levels decay over 5 minutes of no incidents.
- **Disable communications**: Taking out the car's intercom prevents radio
  propagation, containing the blown disguise to the immediate area.

---

## Special Disguise Mechanics

### Jackboot Helmet

The Jackboot helmet hides the player's face entirely. This makes it the most
powerful disguise but also the hardest to maintain:

- Prevents face recognition entirely (no NPC can flag your face)
- Jackboots expect military bearing — sprinting is fine but crouching is suspicious
- Other Jackboots may issue squad-based challenges ("What's your unit?")
  with Cunning DC 18
- If the helmet is removed or knocked off in combat, all Jackboots in
  visual range immediately enter COMBAT

### Crowd Blending

In populated cars (dining, club, market), the player can blend into crowds
while disguised:

- Requires a disguise matching the zone (minimum "Good" quality)
- Player's detection rate is reduced by an additional 0.5x while within
  3m of 3+ NPCs
- Moving through a crowd at walk speed is effectively invisible to
  non-Security NPCs
- Running or performing conspicuous actions breaks the blend immediately

### Zone-Specific Props

Some zones require more than just the right clothes:

| Zone | Required Prop | Effect Without |
|------|--------------|----------------|
| Engineering | Tool belt | Disguise capped at "Partial" |
| Medical | ID badge | Challenged within 15s |
| Security | Radio earpiece | Other Jackboots notice immediately |
| Kitchen | Apron + tray | Can carry items without suspicion |

Props are found in the same locations as uniforms or can be crafted.
