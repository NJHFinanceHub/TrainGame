# Lower Transport Deck — Design Overview

## Concept

A sub-level cargo system running beneath cars 15-82 (Third Class through First Class). The transport deck is the train's logistical backbone — food, medicine, luxury goods, and contraband all move through this cramped industrial corridor via a mini-rail network.

For the player, the transport deck serves three purposes:
1. **Stealth bypass** — circumvent upper-level checkpoints and guards
2. **Smuggling gameplay** — work with (or against) the Kronole Network
3. **Environmental storytelling** — see the inequality engine from below

## Physical Layout

### Dimensions (10x scale)
- **Segment size**: 120m x 40m x 12m (cramped compared to 30m-tall upper cars)
- **Z position**: Lower deck floor sits 13.5m below upper deck floor level
- **Total span**: 68 segments across cars 15-82 (~8.8km of lower deck)

### Zones

| Zone | Cars | Theme | Security |
|------|------|-------|----------|
| Third Class Lower | 15-35 | Food distribution, water pipes, raw | Low |
| Second Class Lower | 36-55 | Cargo sorting, Night Car cellars | Low-Moderate |
| Working Spine Lower | 56-70 | Heavy machinery, loud, industrial | Moderate |
| First Class Lower | 71-82 | Luxury transit, smuggling, security | High |

### Access Points

Players enter/exit the lower deck via three types of vertical connections:

| Type | Speed | Noise | Usage |
|------|-------|-------|-------|
| Floor Hatch | Slow (climb) | Low | Stealth entry/exit |
| Freight Elevator | Fast | High | Cargo transport, alerting |
| Maintenance Stairs | Medium | Medium | Standard traversal |

## Mini-Rail System

### Track Layout
- **Main line**: Single track running full length (cars 15-82)
- **Passing sidings**: At Distribution Hubs (cars 20, 28, 35, 50, 60, 70, 74, 82)
- **Junctions**: Player-switchable at siding locations

### Carts
- Small flatbed carts, ~6m x 2.5m
- Speed: 0-12 m/s (controllable by player when riding)
- Noisy — security can detect moving carts (8m noise radius)
- Can carry cargo (for smuggling missions)

### Player Mechanics
- Board stationary carts at boarding points
- Accelerate/brake with standard movement controls
- Switch junctions before reaching them (timing-based)
- Jump off at any speed (damage scales with speed)

## Security System

### Automated Security (by zone)
- **Third Class Lower**: Pressure plates only
- **Second Class Lower**: Pressure plates + laser tripwires
- **Working Spine Lower**: Tripwires + security cameras
- **First Class Lower**: All types + motion sensors + patrol drones

### Alert Levels
| Level | Trigger | Effect | Decay |
|-------|---------|--------|-------|
| Clear | Default | Normal movement | — |
| Suspicious | Pressure plate | Guards look around | 30s |
| Alert | Tripwire/camera/sensor | Guards investigate | 60s |
| Lockdown | Drone detection | Doors lock, reinforcements | 120s |

### Disabling Security (Cunning skill checks)
| Device | Required CUN | Effect |
|--------|-------------|--------|
| Pressure Plate | 3 | Disable for 60s |
| Laser Tripwire | 4 | Cut wire permanently |
| Motion Sensor | 5 | Loop signal for 45s |
| Security Camera | 6 | Blind camera for 30s |
| Patrol Drone | 7 | Redirect drone path |

## Smuggling Gameplay

### Kronole Network Routes
Specific segments are marked as smuggling routes (purple Kronole markers visible with Perception 5+):

| Route | From | To | Cargo |
|-------|------|----|-------|
| Precursor Run | Car 42 (Workshop) | Car 30 (Night Car) | Raw Kronole chemicals |
| Product Line | Car 30 (Night Car) | Car 59 (Cigar Lounge) | Refined Kronole |
| Arms Channel | Car 31 (Drawers) | Car 20 (Distribution) | Hidden weapons |
| Pass Forge | Car 45 (Tailor) | Car 64 (Penthouse) | Forged access passes |

### Smuggling Missions
1. **Courier**: Transport cargo from A to B without triggering security
2. **Intercept**: Steal from a moving cart before it reaches destination
3. **Plant**: Hide contraband on a legitimate cargo cart
4. **Expose**: Report smuggling routes to Jackboots (faction consequence)

## Notable Segments

### Distribution Hub A (Car 20)
Multi-track junction where cargo gets sorted. Has floor hatch, freight elevator, and maintenance stairs. Central hub for Third Class supplies.

### Night Car Cellar (Car 30)
Hidden beneath the Night Car. Kronole Network's primary lab — if you know where to look. No direct hatch access (stairs only from a hidden panel in the Night Car).

### Penthouse Vault (Car 64)
Heavy security zone beneath the penthouses. Personal safes, valuable cargo, and the primary First Class smuggling dead drop. Security level 3 — drones patrol here.

### Bay Door Mechanism (Car 74)
Massive gears controlling the bay door. Exterior access possible (connects to ExteriorTraversalComponent). Narrative-critical for multiple endings.

## Stealth Integration

The transport deck is the primary alternative route for stealth-focused players:

- **Bypass checkpoints**: Upper-level Jackboot checkpoints at car boundaries can be avoided entirely by descending to the lower deck
- **Noise management**: Machinery masking (3m radius) partially covers player movement
- **Crawlspace connection**: CrawlspaceComponent and TransportDeckComponent can chain — enter crawlspace, drop to transport deck
- **Cart concealment**: Hide among cargo on a moving cart (Perception check by guards)

## Implementation Files

| File | Purpose |
|------|---------|
| `Exploration/TransportDeckTypes.h` | Enums, structs: cargo, access, security, cart types |
| `Exploration/TransportDeckComponent.h/.cpp` | Player component: access, movement, cart riding, stealth |
| `Exploration/MiniRailSubsystem.h/.cpp` | World subsystem: tracks, carts, security, cargo routing |
| `DataTables/DT_Cargo.json` | Cargo manifest definitions (18 entries) |
| `Scripts/build_transport_deck.py` | UE5 editor level builder (68 segments) |

## Streaming Integration

The lower deck uses the same streaming subsystem pattern as upper cars:
- Each segment registers via `USEECarStreamingSubsystem::RegisterCarLevel`
- Uses a separate level name namespace: `TransportDeck_Seg{N}`
- 3-car window applies to deck segments independently
- Entering a deck access point triggers streaming for adjacent lower deck segments
