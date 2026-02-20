# Stealth System Architecture

## Overview

The stealth system governs how NPCs perceive the player through sight and sound,
how detection escalates through discrete alert states, and how the player can
exploit hiding spots, environmental masking, and body concealment to avoid or
de-escalate detection.

Snowpiercer's narrow corridors create unique stealth constraints: sightlines are
long but narrow, sound propagates through metal walls, and there are limited places
to hide. Stealth here is about timing, disguises, and exploiting the train's
mechanical noise — not skulking through open fields.

---

## Sight Detection

### Vision Cones

Every NPC has a vision cone defined by two parameters:

| Parameter | Default | Description |
|-----------|---------|-------------|
| Inner Cone (half-angle) | 30° | Full perception — instant detection at close range |
| Outer Cone (half-angle) | 60° | Peripheral — slower detection, easily lost |
| Max Range | 25m | Beyond this, NPC cannot see the player at all |
| Close Range | 3m | Within this range, always detected regardless of cone |

**Corridor modifier**: In narrow corridors (< 4m width), the effective cone is
clamped to the corridor walls. An NPC facing down a corridor sees everything in
it, but has zero peripheral coverage of branching passages.

### Detection Speed by Distance

Detection is not instant (except at Close Range). The NPC fills a **sight
detection meter** based on distance:

| Distance Band | Fill Rate (per second) | Notes |
|---------------|----------------------|-------|
| Close (0–3m) | Instant (100%) | Face-to-face, no hiding |
| Near (3–8m) | 40% | Quick detection |
| Medium (8–15m) | 20% | Standard corridor engagement |
| Far (15–25m) | 8% | Long corridor, slow buildup |

### Lighting Modifiers

The train has variable lighting — bright dining cars, dim maintenance tunnels,
flickering engine sections. Lighting applies a multiplier to sight detection:

| Lighting Level | Detection Multiplier | Examples |
|---------------|---------------------|----------|
| Bright | 1.0x | First Class, greenhouse car |
| Normal | 0.8x | Standard passenger cars |
| Dim | 0.5x | Third Class dormitories, storage |
| Dark | 0.2x | Maintenance tunnels, engine sub-decks |
| Flickering | 0.4x (avg) | Oscillates between 0.2x and 0.8x on a 2s cycle |

### Movement Speed Modifiers

Moving faster is more conspicuous:

| Movement | Detection Multiplier | Sound Generated |
|----------|---------------------|-----------------|
| Stationary | 0.3x | None |
| Crouch-walk | 0.6x | Minimal |
| Walk | 1.0x | Normal footsteps |
| Run | 1.5x | Loud footsteps |
| Sprint | 2.0x | Very loud footsteps |

### Sight Detection Drain

When the player leaves an NPC's vision cone, the sight meter drains:

- **Drain rate**: 15% per second (takes ~6.7s to fully drain from 100%)
- **Drain delay**: 1.5s after losing sight before drain begins
- **Alert state preservation**: If the meter crossed into Suspicious or Alerted
  before draining, the NPC retains that state (see Detection States below)

---

## Sound Detection

### Sound Model

Sound propagates through the train as **concentric rings** from the source. Metal
walls attenuate but don't fully block sound. Each sound event has a **noise radius**
and an **intensity**.

### Footstep Noise by Surface

| Surface | Noise Radius | Intensity | Notes |
|---------|-------------|-----------|-------|
| Metal grating | 12m | High | Standard train flooring |
| Carpet | 5m | Low | First/Second Class cars |
| Wooden planks | 8m | Medium | Third Class, makeshift areas |
| Water/puddle | 10m | Medium-High | Splashing, can't be silenced |
| Dirt/gravel | 6m | Low-Medium | Greenhouse car, agricultural |
| Ice/frost | 7m | Medium | Exterior or breach areas |

**Crouch modifier**: Halves the noise radius for footsteps.

### Combat & Action Noise

| Action | Noise Radius | Notes |
|--------|-------------|-------|
| Melee attack (blunt) | 8m | Metal-on-metal clang |
| Melee attack (bladed) | 5m | Quieter slash |
| Ranged weapon (crossbow) | 10m | Thwack + impact |
| Ranged weapon (pipe gun) | 30m | Gunshot, alerts entire car |
| Stealth takedown | 3m | Muffled struggle |
| Breaking glass/lock | 15m | Sharp, distinctive |
| Opening door (normal) | 4m | Click and slide |
| Opening door (forced) | 12m | Metal screech |
| Body hitting floor | 6m | Thud, triggers investigation |
| Conversation (normal) | 5m | NPCs notice raised voices |
| Kronole mode activation | 2m | Subtle crackling only |

