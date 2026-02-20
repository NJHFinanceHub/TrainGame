# Collectible Journal & Codex UI

> *"Knowledge is the only thing on this train that multiplies when shared."*

## Overview

The Collectible Journal is the player's primary interface for reviewing, organizing, and cross-referencing all discovered collectibles, lore, and world information. It is accessed via the pause menu or a dedicated keybind (default: J) and serves as both a completion tracker and an interactive lore database.

---

## Journal Structure

### Tab Layout

```
┌──────────────────────────────────────────────────────────────────┐
│  [COLLECTIBLES]  [WEB OF POWER]  [MANIFEST]  [MAP]  [CODEX]    │
│─────────────────────────────────────────────────────────────────│
│                                                                  │
│                     (Active Tab Content)                          │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

---

## Tab 1: Collectibles

### Layout

```
┌─────────────────────────────────────────────────────────────────┐
│  COLLECTIBLES                                    247 total      │
│                                                                 │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐          │
│  │ TRAIN   │  │ AUDIO   │  │ARTIFACTS│  │BLUEPRNTS│          │
│  │ LOGS    │  │RECORDNGS│  │         │  │         │          │
│  │ 12/48   │  │  8/42   │  │  6/38   │  │  5/32   │          │
│  │  25%    │  │  19%    │  │  16%    │  │  16%    │          │
│  └─────────┘  └─────────┘  └─────────┘  └─────────┘          │
│  ┌─────────┐  ┌─────────┐                                     │
│  │FACTION  │  │MANIFEST │                                     │
│  │ INTEL   │  │ PAGES   │                                     │
│  │ 14/52   │  │  7/35   │                                     │
│  │  27%    │  │  20%    │                                     │
│  └─────────┘  └─────────┘                                     │
│                                                                 │
│  Zone Filter: [ALL] [Z1] [Z2] [Z3] [Z4] [Z5] [Z6] [Z7] [EXT] │
│  Sort: [Recent ▼]  [Name]  [Rarity]  [Zone]                   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  ★ Boarding Token #0001          Artifact    Legendary  │   │
│  │  ● The Survivor's Journal        Audio Rec   Rare       │   │
│  │  ● Emergency Protocol Blueprint  Blueprint   Uncommon   │   │
│  │  ○ [UNDISCOVERED]                Train Log   Zone 1     │   │
│  │  ○ [UNDISCOVERED]                Train Log   Zone 1     │   │
│  │  ...                                                    │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

### Collectible Detail View

Selecting a collectible opens a detail panel:

```
┌─────────────────────────────────────────────────────────────┐
│  BOARDING TOKEN #0001                                       │
│  Type: Artifact  │  Rarity: Legendary  │  Zone: Bunker Car  │
│─────────────────────────────────────────────────────────────│
│                                                             │
│  ┌───────────────────┐                                     │
│  │                   │  The very first passenger token      │
│  │   [3D Model       │  issued for Snowpiercer. Stamped    │
│  │    Viewer]        │  aluminum, worn smooth by years      │
│  │                   │  of handling. The name field has     │
│  │   Rotate: L-Stick │  been filed off — deliberately,     │
│  │   Zoom: R-Stick   │  not by wear.                       │
│  │                   │                                     │
│  └───────────────────┘  Found: The Bunker Car              │
│                         Date: Day 47                        │
│                                                             │
│  ─── CONNECTIONS ────────────────────────────────           │
│  → Links to: Manifest Page (Zone 1, pg 1)                  │
│  → Links to: [UNDISCOVERED COLLECTIBLE]                    │
│  → Companion Resonance: [companion name if applicable]     │
│                                                             │
│  [INSPECT 3D]    [PLAY AUDIO]    [MARK ON MAP]             │
└─────────────────────────────────────────────────────────────┘
```

### Detail View Features

- **3D Model Viewer** (Artifacts only): Full rotation, zoom, and inspection. Some artifacts have hidden details visible only at high zoom — scratched text, hidden compartments, maker's marks
- **Audio Playback** (Audio Recordings): Full playback with waveform visualization. Subtitles toggle. Can be added to a "playlist" for ambient listening during gameplay
- **Text Display** (Train Logs, Faction Intel, Manifest Pages): Full readable text with highlighted keywords that link to Codex entries
- **Blueprint View** (Blueprints): Technical schematic display showing the craftable item, required materials, and crafting station tier
- **Connections**: Automatically generated cross-references to related collectibles, NPCs, locations, and Codex entries. Undiscovered connections show as "[UNDISCOVERED]" — a breadcrumb encouraging further exploration

