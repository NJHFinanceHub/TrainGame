# Steam Publishing Checklist

Master checklist for Steam platform launch. Covers Steamworks setup, store page, build deployment, and all platform features.

---

## Phase 1: Steamworks Partner Setup

- [ ] **Steamworks partner account** created and verified
- [ ] **App ID** assigned for main game
- [ ] **App ID** assigned for free demo (separate app)
- [ ] **App ID** assigned for Mod Kit (separate app, 1.0 launch)
- [ ] **Tax and banking** information submitted
- [ ] **Company identity** verified (Gas Town Studios)
- [ ] **Steam Direct fee** paid ($100 USD per app)

---

## Phase 2: Store Page Configuration

### Basic Info
- [ ] **App name**: Snowpiercer: Eternal Engine
- [ ] **Developer**: Gas Town Studios
- [ ] **Publisher**: Gas Town Studios (or publisher if signed)
- [ ] **Release date**: Set to target window (Early Access)
- [ ] **Genre tags** applied (see `steam-store-page.md`)
- [ ] **Feature tags** applied (Steam Achievements, Cloud, Workshop, Full Controller, Trading Cards, Deck Verified)

### Store Page Content
- [ ] **Short description** (300 chars) — see `steam-store-page.md`
- [ ] **Long description** (About This Game) — see `steam-store-page.md`
- [ ] **Early Access FAQ** — see `early-access-strategy.md`
- [ ] **Mature content description** — see `steam-store-page.md`
- [ ] **System requirements** (min/recommended) — see `system-requirements.md`
- [ ] **Supported languages** listed
- [ ] **Legal info** / EULA if needed

### Visual Assets
- [ ] **Header capsule** (460×215) — game logo + train art
- [ ] **Small capsule** (231×87) — game logo
- [ ] **Main capsule** (616×353) — key art
- [ ] **Hero image** (3840×1240) — panoramic train/frozen world
- [ ] **Logo** (1920×1080 with transparency)
- [ ] **Library hero** (3840×1240) — Steam library background
- [ ] **Library logo** (1280×720 with transparency)
- [ ] **Page background** (1438×810) — store page background
- [ ] **Screenshots** — minimum 10, up to 20 (see `screenshot-trailer-plan.md`)
- [ ] **Trailer(s)** uploaded — at least Trailer 1 (Announcement)

### Content Rating
- [ ] **IARC questionnaire** completed in Steamworks
- [ ] **Content descriptors** set correctly
- [ ] **Sensitive content toggles** documented in mature content description
- [ ] Verify auto-generated ratings match expectations (see `content-rating.md`)

---

## Phase 3: Technical Integration

### Steamworks SDK
- [ ] **OnlineSubsystemSteam** plugin enabled in `.uproject`
- [ ] **SteamController** plugin enabled
- [ ] **Build.cs** dependencies added (OnlineSubsystem, OnlineSubsystemSteam, Steamworks)
- [ ] **DefaultEngine.ini** configured (see `steamworks-sdk-integration.md`)
- [ ] **steam_appid.txt** created for development (removed from shipping builds)
- [ ] **USEESteamSubsystem** implemented and tested
- [ ] **Graceful degradation** verified (game works without Steam)

### Achievements (68 total)
- [ ] **Achievement definitions** configured in Steamworks partner portal
- [ ] All 68 achievements with API names, display names, descriptions
- [ ] Hidden flag set on 26 achievements
- [ ] **Achievement icons** uploaded (136 PNGs: 68 locked + 68 unlocked, 256×256 each)
- [ ] **Stats** configured (16 integer stats for progress tracking)
- [ ] **USEEAchievementManager** implemented
- [ ] Achievement triggers connected to all gameplay subsystems
- [ ] Progress display (x/N) working for collectible/kill achievements
- [ ] **QA pass**: All 68 achievements verified to unlock correctly
- [ ] `SteamAPI_RunCallbacks()` ticking properly

### Cloud Saves
- [ ] **Auto-Cloud paths** configured in Steamworks
- [ ] Save file path: `Saves/*.sav`
- [ ] Settings file path: `Settings/UserSettings.json`
- [ ] **Cloud quota** requested (100 MB)
- [ ] **USEECloudSaveManager** implemented
- [ ] Conflict resolution UI implemented
- [ ] Mr. Wilford permadeath cloud sync tested
- [ ] **QA pass**: Save/load across two machines verified

### Rich Presence
- [ ] **Rich Presence strings** configured in Steamworks (localized)
- [ ] **USEERichPresenceManager** implemented
- [ ] Updates on car/zone transitions
- [ ] Clears on main menu
- [ ] **QA pass**: Visible in friend's Steam client

### Controller Support
- [ ] **Gamepad bindings** in DefaultInput.ini (see `controller-bindings.md`)
- [ ] **Steam Input** action sets configured (Exploration, Combat, Stealth, Dialogue, Menu)
- [ ] **Default controller configs** created for Xbox, PlayStation, Steam Deck
- [ ] **Button prompt glyphs** dynamically switch based on input device
- [ ] **Haptic feedback** implemented (vibration + DualSense adaptive triggers)
- [ ] **Gyro aim** support for Steam Deck / DualSense
- [ ] **Accessibility**: Full remapping, dead zones, response curves, one-handed mode
- [ ] **Community configs** enabled in Steamworks
- [ ] **QA pass**: Full playthrough possible on controller only

### Steam Overlay
- [ ] Shift+Tab overlay works without conflicts
- [ ] Screenshot capture (F12) works and includes location metadata
- [ ] Game pauses when overlay opens (single-player)

---

## Phase 4: Build Deployment