### Environmental Sound Masking

The train generates constant mechanical noise that can mask player sounds:

| Environment | Masking Effect | Description |
|-------------|---------------|-------------|
| Engine room | -10m radius | Deafening machinery |
| Between-car gap | -6m radius | Wind and coupling noise |
| Kitchen/workshop | -4m radius | Clanking, steam, activity |
| Dormitory (sleep hours) | +3m radius | Dead quiet, sounds carry further |
| Club car (active) | -5m radius | Music, conversation |
| Train in motion | -2m radius (global) | Base rumble of the train |
| Train stopped | +2m radius (global) | Eerie silence amplifies everything |

Masking subtracts from the noise radius. A stealth takedown (3m) in the engine
room (−10m) produces effective 0m noise — completely silent.

### Sound Detection Response

When an NPC "hears" a sound (source within noise radius):

1. NPC turns toward the sound origin
2. If sound intensity ≥ High: immediate transition to **Suspicious**
3. If sound intensity < High: NPC "notes" it; two sounds within 10s → **Suspicious**
4. If sound intensity = combat (gunshot, etc.): immediate **Alerted**

---

## Detection States

The detection meter drives NPC behavior through four discrete states:

### State Machine

```
UNAWARE ──[meter > 25%]──► SUSPICIOUS ──[meter > 60%]──► ALERTED ──[confirmed]──► COMBAT
   ▲                            │                            │                        │
   │                            │ [meter drains              │ [search timeout         │ [target lost
   │                            │  below 15%]                │  180s]                  │  30s]
   │                            ▼                            ▼                        │
   └────────────────────────────┘                            └──► SUSPICIOUS ◄────────┘
```

### State Definitions

**UNAWARE** (Detection: 0–25%)
- Default state. NPC follows patrol route or idle behavior.
- No special awareness of the player.
- Player icon: hidden (no indicator on HUD).

**SUSPICIOUS** (Detection: 25–60%)
- NPC has noticed something. Stops patrol, looks around.
- Walks toward last known stimulus (sound or brief sight).
- Investigation lasts 20s, then returns to patrol if nothing found.
- If player is in disguise, NPC may initiate a **challenge** (see Disguises doc).
- Player icon: yellow eye, partially open.

**ALERTED** (Detection: 60–100%)
- NPC is actively searching for a threat. Draws weapon.
- Moves to last known player position. Checks hiding spots systematically.
- Will call for backup (see Alert Propagation).
- Search lasts 180s (3 minutes) before de-escalating to Suspicious.
- Player icon: orange eye, fully open.

**COMBAT** (Detection: confirmed visual)
- NPC has positive visual ID on the player. Engages with combat AI.
- All nearby NPCs in the same zone transition to Alerted.
- Player icon: red eye, with exclamation mark.
- De-escalation: only if player breaks line of sight for 30s AND moves beyond
  detection range. Drops to Alerted, not Unaware.

### De-escalation Rules

- States never drop more than one level at a time.
- COMBAT → ALERTED → SUSPICIOUS → UNAWARE.
- Each transition has a minimum hold time (SUSPICIOUS: 20s, ALERTED: 180s).
- If the player triggers any detection event during de-escalation, the timer resets.
- NPCs who have entered COMBAT **never fully return to UNAWARE** for that
  encounter. They retain a "heightened" flag that increases detection rates by 50%
  for the remainder of the zone visit.

---

## Alert Propagation

When an NPC enters ALERTED or COMBAT, the alert propagates:

### Propagation Rules

1. **Vocal alert**: NPC shouts. All NPCs within 15m immediately enter SUSPICIOUS.
   NPCs within 8m enter ALERTED.

2. **Radio/intercom**: If the alerted NPC has a radio (Jackboots, security), ALL
   NPCs in the current train car enter ALERTED after a 3s delay. This represents
   a radio call.

3. **Car lockdown**: If 3+ NPCs in a car are in COMBAT state simultaneously, the
   car enters **lockdown**:
   - Doors at both ends of the car seal (can be forced open with Strength check
     or lock-picking, but this takes time and makes noise)
   - Reinforcements arrive from adjacent cars after 30s
   - Lockdown persists for 5 minutes or until all threats are neutralized
   - Player can prevent lockdown by disabling the car's intercom panel beforehand