### Undiscovered Items

Undiscovered collectibles appear as:
- **Zone known**: Shows type, zone, and rarity. No name or content
- **Zone unknown**: Shows type and rarity only
- **Hint system** (optional, toggleable): After 2+ hours in a zone, undiscovered items in that zone show a vague location hint: "Somewhere in the upper levels of this car" or "Accessible only from outside"

---

## Tab 2: Web of Power

### Overview

The Web of Power is a dynamic relationship diagram built from Faction Intel collectibles. It visualizes the power structure of Snowpiercer — who controls what, who opposes whom, and where the vulnerabilities are.

### Layout

```
┌─────────────────────────────────────────────────────────────────┐
│  WEB OF POWER                         Intel: 14/52 (27%)       │
│                                                                 │
│                        ┌──────────┐                             │
│                        │ WILFORD  │                             │
│                   ┌────│ (Engine) │────┐                        │
│                   │    └──────────┘    │                        │
│              controls              controls                     │
│                   │                   │                          │
│            ┌──────────┐        ┌──────────┐                    │
│            │ SANCTUM  │───?────│JACKBOOTS │                    │
│            │ (Order)  │        │(Military)│                    │
│            └──────────┘        └──────────┘                    │
│                   │                   │                          │
│              influences          enforces                       │
│                   │                   │                          │
│            ┌──────────┐        ┌──────────┐                    │
│            │  FIRST   │───$────│ SECOND   │                    │
│            │  CLASS   │        │  CLASS   │                    │
│            └──────────┘        └──────────┘                    │
│                                       │                         │
│                                   exploits                      │
│                                       │                         │
│            ┌──────────┐        ┌──────────┐                    │
│            │BREACHMEN │───?────│  THIRD   │                    │
│            │(Maintain)│        │  CLASS   │                    │
│            └──────────┘        └──────────┘                    │
│                                       │                         │
│                                   oppresses                     │
│                                       │                         │
│                                ┌──────────┐                    │
│                                │  TAIL    │                    │
│                                │ (Tailies)│                    │
│                                └──────────┘                    │
│                                                                 │
│  [?] = Relationship unknown (need more intel)                  │
│  Click any node for faction detail                             │
│  Connections reveal as Intel is collected                       │
└─────────────────────────────────────────────────────────────────┘
```

### Progressive Revelation

The Web of Power starts nearly empty and fills in as Faction Intel is collected:

| Intel Count | Revealed |
|-------------|----------|
| 0 | Empty web, only player's starting faction (Tailies) visible |
| 5 | Adjacent factions appear (Third Class, Jackboots) |
| 10 | All faction nodes visible, some relationships labeled |
| 20 | Key NPCs appear within faction nodes |
| 30 | Hidden relationships revealed (secret alliances, double agents) |
| 40 | Faction vulnerabilities appear (weak points, internal conflicts) |
| 52 (all) | Complete picture — every relationship, every secret, every vulnerability |

### Faction Detail Panel

Clicking a faction node shows:

```
┌─────────────────────────────────────────────┐
│  THIRD CLASS UNION                          │
│  Intel: 5/9 collected                       │
│                                             │
│  Leader: [UNKNOWN - need 7/9]               │
│  Strength: ~620 members                     │
│  Territory: Zone 2 (Cars 16-30)             │
│                                             │
│  Known Relationships:                       │
│  → Supplies workforce to First Class        │
│  → Secret alliance with [UNKNOWN]           │
│  → Historically supported Tail revolts      │
│                                             │
│  Critical Weakness: [LOCKED - need 9/9]     │
│                                             │
│  Key NPCs:                                  │
│  • Foreman Walsh (known)                    │
│  • [UNKNOWN] (need 6/9)                     │
│  • [UNKNOWN] (need 8/9)                     │
│                                             │
│  Dialogue Unlocked: YES (5+ intel)          │
│  Special dialogue options available with    │
│  Third Class NPCs                           │
└─────────────────────────────────────────────┘
```

---

## Tab 3: Manifest

### Overview

The Manifest tab reconstructs the original passenger manifest from collected Manifest Pages. It functions as a searchable database of every person who boarded Snowpiercer.

### Layout

