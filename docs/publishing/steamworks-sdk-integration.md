# Steamworks SDK Integration — Technical Implementation Guide

## Overview

This document covers the technical integration of the Steamworks SDK into the SnowpiercerEE UE5.7 project. It serves as the implementation reference for all Steam platform features.

**Steam App ID**: (To be assigned — use `480` for development/testing)
**Demo App ID**: (Separate App ID required for free demo)
**Mod Kit App ID**: (Separate App ID required for SnowpiercerEE Mod Kit)

---

## 1. UE5 Plugin Configuration

### Enable OnlineSubsystemSteam

Add to `SnowpiercerEE.uproject`:

```json
{
    "Name": "OnlineSubsystemSteam",
    "Enabled": true
},
{
    "Name": "SteamController",
    "Enabled": true
},
{
    "Name": "SteamSockets",
    "Enabled": true
}
```

### DefaultEngine.ini Additions

```ini
[OnlineSubsystem]
DefaultPlatformService=Steam

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=480
bRelaunchInSteam=true
GameServerQueryPort=27015
bVACEnabled=false
GameVersion=0.1.0

[/Script/Engine.GameEngine]
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="OnlineSubsystemSteam.SteamNetDriver",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver")

[/Script/OnlineSubsystemSteam.SteamNetDriver]
NetConnectionClassName="OnlineSubsystemSteam.SteamNetConnection"
```

### Build.cs Module Dependencies

Add to `SnowpiercerEE.Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "OnlineSubsystem",
    "OnlineSubsystemSteam",
    "OnlineSubsystemUtils",
    "Steamworks"
});
```

### steam_appid.txt

Place `steam_appid.txt` in project root (for development only — removed for shipping builds):
```
480
```

---

## 2. Steam Initialization Subsystem

### USEESteamSubsystem

GameInstance subsystem that initializes Steam and provides centralized access to all Steam features.

```cpp
// SEESteamSubsystem.h
UCLASS()
class SNOWPIERCERDESIGNERENGINE_API USEESteamSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Check if Steam is properly initialized */
    UFUNCTION(BlueprintPure, Category = "Steam")
    bool IsSteamAvailable() const;

    /** Get the player's Steam display name */
    UFUNCTION(BlueprintPure, Category = "Steam")
    FString GetSteamPlayerName() const;

    /** Get Steam ID as string */
    UFUNCTION(BlueprintPure, Category = "Steam")
    FString GetSteamID() const;

    /** Get the Steam language setting */
    UFUNCTION(BlueprintPure, Category = "Steam")
    FString GetSteamLanguage() const;

    /** Access achievement manager */
    UFUNCTION(BlueprintPure, Category = "Steam")
    USEEAchievementManager* GetAchievementManager() const;

    /** Access rich presence manager */
    UFUNCTION(BlueprintPure, Category = "Steam")
    USEERichPresenceManager* GetRichPresenceManager() const;

    /** Access cloud save manager */
    UFUNCTION(BlueprintPure, Category = "Steam")
    USEECloudSaveManager* GetCloudSaveManager() const;

private:
    bool bSteamInitialized = false;

    UPROPERTY()
    TObjectPtr<USEEAchievementManager> AchievementManager;

    UPROPERTY()
    TObjectPtr<USEERichPresenceManager> RichPresenceManager;

    UPROPERTY()
    TObjectPtr<USEECloudSaveManager> CloudSaveManager;
};
```

### Initialization Flow

1. `USEESteamSubsystem::Initialize()` checks `SteamAPI_Init()` via UE5's `IOnlineSubsystem::Get(STEAM_SUBSYSTEM)`
2. If Steam is unavailable (non-Steam launch), all Steam features gracefully degrade — game is fully playable without Steam
3. Achievement, Rich Presence, and Cloud Save managers are created only if Steam is available
4. `SteamAPI_RunCallbacks()` is handled by UE5's OnlineSubsystemSteam tick

---

## 3. Achievement System Implementation

See `steam-achievements.md` for the full 68-achievement design. This section covers the technical implementation.

### USEEAchievementManager

```cpp
UCLASS()
class USEEAchievementManager : public UObject
{
    GENERATED_BODY()

public:
    void Initialize();

    /** Unlock a binary achievement */
    UFUNCTION(BlueprintCallable, Category = "Achievements")
    void UnlockAchievement(FName AchievementID);

    /** Update progress on a stat-based achievement */
    UFUNCTION(BlueprintCallable, Category = "Achievements")
    void UpdateStat(FName StatName, int32 Value);

    /** Increment a stat by 1 */
    UFUNCTION(BlueprintCallable, Category = "Achievements")
    void IncrementStat(FName StatName);

    /** Check if an achievement is unlocked (local cache) */
    UFUNCTION(BlueprintPure, Category = "Achievements")
    bool IsAchievementUnlocked(FName AchievementID) const;

    /** Get current stat value */
    UFUNCTION(BlueprintPure, Category = "Achievements")
    int32 GetStatValue(FName StatName) const;

    /** Reset all achievements (dev only) */
    void ResetAllAchievements();

private:
    /** Local cache of unlocked achievements */
    TSet<FName> UnlockedAchievements;

    /** Local cache of stat values */
    TMap<FName, int32> StatCache;

    /** Queue unlock notification for display */
    void QueueUnlockNotification(FName AchievementID);
};
```