4. **Adjacent car warning**: When a car enters lockdown, adjacent cars receive a
   "warning" — NPCs patrol more actively (detection rates +25%) but don't enter
   ALERTED unless they detect the player themselves.

### Alert Propagation Limiters

- Alerts do NOT propagate through **car boundaries** unless via radio/intercom.
  The between-car gaps act as natural sound barriers.
- Disabling the intercom panel in a car prevents radio-based propagation from
  that car (but vocal alerts still work locally).
- Killing or incapacitating the NPC with a radio before they transmit (within
  the 3s delay) prevents radio propagation.

---

## Hiding Spots

### Hiding Spot Types

| Type | Capacity | Visibility | Found by Search | Notes |
|------|----------|-----------|-----------------|-------|
| Locker/closet | 1 player | Full concealment | Yes (30s) | Common in service areas |
| Under bunk | 1 player | Full concealment | Yes (15s) | Third Class dormitories |
| Behind curtain | 1 player | Partial (legs visible) | Yes (5s) | First/Second Class |
| Ceiling vent | 1 player | Full concealment | No (unless noise) | Requires Agility check |
| Cargo stack | 1-2 players | Full concealment | Yes (45s) | Storage/cargo cars |
| Crowd blend | 1 player | Contextual | Disguise-dependent | Requires disguise |
| Dark alcove | 1 player | Lighting-dependent | Only in Alerted+ | Maintenance areas |

### Hiding Mechanics

- **Enter/exit**: Takes 1.5s (interruptible). Player is vulnerable during transition.
- **Duration**: Unlimited, but some spots have NPC patrol routes that pass them.
- **Detection while hiding**: NPC must be in ALERTED+ state to actively check
  hiding spots. In SUSPICIOUS, they only check spots along their investigation
  path. In UNAWARE, they never check.
- **Noise while hiding**: Player can still generate sound (shifting, coughing in
  cold areas). Holding breath reduces noise but drains a "composure" meter.
- **Body in hiding spot**: A hidden body occupies the spot. Player cannot share
  a spot with a body.

### Composure Meter

While hiding and an NPC is within 5m:
- Composure drains at 10% per second
- At 0%, player involuntarily makes noise (cough, shift)
- Composure resets to 100% when no NPC is within 5m
- Perks can slow composure drain or eliminate it entirely

---

## Body Discovery

### Body States

| State | Visual | Detection Radius | NPC Response |
|-------|--------|-----------------|--------------|
| Standing (stunned) | Swaying | 3m (looks drunk) | SUSPICIOUS only |
| Slumped (unconscious) | On ground | 8m | ALERTED |
| Dead | On ground, blood | 8m | COMBAT (immediate) |
| Hidden (in spot) | Not visible | 0m (unless searched) | None until found |
| Moved (dragged) | Drag marks | 3m (marks only) | SUSPICIOUS |

### Body Discovery Mechanics

- NPCs on patrol routes will discover bodies that are visible along their path.
- Discovered bodies cause the finder to enter ALERTED and call for backup.
- A discovered dead body triggers immediate car-wide alert (treated as confirmed
  hostile presence).
- The player can move bodies by dragging them (slow, makes noise). Dragging leaves
  temporary drag marks that fade after 60s.
- Bodies can be placed in hiding spots to conceal them. Each spot holds one body.
- Unconscious NPCs wake up after 120s (2 minutes) unless restrained. When they
  wake, they enter COMBAT state and alert the entire car via radio.
- Restrained NPCs do not wake up but can be found by patrols (treated as
  ALERTED trigger, not COMBAT).

---

## Integration with Other Systems

### Combat System
- COMBAT detection state hands off to the existing `CombatAIController`.
- Stealth takedowns use `CombatComponent::ReceiveAttack()` with a special
  `EDamageType::StealthTakedown` that applies instant incapacitation.

### Player Stats
- **Cunning**: Reduces detection meter fill rate (−3% per point).
- **Perception**: Shows NPC vision cones on HUD when close enough.
- **Strength**: Required for some hiding spots and forced door entry.
- **Engineer**: Can disable intercom panels and create distractions.

### Dialogue System
- NPCs in SUSPICIOUS state have different dialogue options (can be talked down).
- Failed disguise challenges open a confrontation dialogue branch.
- Some NPCs can be bribed or intimidated to ignore the player.

### Environment
- Lighting is driven by `EnvironmentalHazardComponent` equivalents for stealth.
- Steam vents can be used as noise masking AND visual concealment (brief).