```
┌─────────────────────────────────────────────────────────────────┐
│  PASSENGER MANIFEST                    Pages: 7/35 (20%)       │
│  Reconstructed from recovered pages                            │
│                                                                 │
│  Search: [________________]  Filter: [All Classes ▼]           │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  #    NAME              CLASS    STATUS     CAR         │   │
│  │─────────────────────────────────────────────────────────│   │
│  │  001  [REDACTED]        ████     Active     Engine      │   │
│  │  002  Wilford, Joseph   First    Active     Car 95      │   │
│  │  003  [REDACTED]        First    Deceased   -----       │   │
│  │  ...                                                    │   │
│  │  047  Gilliam, ███████  Tail     Active     Car 5       │   │
│  │  ...                                                    │   │
│  │  156  ████████████████  ████     ████████   ████        │   │
│  │  157  Vasquez, Elena    Second   Missing    Car 55?     │   │
│  │  ...                                                    │   │
│  │  ███  ████████████████  ████     ████████   ████        │   │
│  │  ...                                                    │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ████ = Data not yet recovered (need more Manifest Pages)      │
│                                                                 │
│  Cross-Reference Matches: 3 found                              │
│  → Vasquez, Elena — see: Outside Lab, Audio Recording          │
│  → [REDACTED] #001 — see: Boarding Token #0001                │
│  → Gilliam — see: Faction Intel (Revolt Dossiers)             │
└─────────────────────────────────────────────────────────────────┘
```

### Cross-Reference Engine

The Manifest's key feature is automatic cross-referencing. When a name in the manifest matches:
- A living NPC → Link to their location and faction
- A deceased person → Link to their death circumstances (if known)
- A collectible reference → Link to that collectible
- A companion → Link to companion page with additional context

**Discovery Moments:** Some cross-references reveal surprises:
- A person listed as dead is alive (and an NPC the player has met)
- Two NPCs from different zones share a surname — separated family
- A companion's manifest entry contradicts their stated backstory
- Children's entries marked "Transferred to: [CLASSIFIED]" with Medical Sizing Records cross-reference

### Manifest Truth Quest

At 25+ pages, the player receives the "Manifest Truth" quest marker. The journal adds a new section:

```
┌─────────────────────────────────────────────────┐
│  ⚠ MANIFEST TRUTH                              │
│                                                 │
│  Analysis of recovered pages reveals:           │
│  • 37 children listed as "Transferred"          │
│  • 12 adults listed as "Deceased" are alive     │
│  • 8 adults listed as "Active" are missing      │
│  • Boarding criteria were not random             │
│  • The manifest was destroyed deliberately       │
│                                                 │
│  Why was the manifest destroyed?                │
│  Remaining pages may hold the answer.           │
│                                                 │
│  Progress: 25/35 pages                          │
└─────────────────────────────────────────────────┘
```

---

## Tab 4: Map

### Overview

An interactive map of Snowpiercer showing all visited locations, collectible placements, and traversal routes. Initially sparse, it fills in through exploration and specific collectible discoveries.

### Layout

```
┌─────────────────────────────────────────────────────────────────┐
│  TRAIN MAP                              Cars Visited: 23/103+  │
│                                                                 │
│  Layer: [Interior ▼]  [Rooftop]  [Crawlspace]  [Tunnels]      │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ ZONE 1    │ ZONE 2         │ ZONE 3      │ ZONE 4 ...  │   │
│  │           │                │             │              │   │
│  │ ■■■■■■■■ │ ■■■■■□□□□□□□□□ │ □□□□□□□□□□□ │ □□□□□□□□□□  │   │
│  │ 1  5  10 │ 16    20    25 │ 31    35    │ 49    55    │   │
│  │          │  15             │  30     48  │  62         │   │
│  │ ★3  ●2  │ ★1  ●4  ○12   │ ○18         │ ○14         │   │
│  │          │                │             │              │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ■ = Visited   □ = Unvisited   ★ = Secret Car   ● = Collectible│
│  ○ = Undiscovered collectible (hint mode ON)                   │
│                                                                 │
│  Legend: [Collectibles ✓] [Exits ✓] [Enemies □] [NPCs ✓]     │
│                                                                 │
│  Quick Travel: Available between rest points (4 total)         │
└─────────────────────────────────────────────────────────────────┘
```

### Map Layers

Four map layers can be toggled independently:

