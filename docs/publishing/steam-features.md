# Steam Platform Features

## Steam Cloud Saves

### Sync Strategy
- **Full save sync**: All save files synced via Steam Cloud
- **Save location**: `%APPDATA%/SnowpiercerEE/Saves/` (Windows)
- **Estimated save size**: ~5-15 MB per save slot (world state, Hidden Ledger, companion states, collectible tracking, inventory)
- **Save slots**: 10 manual save slots + 3 autosave slots per difficulty + 1 quicksave
- **Mr. Wilford difficulty**: Single permadeath save slot, still cloud-synced (deletion on death syncs immediately to prevent save-scumming via cloud restore)

### Synced Data
- Save game files (all slots)
- Player settings and preferences (accessibility, controls, audio, display)
- Achievement progress counters (collectible tracking, kill counts)
- Unlocked gallery content (concept art, soundtrack tracks)

### Conflict Resolution
- Most recent save wins by default
- Prompt player on conflict with option to choose local or cloud version
- Display save metadata (timestamp, zone, playtime) to help player decide

---

## Steam Trading Cards

### Card Set (8 cards, craft into 1 badge)

| # | Card Name | Art Description |
|---|-----------|-----------------|
| 1 | **The Tail** | Cramped, rust-toned interior; figures huddled around a protein block distributor |
| 2 | **Third Class** | Industrial corridor with conveyor belts and worker uniforms; Union graffiti |
| 3 | **Second Class** | Courtroom interior with amber lighting; the scales of justice motif |
| 4 | **The Spine** | Maintenance tunnels; pipes, steam, and engineering diagrams |
| 5 | **First Class** | Gilded ballroom; champagne glasses and snow visible through panoramic windows |
| 6 | **The Sanctum** | Cathedral car interior; Kronole vapors and candlelight |
| 7 | **The Engine** | Sacred machinery; the Eternal Engine in motion |
| 8 | **The Frozen World** | Exterior panorama; the train crossing a frozen landscape with hints of thaw |

### Badge Rewards
- **Level 1 Badge**: Profile background (train exterior in blizzard)
- **Level 2 Badge**: Profile background (Engine interior, warm amber)
- **Level 3 Badge**: Animated emoticon set (train wheel spinning)
- **Level 4 Badge**: Animated profile background (train moving through snow)
- **Level 5 Badge**: Showcase item (Wilford's pocket watch)
- **Foil Badge**: Animated profile background (The Bridge ending panorama)

### Emoticons (15 total, unlocked via booster packs and badge crafting)
- :snowpiercer_train:
- :snowpiercer_engine:
- :snowpiercer_kronole:
- :snowpiercer_axe:
- :snowpiercer_protein:
- :snowpiercer_cold:
- :snowpiercer_wilford:
- :snowpiercer_revolution:
- :snowpiercer_tailfire:
- :snowpiercer_disguise:
- :snowpiercer_rivet:
- :snowpiercer_compass:
- :snowpiercer_ledger:
- :snowpiercer_atlas:
- :snowpiercer_thaw:

---

## Steam Workshop Support

### Supported Mod Types

| Category | Description | Technical Notes |
|----------|-------------|-----------------|
| **Custom Cars** | New train car interiors with gameplay | UE5 level templates; modders build within car geometry constraints |
| **Weapon Skins** | Visual weapon model/texture replacements | Material swap system; no gameplay impact |
| **Character Skins** | Player and companion appearance alternatives | Skeletal mesh compatibility required |
| **Dialogue Packs** | Additional dialogue trees for existing NPCs | JSON-based dialogue format with branching logic |
| **Challenge Modes** | Custom rulesets and win conditions | Lua scripting layer for game rule modification |
| **Sound Packs** | Replacement ambient audio, music | WAV/OGG format; tagged by car type for procedural assignment |
| **UI Themes** | HUD color schemes and layout modifications | UMG widget blueprint overrides |

### Workshop Infrastructure
- **Mod SDK**: Shipped as a separate free tool on Steam (SnowpiercerEE Mod Kit)
- **Documentation**: In-game modding wiki accessible from main menu
- **Upload**: Direct Workshop upload from Mod Kit with metadata tagging
- **Curated collections**: Developer-curated "Featured Mods" collection updated monthly
- **Mod load order**: Player-configurable; conflicts flagged automatically
- **Save compatibility**: Mods that affect saves flag this clearly; save files track mod dependency list

### Modding Scope Boundaries
- **Supported**: Visual mods, new car content, dialogue, audio, UI, challenge rules
- **Not supported**: Core engine modifications, multiplayer, new zone bosses, Hidden Ledger modifications
- **Rationale**: The moral choice system and narrative integrity are protected; cosmetic and content-additive mods are fully supported

---

## Additional Steam Features

### Steam Input
- Full Steam Input API integration
- Community controller configurations enabled
- Default configurations for Xbox, PlayStation, and Steam Deck controllers
- Gyro aim support for applicable controllers

### Steam Deck Compatibility
- Target: Steam Deck Verified
- UI scales to handheld resolution (1280x800)
- Text size meets Deck readability requirements
- All controls mappable to Deck inputs
- Performance target: 30 FPS stable on Deck (medium settings)
- Suspend/resume support via Steam Cloud save sync

### Rich Presence
- Display current zone and car number in Steam friends list
- Show playtime and current difficulty tier
- Example: "Zone 3 - Car 42 | Survivor Difficulty | 12.4 hrs"

### Steam Overlay
- Shift+Tab overlay compatible
- In-game browser for Workshop browsing
- Screenshot manager integration (F12 default)
