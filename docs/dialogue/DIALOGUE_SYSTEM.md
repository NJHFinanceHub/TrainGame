# Dialogue System Architecture

## System Type: Hybrid Hub-and-Spoke with Branching

The dialogue system uses a **hub-and-spoke** model as its primary structure, with
**branching trees** for critical story moments. This hybrid approach suits the
Snowpiercer setting where conversations happen in constrained, high-tension
environments.

### Why Hub-and-Spoke

- **Tight spaces, tight conversations.** In a train, you're face-to-face with NPCs
  repeatedly. Hub-and-spoke lets players re-engage topics naturally without replaying
  entire trees.
- **Zone progression locks/unlocks spokes.** As the player moves forward through the
  train, NPCs gain new conversation topics. Old spokes can close permanently based
  on events.
- **Branching for irreversible moments.** Key story beats (betrayals, alliances,
  revelations) use one-shot branching trees that lock out alternatives.

### Structure

```
DialogueNode (Hub)
├── Spoke A: "Ask about the Tail"        [available from start]
├── Spoke B: "What happened to Marcus?"  [unlocked after Zone 2]
├── Spoke C: "I need your help"          [skill check gate]
├── Spoke D: "Trade"                     [opens trade UI]
└── Branch E: "Betray the resistance"    [one-shot, irreversible]
    ├── Accept → loyalty shift, new quest
    └── Refuse → NPC becomes hostile
```

### Node Types

| Type | Purpose | Replayable |
|------|---------|------------|
| Hub | Central conversation anchor | Yes |
| Spoke | Topic thread from a hub | Yes, until locked |
| Branch | Critical decision point | No (one-shot) |
| Bark | Ambient/passing comment | No (contextual) |
| Interrupt | Forced dialogue (combat, event) | No |

---

## Skill Checks in Dialogue

Five stats can be tested during dialogue: **Charisma**, **Cunning**, **Perception**,
**Strength**, and **Engineer**.

### Check Types

**Passive Checks** — Resolved silently before showing options. If the player's stat
meets the threshold, the option appears. If not, it doesn't exist in the menu.
Used for: noticing hidden details, reading body language, spotting lies.

**Active Checks** — The player sees the option and its stat requirement. Selecting it
rolls against the threshold. Success/failure both have unique dialogue branches.
Used for: persuasion attempts, intimidation, technical arguments.

**Contested Checks** — Player stat vs NPC stat. The NPC has a hidden resistance value.
Used for: high-stakes negotiations, interrogations, bluffs against smart NPCs.

### Visibility Rules

| Check Type | Stat Name Shown | Threshold Shown | Success Chance Shown |
|------------|----------------|-----------------|---------------------|
| Passive | No | No | N/A (auto-resolve) |
| Active | Yes | Yes (color-coded) | No |
| Contested | Yes | "Contested" label | No |

**Color coding for active checks:**
- Green: stat exceeds threshold by 3+ (guaranteed success)
- Yellow: stat within ±2 of threshold (uncertain)
- Red: stat below threshold by 3+ (likely failure)
- Locked: stat below threshold by 8+ (impossible, shown grayed out)

### Stat Applications in Dialogue

| Stat | Dialogue Use |
|------|-------------|
| Charisma | Persuasion, inspiration, rallying crowds, romance |
| Cunning | Deception, manipulation, detecting lies, blackmail |
| Perception | Noticing details, reading emotions, catching inconsistencies |
| Strength | Intimidation, physical threats, arm wrestling, breaking things |
| Engineer | Technical arguments, sabotage bluffs, understanding machinery |

### Example Dialogue with Skill Checks