1. **Interior**: Standard car-by-car layout with rooms, doors, interactables
2. **Rooftop**: Roof surfaces, access points, hazard zones (wind, tunnel clearance)
3. **Crawlspace**: Under-car grating, hazards, junction boxes, caches
4. **Tunnels**: Maintenance tunnel network, Breachmen territory, blocked passages

### Map Enhancement Sources

| Source | Effect |
|--------|--------|
| Personal exploration | Reveals visited rooms/areas |
| Rebel Archive Complete Train Map | Reveals all car outlines and connections |
| Breachmen FRIENDLY+ | Reveals maintenance tunnel network |
| Faction Intel (zone-specific) | Reveals enemy patrol routes in that zone |
| Yuri's Wire Map (Artifact) | Reveals Zone 1-2 hidden passages |
| Wind Map (Rooftop collectible) | Reveals safe rooftop routes |

---

## Tab 5: Codex

### Overview

The Codex is an automatically populated encyclopedia of game world knowledge. Entries are created and expanded as the player encounters relevant information through gameplay, dialogue, and collectibles.

### Categories

```
┌─────────────────────────────────────────────────────────────────┐
│  CODEX                                    Entries: 47/200+     │
│                                                                 │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐           │
│  │   PEOPLE     │ │  LOCATIONS   │ │ TECHNOLOGY   │           │
│  │   23/80      │ │   12/45      │ │   5/30       │           │
│  └──────────────┘ └──────────────┘ └──────────────┘           │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐           │
│  │   HISTORY    │ │  FACTIONS    │ │  THE WORLD   │           │
│  │   4/25       │ │   3/12       │ │   0/10       │           │
│  └──────────────┘ └──────────────┘ └──────────────┘           │
│                                                                 │
│  Recent Entries:                                                │
│  • Wilford, Joseph (updated - new intel)                       │
│  • The Year 3 Revolt (new)                                     │
│  • Kronole (updated - chemical composition)                    │
│  • Protein Blocks (updated - ingredient revelation)            │
└─────────────────────────────────────────────────────────────────┘
```

### Codex Entry Format

```
┌─────────────────────────────────────────────────────────────────┐
│  KRONOLE                                                        │
│  Category: Technology  │  Sources: 4 collectibles, 2 dialogues │
│─────────────────────────────────────────────────────────────────│
│                                                                 │
│  An industrial narcotic synthesized from chemical waste         │
│  produced by the train's recycling systems. Highly addictive.   │
│                                                                 │
│  ─── KNOWN FACTS ──────────────────────────────────            │
│  • Primarily consumed in the Tail and Third Class              │
│  • Provides temporary combat enhancement (+15% damage, 60s)   │
│  • Addiction develops after 3+ uses (withdrawal debuff)        │
│  • [NEW] Chemical analysis reveals similarities to CW-7        │
│    counter-agent (Source: Outside Lab specimen analysis)        │
│                                                                 │
│  ─── SOURCES ──────────────────────────────────────            │
│  → Train Log: Protein Block Formula (Car 2)                   │
│  → Audio Recording: Glass's Prophecy (Car 12)                 │
│  → Faction Intel: Kronole Distribution Network                │
│  → Dialogue: Dr. Chen (Car 6) - requires Medicine choice      │
│                                                                 │
│  ─── RELATED ──────────────────────────────────────            │
│  → CW-7 (Codex: Technology)                                   │
│  → The Kronole Den (Codex: Locations)                         │
│  → Glass (Codex: People)                                      │
└─────────────────────────────────────────────────────────────────┘
```

### Codex Growth Mechanic

Codex entries grow over time:

1. **Initial entry**: Created when a topic is first encountered. Contains basic, publicly known information
2. **Expanded entry**: Updated when relevant collectibles are found. "[NEW]" tag marks recent additions
3. **Complete entry**: All sources found. Entry receives a completion checkmark. Some entries reveal a "Hidden Truth" section only visible at completion — recontextualizing everything the player thought they knew

**Auto-linking**: The Codex automatically identifies proper nouns and technical terms in collectible text and creates hyperlinks to relevant Codex entries. This creates a wiki-like browsing experience.

---

## UI/UX Specifications

### Visual Style

