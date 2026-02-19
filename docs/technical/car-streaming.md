# Car Streaming Architecture

## Overview

The train has 1,034 cars but the player only occupies one at a time and can see at most 1-2 adjacent cars through doors/windows. The streaming system loads and unloads car Level Instances to keep memory bounded.

## Loading Strategy: Sliding Window (3-Car Active)

```
                    Direction of travel →
  ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐
  │  N-2 │ │ N-1 │ │  N  │ │ N+1 │ │ N+2 │
  │UNLOAD│ │LOADED│ │ACTIV│ │LOADED│ │PRELOAD│
  └─────┘ └─────┘ └─────┘ └─────┘ └─────┘
```

| State | Cars | Description |
|-------|------|-------------|
| **Active** | Current car (N) | Fully simulated: NPCs tick AI, physics active, full LOD |
| **Loaded** | N-1, N+1 | Geometry and NPCs loaded, AI paused, reduced physics (no ragdoll) |
| **Preloading** | N+2 (or N-2 if moving backward) | Async load in progress, not yet visible |
| **Unloaded** | All others | Only serialized state in memory (per-car save delta) |

### State Transitions

```
Player enters car N:
  1. N becomes Active (full simulation)
  2. N-1 and N+1 become Loaded (if not already)
  3. Async preload begins for N+2
  4. N-3 (if previously Loaded) transitions to Unloaded — serialize state, flush assets
```

### Loading Trigger

The streaming transition begins when the player reaches **75% through the door animation** of the destination car. This gives the preloaded car time to finalize before the player's camera crosses the threshold.

## Door Animation Budget

Door transitions are the primary loading seam. The animation must be long enough to mask streaming but short enough to feel responsive.

| Door Type | Animation Duration | Notes |
|-----------|-------------------|-------|
| Standard interior door | 0.8 s | Slide/swing, minimal loading needed (N+1 already loaded) |
| Bulkhead door (zone boundary) | 2.0 s | Heavy wheel-turn animation, loading mask for zone material swap |
| Security door (locked) | 1.5 s + interaction | Lockpick/keycard animation provides extra buffer |
| Emergency hatch | 1.2 s | Overhead hatch climb-through |
| Exterior access | 3.0 s | Suit-up animation, wind/cold VFX, longest load mask |

### Loading Failure Fallback

If the next car has not finished loading by the time the door animation completes:

1. **Brief hold**: Freeze the door at 95% open, play a "stuck" struggle animation (up to 1.0 s additional)
2. **Last resort**: Fade to black with a contextual loading tip (target: never happens on recommended hardware)

## Preloading Strategy

### Asset Categories & Priority

| Priority | Asset Type | Load Order | Reason |
|----------|-----------|------------|--------|
| 1 (Critical) | Car geometry (static meshes) | First | Player sees walls/floor immediately |
| 2 (Critical) | Lighting data | With geometry | Dark car is jarring |
| 3 (High) | NPC skeletal meshes | After geometry | NPCs must be visible on entry |
| 4 (High) | NPC animations (idle set) | With NPC meshes | Prevent T-pose |
| 5 (Medium) | Textures (mip levels 0-2) | Async stream | Lower mips first, detail streams in |
| 6 (Medium) | Audio ambience | With geometry | Spatial audio should be ready |
| 7 (Low) | Clutter props | After NPCs | Small meshes, fast to place |
| 8 (Low) | Particle effects | Last | VFX can pop in without notice |

### Texture Streaming

UE5's virtual texture streaming handles mip chains automatically. Car textures use a **streaming pool** of 512 MB. Each car's textures target:

- Mip 0-1 (full res): Loaded when car is Active
- Mip 2 (half res): Loaded when car is Loaded/Preloading
- Mip 3+ (quarter and below): Always resident for distant LOD through windows

## Memory Budget

### Per-Car Memory Targets

