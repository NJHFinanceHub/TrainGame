# Train Ambiance Layer Design

The train itself is a constant acoustic presence. This document defines the ambiance layer that underlies all music and gameplay audio — the sonic foundation of the world.

---

## The Train's Voice

The train is not background noise. It is a living acoustic environment with its own rhythms, moods, and responses to gameplay. The ambiance layer is always active, always audible (even if barely), and forms the rhythmic foundation upon which all music is built.

### Base Components (Always Present)

| Component | Description | Behavior |
|-----------|-------------|----------|
| **Wheel Rhythm** | Thunk-thunk cadence of wheels on rail joints | 72 BPM base pulse. Constant. The game's metronome. |
| **Engine Hum** | Low-frequency drone from the Eternal Engine | 30-50 Hz. Volume increases toward front of train. |
| **Metal Stress** | Creaks, groans, flexion of the train's body | Random intervals (8-30 sec). Louder in older cars. |
| **Air Flow** | Climate system hiss, vent circulation | Continuous white/pink noise bed. Varies by zone. |
| **Vibration** | Tactile rumble transmitted through floors/walls | Sub-bass layer. Felt more than heard. |

### Dynamic Components (Situational)

| Component | Trigger | Description |
|-----------|---------|-------------|
| **Coupling Clank** | Car transitions | Heavy metal impact when crossing between cars |
| **Brake Squeal** | Curves/grade changes | High-pitched metal-on-metal during turns |
| **Weather Impact** | Snow/ice conditions | Ice pelting against hull, wind howl variations |
| **Speed Variation** | Story events | Engine pitch shifts during acceleration/deceleration |
| **Emergency Sounds** | Combat/alarms | Pressure changes, emergency valve releases |

---

## Per-Zone Ambiance Profiles

### Zone 1: The Tail (Cars 1-15)

**Acoustic Character:** Raw, uninsulated, exposed

- Wheel rhythm is **loudest** here — minimal sound dampening, metal floors transmit every rail joint
- Dripping water from condensation, irregular (2-8 sec intervals)
- Coughing, breathing, shuffling from unseen inhabitants (crowd ambience at 10-15% volume)
- Harsh fluorescent light buzz (high-frequency 120 Hz hum)
- Occasional distant metallic banging — pipes being struck as communication
- Wind intrusion through poor seals — a constant low whistle
- Temperature: cold. Occasional shiver sounds, teeth chattering in ambient crowd

**Unique Elements:**
- Car 7 (Workshop/Forge): Anvil strikes, grinding wheel, crackling sparks layered over base train sounds
- Car 12 (Kronole Market): Hushed whispers, the crinkle of hidden packages, paranoid footsteps

### Zone 2: Third Class (Cars 16-30)

**Acoustic Character:** Industrial, functional, busy

- Machinery from adjacent maintenance areas bleeds through walls
- Piston rhythms from the HVAC system — a secondary pulse at 96 BPM against the 72 BPM wheels
- Workers' ambient sounds: boots on metal grating, tool clinks, muttered conversations
- Steam releases from heating system at irregular intervals (8-20 sec)
- Better insulation than Tail — wheel rhythm present but muffled by one layer
- Mess hall cars: cutlery on metal trays, scraping of benches, communal eating sounds

**Unique Elements:**
- Hydraulic press sounds in maintenance cars (rhythmic compression/release)
- Work-shift bells at scripted story intervals
- Dietrich's office: metronome-like precision equipment ticking

### Zone 3: Second Class (Cars 31-48)

**Acoustic Character:** Controlled, sanitized, surveilled

