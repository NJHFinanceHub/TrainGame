# VFX System — Technical Design

## Architecture Overview

The VFX system uses a component-based architecture with four main classes:

### Components

| Component | Purpose | Attach To |
|-----------|---------|-----------|
| `USEEVFXComponent` | Character VFX (cold breath, damage, Kronole) | ASEECharacter, ASEENPCCharacter |
| `USEEEnvironmentVFXComponent` | Environmental emitters (steam, sparks, flickers) | Static actors in levels |
| `USEEWeatherVFXComponent` | Weather through windows (blizzard, aurora) | Window actors |
| `USEEVFXSubsystem` | Global weather/quality state | World subsystem (auto) |

### Data Flow

```
Gameplay Components                    VFX Components
┌─────────────────┐                   ┌──────────────────┐
│ HealthComponent  │──OnDamageTaken──→│                  │
│ ColdComponent    │──OnFrostbite────→│  SEEVFXComponent │──→ Niagara Particles
│ CombatComponent  │──OnParry/Block──→│                  │──→ Post-Process
│ KronoleComponent │──(polled)───────→│                  │──→ Decals
└─────────────────┘                   └──────────────────┘

VFXSubsystem ──SetGlobalWeather──→ All WeatherVFXComponents
```

## Effect Catalog

### 1. Steam / Smoke (USEEEnvironmentVFXComponent)

**Placement:** Pipes, vents, kitchen areas, engine car

| Variant | Type Enum | Behavior |
|---------|-----------|----------|
| Steam Vent | `SteamVent` | Cyclic bursts, 1.5s warning hiss, optional DOT damage |
| Pipe Leak | `SteamPipe` | Continuous wispy stream |
| Kitchen Steam | `KitchenSteam` | Rising plume, wider spread |
| Smoke | `Smoke` | Dense, slow-moving, vision-obscuring |

**Niagara Parameters:** `Intensity`, `SpawnRate`, `Lifetime`, `ColorTint`

**Performance:** Each steam vent = 1 emitter, ~50-200 GPU particles

### 2. Sparks (USEEEnvironmentVFXComponent + USEEVFXComponent)

**Placement:** Electrical panels, combat impact points

| Variant | Source | Trigger |
|---------|--------|---------|
| Electric Sparks | Environment component | Cyclic / triggered |
| Combat Sparks | VFX component | OnDamageTaken (Electric type) |
| Welding Sparks | Environment component | Always active |

**Niagara Parameters:** `Intensity`, `SpawnRate`, `SparkColor`

### 3. Cold Breath (USEEVFXComponent)

**Trigger:** Temperature < ColdBreathThreshold (32°F)
**Attachment:** Character head socket, forward offset
**Dynamic Parameters:**
- `BreathRate` — scales with exertion (combat, sprinting)
- `BreathDensity` — scales with frostbite stage
- `BreathOpacity` — scales with cold intensity

### 4. Blood / Damage (USEEVFXComponent)

**Types:**
- `BloodSplatter` — Niagara burst on melee hit (Blunt/Bladed/Piercing)
- Blood decals — projected onto floor, fade after 30s
- Damage vignette — screen-space red overlay below 30% health

**Decal System:** Uses `UGameplayStatics::SpawnDecalAtLocation` with auto-fade.
Random yaw rotation for variety. Scale proportional to damage.

### 5. Kronole Effects (USEEVFXComponent — Post-Process)

**Post-process material parameters:**

| Parameter | Onset | Peak | Comedown | Withdrawal |
|-----------|-------|------|----------|------------|
| DistortionIntensity | 0.24 | 0.8 | 0.12 | 0.48 |
| ChromaticAberration | 0.45 | 1.5 | 0.225 | 0.9 |
| Desaturation | 0 | 0 | 0 | 0.24 |
| ScreenJitter | 0 | 0 | 0 | 0.18 |
| DamageVignetteIntensity | (health-based, independent) |

**Transition:** Smooth blend via `FInterpTo` at rate 3.0

### 6. Weather Through Windows (USEEWeatherVFXComponent)

**Global state managed by USEEVFXSubsystem.**

| Weather | Particles | Ambient Color | Default Frost |
|---------|-----------|---------------|---------------|
| Blizzard | Dense horizontal snow | Cold white-blue | 0.95 |
| Heavy Snow | Moderate falling snow | Dim grey-blue | 0.70 |
| Light Snow | Sparse gentle snow | Soft grey | 0.50 |
| Aurora | Colored light ribbons | Green glow | 0.30 |
| Clear Sky | None | Dark night | 0.30 |
| White-Out | Dense horizontal (reuses blizzard) | Cold white-blue | 0.95 |

**Glass Material Parameters:** `FrostCoverage`, `ExteriorAmbient`, `ExteriorVisibility`

### 7. Lighting Flickers (USEEEnvironmentVFXComponent)

**Patterns:**

| Pattern | Interval | Min Intensity | Behavior |
|---------|----------|---------------|----------|
| Fluorescent | 0.5s avg | 0.0 | Regular buzz-flicker, Tail section feel |
| Damaged | 0.5s avg | 0.0 | Erratic duration (0.5x-3x), broken wiring |
| Emergency | 0.5s avg | 0.3-0.8 | Red-tinted pulse, alert state |
| Dying | 0.5s avg | 0.0-0.2 | Sometimes fully dark, power failing |

**Implementation:** Drives `UPointLightComponent::SetIntensity()` and `SetLightColor()`.
Auto-discovers lights on owner actor or uses manual assignment.

## Performance Budget

From `performance-budgets.md`:
- **Max concurrent Niagara emitters:** 10-20 (configurable via VFXSubsystem)
- **Draw calls for particles:** 10-20 of ~170 total
- **Quality scaling:** `ParticleDensityScale` (0.5-1.5) adjusts spawn rates

## Required Niagara Assets

Content team needs to create these NS_ (Niagara System) assets:

```
Content/FX/
├── NS_ColdBreath.uasset         — Pulsing mist cloud, attaches to head
├── NS_BloodSplatter.uasset      — Burst of red droplets
├── NS_MetalSparks.uasset        — Directional spark shower
├── NS_ParryFlash.uasset         — White flash burst with sparks
├── NS_BlockBreak.uasset         — Shatter/stagger flash
├── NS_SteamVent.uasset          — Directional steam column
├── NS_SteamPipeLeak.uasset      — Thin wispy leak
├── NS_KitchenSteam.uasset       — Wide rising plume
├── NS_ElectricSparks.uasset     — Arcing sparks with light flash
├── NS_WeldingSparks.uasset      — Grinding/welding shower
├── NS_Smoke.uasset              — Thick rolling smoke
├── NS_Embers.uasset             — Floating hot embers
├── NS_Dust.uasset               — Floating dust motes
├── NS_Drips.uasset              — Condensation drops
├── NS_Blizzard.uasset           — Dense horizontal snow + wind
├── NS_HeavySnow.uasset          — Moderate falling snow
├── NS_LightSnow.uasset          — Sparse gentle snow
└── NS_Aurora.uasset             — Colored light ribbons

Content/Materials/
├── M_KronolePostProcess.uasset  — Post-process: distortion, chroma, desaturation
├── MI_BloodDecal_01.uasset      — Blood splatter decal variant 1
├── MI_BloodDecal_02.uasset      — Blood splatter decal variant 2
└── MI_BloodDecal_03.uasset      — Blood splatter decal variant 3
```

## Module Dependency

Added `"Niagara"` to `SnowpiercerEE.Build.cs` PublicDependencyModuleNames.
