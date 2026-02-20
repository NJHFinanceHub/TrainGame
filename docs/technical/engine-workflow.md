# Engine Workflow & Art Pipeline

## Engine Choice: Unreal Engine 5 (C++)

**Decision: Confirmed — UE5 with C++ gameplay modules.**

### Rationale

| Factor | UE5 Strength |
|--------|-------------|
| Linear corridor environments | Nanite mesh streaming, Lumen GI handle dense interiors well |
| NPC-heavy scenes | Skeletal mesh instancing, animation budgets, crowd LOD |
| Platform target (PC/Steam) | Mature shipping pipeline, Steamworks plugin |
| Existing codebase | 59 C++ source files already organized into modular subsystems |
| Team skill | C++ modules already using GameplayAbilities, Enhanced Input, GameplayTags |

### Alternatives Considered

| Alternative | Why Rejected |
|-------------|-------------|
| Unity HDRP | Weaker GI for corridor interiors; existing UE5 codebase would be lost |
| Godot 4 | Immature for AAA-adjacent scope; no equivalent to Nanite/Lumen |
| Custom engine | Unjustifiable development cost for a narrative-heavy RPG |

---

## Art Pipeline Decision: Hybrid (Editor-placed hero assets + Procedural modular kit)

**Decision: UE5 Editor placement using a modular geometry kit, with procedural variation for repeated car types.**

### Why Hybrid

The train has 100+ cars but only ~20 distinct car *types* (sleeper, dining, workshop, etc.). Hand-placing every prop in 100+ cars is prohibitive; pure procedural generation loses the authored feel critical to narrative design. The hybrid approach:

1. **Author each car type once** as a modular template in the Editor
2. **Procedural variation layer** randomizes props, clutter, lighting damage, and wear within authored constraints
3. **Hero cars** (story-critical locations) are fully hand-placed with no procedural variation

### Modular Geometry Kit

All car interiors are assembled from a shared modular kit. Pieces snap to a **25 cm grid** (quarter-meter) with **90-degree rotation**.

#### Kit Categories

| Category | Pieces | Examples |
|----------|--------|---------|
| **Shell** | 12 | Wall panel (solid, windowed, damaged), floor plate (metal, carpet, grate), ceiling panel (intact, exposed pipes, lighting) |
| **Structure** | 8 | Door frame (standard, bulkhead, security), partition wall (full, half, glass), support column, ceiling beam |
| **Fixtures** | 15 | Bunk (single, double, triple-stack), bench, table, sink, toilet, shower, locker, shelf unit, workbench |
| **Infrastructure** | 10 | Pipe run (straight, elbow, T-junction, valve), conduit, vent cover, junction box, breaker panel, steam vent |
| **Clutter** | 20+ | Bottles, blankets, tools, food containers, papers, clothing piles, buckets, wire coils |
| **Lighting** | 6 | Overhead fluorescent, emergency red, strip LED, oil lamp, broken fixture, window light volume |

#### Naming Convention

```
SM_Kit_<Category>_<Piece>_<Variant>
Example: SM_Kit_Shell_WallPanel_Damaged_02
```

#### Material Instances

Each kit piece uses a **master material** with instance parameters:

| Parameter | Type | Purpose |
|-----------|------|---------|
| BaseColor tint | LinearColor | Zone-specific palette (rust for Tail, brass for Engine) |
| Wear | Float 0-1 | Surface degradation |
| Grime | Float 0-1 | Dirt/grease overlay |
| Damage | Float 0-1 | Structural damage (cracks, dents) |
| Emissive | Float 0-1 | Self-illumination (indicator lights, screens) |

#### Car Template Structure

Each car type is a **Level Instance** containing:

```
BP_CarTemplate_<Type>
├── StaticMeshes (kit pieces, hand-placed)
├── LightingRig (zone-appropriate preset)
├── SpawnPoints[] (NPC, loot, interactable markers)
├── ProceduralClutterVolumes[] (randomized clutter within bounds)
├── NavMesh bounds
└── AudioAmbience (zone preset reference)
```

#### Procedural Variation Rules

| Element | Variation Method |
|---------|-----------------|
| Clutter placement | Random scatter within ProceduralClutterVolumes, density parameter per zone |
| Wall damage | Random Damage parameter (0.0-0.3 First Class, 0.5-1.0 Tail) |
| Lighting flicker | Random timer offset on flickering lights |
| Prop swaps | Random selection from prop pool per spawn point |
| Graffiti/stains | Decal projection with random UV offset and rotation |

---

## Build & Iteration Workflow

### Directory Structure

```
unreal/SnowpiercerEE/
├── Content/
│   ├── Core/           ← Master materials, shared assets
│   ├── Kit/            ← Modular geometry kit (meshes, materials)
│   ├── Cars/
│   │   ├── Templates/  ← Car type templates (Level Instances)
│   │   └── Hero/       ← Hand-authored story cars
│   ├── Characters/     ← Skeletal meshes, animations
│   ├── UI/             ← UMG widgets
│   ├── FX/             ← Niagara particles
│   └── Audio/          ← Sound cues, attenuation presets
└── Source/TrainGame/   ← C++ gameplay modules
```

### Asset Workflow

1. **Modeling**: Blender/Maya → FBX export at kit grid scale (25 cm units)
2. **Texturing**: Substance Painter → 2K textures (albedo, normal, ORM packed)
3. **Import**: FBX → UE5 with Nanite enabled for static meshes >1K triangles
4. **Assembly**: Snap kit pieces in Editor, configure material instances per zone
5. **Variation**: Place ProceduralClutterVolumes, configure spawn pools
6. **Lighting**: Lumen GI (no baked lightmaps — dynamic for streaming compatibility)
7. **Validation**: Per-car performance profile against budgets (see performance-budgets.md)
