# Adaptive Music System

The game uses a layered adaptive music system where musical elements respond dynamically to gameplay state. All music is authored as stems that can be independently mixed, muted, and crossfaded in real-time.

---

## System Architecture

### Layer Stack (Bottom to Top)

| Layer | Content | Always Playing | Adaptive |
|-------|---------|:-:|:-:|
| 0. Train Ambience | Wheels, engine hum, metal creaks | Yes | Volume only |
| 1. Zone Base | Zone musical identity (see zone-musical-identity.md) | Yes | Intensity |
| 2. Exploration | Melodic content, harmonic movement | Conditional | State-driven |
| 3. Tension | Suspense elements, dissonant layers | Conditional | Detection level |
| 4. Combat | Full combat arrangement | Conditional | Combat state |
| 5. Narrative | Scripted musical moments, stingers | Conditional | Event-triggered |

Layers 0-1 always play (at minimum volume). Layers 2-5 activate based on game state.

### Music States

The system recognizes six primary states with transitions between them:

```
EXPLORATION ←→ SUSPICION → DETECTION → COMBAT → POST-COMBAT → EXPLORATION
                                                       ↓
                                                  MORAL_CHOICE
```

---

## State Definitions

### 1. Exploration

**Trigger:** No enemies aware, no combat, no scripted events

**Active Layers:** Train Ambience + Zone Base + Exploration

**Behavior:**
- Zone-appropriate melodic content plays at medium intensity
- Exploration layer selects from a pool of 3-4 musical phrases per zone, shuffled to avoid repetition
- Dynamic mixing: louder in safe spaces (companion conversations, rest areas), quieter near hostile territory
- Responds to time-in-zone: music becomes more familiar/comfortable the longer the player stays, introducing new melodic variations after 10+ minutes

**Transition Out:** Enemy awareness > 0% triggers crossfade to Suspicion (2 sec)

### 2. Suspicion

**Trigger:** Enemy awareness 1-49%, player near restricted area, stealth active

**Active Layers:** Train Ambience + Zone Base + Exploration (reduced) + Tension (low)

**Behavior:**
- Exploration melody thins — fewer instruments, wider spacing
- Tension layer introduces: sustained dissonant strings, heartbeat-like low percussion, high-frequency tones
- Intensity scales linearly with enemy awareness percentage
- Rhythmic elements accelerate subtly (tempo remains constant but subdivision increases)
- Stealth-specific: pizzicato strings pulse with the player's movement speed

**Transition Out:**
- Awareness drops to 0% → 4 sec fade back to Exploration
- Awareness hits 50%+ → immediate cut to Detection

### 3. Detection

**Trigger:** Enemy awareness 50-99%, active search, alarm raised

**Active Layers:** Train Ambience (reduced) + Zone Base + Tension (full)

**Behavior:**
- Exploration layer fully muted
- Tension layer at maximum: aggressive percussion, brass stabs, alarm-tone motifs
- Zone-specific detection sounds:
  - Tail: Pipe-banging alarm rhythms
  - Third Class: Factory whistle loops
  - Second Class: Staccato string figures
  - Working Spine: Machinery accelerating
  - First Class: Discordant piano crashes
  - Sanctum: Choir shouts/chants
  - Engine: The Engine's rhythm becoming hostile
- "Chase" percussion variant activates if player is fleeing

**Transition Out:**
- Combat initiated → 0.5 sec smash cut to Combat
- Awareness drops below 50% → 3 sec transition to Suspicion
- Full escape → 6 sec decompression transition to Exploration

### 4. Combat

**Trigger:** Active melee/ranged engagement with enemies

**Active Layers:** Train Ambience (minimal) + Zone Base (minimal) + Combat

**Behavior:**
- Combat layer takes over almost entirely — zone base reduced to subtle coloring
- Per-zone combat music (see zone-musical-identity.md for palette per zone)
- Dynamic intensity based on:
  - Number of active enemies (more = fuller arrangement)
  - Player health (low health = more frantic, higher pitch)
  - Companion presence (companions add their instrumental voice to combat mix)
  - Environmental hazards active (steam/electrical = industrial stingers)