### SteamPipe Configuration
- [ ] **Depot** created for Windows game content
- [ ] **Depot** created for demo content (separate app)
- [ ] **app_build.vdf** configured (see `steamworks-sdk-integration.md`)
- [ ] **File exclusions** set (*.pdb, *.debug, steam_appid.txt)
- [ ] **Branches** configured:
  - `default` — public release
  - `beta` — pre-release testing (password-protected)
  - `ea-update` — Early Access update staging (password-protected)
  - `demo` — demo build (separate App ID)
  - `internal` — development builds (developer-only)

### Build Process
- [ ] **UE5 packaging** configured for Shipping build (Windows)
- [ ] **Content cooking** verified (all cars, all zones included per target)
- [ ] **Build script** integrated with SteamPipe upload
- [ ] **CEF (Chromium)** redistributable included (if using in-game browser)
- [ ] **DirectX 12 redistributable** included
- [ ] **VC++ redistributable** included
- [ ] **Build size** verified (target: ~50 GB)
- [ ] **Launch options** configured (executable path, arguments)

### Demo Build
- [ ] **Demo content scope**: Cars 1-10 (see `demo-plan.md`)
- [ ] **Demo restrictions** applied (no achievements, content boundary at Car 10)
- [ ] **Save transfer** tested (demo save → full game)
- [ ] **End-of-demo screen** implemented
- [ ] **Demo App ID** correctly references base game for "Buy Full Game" CTA
- [ ] **Demo uploaded** to separate depot

---

## Phase 5: Pre-Launch

### Internal Testing
- [ ] **Playtest** branch created and populated
- [ ] **Closed beta** invites sent (if applicable)
- [ ] **Feedback collection** pipeline set up (Steam forums, Discord)
- [ ] **Crash reporting** integration active
- [ ] **Analytics** opt-in system tested (see `demo-plan.md` analytics section)

### Marketing
- [ ] **Wishlist page** live (store page in "Coming Soon" state)
- [ ] **Announcement trailer** uploaded (Trailer 1)
- [ ] **Steam Next Fest** application submitted (if aligning with demo launch)
- [ ] **Community hub** configured (forums, artwork, screenshots, guides sections)
- [ ] **Social media** links added to store page
- [ ] **Developer blog** initial post published

### Trading Cards (1.0 only, not Early Access)
- [ ] **8 card designs** submitted (see `steam-features.md`)
- [ ] **Badge rewards** configured (5 levels + foil)
- [ ] **Emoticons** (15) created and submitted
- [ ] **Profile backgrounds** created and submitted
- [ ] **Drop rates** configured

---

## Phase 6: Launch Day

### Release
- [ ] **Build** set as default on `default` branch
- [ ] **Demo build** set as default on demo app
- [ ] **Release date** set to "now" (or scheduled)
- [ ] **Price** set ($29.99 USD for Early Access)
- [ ] **Regional pricing** configured
- [ ] **Launch discount** set (if any — typically none for EA launch)

### Monitoring
- [ ] **Crash reports** monitored for first 24h
- [ ] **Steam reviews** monitored
- [ ] **Community hub** moderated
- [ ] **Achievement stats** verified in Steamworks dashboard
- [ ] **Cloud save** error rates checked
- [ ] **Refund rate** monitored

### Post-Launch (Week 1)
- [ ] **Hotfix branch** ready if needed
- [ ] **Known issues** list posted in community hub
- [ ] **Player feedback** triaged into backlog
- [ ] **Sales data** reviewed

---

## Phase 7: Early Access Updates

For each EA update (see `early-access-strategy.md` timeline):

- [ ] **Content** complete and tested for the update zone
- [ ] **Save compatibility** verified (old saves load with new content)
- [ ] **Build** uploaded to `ea-update` branch for staging
- [ ] **Beta** testing period (1-2 weeks on `beta` branch)
- [ ] **Patch notes** written
- [ ] **Update announcement** posted
- [ ] **Build** promoted to `default` branch
- [ ] **New achievements** activated (if zone-specific achievements added)
- [ ] **New trading cards** (if applicable)

---

## Phase 8: 1.0 Launch

- [ ] **All 7 zones** complete and tested
- [ ] **All 68 achievements** verified
- [ ] **All 6 endings** verified
- [ ] **Workshop / Mod Kit** ready (separate App ID)
- [ ] **Trading cards** activated
- [ ] **Price increase** ($29.99 → $44.99) applied
- [ ] **Launch discount** (10%) configured
- [ ] **"Early Access" label** removed from store page
- [ ] **Store page description** updated for full release
- [ ] **New screenshots** reflecting full game content
- [ ] **Launch trailer** (Trailer 3) uploaded
- [ ] **Press embargo** lifted
- [ ] **Review copies** distributed

---

## Reference Documents

| Document | Path |
|----------|------|
| Steamworks SDK Integration | `docs/publishing/steamworks-sdk-integration.md` |
| Steam Achievements (Design) | `docs/publishing/steam-achievements.md` |
| Steam Features (Cloud, Workshop, Trading Cards) | `docs/publishing/steam-features.md` |
| Steam Store Page Copy | `docs/publishing/steam-store-page.md` |
| Content Rating | `docs/publishing/content-rating.md` |
| Controller Bindings | `docs/publishing/controller-bindings.md` |
| Demo Plan | `docs/publishing/demo-plan.md` |
| Early Access Strategy | `docs/publishing/early-access-strategy.md` |
| Screenshot & Trailer Plan | `docs/publishing/screenshot-trailer-plan.md` |
| System Requirements | `docs/publishing/system-requirements.md` |
| Mod Support | `docs/publishing/mod-support.md` |
