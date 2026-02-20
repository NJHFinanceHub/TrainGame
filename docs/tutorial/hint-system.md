# Hint System & Difficulty Integration

## Design Philosophy

The hint system is invisible infrastructure. Players who need help get it; players who don't, never see it. Hints are always diegetic (delivered through companions, environment, or NPC behavior) on Normal and Hard. Only on Easy do context-sensitive tooltips appear as UI elements.

The system has three layers:
1. **Environmental cues** (always present, all difficulties)
2. **Companion dialogue** (present when companions are nearby)
3. **Context-sensitive tooltips** (Easy only, or enabled manually in settings)

## Layer 1: Environmental Cues

Environmental cues are built into the level design and are always active regardless of difficulty. They are not "hints" in the traditional sense - they are part of the world.

### Visual Language

| Cue Type | Visual Treatment | What It Means |
|----------|-----------------|---------------|
| Interactive object | Subtle metallic glint or wet-surface reflection | Can be interacted with |
| Climbable surface | Worn handholds, scratched metal | Can be climbed (STR/END check) |
| Breakable barrier | Visible cracks, stress fractures | Can be forced (STR check) |
| Hidden passage | Slight draft (particle effect), scuff marks on floor | Alternative route (PER check to notice) |
| Danger zone | Discoloration, chemical stains, heat shimmer | Environmental hazard |
| NPC with quest | Distinct idle animation (pacing, looking around) | Has something to say |

### Audio Language

| Cue Type | Sound | What It Means |
|----------|-------|---------------|
| Nearby loot | Faint metallic rattle (loose parts) | Scavengeable items nearby |
| Hidden NPC | Breathing, shuffling, whispered conversation | Someone is nearby but not visible |
| Mechanical interaction | Clicking, hissing, grinding | Machinery that can be used |
| Danger | Electrical crackle, gas hiss, structural groan | Hazard ahead |
| Train movement | Sway intensifies, wheels screech | Curve or terrain change (environmental combat opportunity) |

### Spatial Design

- **Critical path lighting:** The route forward is always slightly better lit than dead ends. Not spotlights - just the natural logic of where light sources would be (lamps, emergency lights, gaps in the hull).
- **Breadcrumb objects:** Small items (a dropped rag, a bloodstain, footprints) mark the path of NPCs the player is following.
- **Dead-end rewards:** Dead ends always contain something (loot, lore, a hidden NPC) so exploration is never punished.

## Layer 2: Companion Dialogue

Companions provide the primary hint delivery for Normal and Hard difficulty. Their hints are in-character and context-sensitive.

### Trigger Conditions

Companion hints fire when:
- The player is idle in a room for longer than a threshold (varies by difficulty)
- The player attempts an action and fails (e.g., failed stat check)
- The player is near an interactive object but hasn't interacted with it
- A combat encounter is approaching (proximity trigger)
- The player opens the wrong door / goes the wrong way for the current objective

### Idle Thresholds

| Difficulty | Idle before first hint | Idle before second hint | Max hints per room |
|------------|----------------------|------------------------|--------------------|
| Easy | 8 seconds | 15 seconds | 3 |
| Normal | 20 seconds | 45 seconds | 2 |
| Hard | Never | Never | 0 |

### Hint Escalation

Companion hints escalate from vague to specific:

**Example: Player needs to find a key in a room**

**Hint 1 (vague):** "There's got to be something useful in here."
**Hint 2 (directional):** "Check near the desk. People hide things in drawers."
**Hint 3 (explicit, Easy only):** "That drawer - see the scratch marks? Someone opens it a lot."

**Example: Player is stuck in combat**

**Hint 1:** "Watch their pattern - they always swing the same way."
**Hint 2:** "Get behind them! They can't guard both sides."
**Hint 3 (Easy only):** "Block when they wind up, then counter-attack."

### Companion Personality in Hints

Each companion hints differently, maintaining character consistency:

**Whisper** (observant, analytical):
- "That wall panel's been removed before. See the wear marks?"
- "Two guards. One watches left, one watches right. Timing gap every twelve seconds."
- "Your blade's getting dull. Find a grindstone before the next fight."

**Grey** (direct, practical):
- "Hit it harder."
- "Stop overthinking. Door's weak. Kick it."
- "You need better gear. Loot the dead ones."

**Edgar** (eager, sometimes wrong):
- "I think we go left! ...or maybe right."
- "That looks climbable! Want me to try?"
- "I saw something shiny over there - could be useful?"

### No-Companion Fallback

When no companion is present, hints fall back to:
- Environmental cues only (Hard)
- Internal monologue on Normal: italicized text like *"There has to be another way through..."*
- Context-sensitive tooltips on Easy (same as Layer 3)

## Layer 3: Context-Sensitive Tooltips

Tooltips are UI elements that appear on Easy difficulty by default. They can be enabled on any difficulty via Settings > Accessibility > Show Tooltips.

### Tooltip Design

