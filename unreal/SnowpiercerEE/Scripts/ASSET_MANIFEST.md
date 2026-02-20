# Snowpiercer: Eternal Engine — Asset Manifest

## Quick Start
1. Open project in UE 5.7
2. Tools > Execute Python Script > browse to `Scripts/import_assets.py`
3. Wait for import to complete (~200 assets)
4. Open materials in `/Game/Materials/` and connect texture nodes
5. Create `DevTestCar` map in `/Game/Maps/`

## Source → Content Mapping

### 3D Models (FBX) — 13 meshes
| Source | UE Destination | Description |
|--------|---------------|-------------|
| `Assets/Meshes/ModularPipes/PipeKit_Base_Floor.fbx` | `/Game/Meshes/ModularPipes/` | Floor-mounted pipe base |
| `Assets/Meshes/ModularPipes/PipeKit_Base_Wall.fbx` | `/Game/Meshes/ModularPipes/` | Wall-mounted pipe base |
| `Assets/Meshes/ModularPipes/PipeKit_Connector_*.fbx` (x8) | `/Game/Meshes/ModularPipes/` | Pipe connectors (2-way, 3-way, 4-way, 6-way, crossover, straight) |
| `Assets/Meshes/ModularPipes/PipeKit_Pipe_*.fbx` (x3) | `/Game/Meshes/ModularPipes/` | Pipe segments (short, medium, long) |

### PBR Texture Sets — 4 pipe material sets
| Source | UE Material | Channels |
|--------|------------|----------|
| `Assets/Meshes/ModularPipes/Textures/BareMetal/` | `/Game/Materials/ModularPipes/M_BareMetal` | BaseColor, Height, Metallic, Normal, Roughness |
| `Assets/Meshes/ModularPipes/Textures/PaintedMetal/` | `/Game/Materials/ModularPipes/M_PaintedMetal` | BaseColor, Height, Metallic, Normal, Roughness |
| `Assets/Meshes/ModularPipes/Textures/RustyMetal/` | `/Game/Materials/ModularPipes/M_RustyMetal` | BaseColor, Height, Metallic, Normal, Roughness |
| `Assets/Meshes/ModularPipes/Textures/RustyPaintedMetal/` | `/Game/Materials/ModularPipes/M_RustyPaintedMetal` | BaseColor, Height, Metallic, Normal, Roughness |

### Brass PBR Set — 4K
| Source | UE Destination |
|--------|---------------|
| `Assets/Textures/Brass/4K-Brass_Base Color.jpg` | `/Game/Textures/Brass/` |
| `Assets/Textures/Brass/4K-Brass_Height.jpg` | `/Game/Textures/Brass/` |
| `Assets/Textures/Brass/4K-Brass_Metallic.jpg` | `/Game/Textures/Brass/` |
| `Assets/Textures/Brass/4K-Brass_Normal.jpg` | `/Game/Textures/Brass/` |
| `Assets/Textures/Brass/4K-Brass_Roughness.jpg` | `/Game/Textures/Brass/` |
| **Material** | `/Game/Materials/Metals/M_Brass_4K` |

### Baked Prop Textures — ~100 textures, ~20 props
Each prop has Diffuse + Normal + Specular + AO channels.

