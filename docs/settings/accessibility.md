# Accessibility Options

## Overview

Full accessibility suite to ensure Snowpiercer: Eternal Engine is playable by the widest possible audience. All options are available from the main menu before starting a game and from the pause menu at any time. Changes take effect immediately.

## Subtitle & Caption Options

| Setting | Options | Default |
|---------|---------|---------|
| Subtitles | Off / On | On |
| Speaker identification | Off / Name Only / Name + Color | Name + Color |
| Speaker direction indicator | Off / Arrow / Compass | Arrow |
| Subtitle background | None / Semi-transparent / Opaque | Semi-transparent |
| Subtitle text size | Small / Medium / Large / Extra Large | Medium |
| Subtitle max lines | 2 / 3 / 4 | 3 |
| Closed captions (sound effects) | Off / On | Off |
| Caption descriptions | Minimal / Descriptive | Descriptive |

**Speaker color coding:**
- Player character: White
- Companions: Cyan
- Hostile NPCs: Red
- Neutral NPCs: Yellow
- Intercom/PA system: Grey
- Wilford: Gold

Colors are overridden when colorblind modes are active (uses pattern/icon differentiation instead).

## Colorblind Modes

| Mode | Adjustment |
|------|------------|
| Protanopia (red-weak) | Shifts red UI elements to blue; enemy highlights become yellow-blue contrast |
| Deuteranopia (green-weak) | Replaces green indicators with blue; loot rarity uses shape coding alongside color |
| Tritanopia (blue-weak) | Shifts blue elements to red-orange; cold warning UI uses pattern overlays |
| Custom | Per-channel hue rotation sliders (0-360 degrees per channel) |

**Affected systems:**
- Health/stamina bars use pattern fills (solid, striped, dotted) in addition to color
- Loot rarity adds icon shapes: Common (circle), Uncommon (diamond), Rare (star), Very Rare (hexagon)
- Zone map markers use distinct shapes per type (quest, merchant, danger, safe room)
- Faction reputation uses numbered indicators alongside colored bars
- Enemy awareness indicators (stealth) use size scaling in addition to color transitions

## Visual Accessibility

| Setting | Options | Default |
|---------|---------|---------|
| High contrast mode | Off / On | Off |
| High contrast intensity | 50-200% (slider) | 100% |
| HUD scale | 75% / 100% / 125% / 150% | 100% |
| HUD opacity | 25-100% (slider) | 100% |
| Screen shake intensity | 0-100% (slider) | 100% |
| Motion blur | Off / On | On |
| Camera bob | 0-100% (slider) | 100% |
| Field of view | 60-120 degrees (slider) | 90 |
| Film grain | Off / On | On |
| Chromatic aberration | Off / On | Off |
| Flashing/strobe reduction | Off / On | Off |
| Environmental particle density | Low / Medium / High | High |

**High contrast mode details:**
- Interactable objects get bright outlines (configurable color)
- NPCs get silhouette outlines visible through moderate geometry
- Traversal paths (climbable surfaces, ledges) are highlighted
- Trap/hazard zones get patterned ground overlays
- Text gets drop shadow and increased weight

**Flashing/strobe reduction:**
- Eliminates rapid light transitions (explosions, electrical sparks, strobe rooms)
- Kronole vision effects use slow color shifts instead of rapid flashing
- Muzzle flash replaced with sustained glow
- The Dark Car (Car 10) strobe sections use steady dim lighting instead

## Font & Text

| Setting | Options | Default |
|---------|---------|---------|
| Dyslexia-friendly font | Off / On | Off |
| Global text size | 80% / 100% / 120% / 140% | 100% |
| Text weight | Normal / Bold | Normal |
| Letter spacing | Normal / Wide / Extra Wide | Normal |
| Menu narration (text-to-speech) | Off / On | Off |

**Dyslexia-friendly font:**
- Replaces all in-game text with OpenDyslexic or equivalent weighted-bottom font
- Applies to subtitles, menus, journal entries, item descriptions, HUD elements
- Letter spacing automatically increases by one tier when enabled
- Does not affect stylized logo or decorative environmental text (posters, graffiti)

## Control Remapping

### General

- All controls are fully remappable for keyboard, mouse, and gamepad
- No hardcoded keys (including system keys like Escape — though a failsafe combo always opens the pause menu)
- Duplicate binding detection with warning (allows override)
- Reset to defaults per input device

### One-Handed Layouts

| Preset | Description |
|--------|-------------|
| Left hand only (keyboard) | All actions mapped to left-side keys (QWERT/ASDFG/ZXCVB) + mouse |
| Right hand only (keyboard) | All actions mapped to numpad + right-side keys + mouse |
| Left hand only (gamepad) | Left stick + L1/L2 + D-pad handle all inputs; right stick actions auto-assisted |
| Right hand only (gamepad) | Right stick + R1/R2 + face buttons handle all inputs; left stick actions auto-assisted |

**Auto-assist for one-handed gamepad:**
- Camera follows movement direction with adjustable lag (0-500ms slider)
- Auto-center camera on interaction targets
- Combat lock-on enabled by default (can be overridden)