```
Guard Captain Vasquez — Checkpoint to Second Class

Hub: "State your business, Tailer."

├── [Charisma 6] "I'm here on behalf of the Committee. They sent word ahead."
│   ├── Success: "Hmm... Fine. But I'm watching you."
│   └── Failure: "Nice try. The Committee doesn't send Tailers anywhere."
│
├── [Cunning 8] "Check your manifest. Maintenance crew, car 47."
│   ├── Success: Guard checks fake manifest, waves through
│   └── Failure: "There is no car 47, idiot. Hands on the wall."
│
├── [Strength 5] *Crack knuckles* "You really want to do this?"
│   ├── Success: Guard steps aside, but raises alert level
│   └── Failure: Guard calls backup, combat encounter
│
├── [Perception — Passive] Notice guard's injured hand
│   └── Unlocks: "That hand looks bad. I know someone who can help."
│       └── Opens non-violent path via favor exchange
│
├── [Engineer 7] "Your pressure coupling on the door seal is about to blow."
│   ├── Success: Guard panics, lets you "fix" it, you slip through
│   └── Failure: "I know my own door, grease monkey."
│
└── "Never mind." → Exit
```

---

## Timed Dialogue Decisions

Certain high-tension moments impose a **countdown timer** on dialogue choices.
If the timer expires, the **default action** triggers (usually the worst outcome).

### When Timers Activate

- During combat interruptions (enemies approaching)
- When an NPC is about to do something irreversible (execute someone, pull a lever)
- During negotiations that are going south
- Train-wide emergencies (breach alarms, derailment warnings)

### Timer Behavior

| Urgency | Duration | Visual | Default Action |
|---------|----------|--------|---------------|
| Low | 15 seconds | Subtle pulse on border | Silence (NPC continues) |
| Medium | 8 seconds | Amber flash, ticking sound | NPC picks for you (bad) |
| High | 4 seconds | Red flash, alarm tone | Worst outcome |
| Critical | 2 seconds | Screen shake, siren | Immediate failure/death |

### Design Rules for Timed Dialogue

1. **Never timer a first-time story choice.** Players need to read and process.
   Timers are for reactive moments, not deliberative ones.
2. **Timer duration scales with option count.** 2 options = short timer. 4 options =
   longer timer.
3. **Perception checks can extend timers** by +3 seconds (passive check, no UI cost).
4. **Timer pauses during skill check resolution** so players see the outcome before
   the next decision.
