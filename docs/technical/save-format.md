# Save File Format & Persistence

## Format Choice: Binary with JSON Header

**Decision: Custom binary format (.sav) with a JSON metadata header.**

### Rationale

| Option | Pros | Cons | Verdict |
|--------|------|------|---------|
| Pure JSON | Human-readable, easy debugging | Large file size (~5-10 MB for full train state), slow parse, easy to cheat-edit | Rejected |
| SQLite | Queryable, partial reads | Overkill for linear save data, larger runtime footprint, WAL mode complicates Steam Cloud | Rejected |
| Pure binary | Compact, fast | Hard to debug, brittle versioning | Partial — too rigid |
| **Binary + JSON header** | Compact payload, readable metadata, version-resilient | Slightly more complex serialization | **Selected** |

### File Structure

```
┌─────────────────────────────────────┐
│ Magic bytes: "SPEE" (4 bytes)       │
│ Format version: uint32              │
│ JSON header length: uint32          │
├─────────────────────────────────────┤
│ JSON Header (UTF-8, uncompressed)   │
│ {                                   │
│   "version": 3,                     │
│   "gameVersion": "0.8.2",           │
│   "timestamp": "2026-02-19T...",    │
│   "playTime": 43200,               │
│   "slotName": "Slot 1",            │
│   "playerName": "...",             │
│   "currentCar": 47,                │
│   "currentZone": "SecondClass",    │
│   "completionPct": 0.42,           │
│   "checksum": "sha256:..."         │
│ }                                   │
├─────────────────────────────────────┤
│ Binary Payload (LZ4 compressed)     │
│ ├─ Global State Block               │
│ ├─ Player State Block               │
│ ├─ Car State Blocks (per-car)       │
│ ├─ NPC State Blocks                 │
│ ├─ Quest State Block                │
│ └─ Companion State Block            │
└─────────────────────────────────────┘
```

### JSON Header

The header is always readable without parsing the binary payload. This enables:
- Save slot UI: show player name, playtime, zone, screenshot thumbnail path without full deserialization
- Steam Cloud conflict resolution: compare timestamps and versions
- Mod/debug tools: inspect save metadata externally

### Binary Payload Sections

#### Global State

| Field | Type | Description |
|-------|------|-------------|
| worldTime | float64 | Elapsed game-world time (seconds) |
| difficulty | uint8 | 0=Passenger, 1=Survivor, 2=EternalEngine, 3=MrWilford |
| revolutionFlags | bitfield[32] | Major story flags |
| factionRep[8] | int16[8] | Faction reputation values (-1000 to +1000) |
| discoveredCars | bitfield[128] | Which cars have been visited |
| globalEventLog | VarArray | Timestamped event records for rumor/consequence systems |

#### Player State

| Field | Type | Description |
|-------|------|-------------|
| position | float32[3] | World position (x, y, z) |
| rotation | float32[3] | Euler angles |
| currentCarIndex | uint16 | Car number (1-1034) |
| stats[6] | uint8[6] | STR, END, CUN, CHA, PER, SRV |
| level | uint8 | Player level |
| xp | uint32 | Current XP |
| health | float32 | Current HP |
| stamina | float32 | Current stamina |
| hunger | float32 | 0.0 (starving) to 1.0 (full) |
| coldExposure | float32 | 0.0 (warm) to 1.0 (hypothermia) |
| morale | float32 | 0.0 to 1.0 |
| kronoleAddiction | float32 | 0.0 (clean) to 1.0 (dependent) |
| inventory | VarArray | Serialized inventory items |
| activePerks | VarArray | Perk IDs and ranks |
| activeEffects | VarArray | Temporary buffs/debuffs with remaining duration |
| equippedWeapon | uint32 | Weapon asset ID |
| equippedArmor | uint32 | Armor asset ID |
| disguise | uint32 | Active disguise ID (0 = none) |

#### Per-Car State

Each car that has been modified from its template stores a delta:

| Field | Type | Description |
|-------|------|-------------|
| carIndex | uint16 | Car number |
| lootState | bitfield | Which containers have been opened/looted |
| doorStates | uint8[] | Door open/closed/locked/broken per door |
| destructibles | bitfield | Which destructible objects are destroyed |
| npcOverrides | VarArray | NPCs with non-default state (dead, moved, alerted) |
| clutter seed | uint32 | Procedural clutter RNG seed (for consistent regeneration) |
| customFlags | VarArray | Car-specific scripted triggers (quest items placed, traps set) |