- Hit sync: percussive accents sync to successful player attacks (within 100ms window)
- Weapon-type influence: combat rhythmic feel adjusts to equipped weapon speed
  - Fast weapons (shivs, hairpin): 16th-note rhythmic base
  - Medium weapons (axes, wrenches): 8th-note rhythmic base
  - Slow weapons (pipe mace, heavy wrench): quarter-note hits with anticipation fills
  - Ranged (crossbow): sustained tension between shots, sharp accent on release
- Stamina influence: as stamina depletes, music becomes more ragged, less rhythmically precise
- Kronole Mode: all combat music pitch-shifted down 30%, time-stretched, reverb dramatically increased — the world sounds underwater

**Special Combat States:**
- **Last Enemy Standing:** Music drops to minimal — just percussion and bass. Heightens the 1v1 tension.
- **Near Death (player):** Heartbeat takes over as primary rhythm, all other elements filter to muffled distance
- **Environmental Kill:** Sharp stinger + 1 sec silence before combat music resumes

**Transition Out:**
- All enemies defeated → Post-Combat
- Player death → Immediate silence, then death stinger (2 sec)

### 5. Post-Combat

**Trigger:** Combat ends (enemies defeated or fled)

**Active Layers:** Train Ambience (rising) + Zone Base (rising) + Post-Combat wind-down

**Behavior:**
- 4-8 second transition period
- Combat energy dissipates: percussion fades first, then harmony, melody lingers
- "Victory" is never triumphant — post-combat music is always tinged with cost
  - Tail/Third: Somber, acknowledging violence
  - Second/First: Unsettled, "what have we become"
  - Sanctum/Engine: Solemn, ritualistic
- If player showed mercy (knocked out rather than killed), post-combat adds a warmer harmonic resolution
- If player killed all enemies, post-combat is more dissonant, unresolved
- Companion reactions layer in: audible sighs, murmurs, weapon-lowering sounds

**Transition Out:** After wind-down completes → Exploration (seamless)

### 6. Moral Choice

**Trigger:** Scripted moral decision points (boss mercy, companion loyalty, major story beats)

**Active Layers:** Minimal — often near-silence

**Behavior:**
- All adaptive layers fade to near-silence over 3 seconds
- Only the Train Ambience (heartbeat pulse) and a single sustained tone remain
- The sustained tone is the Revolutionary Motif's root note — a thread connecting all choices
- No musical "guidance" toward either option — the system refuses to editorialize
- On choice made: a brief musical response (2-4 sec) reflecting the weight of the decision
  - Mercy choices: the sustained tone resolves upward (hopeful but uncertain)
  - Harsh choices: the sustained tone resolves downward (decisive but heavy)
  - These resolutions are subtle — the game doesn't judge, only acknowledges

---

## Technical Implementation

### Stem Format
- All music authored as multi-track stems in Wwise/FMOD
- Minimum 4 stems per zone per state: Percussion, Bass, Harmony, Melody
- Additional stems for state-specific elements (combat hits, chase percussion)
- All stems pre-synced to zone tempo and key

### Crossfade Rules
- State transitions use equal-power crossfades (no volume dip)
- Transition timing respects musical bar boundaries when possible (quantized to next beat or bar)
- Emergency transitions (detection, combat start) override bar-quantization for responsiveness
- Maximum simultaneous active stems: 12 (performance budget)

### Vertical Remixing
- Within a state, stems are added/removed based on intensity
- Intensity is a 0.0-1.0 float driven by game state parameters
- Each stem has an activation threshold (e.g., combat percussion at 0.3, brass at 0.6)
- Hysteresis prevents rapid on/off flicker: activation threshold is higher than deactivation by 0.1

### Horizontal Re-sequencing
- Exploration and Suspicion states use horizontal segments (4-8 bar phrases)
- The system selects next segment based on current state and recent history (no immediate repeats)
- Segments are composed to work in any order (compatible endings/beginnings)
- Pool of 8-12 segments per zone per state provides 30+ minutes before repetition becomes noticeable

### Memory Budget
- Streaming from disk, not loaded into memory
- Maximum concurrent streams: 16 (12 music stems + 4 ambience)
- Estimated music data per zone: 80-120 MB uncompressed, 20-30 MB compressed (Vorbis)
- Total music footprint: ~200 MB compressed across all zones