| Component | Budget | Notes |
|-----------|--------|-------|
| Static meshes (Nanite) | 15 MB | Kit-based, high instancing |
| Textures (resident mips) | 20 MB | 2K textures, ORM packed |
| NPC skeletal meshes | 5 MB per NPC | Shared skeleton, unique heads |
| NPC animations | 3 MB per NPC | Shared animation blueprint |
| Audio | 2 MB | Streaming audio, small resident cues |
| Collision/NavMesh | 2 MB | Simple box/capsule collision |
| Game state (serialized) | < 50 KB | Per-car delta only |
| **Total per car** | **~60 MB** (typical, 5 NPCs) | |

### System Memory Budget

| Category | Budget |
|----------|--------|
| 3 loaded cars (Active + 2 Loaded) | 180 MB |
| 1 preloading car | 60 MB |
| Shared assets (player, weapons, UI) | 200 MB |
| Texture streaming pool | 512 MB |
| Audio streaming pool | 64 MB |
| Engine overhead (UE5 runtime) | 400 MB |
| Game systems (quest, faction, NPC global state) | 50 MB |
| Headroom | ~500 MB |
| **Total target** | **~2 GB** |

This fits within a 4 GB VRAM / 8 GB system RAM minimum spec (see performance-budgets.md).

## LOD for Visible Cars

When looking through doors or windows into adjacent cars:

### Through Open Doors (Loaded Cars)

Adjacent cars are fully loaded — no LOD reduction needed. The player can see into N-1 and N+1 at full detail.

### Through Windows (Non-Adjacent Cars)

Windows show the train exterior and possibly distant car silhouettes:

| Distance | LOD Level | Detail |
|----------|-----------|--------|
| Adjacent car exterior | LOD1 | Simplified exterior mesh, window glow |
| 2-5 cars away | LOD2 | Box silhouette with material, no interior |
| 5+ cars away | LOD3 | Billboard impostor (pre-rendered) |
| Horizon/landscape | Skybox | Frozen landscape panorama, parallax scrolling |

### Exterior Car Shell

Each car has a low-poly exterior shell (500-1000 triangles) that is always loaded for cars within visual range through windows. These use a shared instanced material with per-car tint variation.

## NPC Lifecycle Across Cars

| Car State | NPC Behavior |
|-----------|-------------|
| Active | Full AI tick (behavior tree, perception, dialogue availability) |
| Loaded | AI paused, animations frozen at last pose, collision active |
| Unloaded | Serialized to per-car state delta. On reload: restore position, disposition, alive/dead |

### NPC Schedule Simulation

When a car transitions from Unloaded to Loaded, NPCs are placed according to their **schedule** for the current game-world time, not their last saved position. This prevents NPCs from being frozen in time while the player is elsewhere.

```cpp
void UNPCScheduleSubsystem::OnCarLoaded(int32 CarIndex)
{
    for (auto& NPC : GetNPCsForCar(CarIndex))
    {
        FVector ScheduledPos = NPC.Schedule.GetPositionForTime(WorldTime);
        NPC.SetActorLocation(ScheduledPos);
        NPC.RestoreStateFromDelta(CarSaveState);
    }
}
```

## Streaming Metrics & Monitoring

### Runtime Profiling

| Metric | Target | Alert Threshold |
|--------|--------|-----------------|
| Car load time (async) | < 500 ms | > 800 ms |
| Door-to-playable time | < door animation duration | > animation duration (triggers fallback) |
| Memory (3-car window) | < 200 MB | > 250 MB |
| Texture pool utilization | < 80% | > 90% |
| Frame time during load | < 16.6 ms (60 fps) | > 20 ms (visible hitch) |

### Debug Commands

```
// Console commands for development
ShowFlag.Streaming 1        // Visualize streaming state
stat streaming              // Streaming pool stats
TrainGame.ShowCarStates      // Custom: show Active/Loaded/Preloading/Unloaded overlay
TrainGame.ForceLoadCar <N>   // Force-load a specific car for testing
```