**Unvisited cars store nothing** — their state is generated from templates + seed on load.

#### NPC State (Global)

| Field | Type | Description |
|-------|------|-------------|
| npcId | uint32 | Unique NPC identifier |
| alive | bool | Permadeath flag |
| currentCar | uint16 | Location |
| disposition | int16 | -100 to +100 toward player |
| memoryEntries | VarArray | What this NPC knows/remembers |
| activeSchedule | uint8 | Current behavior schedule |
| conversationFlags | bitfield | Which dialogue nodes have been seen |

#### Quest State

| Field | Type | Description |
|-------|------|-------------|
| questId | uint32 | Quest identifier |
| status | uint8 | 0=unknown, 1=active, 2=completed, 3=failed, 4=abandoned |
| currentStep | uint16 | Active objective index |
| stepFlags | VarArray | Per-step completion/failure flags |
| choices | VarArray | Recorded player choices (for consequence tracking) |

#### Companion State

| Field | Type | Description |
|-------|------|-------------|
| companionId | uint8 | Companion index (0-11) |
| recruited | bool | In roster |
| alive | bool | Permadeath |
| loyalty | int16 | -100 to +100 |
| internalState | uint8 | Hostile through Bonded (7 levels) |
| inParty | bool | Currently in active party |
| personalQuestStep | uint16 | Progress in personal quest |
| romanceState | uint8 | 0=none, 1=interested, 2=active, 3=committed |
| giftHistory | VarArray | Items gifted, for diminishing returns |

---

## Save Slot Configuration

| Parameter | Value |
|-----------|-------|
| Manual save slots | 10 |
| Quick save slots | 3 (rotating) |
| Autosave slots | 3 (rotating) |
| Total slots | 16 |
| Estimated save size | 500 KB - 2 MB (compressed) |
| Max total save storage | ~32 MB |

### Autosave Triggers

Autosave fires on any of these events (with a 60-second cooldown between saves):

| Trigger | Condition |
|---------|-----------|
| Car transition | Player moves between cars |
| Quest completion | Any quest objective completed |
| Companion recruitment | New companion joins roster |
| Major choice | Irreversible dialogue branch taken |
| Timed interval | Every 10 minutes of gameplay (configurable) |
| Pre-combat | Entering a scripted combat encounter |

Autosave is **asynchronous** — serialization runs on a background thread with a snapshot of game state. The game does not pause during autosave. A brief save indicator appears in the corner.

---

## Versioning & Migration

### Version Strategy

Each save format version is an integer (`"version": 3`). When the format changes:

1. **Additive changes** (new fields): Default values are injected during load. No version bump required if the deserializer handles missing fields gracefully.
2. **Breaking changes** (removed/retyped fields): Version number increments. A migration function converts version N to version N+1.

### Migration Chain

```
Load save → Read version → Apply migrations sequentially → Current version
```

```cpp
// Migration registry
TMap<int32, TFunction<void(FArchive&)>> Migrations = {
    {1, MigrateV1ToV2},  // Added kronoleAddiction field
    {2, MigrateV2ToV3},  // Refactored faction rep from 5 to 8 factions
};
```

### Forward Compatibility

Saves from newer game versions **cannot** be loaded in older versions. The loader checks `gameVersion` and warns the player.

---

## Steam Cloud Compatibility

| Requirement | Implementation |
|-------------|----------------|
| File location | `<SteamUserDir>/SnowpiercerEE/Saves/` |
| File pattern | `slot_<N>.sav`, `quick_<N>.sav`, `auto_<N>.sav` |
| Conflict resolution | Compare `timestamp` in JSON header; newest wins with user prompt |
| Max cloud storage | ~50 MB (well within Steam Cloud limits) |
| Sync frequency | On save write and on game launch |
| Cross-platform | Binary format is endian-explicit (little-endian); no platform-specific paths in save data |

### Steam Cloud Integration

```cpp
// ISteamRemoteStorage integration
void USaveSubsystem::SyncToCloud(const FString& SlotPath)
{
    // 1. Write local file
    // 2. Call SteamRemoteStorage()->FileWrite()
    // 3. On conflict: compare JSON header timestamps
    // 4. Present conflict UI if timestamps diverge
}
```

### Integrity

- **Checksum**: SHA-256 of the binary payload stored in the JSON header
- **Validation**: On load, recompute checksum and compare. Mismatch → warn player, offer to load anyway (modded saves) or reject
- **Corruption recovery**: If the binary payload fails decompression, fall back to the most recent autosave