| Prop | Files | UE Material |
|------|-------|-------------|
| big_tank | 5 (diff, diff_edited, norm, spec, ao) | `/Game/Materials/Props/M_big_tank` |
| chimney | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_chimney` |
| container | 3 (diff, norm, spec) | `/Game/Materials/Props/M_container` |
| control_terminal | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_control_terminal` |
| garage | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_garage` |
| garage_2 | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_garage_2` |
| ladder | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_ladder` |
| ladder_base | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_ladder_base` |
| roof_thing | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_roof_thing` |
| roof_thing_2 | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_roof_thing_2` |
| roof_thing_3 | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_roof_thing_3` |
| station | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_station` |
| tank | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_tank` |
| tank_2 | 5 (diff, diff_edited, norm, spec, ao) | `/Game/Materials/Props/M_tank_2` |
| tank_capsule | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_tank_capsule` |
| tank_control_panel | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_tank_control_panel` |
| tank_sphere | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_tank_sphere` |
| tank_sphere_holder | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_tank_sphere_holder` |
| tower | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_tower` |
| tower_2 | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_tower_2` |
| tower_part | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_tower_part` |
| vent | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_vent` |
| vent_2 | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_vent_2` |
| vent_3 | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_vent_3` |
| wall_thing | 4 (diff, norm, spec, ao) | `/Game/Materials/Props/M_wall_thing` |

### Loose Metal/Environment Textures — ~45 textures
| Source | UE Destination | Notes |
|--------|---------------|-------|
| `Assets/Textures/Metals/brushed_metal*.jpg/png` (x4) | `/Game/Textures/Metals/` | Diff, variant, normal, spec |
| `Assets/Textures/Metals/concrete*.jpg/png` (x3) | `/Game/Textures/Metals/` | Diff, normal, spec |
| `Assets/Textures/Metals/concrete_wall*.jpg` (x3) | `/Game/Textures/Metals/` | Diff, normal, spec |
| `Assets/Textures/Metals/dirty_metal*.jpg/png` (x3) | `/Game/Textures/Metals/` | Diff, normal, spec |
| `Assets/Textures/Metals/marble001_color_2k.png` | `/Game/Textures/Metals/` | First Class car marble |
| `Assets/Textures/Metals/metal001_color_2k.png` | `/Game/Textures/Metals/` | Generic metal |
| `Assets/Textures/Metals/metal1*.jpg/png` (x3) | `/Game/Textures/Metals/` | Diff, normal, spec |
| `Assets/Textures/Metals/metal2*.jpg` (x2) | `/Game/Textures/Metals/` | Diff, spec |
| `Assets/Textures/Metals/metal_ground*.jpg` (x3) | `/Game/Textures/Metals/` | Floor panels |
| `Assets/Textures/Metals/metal_orange*.jpg` (x2) | `/Game/Textures/Metals/` | Painted metal |
| `Assets/Textures/Metals/metal_pipe*.jpg` (x4) | `/Game/Textures/Metals/` | Pipe detail textures |
| `Assets/Textures/Metals/metal_planks.png` | `/Game/Textures/Metals/` | Walking surface |
| `Assets/Textures/Metals/metal_plate*.jpg/png` (x4) | `/Game/Textures/Metals/` | Wall/floor plates |
| `Assets/Textures/Metals/metal_stuff*.jpg` (x4) | `/Game/Textures/Metals/` | Industrial detail |
| `Assets/Textures/Metals/metals_norm.jpg` | `/Game/Textures/Metals/` | Shared normal |
| `Assets/Textures/Metals/red_metal*.jpg` (x3) | `/Game/Textures/Metals/` | Emergency/signage |
| `Assets/Textures/Metals/rusty_metal_04_diff_4k.png` | `/Game/Textures/Metals/` | 4K rust overlay |
| `Assets/Textures/Metals/scratchy_metal*.jpg` (x4) | `/Game/Textures/Metals/` | Worn surfaces |
| `Assets/Textures/Metals/wire*.jpg/png` (x3) | `/Game/Textures/Metals/` | Electrical wiring |
| `Assets/Textures/Environment/crepe_satin_diff_4k.png` | `/Game/Textures/Environment/` | First Class fabric |

### Audio — 2 files
| Source | UE Destination | Usage |
|--------|---------------|-------|
| `Assets/Audio/train_interior.mp3` | `/Game/Audio/` | Ambient loop for SEETrainMovementComponent |
| `Assets/Audio/train_whistle.mp3` | `/Game/Audio/` | Event SFX |

### Concept Art — 6 reference images
| Source | UE Destination |
|--------|---------------|
| `Assets/ConceptArt/engine_concept.png` | `/Game/Reference/ConceptArt/` |
| `Assets/ConceptArt/first_class_concept.png` | `/Game/Reference/ConceptArt/` |
| `Assets/ConceptArt/sanctum_concept.png` | `/Game/Reference/ConceptArt/` |
| `Assets/ConceptArt/second_class_concept.png` | `/Game/Reference/ConceptArt/` |
| `Assets/ConceptArt/tail_zone_concept.png` | `/Game/Reference/ConceptArt/` |
| `Assets/ConceptArt/third_class_concept.png` | `/Game/Reference/ConceptArt/` |

### Blender Source Files (NOT imported — reference only)
| Source | Description |
|--------|-------------|
| `Assets/Blender/industrial_final.blend` | Final industrial scene (export FBX for additional props) |
| `Assets/Blender/industrial_old.blend` | Legacy version |

## Content Directory Structure (after import)
```
Content/
├── Audio/
│   ├── train_interior          (SoundWave)
│   └── train_whistle           (SoundWave)
├── Blueprints/                 (future BP actors)
├── DataTables/                 (future CollectibleDataTable)
├── Maps/
│   ├── MainMenu                (to create)
│   └── DevTestCar              (to create)
├── Materials/
│   ├── Metals/
│   │   └── M_Brass_4K
│   ├── ModularPipes/
│   │   ├── M_BareMetal
│   │   ├── M_PaintedMetal
│   │   ├── M_RustyMetal
│   │   └── M_RustyPaintedMetal
│   └── Props/
│       ├── M_big_tank
│       ├── M_chimney
│       ├── M_container
│       └── ... (25 prop materials)
├── Meshes/
│   └── ModularPipes/
│       ├── PipeKit_Base_Floor
│       ├── PipeKit_Base_Wall
│       ├── PipeKit_Connector_*  (x8)
│       └── PipeKit_Pipe_*       (x3)
├── Reference/
│   └── ConceptArt/              (6 zone concepts)
└── Textures/
    ├── BakedProps/              (~100 prop textures)
    ├── Brass/                   (5 PBR channels)
    ├── Environment/             (fabric, satin)
    └── Metals/                  (~45 metal/surface textures)
```

## Demo Build Checklist
- [ ] Run `import_assets.py` in UE Editor
- [ ] Connect texture nodes in all M_* materials
- [ ] Create `DevTestCar` level — BSP box 12m x 3m x 2.8m
- [ ] Place modular pipes along walls/ceiling
- [ ] Apply metal materials to BSP surfaces
- [ ] Place `PlayerStart` actor
- [ ] Attach `SEETrainMovementComponent` to level BP
- [ ] Set `train_interior` SoundWave as TrainLoopSound
- [ ] Place test `CollectibleComponent` actors
- [ ] Set DevTestCar as Editor Startup Map
- [ ] PIE test: walk around, pick up collectibles, toggle view (V key)
