# Combat Audio Design

Per-weapon-type sound design, combat feedback systems, and environmental combat audio.

---

## Design Philosophy

Combat audio serves three functions:
1. **Feedback** — Every action has an immediate, clear sonic response
2. **Weight** — Sounds communicate the physical reality of violence (this is not stylized action — it's desperate, ugly, costly)
3. **Information** — Audio cues communicate game-state changes the player needs to react to

---

## Weapon Type Audio Profiles

### Blunt Weapons (Pipe Club, Nail Bat, Pipe Mace, Jackboot Baton, Reinforced Wrench)

**Impact Sounds:**
- Primary: Deep, meaty thud with bone resonance — not the Hollywood crack, but the real dull weight of metal hitting body
- Variation pool: 6-8 impact samples per weapon, randomized with ±5% pitch shift
- Hit on armor/shield: metallic clang, higher frequency, more ring
- Hit on wall/environment: material-appropriate (metal-on-metal, metal-on-wood, etc.)

**Swing Sounds:**
- Whoosh profile: slow, heavy, moving-air sound
- Speed correlates to swing arc — larger weapons have longer whoosh buildup
- Miss penalty: the whoosh continues past where impact should be, ending in a dissatisfying trail-off

**Weapon-Specific:**
| Weapon | Unique Audio Element |
|--------|---------------------|
| Pipe Club | Hollow pipe resonance on impact (the pipe rings) |
| Nail Bat | Tearing/ripping secondary sound on hit (nails catching) |
| Pipe Mace | Chain rattle on wind-up, heavy crunch on impact |
| Jackboot Baton | Clean, professional impact — military-grade, no improvised rattle |
| Reinforced Wrench | Mechanical click before each swing (ratchet mechanism) |

**Stagger Sound:** When blunt weapons cause stagger, a distinct "structural failure" sound — like something giving way internally. Not cartoonish — subtle, sickening.

### Bladed Weapons (Shiv, Reinforced Axe, Concealed Blade, Officer's Sword)

**Impact Sounds:**
- Primary: Sharp, slicing sound — shorter attack, faster decay than blunt
- Flesh hit: wet, immediate, with brief silence after (the moment of realization)
- Armor hit: scraping, deflection, the sound of edge-on-surface
- Bleed-inducing hit: a subtle sustained "drip" sound begins after connecting

**Swing Sounds:**
- Whoosh profile: sharper, thinner than blunt — cutting air, not displacing it
- Fast weapons (shiv, concealed blade): minimal whoosh, mostly the snap of wrist movement
- Heavy blades (axe): whoosh with a weighted quality, air compression sound

**Weapon-Specific:**
| Weapon | Unique Audio Element |
|--------|---------------------|
| Shiv | Almost silent swing — the weapon you don't hear coming. Impact is sudden, personal. |
| Reinforced Axe | Wood-handle creak on wind-up, heavy chop on impact, brief stick-in moment |
| Concealed Blade | Click of deployment, whisper-thin swing, precise surgical impact |
| Officer's Sword | The only weapon with a "clean" sound — proper steel, military craftsmanship |

### Piercing Weapons (Jeweled Hairpin, Ice Pick, Pipe Spear)

**Impact Sounds:**
- Primary: Puncture sound — a compressed pop followed by material resistance
- The sound is invasive and uncomfortable by design
- Hairpin/Ice Pick: thin, precise, clinical
- Spear: deeper, with more body to the impact

**Swing Sounds:**
- Thrust-based: forward jab motion rather than arc — short, stabbing sounds
- Spear: longer extension, with shaft flex sound
- Ice Pick: overhead stab arc, sharp downward

**Weapon-Specific:**
| Weapon | Unique Audio Element |
|--------|---------------------|
| Jeweled Hairpin | Near-silent. Poison application: a subtle hiss after hit. DOT ticks are quiet chemical bubbling. |
| Ice Pick | Crystalline impact quality — cold, brittle, sharp. Against cold-debuffed enemies: a cracking/shattering overtone |
| Pipe Spear | Hollow resonance through the pipe shaft. Corridor control: a sweeping whoosh for wide attacks |

### Ranged Weapons (Modified Crossbow, Chemical Flasks)

**Crossbow:**
- Draw: Creaking tension, string stretching, mechanical lock clicking into place
- Release: Sharp snap of string, bolt whistle through air
- Impact: Thunk of bolt embedding — heavy, final
- Reload: Mechanical cranking, effort grunt from player
- Critical importance: the silence between shots is the weapon's character — each shot must count

**Chemical Flasks:**
- Draw/Ready: Liquid sloshing in glass container
- Throw: Glass whistling through air (lighter than metal weapons)
- Impact: Glass shatter + chemical-specific reaction sound
  - Acid: Hissing, bubbling, corrosion
  - Smoke: Whoosh of gas expansion, coughing
  - Fire: Ignition crack, sustained burning
- Area effect: continuous reaction sounds at impact point for duration

### Unarmed / Knuckle Wraps

**Impact Sounds:**
- Bare fists: Skin-on-skin/skin-on-body. Not the movie punch — quieter, more personal, more painful-sounding
- Knuckle wraps: Adds a wrapping-tension sound and a slightly harder impact
- Grapple: Cloth grabbing, body weight shifting, breathing effort

**Unique Character:** Unarmed combat is the most intimate acoustically. No weapon sounds mask the human reality — breathing, grunting, the wet sound of exertion.

---

## Defensive Audio

### Blocking
- **Directional match (successful block):** Sharp, satisfying clang/thud depending on weapon type. Brief moment of silence after — the impact absorbed.
- **Directional mismatch (failed block):** Glancing deflection sound + damage impact. Less satisfying, more chaotic.
- **Shield block (Scrap Shield):** Heavy metal ring, reverberating. Conveys the force absorbed.
- **Perfect parry (precise timing):** Unique high-pitched ring + brief slow-motion whoosh. The most satisfying defensive sound.

### Dodge/Roll
- Cloth movement, foot scrape, exhaled breath
- Near-miss whoosh from the avoided attack
- Stamina cost audible: heavier breathing after dodge

---

## Stamina & Fatigue Audio

The player's physical state is communicated through audio:

| Stamina Level | Audio Cue |
|---------------|-----------|
| 100-75% | Normal breathing, clean weapon sounds |
| 75-50% | Heavier breathing, slight grunt on swings |
| 50-25% | Labored breathing, swings sound slower/heavier, grip adjustments audible |
| 25-0% | Gasping, weapon swings have diminished whoosh, stumbling footsteps |
| Exhausted | Wheezing, weapon drags, movement sounds become sluggish. Near-miss dodge replaced by stumble. |

Fatigue accumulates across encounters — the player character sounds progressively more worn through a combat-heavy session.

---

## Weapon Degradation Audio

| Durability | Audio Change |
|------------|-------------|
| 100-50% | Normal weapon sounds |
| 50-25% | Rattling/looseness in weapon. Impacts lose crispness. A subtle "something's wrong" quality. |
| 25-1% | Weapon sounds broken: cracking handle, bent metal scraping, split wood. Warning creak on each swing. |
| 0% (Broken) | Impact becomes a dull, ineffective thud. The weapon sounds dead. Minimal damage audio. |

Breaking point: when durability hits 0, a distinct snap/crack/shatter sound. The player immediately knows.

---

## Environmental Combat Audio

### Steam Vents
- Warning: hissing pressure buildup (1.5 sec before release)
- Release: explosive steam blast, scalding whistle
- Damage: flesh-searing sizzle on hit
- Ambient: intermittent pressurized hiss between releases

### Electrical Panels / Electrified Railings
- Warning: crackling, buzzing intensification
- Contact: sharp electrical snap + sustained buzz
- Damage: victim convulsion sounds, electrical arcing
- Ambient: low-level electrical hum (60 Hz)

### Window Breach (Instant Kill)
- Glass shatter: explosive, multi-layered crash
- Decompression: roaring wind suction, pressure equalization
- Environmental aftermath: sustained wind howl through breach, cold air sounds
- Emotional weight: a 1-second delay of near-silence before the wind, letting the action register

### Cold/Exterior Exposure
- Freezing: crystallization sounds, ice forming
- Wind: howling, biting
- Cold-debuffed enemy: movement sounds become brittle, cracking

---

## Kronole Mode Audio

When the player activates Kronole Mode (slow-motion perception at health cost):

**Activation:** A wet, chemical sound — something entering the bloodstream. A heartbeat accelerates then dramatically slows.

**During:**
- All sounds pitch-shifted down 30%
- Time-stretched with granular processing
- Massive reverb increase (the world sounds like a cathedral)
- Player's heartbeat becomes the dominant rhythm — slow, heavy, echoing
- Individual sounds become hyper-detailed: each footstep has texture, each weapon swing has sub-components revealed
- The Engine's frequency becomes audible as a subsonic presence

**Deactivation:** Sounds snap back to normal speed — jarring, disorienting. A gasp of re-entry. Health cost manifests as a brief ringing tinnitus.

---

## Combat Vocalization

### Player Character
- Hit reactions: grunts scaled to damage severity (4 tiers: scratch, wound, serious, critical)
- Effort sounds: exertion on heavy swings, blocks, dodges
- Death: not dramatic — a final exhale, the sound of giving out
- Recovery: steadying breath, pain-suppressed inhale

### Enemies
- Each enemy type has a vocal profile:
  - **Jackboots:** Professional, minimal vocalization. Orders barked, impacts barely acknowledged.
  - **Tail residents (if hostile):** Desperate, ragged, emotional. Fighting to survive, not to serve.
  - **Sanctum Acolytes:** Chanting continues during combat. Hits interrupt chanting briefly — disturbing dedication.
- Death sounds: varied, human, never generic. Each death sounds like a person dying, not a game entity being removed.

### Companions
- Combat barks: personality-appropriate call-outs (see voice-acting-scope.md for line counts)
- Injury reactions matching character (Marta is clinical about her own wounds, Edgar is defiant, Big Andrei barely reacts)
- Companion death (permadeath): extended, devastating audio moment. Their theme instrument plays a final fragment. Other companions react vocally.