- **Color palette**: Dark background (charcoal #1a1a2e), warm amber text (#e6b347), ice-blue accents (#7ec8e3)
- **Typography**: Monospace for manifest/logs (typewriter feel), sans-serif for UI elements, hand-drawn for journal margins
- **Rarity borders**: Bronze (#cd7f32), Silver (#c0c0c0), Gold (#ffd700), Platinum (#e5e4e2) with subtle animated glow
- **New item indicator**: Pulsing amber dot, persists until item is viewed
- **Paper texture**: Subtle vellum/parchment overlay on all journal pages — this is a physical object in the game world

### Animation & Transitions

- **Page turns**: Physical page-turn animation when switching between tabs (0.3s, can be skipped)
- **New collectible**: Item slides into its slot with a satisfying click sound
- **Connections revealed**: Golden thread animation draws between linked items (0.5s)
- **Web of Power updates**: Nodes fade in, connections draw themselves, faction colors pulse once
- **Manifest reconstruction**: Page assembles from fragments when enough pages collected in a section
- **Codex updates**: Entry text types itself in (typewriter effect, 0.1s/character, skippable)

### Audio Design

- **Journal open**: Leather-bound book opening sound + soft paper rustle
- **Tab switch**: Page turn + subtle tab click
- **New collectible viewed**: Soft chime, pitch varies by rarity (higher = rarer)
- **Audio Recording playback**: Starts with tape hiss/static, then voice. Background train noise faintly audible
- **Connection discovered**: Musical sting — two notes resolving into harmony
- **Codex update**: Typewriter key sounds, soft mechanical rhythm

### Accessibility Features

- **Text size**: Adjustable (small/medium/large/extra-large), affects all journal text
- **High contrast mode**: Increases border/text contrast, adds underlines to links
- **Audio descriptions**: Toggle for verbal descriptions of 3D artifact models
- **Colorblind modes**: Rarity indicators use shape + color (circle, triangle, diamond, star)
- **Screen reader support**: All journal content exposed to screen readers with proper hierarchy
- **Subtitle options**: Full subtitle customization for Audio Recordings (size, background, speaker labels)
- **Reduced motion**: Disables page-turn animations, connection thread animations, typewriter effects
- **Dyslexia font**: Toggle for OpenDyslexic font throughout journal

### Performance Considerations

- Journal UI loads incrementally — tab content loads on switch, not all at once
- 3D artifact models use LOD system — high detail in viewer, low detail in list
- Audio Recordings stream from disk, not held in memory
- Web of Power graph computed once on open, cached until new intel collected
- Map layers rendered independently — toggle doesn't re-render entire map
- Codex search uses pre-built index, updates on collectible acquisition

### Controller Layout (Default)

| Action | Controller | Keyboard |
|--------|-----------|----------|
| Open Journal | D-Pad Down | J |
| Switch Tab | L1/R1 | Q/E |
| Navigate Items | D-Pad / L-Stick | Arrow Keys / WASD |
| Select/Inspect | A / X | Enter / Space |
| Back | B / Circle | Escape |
| Toggle Layer (Map) | Y / Triangle | Tab |
| Zoom (3D/Map) | R-Stick | Scroll Wheel |
| Rotate (3D) | L-Stick (in viewer) | Mouse Drag |
| Play/Pause Audio | X / Square | P |
| Mark on Map | Y / Triangle | M |
| Search | Start | Ctrl+F |

---

## Completion Rewards

### Percentage-Based Rewards

| Completion % | Reward |
|-------------|--------|
| 25% (62 items) | "Curious" title + Journal gains a "Recent Nearby" section showing closest undiscovered items |
| 50% (124 items) | "Investigator" title + All undiscovered items show their zone in the journal |
| 75% (185 items) | "Archivist" title + Collectible proximity range doubled (all PER tiers) |
| 100% (247 items*) | "Chronicler of Snowpiercer" title + New Game+ exclusive: Narrator Mode (VO commentary on revisited collectibles providing developer/designer insights) |

*Note: 100% requires multiple playthroughs due to choice-dependent collectibles. Single playthrough max: ~235 items (95%).

### Type-Specific Completion

| All of Type | Reward |
|-------------|--------|
| All Train Logs | "Systems Access" for all zones simultaneously |
| All Audio Recordings | "Keen Ear" upgraded — hear through walls (detect NPCs in adjacent rooms) |
| All Artifacts | "Historian" max — unlocks artifact trading post (exchange duplicates in NG+) |
| All Blueprints | "Master Engineer" — all crafted items gain +1 durability tier |
| All Faction Intel | "Spymaster" — Web of Power becomes interactive (can trigger faction events) |
| All Manifest Pages | "Manifest Truth" conclusion — the full, unredacted manifest reveals the final secret |