### Toggle vs. Hold

Every hold-action has an independent toggle/hold setting:

| Action | Default | Toggle Available |
|--------|---------|-----------------|
| Sprint | Hold | Yes |
| Crouch/Stealth | Hold | Yes |
| Aim/Iron sights | Hold | Yes |
| Block/Guard | Hold | Yes |
| Interact (long press) | Hold | Yes |
| Grab/Carry | Hold | Yes |
| Breath hold (sniping) | Hold | Yes |

## Aim Assistance

| Setting | Options | Default |
|---------|---------|---------|
| Auto-aim | Off / Low / Medium / High | Off |
| Aim assist (sticky) | Off / On | On (gamepad) / Off (mouse) |
| Aim assist strength | 0-100% (slider) | 50% |
| Lock-on targeting | Off / On | Off |
| Lock-on switch (multi-target) | Nearest / Left-Right Flick | Nearest |
| Aim snap to target | Off / On | Off |

**Auto-aim levels:**
- **Low:** Slight bullet magnetism (±5 degrees) when aiming near an enemy
- **Medium:** Moderate magnetism (±10 degrees) + reticle slows near targets
- **High:** Strong magnetism (±15 degrees) + reticle slows significantly + headshot region expanded by 25%

**Lock-on targeting:**
- Snaps to nearest hostile when activated
- Camera tracks locked target
- Works with melee and ranged combat
- Breaks on: target death, target behind full cover for >3 seconds, manual cancel, distance >30m

## Audio Accessibility

| Setting | Options | Default |
|---------|---------|---------|
| Master volume | 0-100% | 100% |
| Music volume | 0-100% | 80% |
| SFX volume | 0-100% | 100% |
| Voice volume | 0-100% | 100% |
| Ambient volume | 0-100% | 80% |
| Mono audio | Off / On | Off |
| Audio visualization | Off / Radar / Waveform | Off |
| Directional audio indicator | Off / On | Off |

**Audio visualization:**
- **Radar:** On-screen radar showing sound source direction and intensity
- **Waveform:** Edge-of-screen waveform pulses indicating direction and type (footstep, gunshot, voice, environmental)

**Directional audio indicator:**
- Visual arrow pointing toward significant sound sources
- Color/shape coded by threat level (ambient, caution, danger)
- Useful for deaf/hard-of-hearing players to detect off-screen events

## Motor Accessibility

| Setting | Options | Default |
|---------|---------|---------|
| QTE (quick-time events) | Normal / Simplified / Auto-Complete | Normal |
| QTE timing window | 0.5x / 1x / 2x / 3x | 1x |
| Repeated button press | Normal / Hold-to-Fill / Auto | Normal |
| Input timing tolerance | Tight / Normal / Generous | Normal |
| Menu cursor speed | 25-200% (slider) | 100% |
| Double-click/press speed | 200-800ms (slider) | 400ms |
| Simultaneous input | Required / Sequential | Required |

**QTE simplification:**
- **Simplified:** Single button prompts instead of sequences; longer windows
- **Auto-Complete:** QTEs resolve automatically with a brief cinematic pause; narrative choices still require input

**Simultaneous input (sequential mode):**
- Actions requiring two simultaneous buttons (e.g., sprint + jump) can be performed sequentially within a configurable window (200-1000ms)

## Gameplay Assists

These options provide gameplay help independent of difficulty tier:

| Setting | Options | Default |
|---------|---------|---------|
| Navigation assist | Off / Breadcrumb Trail / Waypoint Pulse | Off |
| Puzzle hints | Off / After 3 min / After 5 min / On Request | Off |
| Skip combat encounter | Off / On (after 3 failures) | Off |
| Inventory auto-sort | Off / On | Off |
| Item comparison tooltip | Off / On | On |
| Threat indicator | Off / On | Off |
| Tutorial reminders | Off / On | On |

**Navigation assist:**
- **Breadcrumb Trail:** Faint trail on the ground showing path to active objective
- **Waypoint Pulse:** Periodic visual pulse emanating from objective direction

**Skip combat (when enabled):**
- After failing the same combat encounter 3 times, an option appears: *"Skip this fight?"*
- Skipping resolves the encounter as if the player won with minimum loot
- Does not apply to boss fights or story-critical encounters
- Tracked separately from difficulty — does not affect achievements

## Photosensitivity

| Setting | Options | Default |
|---------|---------|---------|
| Photosensitivity mode | Off / On | Off |
| Reduce bright flashes | Off / On | Off |
| Reduce rapid color changes | Off / On | Off |
| Screen brightness limiter | Off / On | Off |
| Brightness limiter cap | 50-90% (slider) | 80% |

**When photosensitivity mode is On:**
- Automatically enables: reduce bright flashes, reduce rapid color changes, flashing/strobe reduction
- Explosion effects use expanding rings instead of bright flash
- Electrical hazards use steady glow instead of arcing
- Zone transitions use fade-to-black instead of white flash
- Screen damage effects (blood, frost) fade in gradually instead of appearing instantly
