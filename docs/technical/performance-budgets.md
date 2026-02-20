# Performance Budgets

## Target Framerate

| Scenario | Target | Hard Floor |
|----------|--------|------------|
| Gameplay (exploration, dialogue, stealth) | 60 fps | 45 fps |
| Combat (melee, up to 8 NPCs) | 60 fps | 30 fps |
| Scripted sequences (explosions, breaches) | 30 fps | 24 fps |

**V-Sync**: On by default, with an option to uncap.

**Frame time budget at 60 fps: 16.6 ms**

| System | Budget | Notes |
|--------|--------|-------|
| Rendering (GPU) | 10.0 ms | Geometry, lighting, post-process |
| Game thread (CPU) | 4.0 ms | AI, gameplay logic, animation |
| Physics | 1.5 ms | Collision, ragdoll, cloth |
| Audio | 0.5 ms | Spatial audio mixing |
| Streaming overhead | 0.5 ms | Async load dispatch, GC |
| Headroom | 0.6 ms | Spike absorption |

---

## Hardware Specifications

### Minimum (720p, 30 fps, Low settings)

| Component | Spec |
|-----------|------|
| CPU | Intel i5-8400 / AMD Ryzen 5 2600 (6-core, 3.0 GHz) |
| GPU | NVIDIA GTX 1060 6GB / AMD RX 580 8GB |
| RAM | 8 GB |
| VRAM | 4 GB |
| Storage | 30 GB SSD (HDD supported but not recommended — streaming hitches) |
| OS | Windows 10 64-bit |

### Recommended (1080p, 60 fps, High settings)

| Component | Spec |
|-----------|------|
| CPU | Intel i5-12400 / AMD Ryzen 5 5600X (6-core, 3.7+ GHz) |
| GPU | NVIDIA RTX 3060 12GB / AMD RX 6700 XT 12GB |
| RAM | 16 GB |
| VRAM | 8 GB |
| Storage | 30 GB NVMe SSD |
| OS | Windows 10/11 64-bit |

### Ultra (1440p, 60 fps, Epic settings with ray-traced reflections)

| Component | Spec |
|-----------|------|
| CPU | Intel i7-13700K / AMD Ryzen 7 7700X |
| GPU | NVIDIA RTX 4070 / AMD RX 7800 XT |
| RAM | 32 GB |
| VRAM | 12 GB |
| Storage | 30 GB NVMe SSD |

---

## Per-Car Polygon Budgets

All triangle counts are Nanite virtual geometry counts (source mesh). Nanite dynamically reduces on-screen triangles based on screen coverage.

### Static Geometry (Car Interior)

| Element | Triangle Budget | Count per Car | Subtotal |
|---------|----------------|---------------|----------|
| Shell (walls, floor, ceiling) | 2,000 | 6–10 pieces | 12,000–20,000 |
| Structure (doors, partitions) | 1,500 | 3–5 pieces | 4,500–7,500 |
| Fixtures (furniture, equipment) | 3,000 | 5–10 pieces | 15,000–30,000 |
| Infrastructure (pipes, vents) | 500 | 8–15 pieces | 4,000–7,500 |
| Clutter props | 200 | 15–30 pieces | 3,000–6,000 |
| **Car interior total** | | | **38,500–71,000** |

**Target: 50,000 triangles per car (source mesh)**

Nanite will render ~5,000–15,000 on-screen triangles per car depending on camera distance and screen coverage.

### Characters

| Element | Triangle Budget | Notes |
|---------|----------------|-------|
| Player character | 50,000 | Full detail, always on screen |
| NPC (hero/named) | 30,000 | Companions, key story NPCs |
| NPC (generic) | 15,000 | Crowd/background NPCs |
| NPC (distant/LOD1) | 5,000 | Visible through doors in adjacent cars |

### Total On-Screen Triangle Budget

| Scenario | Budget |
|----------|--------|
| Active car (player + 5 NPCs + car geometry) | 200,000 source / ~50,000 rendered |
| Active + 2 adjacent visible through doors | 350,000 source / ~80,000 rendered |
| Combat (8 NPCs, effects, debris) | 400,000 source / ~100,000 rendered |

---

## Texture Budgets

### Per-Car Textures

| Texture Type | Resolution | Format | Size (Compressed) |
|-------------|------------|--------|--------------------|
| Albedo | 2048x2048 | BC1 | 2.7 MB |
| Normal | 2048x2048 | BC5 | 5.3 MB |
| ORM (Occlusion/Roughness/Metallic) | 2048x2048 | BC1 | 2.7 MB |
| **Per material set** | | | **~10.7 MB** |

Kit pieces share material sets. A typical car uses 3–5 unique material sets:

**Target: 30–50 MB of unique textures per car** (before virtual texture streaming reduces resident memory).

### Character Textures

| Element | Resolution | Sets | Total |
|---------|------------|------|-------|
| Player | 4096x4096 | 3 (body, head, gear) | ~60 MB |
| Named NPC | 2048x2048 | 2 (body, head) | ~20 MB |
| Generic NPC | 1024x1024 | 2 (body, head) | ~5 MB |

---

## NPC Budgets