- Carpet and upholstery dampen the wheel rhythm significantly — still felt, less heard
- Climate control is precise: a steady, even hiss with no variation
- Clock ticking in bureaucratic offices (100 BPM — slightly ahead of the train's rhythm, suggesting anxiety)
- Typewriter/keyboard clatter from administrative areas
- Intercom announcements at scripted intervals (public address system crackle)
- Surveillance camera servo motors — subtle, paranoid
- Footsteps echo differently: hard shoes on polished floors

**Unique Elements:**
- Courtroom (Car 48): Acoustically "dead" — sound-dampened, formal, oppressive silence broken only by voices and the gavel
- Library cars: near-silence, page turns, clock ticking, the loudest sound is breathing

### Zone 4: The Working Spine (Cars 49-62)

**Acoustic Character:** The train's internal organs exposed

- Engine hum is **dominant** — the player is closer to the source
- Multiple machine rhythms create a polyrhythmic ambient bed
- Electrical buzz from power distribution (various frequencies)
- Water treatment: flowing water, pumps cycling, chemical drip
- Pressure gauges: periodic release valves (hiss-click pattern)
- Warning lights: associated low-frequency alarm tones at 2% volume (subliminal)
- Temperature extremes: hot near engines (metal expansion pings), cold near exterior-adjacent cars

**Unique Elements:**
- The Bridge (Car 62, Kazakov's arena): Retractable bridge mechanisms — heavy hydraulics, metal-on-metal slides, the vertigo-inducing sound of open air below
- Power junction rooms: electrical arcing, transformer hum (60 Hz dominant)

### Zone 5: First Class (Cars 63-82)

**Acoustic Character:** Engineered luxury, acoustic perfection hiding mechanical truth

- Wheel rhythm is nearly **inaudible** — multi-layer sound insulation, active noise cancellation
- Climate control is silent — only detectable as a gentle warmth
- Ambient music within the world: live musicians in restaurants, recorded classical in corridors
- Crystal and glass: subtle chiming from chandeliers responding to train movement
- Fabric rustle: silk, velvet, the soft sounds of expensive materials
- Water features: decorative fountains in garden cars, aquarium bubbles
- Conversation: refined murmurs, laughter, the acoustic signature of privilege

**Unique Elements:**
- Nightclub/Casino (Cars 70-72): Jazz ensemble, slot machines, ice in glasses, roulette wheel
- Opera House (Car 74): Resonant acoustic space — sounds carry differently, natural reverb
- Aquarium car: deep water ambience, bubbling, whale-song-like pump harmonics
- Beaumont's penthouse: unnatural silence — too perfect, actively engineered, uncanny

### Zone 6: The Sanctum (Cars 83-95)

**Acoustic Character:** Cathedral acoustic, reverent, unsettling

- Engine hum is audible again — reframed as sacred vibration, "the Engine's voice"
- Massive reverb — open vaulted spaces in cathedral cars, sound decays over 3-4 seconds
- Chanting from distant Acolytes — always present, always just below comprehension
- Incense burners: crackling, hissing of combustion
- Singing bowls: sustained tones activated by train vibration (the train plays them)
- Cloth and stone: the acoustic signature of robes on marble, bare feet on tile
- Kronole processing: chemical sounds, bubbling, crystallization clicks

**Unique Elements:**
- Cathedral car (Car 90): Organ pedal tones that seem to emerge from the train itself
- Ezra's chambers: the Engine's frequency made audible — a pure tone that is simultaneously beautiful and migraine-inducing
- Meditation cells: near-perfect silence punctuated by single singing bowl strikes

### Zone 7: The Engine (Cars 96-103+)

**Acoustic Character:** The source. Everything the player has heard has been an echo of this.

- The Engine is everything: it IS the ambient sound
- Complex harmonic content: fundamental frequency at ~36 Hz with overtones creating an almost musical drone
- Mechanical precision at scale: thousands of components creating an aggregate sound that approaches white noise but never quite reaches it
- The 72 BPM wheel rhythm is revealed to originate here — the Engine sets the train's heartbeat
- Temperature: extreme heat near the Engine core, cooling systems working at maximum
- Child labor sounds (the dark secret): small hands on mechanisms, bare feet on metal, coughing — almost inaudible, requiring attention to notice
- Wilford's cabin: the Engine filtered through luxury insulation — a refined version of the raw sound, the curated version of truth

**Unique Elements:**
- The Engine room: sub-bass frequencies cause controller vibration/haptic feedback (if hardware supports)
- Transition from Zone 6 to 7: choral ambience seamlessly morphs into Engine harmonics — were they ever different?

---

## Exterior Ambiance

For EVA/Cold Suit sequences outside the train:

**Acoustic Character:** Hostile void with distant train

- Wind: howling, relentless, dominant — wind speed varies creating dynamic volume/pitch
- Cold: crystalline, sharp sounds — ice forming on suit, frost cracking
- The train from outside: a muffled roar, wheels on rail now a continuous scrape rather than rhythmic thunk
- Breath: the player's own breathing, amplified inside the suit helmet, fogging
- Heartbeat: becomes audible in extreme cold (body-awareness increases with danger)
- Radio crackle: communication with inside companions, intermittent, storm-disrupted
- Eerie beauty: in calm moments between storms, the wind becomes almost melodic — the frozen world has its own voice

---

## Technical Specifications

### Layer Architecture
- Train ambiance runs as 4-6 simultaneous audio streams
- Streams crossfade based on player position within car (front/middle/back)
- Car transitions use 1-second crossfade between adjacent car profiles
- Zone transitions use 3-second crossfade for dramatic shift

### Occlusion
- Sounds from adjacent cars are occluded (low-pass filtered, -12 dB)
- Sounds from 2+ cars away are inaudible
- Opening doors briefly removes occlusion (sound floods in, then closes off)

### Randomization
- All irregular ambient events (creaks, drips, coughs) use randomized timing, pitch (±5%), and volume (±3 dB)
- Seed-based randomization tied to car number ensures consistent character per car across playthroughs
- No ambient event repeats identically within 60 seconds

### Performance
- Ambient streams: maximum 6 concurrent
- Sample rate: 48 kHz (matches cinematic standard)
- Ambient audio memory budget: 40 MB compressed per zone loaded, with adjacent zone pre-cached