5. **Accessibility option: disable timers entirely** (replaces with "NPC is getting
   impatient" flavor text and no mechanical consequence).

---

## NPC Memory System

NPCs remember what the player has done. This drives dynamic dialogue and
consequences across the entire train.

### Memory Categories

| Category | Scope | Persistence | Example |
|----------|-------|-------------|---------|
| Personal | Single NPC | Permanent | "You helped me in Zone 1" |
| Factional | All NPCs in faction | Permanent | "Tail rebels killed our guards" |
| Witnessed | NPCs in line-of-sight | Permanent | "I saw you steal that" |
| Rumor | Spreads over time | Semi-permanent | "They say you spared the boss" |
| Mood | Single NPC | Temporary (decays) | "You annoyed me 5 minutes ago" |

### Rumor Propagation

When the player performs a notable action, it generates a **rumor tag**. Rumors
propagate forward through the train at a rate of ~1 car per 15 minutes of game
time. NPCs in reached cars may reference the rumor.

**Rumor fidelity degrades with distance:**
- Same car: accurate
- 1-2 cars away: mostly accurate
- 3-5 cars away: exaggerated or distorted
- 6+ cars away: mythologized or inverted

Example: Player spares a guard captain in Zone 2.
- Zone 2 NPCs: "You let Vasquez live. Unexpected."
- Zone 3 NPCs: "Heard you showed mercy to some Jackboot. Soft move."
- Zone 4 NPCs: "Word is you're cutting deals with the guards."
- Zone 5 NPCs: "They say you're Wilford's new pet."

### Memory Impact on Dialogue

- **Locked/unlocked spokes** based on memory flags
- **Tone shifts** — NPCs with positive memory use friendlier lines
- **Price adjustments** — Traders with good memory give discounts
- **Combat avoidance** — Feared players get surrender options
- **Betrayal triggers** — NPCs who remember lies become hostile

### NPC Disposition Model

Each NPC tracks a simple disposition value:

```
Disposition: int (-100 to +100)
  -100..-50: Hostile (attacks on sight, no dialogue)
   -49..-20: Antagonistic (terse, unhelpful, may betray)
   -19..+19: Neutral (transactional)
   +20..+49: Friendly (helpful, shares info, discounts)
   +50..+100: Loyal (will fight for player, sacrifice)
```

---

## Interrupted Dialogue Handling

Dialogue can be interrupted by external events. The system handles this gracefully.

### Interruption Sources

| Source | Behavior |
|--------|----------|
| Combat starts | Dialogue ends, NPC enters combat state |
| Train event (breach, alarm) | Dialogue pauses, NPC reacts to event |
| Third NPC approaches | NPC acknowledges intruder, may modify dialogue |
| Timer expires | Default action, dialogue may continue or end |
| Player walks away | NPC makes comment, disposition -5, spoke remains available |

### Resume Behavior

After an interruption resolves:
- **Combat interruption**: NPC returns to hub with new "What was that?" spoke
- **Train event**: NPC returns to hub, may have new event-specific spoke
- **Third party**: Dialogue tree may branch to include the newcomer
- **Walk-away**: Hub available again, but the specific spoke context resets

### Anti-Exploit Rule

Walking away from a skill check to re-attempt it is blocked: failed active checks
are flagged per-NPC and cannot be retried in the same conversation. The player must
leave and return later (or use a different approach).

---

## Dialogue Density by Zone

The train's social structure dictates how much talking happens in each area.

| Zone | Density | Style | Rationale |
|------|---------|-------|-----------|
| Zone 1: Tail | **Sparse** | Clipped, desperate, action-oriented | People are starving. Words are a luxury. Most communication is gestures, grunts, urgency. |
| Zone 2: Third Class | **Moderate** | Working-class, practical, suspicious | People talk, but carefully. Trust is earned. Information is currency. |
| Zone 3: Second Class | **Verbose** | Performative, political, layered | This is where people scheme. Long conversations with subtext. Multiple skill checks per dialogue. |
| Zone 4: First Class | **Dense but guarded** | Polite, coded, dangerous | Every word has double meaning. Charisma and Cunning checks dominate. Silence is a weapon. |
| Zone 5: Greenhouse/Aquarium | **Moderate** | Scientific, detached, philosophical | Engineers and scientists. Conversations about systems, ethics, the engine. Engineer checks shine here. |
| Zone 6: Engine Approach | **Sparse** | Ritualistic, reverent, terrified | Wilford's cult territory. Dialogue is prayer-like. Few options, high consequences. |
| Zone 7: Engine | **Minimal** | Monologue, revelation, final choice | Wilford talks. You listen. Then you choose. |

### Density Metrics (per car)

| Zone | Talkable NPCs | Avg Spokes/NPC | Skill Checks/Car | Estimated Play Time in Dialogue |
|------|---------------|----------------|-------------------|---------------------------------|
| Tail | 2-3 | 2-3 | 1-2 | 3-5 min |
| Third | 4-6 | 3-5 | 3-4 | 8-12 min |
| Second | 6-10 | 5-8 | 5-8 | 15-25 min |
| First | 4-8 | 4-6 | 4-6 | 10-18 min |
| Green | 3-5 | 4-6 | 3-5 | 8-12 min |
| Approach | 1-3 | 1-3 | 1-2 | 3-5 min |
| Engine | 1 | N/A (scripted) | 1 (final) | 5-10 min |

---

## Persuasion, Intimidation & Deception

These are the three primary social combat mechanics. Each maps to a stat and has
distinct risk/reward profiles.

### Persuasion (Charisma)

**How it works:** Convince the NPC that your request aligns with their interests.
Requires understanding what the NPC wants.

| Outcome | Effect |
|---------|--------|
| Success | NPC cooperates willingly. Disposition +5 to +15. |
| Failure | NPC is unimpressed. Disposition -5. Topic locked for this conversation. |
| Critical Success | NPC becomes an ally. Disposition +25. May offer unsolicited help. |

**Design note:** Persuasion is the "safe" option. Low risk, moderate reward. It
should feel like the diplomatic path that builds long-term relationships.

### Intimidation (Strength)

**How it works:** Use physical presence, reputation, or implied violence to force
compliance. Works best when the player has demonstrated combat ability.

| Outcome | Effect |
|---------|--------|
| Success | NPC complies out of fear. Disposition -10 to -20. Cooperation is grudging. |
| Failure | NPC calls bluff. May trigger combat. Disposition -15. |
| Critical Success | NPC is terrified. Disposition -30 but will not betray you (too scared). |

**Design note:** Intimidation is the "fast" option. Quick results but burns bridges.
Intimidated NPCs may sabotage you later or refuse to help in crises.

**Reputation modifier:** If the player has killed 3+ NPCs in the current zone,
Strength checks get +2 bonus. If they've killed 10+, some NPCs surrender without
a check.

### Deception (Cunning)

**How it works:** Lie, manipulate, misdirect. The most powerful social tool but
the most dangerous on failure.

| Outcome | Effect |
|---------|--------|
| Success | NPC believes lie. Acts on false info. Disposition unchanged (they don't know). |
| Failure | NPC catches the lie. Disposition -20. May alert others. Rumors spread negatively. |
| Critical Success | NPC not only believes it but spreads the lie for you. |
| Discovered Later | If truth emerges, NPC disposition drops to -50. Betrayal event. |

**Design note:** Deception is the "risky" option. Best immediate payoff but creates
time bombs. Lies can unravel hours later when an NPC cross-references information.

**Lie tracking:** The system tracks active lies. If the player tells NPC A one thing
and NPC B a contradictory thing, and those NPCs talk (rumor propagation), the lie
is exposed. This creates a meta-game of managing your web of deceptions.

### Social Combat Matrix

Some NPCs are resistant to certain approaches:

| NPC Type | Persuasion | Intimidation | Deception |
|----------|-----------|--------------|-----------|
| Guard | Normal | Effective (+2) | Hard (-2) |
| Trader | Effective (+2) | Normal | Normal |
| Engineer | Hard (-2) | Normal | Hard (-2) |
| Politician | Normal | Hard (-2) | Effective (+2) |
| Cult Member | Hard (-2) | Hard (-2) | Normal |
| Child | Effective (+2) | Immune (auto-fail) | Hard (-2) |
| Wilford | Immune | Immune | Immune |

---

## Dialogue-Bypassed Encounters

Certain combat or stealth encounters can be entirely avoided through dialogue.
This is a core design pillar: **the train rewards talkers**.

### Bypassable Encounters

| Encounter | Zone | Bypass Method | Consequence of Bypass |
|-----------|------|---------------|----------------------|
| Tail Guard Checkpoint | 1 | Intimidation or Deception | Guards remain at post (future obstacle) |
| Rat Merchant Ambush | 1 | Perception (spot ambush) + Persuasion | Merchant becomes trader contact |
| Third Class Bar Fight | 2 | Charisma (calm crowd) | Earn respect, skip combat XP |
| Jackboot Patrol | 2-3 | Engineer (fix their radio) or Deception | Patrol leaves area temporarily |
| Second Class Murder Mystery | 3 | Perception + Cunning (solve without violence) | Unlock unique companion |
| First Class Duel | 4 | Charisma (appeal to honor code) | Duel becomes political favor |
| Greenhouse Saboteur | 5 | Engineer (prove sabotage is futile) + Persuasion | Saboteur becomes informant |
| Guard Captain Boss | 2 | Strength (physical contest) or Charisma (appeal to duty) | Captain becomes reluctant ally |
| Cult Sacrifice Ritual | 6 | Cunning (subvert ritual) or Charisma (counter-sermon) | Cult fractures, some join you |

### Design Rules for Bypass

1. **No bypass should be "free."** Even successful dialogue bypasses should have
   costs: missed loot, altered NPC attitudes, or closed-off future options.
2. **Combat XP vs Dialogue XP.** Bypassed encounters award dialogue XP (same
   amount as combat XP, different pool). Both pools contribute to leveling.
3. **At least one bypass per zone.** Every zone must have at least one encounter
   that can be fully resolved through dialogue.
4. **Bosses can be weakened, not always bypassed.** Zone bosses generally require
   combat, but dialogue can: remove their allies, discover weaknesses, secure
   favorable terrain, or reduce their stats before the fight.
5. **Pacifist route exists but is hard.** A theoretical pacifist run is possible
   through Zone 4. From Zone 5 onward, at least some combat is unavoidable (but
   minimizable). The Engine confrontation can end without fighting.