### Maximum Concurrent NPCs

| Scenario | Max NPCs | AI Ticking | Notes |
|----------|----------|------------|-------|
| Active car | 12 | Full behavior tree | Hard limit for combat/dialogue |
| Adjacent loaded cars | 8 per car | Paused | Visible through doors, frozen pose |
| Global simulated (unloaded) | Unlimited | Schedule lookup only | No tick cost, just data |

**Hard limit: 12 fully-ticking NPCs in the active car.**

### NPC AI Cost Breakdown (Per NPC)

| System | CPU Cost | Notes |
|--------|----------|-------|
| Behavior tree tick | 0.15 ms | Runs at 10 Hz (every 100 ms), amortized |
| Perception (sight + sound) | 0.10 ms | Line traces, sphere overlaps |
| Navigation/pathfinding | 0.08 ms | NavMesh queries, path following |
| Animation blueprint | 0.05 ms | State machine evaluation |
| Dialogue availability check | 0.02 ms | Flag lookup |
| **Total per NPC** | **~0.40 ms** | |
| **12 NPCs total** | **~4.8 ms** | Exceeds 4.0 ms game thread budget |

**Mitigation**: Stagger AI ticks across frames. Not all 12 NPCs run full perception every frame:
- 4 NPCs tick full AI per frame (round-robin)
- All 12 tick behavior tree at 10 Hz (amortized to ~0.15 ms/frame each)
- Net cost: ~2.5 ms for 12 NPCs

---

## Draw Call Budget

| Category | Draw Calls | Notes |
|----------|-----------|-------|
| Car geometry (Nanite) | ~50 | Nanite merges into cluster hierarchy |
| NPCs (skeletal meshes) | 12–24 | 1-2 draw calls per NPC (body + head) |
| Particle effects | 10–20 | Niagara GPU particles, few CPU calls |
| UI (UMG) | 20–30 | Batched slate draws |
| Post-processing | 5 | Bloom, color grading, DoF |
| Shadows | ~30 | Virtual shadow maps (Nanite-integrated) |
| **Total** | **~130–170** | |

**Target: < 200 draw calls per frame.**

Nanite significantly reduces draw call overhead compared to traditional instanced static meshes. The corridor environment is favorable — limited view distance means few objects on screen.

---

## Physics Object Budget

| Category | Max Active | Simulation |
|----------|-----------|------------|
| Ragdoll NPCs | 3 simultaneous | Full skeletal physics, despawn after 10s |
| Destructible props (tables, crates) | 8 per car | Geometry collection fracture |
| Loose physics objects (bottles, tools) | 15 per car | Simple rigid body |
| Cloth simulation | 2 NPCs | Chaos cloth on active companions only |
| Doors/hatches | 4 | Kinematic, physics-driven open/close |
| **Total active physics bodies** | **~80** | |

**Hard limit: 100 active physics bodies.** Beyond this, oldest/least-visible bodies are put to sleep.

### Physics Substep

- Fixed timestep: 60 Hz (matching target framerate)
- Max substeps per frame: 2 (caps physics cost during frame drops)
- Solver iterations: 4 (position), 1 (velocity)

---

## Quality Settings Presets

| Setting | Low | Medium | High | Epic |
|---------|-----|--------|------|------|
| Resolution scale | 67% | 85% | 100% | 100% |
| Nanite detail | Reduced clusters | Default | Full | Full + over-detail |
| Lumen GI | Screen-space only | Software ray trace | Software ray trace | Hardware ray trace |
| Lumen reflections | Screen-space | Software | Software | Hardware |
| Shadow quality | Low-res VSM | Medium VSM | High VSM | High VSM + contact shadows |
| Texture quality | Mip bias +2 | Mip bias +1 | Full mips | Full mips |
| Post-processing | Minimal | Standard | Full | Full + film grain |
| NPC LOD distance | 8m | 12m | 20m | 25m |
| Max active ragdolls | 1 | 2 | 3 | 3 |
| Cloth simulation | Off | Player only | Player + 1 companion | Player + 2 companions |
| Particle density | 50% | 75% | 100% | 150% |

---

## Profiling & Validation

### Per-Car Validation Checklist

Before a car template ships, it must pass:

- [ ] Source mesh triangle count < 50,000
- [ ] Rendered triangle count < 20,000 (at gameplay camera distance)
- [ ] Texture memory (resident) < 50 MB
- [ ] NPC count <= 12
- [ ] Physics bodies < 30 (at rest)
- [ ] Frame time < 16.6 ms on recommended hardware (automated test)
- [ ] Load time < 500 ms (async, from unloaded state)
- [ ] Memory delta < 60 MB when loading car

### Automated Performance Tests

```cpp
// Automated flythrough test for each car template
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCarPerformanceTest, "TrainGame.Performance.CarBudget",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)

bool FCarPerformanceTest::RunTest(const FString& Parameters)
{
    // 1. Load car template
    // 2. Spawn max NPCs
    // 3. Run 10-second flythrough
    // 4. Assert: avg frame time < 16.6 ms
    // 5. Assert: peak memory < 60 MB
    // 6. Assert: draw calls < 200
    return true;
}
```