### Achievement ID Mapping

Steam achievement API names follow the pattern: `ACH_<CATEGORY>_<NUMBER>`

| Category | Prefix | Example |
|----------|--------|---------|
| Story | `ACH_STORY_` | `ACH_STORY_ZONE1_COMPLETE` |
| Combat | `ACH_COMBAT_` | `ACH_COMBAT_100_MELEE_KILLS` |
| Stealth | `ACH_STEALTH_` | `ACH_STEALTH_GHOST_ZONE` |
| Exploration | `ACH_EXPLORE_` | `ACH_EXPLORE_SECRET_CAR_1` |
| Social | `ACH_SOCIAL_` | `ACH_SOCIAL_CHA_18_CHECK` |
| Companion | `ACH_COMP_` | `ACH_COMP_ROMANCE_COMPLETE` |
| Collectible | `ACH_COLLECT_` | `ACH_COLLECT_ALL_TRAIN_LOGS` |

### Stats for Progress-Tracking Achievements

| Stat Name | Type | Achievement Trigger |
|-----------|------|-------------------|
| `STAT_MELEE_KILLS` | INT | >= 100 → `ACH_COMBAT_100_MELEE_KILLS` |
| `STAT_BODIES_HIDDEN` | INT | >= 25 → `ACH_STEALTH_BODY_HIDER` |
| `STAT_CROWD_BLENDS` | INT | >= 10 → `ACH_STEALTH_CROWD_BLEND` |
| `STAT_VENTS_CRAWLED` | INT | >= 20 → `ACH_STEALTH_VENT_CRAWLER` |
| `STAT_ROOFTOP_CARS` | INT | >= 10 → `ACH_EXPLORE_ROOFTOP_10` |
| `STAT_WINDOWS_VIEWED` | INT | >= 10 → `ACH_EXPLORE_WINDOW_WATCHER` |
| `STAT_INTERCOMS_DISABLED` | INT | >= 10 → `ACH_STEALTH_RADIO_SILENCE` |
| `STAT_CORRIDOR_FIGHTS` | INT | >= 10 → `ACH_COMBAT_CORRIDOR_FIGHTER` |
| `STAT_TRAIN_LOGS` | INT | >= 48 → `ACH_COLLECT_ALL_TRAIN_LOGS` |
| `STAT_AUDIO_RECORDINGS` | INT | >= 42 → `ACH_COLLECT_ALL_AUDIO` |
| `STAT_ARTIFACTS` | INT | >= 38 → `ACH_COLLECT_ALL_ARTIFACTS` |
| `STAT_BLUEPRINTS` | INT | >= 32 → `ACH_COLLECT_ALL_BLUEPRINTS` |
| `STAT_FACTION_INTEL` | INT | >= 52 → `ACH_COLLECT_ALL_INTEL` |
| `STAT_MANIFEST_PAGES` | INT | >= 35 → `ACH_COLLECT_ALL_MANIFEST` |
| `STAT_SECRET_CARS` | INT | >= 9 → `ACH_EXPLORE_ALL_SECRET_CARS` |
| `STAT_COLLECTIBLES_TOTAL` | INT | >= 247 → `ACH_COLLECT_ALL` |

### Integration Points

Achievement triggers are distributed across gameplay subsystems:

| Subsystem | Achievements Triggered |
|-----------|----------------------|
| `SEEQuestManager` | Story progression (zone completions, endings) |
| `SEECombatComponent` | Kill counts, corridor fights, disarms, non-lethal runs |
| `StealthComponent` | Ghost runs, crowd blends, disguise duration |
| `CollectibleJournalSubsystem` | All collectible-tracking achievements |
| `SecretCarManager` | Secret car discoveries |
| `SEEFactionManager` | Faction reputation achievements |
| `CompanionRosterSubsystem` | Recruitment, companion quests, romances |
| `ExteriorTraversalComponent` | Rooftop traversal, tunnel survival |
| `SEEDialogueManager` | Dialogue skill check achievements |

### Steamworks Partner Configuration

The following must be configured in the Steamworks partner portal:

