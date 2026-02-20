# Mod Support Scope

## Overview

Mod support is available at 1.0 launch (not during Early Access). The modding framework prioritizes content-additive and cosmetic modifications while protecting narrative integrity.

---

## Supported Mod Categories

### Tier 1: Full Support (official tools, documentation, Workshop integration)

| Category | Description | Technical Approach |
|----------|-------------|-------------------|
| **Custom Cars** | New train car interiors with NPCs, loot, and gameplay encounters | UE5 level template system; modders build within standardized car geometry (width, height, connection points) |
| **Weapon Skins** | Visual replacements for weapon models and textures | Material instance swap system; weapon mesh slot architecture |
| **Character Skins** | Player and companion appearance alternatives | Skeletal mesh compatibility layer; cosmetic-only slot system |
| **UI Themes** | HUD color schemes, font replacements, layout modifications | UMG widget blueprint overrides; theme data asset system |
| **Sound Packs** | Replacement ambient audio, environmental sounds, music | Tagged audio asset system (by car type, zone, context); WAV/OGG format |

### Tier 2: Supported (documentation, Workshop integration, limited tools)

| Category | Description | Technical Approach |
|----------|-------------|-------------------|
| **Dialogue Packs** | Additional dialogue trees for existing NPCs; new NPC dialogue | JSON-based dialogue format with branching logic and condition checks |
| **Challenge Modes** | Custom rulesets, win conditions, modifiers (e.g., all-stealth, time trial, pacifist enforcement) | Lua scripting layer for game rule parameters; predefined modifier hooks |
| **Item Packs** | New weapons, armor, consumables with custom stats | Data table extension system; items follow existing tier framework |

### Tier 3: Community (no official tools, but not blocked)

| Category | Description | Notes |
|----------|-------------|-------|
| **Reshade/ENB presets** | Post-processing visual overhauls | Standard UE5 compatibility; not officially supported but not blocked |
| **Config tweaks** | INI-based performance and visual tuning | Standard UE5 config exposure |
| **Save editors** | External tools for save file modification | Save format documented; community-maintained tools expected |

---

## Protected Systems (Not Moddable)

| System | Reason |
|--------|--------|
| **Hidden Ledger** | Moral choice tracking is the game's narrative backbone; modifying it would invalidate ending logic |
| **Ending conditions** | Ending determination must remain consistent with the authored experience |
| **Achievement triggers** | Prevent achievement cheating via mods |
| **Zone boss encounters** | Boss AI, dialogue, and resolution paths are tightly authored narrative content |
| **Companion quest chains** | Personal quests are deeply integrated with the main narrative; modifying them risks incoherence |
| **Core progression** | Zone gating, car-by-car progression, and the revolution's narrative arc |
| **Anti-cheat integration** | If applicable to leaderboards or difficulty-verified achievements |

### Rationale
The line is drawn at **narrative integrity**. Anything cosmetic, additive, or systemic (new content, new rules, new looks) is supported. Anything that alters the authored moral choice system or its consequences is protected. Players should be able to add cars, not rewrite Wilford.

---

## Mod SDK: SnowpiercerEE Mod Kit

### Distribution
- Free standalone tool on Steam (separate app ID)
- Includes UE5 editor subset configured for car creation
- Ships with documentation, sample car project, and asset library

### Features
- **Car Builder**: Simplified UE5 level editor with car geometry templates, lighting presets, and NPC placement tools
- **Asset Browser**: Access to the game's modular kit pieces (walls, floors, furniture, props) for use in custom cars
- **Dialogue Editor**: Visual node-based editor for creating dialogue trees with condition branches
- **Challenge Editor**: Rule modifier interface for creating custom challenge modes
- **Workshop Publisher**: Direct upload to Steam Workshop with metadata tagging, thumbnail generation, and category selection
- **Mod Validator**: Automated testing tool that checks mods for compatibility issues, missing assets, and performance concerns

### Documentation
- In-tool documentation and tooltips
- Online wiki (community-maintained, developer-seeded)
- Sample projects: one complete custom car, one dialogue pack, one challenge mode
- Video tutorials for common mod types

---

## Workshop Integration

### Upload Flow
1. Create mod in Mod Kit
2. Run Mod Validator (required before upload)
3. Set metadata: category, tags, description, thumbnail
4. Publish to Workshop (public or unlisted)
5. Updates via Mod Kit re-publish

### Player Experience
- Subscribe to mods from Workshop or in-game browser
- Mod Manager accessible from main menu: enable/disable mods, set load order
- Automatic conflict detection with player notification
- Save files track mod dependencies; loading a save with missing mods prompts the player
- Mods disabled by default on Mr. Wilford difficulty (can be overridden with a warning)

### Curation
- Developer-curated "Featured Mods" collection updated monthly
- Community rating and review system (standard Workshop)
- Report system for inappropriate content
- No mod approval gate — standard Workshop moderation applies

---

## Post-Launch Mod Support

### Commitment
- Mod Kit updates alongside major game patches
- Asset library expanded with each zone update (during Early Access)
- Community feedback incorporated into Mod Kit quality-of-life updates
- No plans to monetize mods (no paid mods, no mod marketplace)

### Community Engagement
- Dedicated modding channel in official Discord
- Monthly mod spotlight in community updates
- Developer engagement with high-quality mod creators for potential official integration