- Appear at the bottom of the screen in a semi-transparent bar
- Use plain language, no jargon
- Fade in over 0.5 seconds, persist for 5 seconds, fade out over 0.5 seconds
- Maximum one tooltip visible at a time (queue system, newest replaces oldest)
- Player can dismiss with any button press

### Tooltip Categories

**Mechanic Introduction (first encounter only):**
- "Hold [interact] to search containers for materials."
- "Approach a workbench with materials to discover new recipes."
- "Your Cunning stat affects the quality of items you craft."
- "Some dialogue options require minimum stat values."
- "Your choices shape how the train's people see you."

**Contextual (situation-specific):**
- Near a workbench with materials: "You have materials that could be crafted here."
- Low health near a safe area: "Rest at a bed to recover health."
- Weapon degradation warning: "Your weapon is wearing down. Repair or replace it."
- Before a major choice: "This decision will have lasting consequences."
- New faction encountered: "Faction reputation affects how groups treat you throughout the game."

**Post-Action (feedback):**
- After first craft: "Item crafted. Higher Cunning produces better results."
- After first moral choice: "Your choices shape who you become. The train remembers."
- After first stat check failure: "Some checks require higher stats. You can improve stats by leveling up."
- After companion loyalty shift: "Your companions notice your decisions."

### Tooltip Frequency Limiting

- Each unique tooltip shows only once per playthrough (tracked in save data)
- Maximum 3 tooltips per 5-minute window (prevents tooltip fatigue)
- Combat tooltips are suppressed during active combat (shown after combat ends if still relevant)
- Tooltips are suppressed during cutscenes and major dialogue sequences

## Difficulty Presets

### Easy: "Survivor"
- All three hint layers active
- Tooltips enabled by default
- Companion hints at short intervals
- Stat check requirements shown before attempting ("Requires STR 6 - Your STR: 4")
- Failed checks show what was needed
- Quest objectives marked with subtle waypoints (not arrows - a gentle glow on the target area)
- Practice room in The Pit always highlighted on the map

### Normal: "Revolutionary"
- Environmental cues and companion hints active
- Tooltips disabled by default (can be enabled in settings)
- Companion hints at medium intervals
- Stat check requirements shown (but not your current value)
- Failed checks say "Not strong enough" / "Not persuasive enough" (category, not number)
- No waypoints; rely on NPC directions and environmental breadcrumbs
- Practice room available but not highlighted

### Hard: "Eternal Engine"
- Environmental cues only
- No companion hints (companions still speak in-character but never hint at solutions)
- No tooltips
- Stat checks show only the stat name, not the value: "[Strength]" rather than "[STR 6]"
- Failed checks produce only the in-world response (NPC reacts, door doesn't budge)
- No waypoints, no journal markers, no map indicators
- Practice room available but Grey doesn't mention it

## Accessibility Options (Independent of Difficulty)

These options can be toggled regardless of difficulty selection:

| Option | Default | What It Does |
|--------|---------|-------------|
| Show Tooltips | Easy: On, Others: Off | Enables Layer 3 tooltips |
| Show Stat Values | Easy: On, Normal: On, Hard: Off | Shows numbers on stat checks |
| Waypoint Assist | Easy: On, Others: Off | Subtle glow on objective areas |
| Combat Speed | Normal | Slow (50%), Normal (100%), or Fast (150%) combat pacing |
| Companion Hint Frequency | Matches difficulty | Slider: Off / Rare / Normal / Frequent |
| Colorblind Mode | Off | Adjusts visual cues for protanopia, deuteranopia, tritanopia |
| Screen Reader Support | Off | Adds text descriptions to environmental cues |
| One-Handed Mode | Off | Remaps controls for single-hand play |
| Auto-Block | Off | Character automatically blocks telegraphed attacks |

## The Practice Room: Permanent Tutorial Space

The Pit's practice room (Car 3) serves as the game's only permanent tutorial space. It remains accessible throughout the Tail section (Cars 1-15) and can be returned to at any time.

### Practice Room Contents (Progressive)

The practice room expands as the player progresses:

**Available from Car 3:**
- Training dummy (basic attacks, combos)
- Formation post (party positioning)
- Sparring partners (combat practice)
- Non-lethal equipment display

**Added after Car 5:**
- Weapon rack with one of each weapon type (test before crafting)
- Companion AI behavior tutorial (toggle settings with visual feedback)

**Added after Zone 2:**
- Environmental hazard simulation (steam vent, electrical panel)
- Advanced combo training (counter-attacks, disarms)

### Grey's Role

Grey acts as the practice room's custodian. He provides:
- Post-fight analysis: "You're dropping your guard after the second swing. Stop that."
- Mechanic explanation when asked: "Formations? Front takes the hits, back takes the shots. Simple."
- Challenge fights that scale to the player's level (optional, for rations)

Grey never breaks character. He's not a tutorial NPC - he's a fight trainer who happens to explain things clearly because that's his job.