1. **Achievement definitions**: All 68 achievements with names, descriptions, icons (locked/unlocked), hidden flag
2. **Stat definitions**: All 16 stats with types and default values
3. **Achievement icons**: 68 × 2 (locked + unlocked) = 136 PNG icons, 256×256 pixels each

---

## 4. Steam Cloud Save Integration

### USEECloudSaveManager

Works alongside the existing `USEESaveGameSubsystem` to sync saves to Steam Cloud.

```cpp
UCLASS()
class USEECloudSaveManager : public UObject
{
    GENERATED_BODY()

public:
    void Initialize();

    /** Sync local saves to Steam Cloud */
    UFUNCTION(BlueprintCallable, Category = "CloudSave")
    void SyncToCloud();

    /** Download saves from Steam Cloud */
    UFUNCTION(BlueprintCallable, Category = "CloudSave")
    void SyncFromCloud();

    /** Check for save conflicts */
    UFUNCTION(BlueprintCallable, Category = "CloudSave")
    bool HasConflicts() const;

    /** Resolve conflict — keep local or cloud version */
    UFUNCTION(BlueprintCallable, Category = "CloudSave")
    void ResolveConflict(FName SlotName, bool bKeepLocal);

    /** Get cloud storage quota usage */
    UFUNCTION(BlueprintPure, Category = "CloudSave")
    void GetCloudQuota(int64& TotalBytes, int64& AvailableBytes) const;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCloudConflict, FName, SlotName, FDateTime, CloudTimestamp);
    UPROPERTY(BlueprintAssignable, Category = "CloudSave")
    FOnCloudConflict OnCloudConflict;
};
```

### Steamworks Cloud Configuration

In `steam_appid.txt` partner config, set the following Auto-Cloud paths:

| Path Pattern | Platform | Root |
|-------------|----------|------|
| `Saves/*.sav` | All | `<user>/AppData/Local/SnowpiercerEE/Saved/SaveGames/` |
| `Settings/UserSettings.json` | All | `<user>/AppData/Local/SnowpiercerEE/Saved/Config/` |

**Cloud quota request**: 100 MB (16 save slots × ~2 MB each + settings + achievement progress = ~40 MB typical, 100 MB maximum)

### Mr. Wilford Difficulty — Permadeath Cloud Sync

Special handling for the permadeath difficulty:
- On player death: delete save file locally AND call `ISteamRemoteStorage::FileDelete()` immediately
- Prevents save-scumming via Steam Cloud restore
- The `MrWilfordDeath` event triggers immediate cloud sync before showing death screen

---

## 5. Rich Presence Implementation

### USEERichPresenceManager

```cpp
UCLASS()
class USEERichPresenceManager : public UObject
{
    GENERATED_BODY()

public:
    void Initialize();

    /** Update rich presence with current game state */
    UFUNCTION(BlueprintCallable, Category = "RichPresence")
    void UpdatePresence(int32 ZoneNumber, int32 CarNumber, ESEEDifficulty Difficulty, float PlaytimeHours);

    /** Clear rich presence (main menu, loading) */
    UFUNCTION(BlueprintCallable, Category = "RichPresence")
    void ClearPresence();

private:
    /** Throttle updates to avoid API spam */
    float LastUpdateTime = 0.f;
    static constexpr float UpdateInterval = 30.f; // seconds
};
```

### Rich Presence Strings

Configure in Steamworks partner portal under Rich Presence Localization:

```vdf
"tokens"
{
    "status_playing"    "#StatusPlaying"
    "status_menu"       "#StatusMenu"
}
```

Localized strings:
```
#StatusPlaying = "Zone %zone% - Car %car% | %difficulty% Difficulty | %playtime% hrs"
#StatusMenu = "In Menus"
```

### Integration Points

Rich Presence updates on:
- Car transitions (`SEECarStreamingSubsystem::OnCarTransition`)
- Zone transitions
- Main menu entry/exit
- Game pause/resume

### Zone Name Mapping

| Zone | Display Name |
|------|-------------|
| 1 | The Tail |
| 2 | Third Class |
| 3 | Second Class |
| 4 | Working Spine |
| 5 | First Class |
| 6 | The Sanctum |
| 7 | The Engine |

---

## 6. Steam Input / Controller Support

See `controller-bindings.md` for the full gamepad binding configuration.

### UE5 Enhanced Input Migration

The project currently uses legacy `DefaultInput.ini` axis/action mappings. For Steam Input and modern controller support, migrate to UE5 Enhanced Input:

1. Create `InputMappingContext` assets for each context (Exploration, Combat, Stealth, Dialogue, Menu)
2. Create `InputAction` assets for each action
3. Map keyboard/mouse and gamepad bindings per context
4. Steam Input wraps the OS-level controller input — UE5 Enhanced Input handles the game-side mapping

### Steam Input Action Sets

| Action Set | Context | Description |
|-----------|---------|-------------|
| `InGame_Exploration` | Default walking/looking | Movement, interaction, inventory |
| `InGame_Combat` | Combat mode active | Attack, block, dodge, quick slots |
| `InGame_Stealth` | Stealth mode active | Crouch, peek, takedown |
| `InGame_Dialogue` | Dialogue UI open | Navigate choices, confirm, back |
| `Menu` | Any menu screen | Navigate, confirm, back, tabs |

### Steam Deck Verification Checklist

- [ ] All gameplay accessible via controller (no keyboard-only features)
- [ ] Text readable at 1280x800 (minimum 18px equivalent)
- [ ] No reliance on mouse hover states
- [ ] Steam Input glyphs display correctly
- [ ] Default controller config ships with game
- [ ] Suspend/resume works without issues (cloud save sync)
- [ ] Performance: 30 FPS stable on Medium at native resolution
- [ ] On-screen keyboard supported for character name entry
- [ ] No launcher or external dependencies

---

## 7. Steam Overlay & Screenshot Integration

### Screenshot Manager

```ini
; DefaultEngine.ini additions
[/Script/Engine.Engine]
bAllowScreenshots=true
```

Hook into Steam's screenshot notification to add game metadata (zone, car, playtime) as screenshot caption via `ISteamScreenshots::SetLocation()`.

### Overlay Compatibility

- Ensure Shift+Tab opens Steam Overlay without conflicting with game keybinds
- Shift is Sprint modifier — no conflict (Tab is not bound)
- In-game pause when overlay opens (single-player, no disadvantage)

---

## 8. Anti-Tamper / VAC Considerations

**VAC is disabled** (`bVACEnabled=false`) — single-player game with mod support. Anti-cheat is counterproductive when mods are officially supported.

Achievement integrity is handled by:
- Server-side stat tracking through Steamworks (stats are authoritative)
- Mods cannot directly call achievement unlock functions (protected API surface)
- Mr. Wilford difficulty achievements verified by save file integrity hash

---

## 9. Build & Distribution

### SteamPipe Depot Configuration

| Depot ID | Content | Platform |
|----------|---------|----------|
| (App+1) | Windows game content | Windows |
| (App+2) | Demo content | Windows |
| (App+3) | Mod Kit content | Windows |

### Build Script Integration

```bash
# SteamPipe build command (CI/CD)
steamcmd +login <username> +run_app_build ../scripts/app_build.vdf +quit
```

### app_build.vdf Template

```vdf
"AppBuild"
{
    "AppID" "<APP_ID>"
    "Desc" "Snowpiercer: Eternal Engine build"
    "ContentRoot" "../output/"
    "BuildOutput" "../build_output/"
    "Depots"
    {
        "<DEPOT_ID>"
        {
            "FileMapping"
            {
                "LocalPath" "WindowsNoEditor/*"
                "DepotPath" "."
                "recursive" "1"
            }
            "FileExclusion" "*.pdb"
            "FileExclusion" "*.debug"
            "FileExclusion" "steam_appid.txt"
        }
    }
}
```

### Branch Configuration

| Branch | Purpose | Access |
|--------|---------|--------|
| `default` | Public release branch | Public |
| `beta` | Pre-release testing | Password-protected |
| `ea-update` | Early Access update staging | Password-protected |
| `demo` | Demo build | Public (separate App ID) |
| `internal` | Development builds | Developer-only |

---

## 10. Development & Testing Workflow

### Local Development (No Steam)

The game must run without Steam for development:
- `USEESteamSubsystem::IsSteamAvailable()` returns false
- All Steam features gracefully degrade
- Achievements stored locally in save file (synced to Steam on next Steam-connected launch)
- No Steam Overlay, no Rich Presence, no Cloud Save

### Steam Development

1. Use App ID `480` (Spacewar) for initial integration testing
2. Switch to assigned App ID once Steamworks partner access is granted
3. Use `SteamAPI_RestartAppIfNecessary()` for proper Steam launch flow
4. Test with Steam client running in Developer Comp mode

### QA Checklist — Steam Features

- [ ] Game launches from Steam correctly
- [ ] Game launches without Steam (graceful degradation)
- [ ] All 68 achievements unlock correctly
- [ ] Achievement progress tracking (x/N display)
- [ ] Hidden achievements show "???" until unlocked
- [ ] Cloud saves sync between machines
- [ ] Cloud save conflict resolution UI works
- [ ] Mr. Wilford permadeath cloud sync works
- [ ] Rich Presence updates in friends list
- [ ] Controller detected and works end-to-end
- [ ] Steam Deck runs at 30 FPS on Medium
- [ ] Steam Overlay opens without conflicts
- [ ] Screenshot capture works with metadata
- [ ] Demo save transfers to full game
- [ ] Trading cards drop during gameplay
