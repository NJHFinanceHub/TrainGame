"""
Snowpiercer: Eternal Engine -- Zone 1 (The Tail) Level Builder
Run in editor: Tools > Execute Python Script

Builds all 15 Zone 1 cars as STREAMING SUBLEVELS under a persistent level.
Persistent level: /Game/Maps/Zone1_Tail
Sublevels: /Game/Maps/Zone1/Z1_Car00_Caboose .. /Game/Maps/Zone1/Z1_Car14_Martyrs_Gate

Each sublevel contains one car's geometry, lighting, props, and spawn points
at its correct world-space position along the X axis.  The persistent level
holds atmosphere, global lighting, PlayerStart, and the sublevel streaming
references consumed by USEECarStreamingSubsystem at runtime.

Car Roster:
  0  Caboose (Dead End)     -- sealed rear wall, memorial items, dim lighting
  1  Tail Quarters A        -- triple-stacked bunks, cramped corridors, NPC spawns
  2  Tail Quarters B        -- bunks, communal area, candle-like point lights
  3  The Pit                -- open arena center, raised platform, combat ring
  4  Nursery                -- partitioned spaces, soft lighting, lockable storage
  5  Elders' Car            -- meeting area, bench geometry, quest giver spawn
  6  Sickbay               -- bed geometry, medical station, green-tinted light
  7  Workshop               -- workbench, tool racks, crafting station, industrial light
  8  Listening Post         -- comms equipment, intel collectible spawns
  9  Blockade               -- barricade geometry, combat encounter, 8 enemy spawns
  10 Dark Car               -- minimal lighting, atmospheric fog, hiding spots
  11 Freezer Breach         -- blue-white lighting, ice geometry, cold zone volume
  12 Kronole Den            -- purple/sickly lighting, makeshift furniture, dealer NPC
  13 Smuggler's Cache       -- hidden compartment, trade goods, merchant NPC spawn
  14 Martyr's Gate          -- grand entrance, historical markers, boss arena, dramatic light

Each car: 12000cm x 4000cm x 3000cm (120m x 40m x 30m)  [10x scale]
"""

import unreal

# ---------------------------------------------------------------------------
# API shortcuts
# ---------------------------------------------------------------------------

level_lib = unreal.EditorLevelLibrary
editor_util = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mat_lib = unreal.MaterialEditingLibrary
level_utils = unreal.EditorLevelUtils

# ---------------------------------------------------------------------------
# Dimensions (cm) — 10x scale for massive building-interior feel
# ---------------------------------------------------------------------------

CAR_LENGTH  = 12000.0
CAR_WIDTH   = 4000.0
CAR_HEIGHT  = 3000.0
WALL_THICK  = 150.0
DOOR_WIDTH  = 1500.0
DOOR_HEIGHT = 2300.0
CAR_GAP     = 1000.0      # connector gangway between cars

NUM_CARS    = 15

# ---------------------------------------------------------------------------
# Car definitions
# ---------------------------------------------------------------------------

CAR_DEFS = {
    # idx: (name, light_color_rgb, light_intensity, description)
    0:  ("Caboose",          (200, 120,  60),  1500.0, "Sealed rear wall, memorial items, dim lighting"),
    1:  ("Tail_Quarters_A",  (200, 130,  70),  1800.0, "Triple-stacked bunks, cramped corridors"),
    2:  ("Tail_Quarters_B",  (220, 170, 100),  1600.0, "Bunks, communal area, candle-like lighting"),
    3:  ("The_Pit",          (255, 240, 220),  4000.0, "Open arena center, combat ring"),
    4:  ("Nursery",          (255, 220, 180),  2500.0, "Partitioned spaces, soft lighting"),
    5:  ("Elders_Car",       (240, 200, 140),  2200.0, "Meeting area, bench geometry"),
    6:  ("Sickbay",          (140, 220, 160),  2800.0, "Beds along walls, medical station"),
    7:  ("Workshop",         (255, 200, 120),  3500.0, "Workbench, tool racks, crafting station"),
    8:  ("Listening_Post",   (160, 180, 220),  2000.0, "Comms equipment, intel collectible spawns"),
    9:  ("Blockade",         (255, 160, 100),  3000.0, "Barricade geometry, combat encounter area"),
    10: ("Dark_Car",         (80,   80, 100),   400.0, "Minimal lighting, atmospheric fog, horror"),
    11: ("Freezer_Breach",   (160, 200, 255),  3200.0, "Blue-white lighting, ice geometry"),
    12: ("Kronole_Den",      (180, 100, 200),  1800.0, "Purple/sickly lighting, makeshift furniture"),
    13: ("Smugglers_Cache",  (200, 180, 140),  2000.0, "Hidden compartment, trade goods"),
    14: ("Martyrs_Gate",     (255, 200, 160),  3800.0, "Grand entrance, boss arena, dramatic lighting"),
}

# ---------------------------------------------------------------------------
# Sublevel path helpers
# ---------------------------------------------------------------------------

PERSISTENT_MAP = "/Game/Maps/Zone1_Tail"
SUBLEVEL_DIR   = "/Game/Maps/Zone1"


def sublevel_path(car_idx):
    """Return the asset path for a car sublevel."""
    name = CAR_DEFS[car_idx][0]
    return f"{SUBLEVEL_DIR}/Z1_Car{car_idx:02d}_{name}"


def sublevel_name(car_idx):
    """Return the short level name (FName) used by streaming subsystem."""
    name = CAR_DEFS[car_idx][0]
    return f"Z1_Car{car_idx:02d}_{name}"


# ---------------------------------------------------------------------------
# Material colours per surface type (base_color r, g, b as 0-1 floats)
# ---------------------------------------------------------------------------

SURFACE_COLORS = {
    "Tail_Floor":   (0.03, 0.03, 0.03),
    "Tail_Wall":    (0.05, 0.04, 0.03),
    "Tail_Ceiling": (0.04, 0.03, 0.02),
    "Tail_Door":    (0.07, 0.05, 0.04),
    "Connector":    (0.04, 0.04, 0.05),
    # Per-car accent materials
    "Pit_Sand":       (0.15, 0.12, 0.08),
    "Sickbay_Green":  (0.04, 0.08, 0.05),
    "Workshop_Metal": (0.08, 0.06, 0.04),
    "Ice_Blue":       (0.60, 0.75, 0.90),
    "Kronole_Purple": (0.10, 0.04, 0.12),
    "Martyrs_Stone":  (0.08, 0.07, 0.06),
    "Memorial_Dark":  (0.02, 0.02, 0.02),
    "Nursery_Soft":   (0.09, 0.08, 0.07),
    "Bunk_Wood":      (0.08, 0.06, 0.04),
    "Barricade_Rust": (0.10, 0.05, 0.03),
    "Platform_Metal": (0.06, 0.05, 0.04),
    "Bench_Wood":     (0.07, 0.05, 0.03),
    "Comms_Metal":    (0.06, 0.06, 0.08),
    "Cache_Crate":    (0.06, 0.05, 0.03),
}

# ---------------------------------------------------------------------------
# Material cache & creation
# ---------------------------------------------------------------------------

_mat_cache = {}


def get_material(name, r, g, b, roughness=0.7, metallic=0.8):
    """Get or create a simple PBR material with given base color."""
    if name in _mat_cache:
        return _mat_cache[name]

    mat_path = f"/Game/Materials/Zone1/M_{name}"
    if editor_util.does_asset_exist(mat_path):
        mat = editor_util.load_asset(mat_path)
        _mat_cache[name] = mat
        return mat

    dir_path = "/Game/Materials/Zone1"
    if not editor_util.does_directory_exist(dir_path):
        editor_util.make_directory(dir_path)

    mat = asset_tools.create_asset(f"M_{name}", dir_path,
                                   unreal.Material, unreal.MaterialFactoryNew())
    if not mat:
        unreal.log_warning(f"  Failed to create material: {name}")
        return None

    color_node = mat_lib.create_material_expression(
        mat, unreal.MaterialExpressionConstant3Vector, -300, -200)
    if color_node:
        color_node.set_editor_property("constant", unreal.LinearColor(r, g, b, 1.0))
        mat_lib.connect_material_property(color_node, "", unreal.MaterialProperty.MP_BASE_COLOR)

    rough_node = mat_lib.create_material_expression(
        mat, unreal.MaterialExpressionConstant, -300, 0)
    if rough_node:
        rough_node.set_editor_property("r", roughness)
        mat_lib.connect_material_property(rough_node, "", unreal.MaterialProperty.MP_ROUGHNESS)

    metal_node = mat_lib.create_material_expression(
        mat, unreal.MaterialExpressionConstant, -300, 200)
    if metal_node:
        metal_node.set_editor_property("r", metallic)
        mat_lib.connect_material_property(metal_node, "", unreal.MaterialProperty.MP_METALLIC)

    mat_lib.recompile_material(mat)

    # Persist the material as a .uasset so it survives editor restarts
    try:
        editor_util.save_asset(mat_path, only_if_is_dirty=False)
    except Exception as e:
        unreal.log_warning(f"  Could not save material {mat_path}: {e}")

    _mat_cache[name] = mat
    return mat


def _load_surface_mats():
    """Pre-create all surface materials."""
    mats = {}
    for name, (r, g, b) in SURFACE_COLORS.items():
        rough = 0.85 if "Floor" in name else 0.75
        metal = 0.9 if "Metal" in name else 0.7
        mats[name] = get_material(name, r, g, b, roughness=rough, metallic=metal)
    return mats


# ---------------------------------------------------------------------------
# Geometry helpers
# ---------------------------------------------------------------------------

_cube_mesh = None


def get_cube():
    global _cube_mesh
    if _cube_mesh is None:
        _cube_mesh = editor_util.load_asset("/Engine/BasicShapes/Cube")
    return _cube_mesh


def place_box(label, location, size, material=None):
    """Spawn a scaled cube static mesh actor. size = (x, y, z) in cm."""
    actor = level_lib.spawn_actor_from_class(
        unreal.StaticMeshActor,
        location,
        unreal.Rotator(0.0, 0.0, 0.0)
    )
    if not actor:
        return None
    actor.set_actor_label(label)
    sm = actor.get_component_by_class(unreal.StaticMeshComponent)
    if sm:
        cube = get_cube()
        if cube:
            sm.set_static_mesh(cube)
            # UE Cube is 100x100x100 by default; scale accordingly
            actor.set_actor_scale3d(unreal.Vector(
                size[0] / 100.0,
                size[1] / 100.0,
                size[2] / 100.0
            ))
            if material:
                sm.set_material(0, material)
    return actor


def place_light(label, location, color_rgb, intensity, radius=8000.0):
    """Spawn a point light with given color and intensity."""
    pl = level_lib.spawn_actor_from_class(
        unreal.PointLight,
        location,
        unreal.Rotator(0.0, 0.0, 0.0)
    )
    if not pl:
        return None
    pl.set_actor_label(label)
    plc = pl.get_component_by_class(unreal.PointLightComponent)
    if plc:
        plc.set_editor_property("intensity", intensity)
        plc.set_editor_property("attenuation_radius", radius)
        plc.set_editor_property("light_color",
            unreal.Color(int(color_rgb[0]), int(color_rgb[1]), int(color_rgb[2]), 255))
    return pl


def place_target_point(label, location):
    """Spawn a TargetPoint actor (used as NPC/item spawn marker)."""
    tp = level_lib.spawn_actor_from_class(
        unreal.TargetPoint,
        location,
        unreal.Rotator(0.0, 0.0, 0.0)
    )
    if tp:
        tp.set_actor_label(label)
    return tp


def try_place_pipe(label, mesh_name, location, rotation=None):
    """Attempt to place a pipe mesh from /Game/Meshes/ModularPipes/.
    Falls back to a thin box if the mesh is not available."""
    if rotation is None:
        rotation = unreal.Rotator(0.0, 0.0, 0.0)
    mesh_path = f"/Game/Meshes/ModularPipes/{mesh_name}"
    if editor_util.does_asset_exist(mesh_path):
        mesh = editor_util.load_asset(mesh_path)
        actor = level_lib.spawn_actor_from_class(
            unreal.StaticMeshActor, location, rotation)
        if actor:
            actor.set_actor_label(label)
            sm = actor.get_component_by_class(unreal.StaticMeshComponent)
            if sm:
                sm.set_static_mesh(mesh)
                # Try to apply rusty metal material
                mat_path = "/Game/Materials/ModularPipes/M_RustyMetal"
                if editor_util.does_asset_exist(mat_path):
                    mat = editor_util.load_asset(mat_path)
                    sm.set_material(0, mat)
            return actor
    # Fallback: thin box as pipe stand-in
    mat_metal = _mat_cache.get("Tail_Wall")
    return place_box(label, location, (2000.0, 80.0, 80.0), mat_metal)


# ---------------------------------------------------------------------------
# Cleanup -- remove previously placed Zone1 actors from ALL sublevels
# ---------------------------------------------------------------------------

def cleanup():
    unreal.log("=== Cleaning previous Zone 1 actors ===")
    actors = level_lib.get_all_level_actors()
    removed = 0
    prefixes = (
        "Z1_", "Car", "Conn_", "Light_", "Pipe_Z1", "Spawn_", "Sign_",
        "PlayerStart", "Sky", "Sun", "Fog_Z1", "PostProcess_Z1",
        "Ambient", "Volume_",
    )
    for actor in actors:
        if isinstance(actor, unreal.WorldSettings):
            continue
        label = actor.get_actor_label()
        if any(label.startswith(p) for p in prefixes):
            level_lib.destroy_actor(actor)
            removed += 1
    unreal.log(f"  Removed {removed} actors")


# ---------------------------------------------------------------------------
# Shared car shell builder
# ---------------------------------------------------------------------------

_actor_count = 0


def _inc(n=1):
    global _actor_count
    _actor_count += n


def build_car_shell(car_idx, car_x, mats):
    """Build floor, ceiling, walls, doors for a single car. Returns nothing."""
    name = CAR_DEFS[car_idx][0]
    prefix = f"Z1_Car{car_idx:02d}_{name}"

    half_l = CAR_LENGTH / 2.0
    half_w = CAR_WIDTH / 2.0
    half_h = CAR_HEIGHT / 2.0
    mat_floor = mats["Tail_Floor"]
    mat_wall  = mats["Tail_Wall"]
    mat_ceil  = mats["Tail_Ceiling"]
    mat_door  = mats["Tail_Door"]
    mat_conn  = mats["Connector"]

    # Floor
    place_box(f"{prefix}_Floor",
              unreal.Vector(car_x, 0.0, -WALL_THICK / 2.0),
              (CAR_LENGTH, CAR_WIDTH, WALL_THICK), mat_floor)
    # Ceiling
    place_box(f"{prefix}_Ceiling",
              unreal.Vector(car_x, 0.0, CAR_HEIGHT + WALL_THICK / 2.0),
              (CAR_LENGTH, CAR_WIDTH, WALL_THICK), mat_ceil)
    # Left wall
    place_box(f"{prefix}_WallL",
              unreal.Vector(car_x, half_w + WALL_THICK / 2.0, half_h),
              (CAR_LENGTH, WALL_THICK, CAR_HEIGHT), mat_wall)
    # Right wall
    place_box(f"{prefix}_WallR",
              unreal.Vector(car_x, -(half_w + WALL_THICK / 2.0), half_h),
              (CAR_LENGTH, WALL_THICK, CAR_HEIGHT), mat_wall)
    _inc(4)

    # ---- Front wall / door ----
    front_x = car_x + half_l + WALL_THICK / 2.0
    if car_idx < NUM_CARS - 1:
        side_w = (CAR_WIDTH - DOOR_WIDTH) / 2.0
        place_box(f"{prefix}_FrontL",
                  unreal.Vector(front_x, DOOR_WIDTH / 2.0 + side_w / 2.0, half_h),
                  (WALL_THICK, side_w, CAR_HEIGHT), mat_wall)
        place_box(f"{prefix}_FrontR",
                  unreal.Vector(front_x, -(DOOR_WIDTH / 2.0 + side_w / 2.0), half_h),
                  (WALL_THICK, side_w, CAR_HEIGHT), mat_wall)
        header_h = CAR_HEIGHT - DOOR_HEIGHT
        place_box(f"{prefix}_FrontHeader",
                  unreal.Vector(front_x, 0.0, DOOR_HEIGHT + header_h / 2.0),
                  (WALL_THICK, DOOR_WIDTH, header_h), mat_door)

        # Connector gangway (belongs to this car's sublevel)
        conn_x = car_x + half_l + WALL_THICK + CAR_GAP / 2.0
        place_box(f"Conn_{car_idx:02d}_Floor",
                  unreal.Vector(conn_x, 0.0, -WALL_THICK / 2.0),
                  (CAR_GAP, DOOR_WIDTH, WALL_THICK), mat_conn)
        place_box(f"Conn_{car_idx:02d}_WL",
                  unreal.Vector(conn_x, DOOR_WIDTH / 2.0 + WALL_THICK / 2.0, DOOR_HEIGHT / 2.0),
                  (CAR_GAP, WALL_THICK, DOOR_HEIGHT), mat_conn)
        place_box(f"Conn_{car_idx:02d}_WR",
                  unreal.Vector(conn_x, -(DOOR_WIDTH / 2.0 + WALL_THICK / 2.0), DOOR_HEIGHT / 2.0),
                  (CAR_GAP, WALL_THICK, DOOR_HEIGHT), mat_conn)
        place_box(f"Conn_{car_idx:02d}_Ceil",
                  unreal.Vector(conn_x, 0.0, DOOR_HEIGHT + WALL_THICK / 2.0),
                  (CAR_GAP, DOOR_WIDTH + WALL_THICK * 2.0, WALL_THICK), mat_conn)
        _inc(7)
    else:
        # Last car -- solid front wall
        place_box(f"{prefix}_FrontWall",
                  unreal.Vector(front_x, 0.0, half_h),
                  (WALL_THICK, CAR_WIDTH, CAR_HEIGHT), mat_wall)
        _inc(1)

    # ---- Back wall / door ----
    back_x = car_x - half_l - WALL_THICK / 2.0
    if car_idx > 0:
        side_w = (CAR_WIDTH - DOOR_WIDTH) / 2.0
        place_box(f"{prefix}_BackL",
                  unreal.Vector(back_x, DOOR_WIDTH / 2.0 + side_w / 2.0, half_h),
                  (WALL_THICK, side_w, CAR_HEIGHT), mat_wall)
        place_box(f"{prefix}_BackR",
                  unreal.Vector(back_x, -(DOOR_WIDTH / 2.0 + side_w / 2.0), half_h),
                  (WALL_THICK, side_w, CAR_HEIGHT), mat_wall)
        header_h = CAR_HEIGHT - DOOR_HEIGHT
        place_box(f"{prefix}_BackHeader",
                  unreal.Vector(back_x, 0.0, DOOR_HEIGHT + header_h / 2.0),
                  (WALL_THICK, DOOR_WIDTH, header_h), mat_door)
        _inc(3)
    else:
        # Car 0 -- sealed rear wall
        place_box(f"{prefix}_BackWall_Sealed",
                  unreal.Vector(back_x, 0.0, half_h),
                  (WALL_THICK, CAR_WIDTH, CAR_HEIGHT), mat_wall)
        _inc(1)


def build_car_lights(car_idx, car_x):
    """Place themed point lights for a car."""
    name, light_c, light_i, _ = CAR_DEFS[car_idx]
    prefix = f"Light_Z1_Car{car_idx:02d}"
    half_l = CAR_LENGTH / 2.0

    # At least 2 lights per car, spaced along length
    num_lights = max(2, int(CAR_LENGTH / 5000.0))
    for li in range(num_lights):
        lx = car_x - half_l + (li + 0.5) * (CAR_LENGTH / num_lights)
        ly = 800.0 if (li % 2 == 0) else -800.0
        place_light(f"{prefix}_{li}",
                    unreal.Vector(lx, ly, CAR_HEIGHT - 400.0),
                    light_c, light_i, radius=7000.0)
        _inc()


def build_car_pipes(car_idx, car_x):
    """Place pipe meshes along ceiling/walls."""
    name = CAR_DEFS[car_idx][0]
    prefix = f"Pipe_Z1_Car{car_idx:02d}"
    half_l = CAR_LENGTH / 2.0
    half_w = CAR_WIDTH / 2.0

    pipe_meshes = [
        "PipeKit_Pipe_Long",
        "PipeKit_Pipe_Medium",
        "PipeKit_Pipe_Short",
    ]

    # Two pipe runs along left ceiling, two along right ceiling
    positions = [
        # (y_offset, z_offset)
        (half_w - 300.0,  CAR_HEIGHT - 200.0),
        (-(half_w - 300.0), CAR_HEIGHT - 200.0),
    ]

    pipe_idx = 0
    for y_off, z_off in positions:
        # Place 2 pipe segments per run
        for seg in range(2):
            seg_x = car_x - half_l / 2.0 + seg * (half_l)
            mesh_name = pipe_meshes[pipe_idx % len(pipe_meshes)]
            try_place_pipe(
                f"{prefix}_{pipe_idx}",
                mesh_name,
                unreal.Vector(seg_x, y_off, z_off),
                unreal.Rotator(0.0, 0.0, 0.0)
            )
            pipe_idx += 1
            _inc()


def build_car_sign(car_idx, car_x):
    """Place a directional sign/marker near the front door of each car."""
    name = CAR_DEFS[car_idx][0]
    half_l = CAR_LENGTH / 2.0
    half_w = CAR_WIDTH / 2.0
    # Small box as sign placeholder near front-left wall
    mat = _mat_cache.get("Tail_Door")
    sign_x = car_x + half_l - 600.0
    place_box(f"Sign_Z1_Car{car_idx:02d}_{name}",
              unreal.Vector(sign_x, half_w - 50.0, 2000.0),
              (400.0, 20.0, 200.0), mat)
    _inc()


# ---------------------------------------------------------------------------
# Per-car interior detail builders
# ---------------------------------------------------------------------------

def _bunk_stack(prefix, cx, cy, cz, mat, count=3):
    """Place a vertical stack of bunk boxes."""
    bunk_h = 500.0
    bunk_gap = 100.0
    actors = []
    for i in range(count):
        bz = cz + i * (bunk_h + bunk_gap)
        a = place_box(f"{prefix}_bunk{i}",
                      unreal.Vector(cx, cy, bz),
                      (1800.0, 800.0, bunk_h), mat)
        actors.append(a)
        _inc()
    return actors


def build_car00_caboose(car_x, mats):
    """Car 0: Caboose -- memorial wall, narrow space, dim ambiance."""
    p = "Z1_Car00_Caboose"
    half_l = CAR_LENGTH / 2.0
    bunk_mat = mats.get("Bunk_Wood")
    memorial_mat = mats.get("Memorial_Dark")

    # Memorial items on rear wall -- small boxes representing photos/flowers
    rear_x = car_x - half_l + 300.0
    for i in range(5):
        place_box(f"{p}_Memorial_{i}",
                  unreal.Vector(rear_x, -1200.0 + i * 600.0, 1500.0 + (i % 3) * 300.0),
                  (50.0, 300.0, 300.0), memorial_mat)
        _inc()

    # Narrow partitions dividing the space
    place_box(f"{p}_Partition_A",
              unreal.Vector(car_x - 2000.0, 0.0, CAR_HEIGHT / 2.0),
              (50.0, CAR_WIDTH * 0.6, CAR_HEIGHT * 0.7), mats.get("Tail_Wall"))
    place_box(f"{p}_Partition_B",
              unreal.Vector(car_x + 2000.0, 0.0, CAR_HEIGHT / 2.0),
              (50.0, CAR_WIDTH * 0.6, CAR_HEIGHT * 0.7), mats.get("Tail_Wall"))
    _inc(2)

    # Bunks along walls
    _bunk_stack(f"{p}_BunkL1", car_x - 3500.0, 1500.0, 300.0, bunk_mat)
    _bunk_stack(f"{p}_BunkR1", car_x - 3500.0, -1500.0, 300.0, bunk_mat)
    _bunk_stack(f"{p}_BunkL2", car_x + 1000.0, 1500.0, 300.0, bunk_mat)
    _bunk_stack(f"{p}_BunkR2", car_x + 1000.0, -1500.0, 300.0, bunk_mat)

    # Brazier (central gathering fire -- small bright warm light)
    place_box(f"{p}_Brazier",
              unreal.Vector(car_x, 0.0, 300.0),
              (400.0, 400.0, 500.0), memorial_mat)
    place_light(f"{p}_BrazierLight",
                unreal.Vector(car_x, 0.0, 800.0),
                (255, 140, 40), 800.0, radius=3000.0)
    _inc(2)

    unreal.log(f"  Car 00 Caboose: memorial wall, brazier, bunks")


def build_car01_quarters_a(car_x, mats):
    """Car 1: Tail Quarters A -- triple bunks, cramped corridors, NPC spawns."""
    p = "Z1_Car01_TailQA"
    bunk_mat = mats.get("Bunk_Wood")
    half_l = CAR_LENGTH / 2.0

    # 6 bunk stacks along the length (3 left, 3 right)
    for i in range(3):
        bx = car_x - half_l + 1500.0 + i * 3500.0
        _bunk_stack(f"{p}_BunkL{i}", bx, 1400.0, 300.0, bunk_mat)
        _bunk_stack(f"{p}_BunkR{i}", bx, -1400.0, 300.0, bunk_mat)

    # Narrow corridor partition
    place_box(f"{p}_Corridor",
              unreal.Vector(car_x, 0.0, 100.0),
              (CAR_LENGTH * 0.8, 50.0, 200.0), mats.get("Tail_Floor"))
    _inc()

    # 5 NPC spawn points
    for i in range(5):
        sx = car_x - half_l + 1200.0 + i * 2200.0
        sy = 500.0 if (i % 2 == 0) else -500.0
        place_target_point(f"Spawn_{p}_NPC_{i}",
                           unreal.Vector(sx, sy, 500.0))
        _inc()
    unreal.log(f"  Car 01 Tail Quarters A: 6 bunk stacks, 5 NPC spawns")


def build_car02_quarters_b(car_x, mats):
    """Car 2: Tail Quarters B -- bunks, communal center, candle lights."""
    p = "Z1_Car02_TailQB"
    bunk_mat = mats.get("Bunk_Wood")
    half_l = CAR_LENGTH / 2.0

    # Bunks on ends, open communal area in center
    for i in range(2):
        bx = car_x - half_l + 1500.0 + i * 8000.0
        _bunk_stack(f"{p}_BunkL{i}", bx, 1400.0, 300.0, bunk_mat)
        _bunk_stack(f"{p}_BunkR{i}", bx, -1400.0, 300.0, bunk_mat)

    # Communal table in center
    place_box(f"{p}_Table",
              unreal.Vector(car_x, 0.0, 400.0),
              (2000.0, 1000.0, 100.0), bunk_mat)
    _inc()

    # Crate seats around table
    for i, (dx, dy) in enumerate([(-800, 600), (800, 600), (-800, -600), (800, -600)]):
        place_box(f"{p}_Seat_{i}",
                  unreal.Vector(car_x + dx, dy, 200.0),
                  (300.0, 300.0, 300.0), mats.get("Cache_Crate"))
        _inc()

    # Candle-like warm point lights (small, low intensity, warm orange)
    for i in range(3):
        cx = car_x - 3000.0 + i * 3000.0
        place_light(f"{p}_Candle_{i}",
                    unreal.Vector(cx, 0.0, 600.0),
                    (255, 180, 80), 600.0, radius=2500.0)
        _inc()
    unreal.log(f"  Car 02 Tail Quarters B: bunks, communal table, candle lights")


def build_car03_pit(car_x, mats):
    """Car 3: The Pit -- open arena, raised platform, combat ring."""
    p = "Z1_Car03_ThePit"
    pit_mat = mats.get("Pit_Sand")
    platform_mat = mats.get("Platform_Metal")

    # Raised platform/ring in center (slightly elevated floor)
    place_box(f"{p}_RingFloor",
              unreal.Vector(car_x, 0.0, 100.0),
              (5000.0, 3000.0, 200.0), pit_mat)
    _inc()

    # Corner posts (4)
    for i, (dx, dy) in enumerate([(-2000, 1200), (2000, 1200), (-2000, -1200), (2000, -1200)]):
        place_box(f"{p}_Post_{i}",
                  unreal.Vector(car_x + dx, dy, 600.0),
                  (100.0, 100.0, 1000.0), platform_mat)
        _inc()

    # Spectator benches along walls (crate seats)
    half_l = CAR_LENGTH / 2.0
    for i in range(4):
        bx = car_x - half_l + 1000.0 + i * 3000.0
        place_box(f"{p}_BenchL_{i}",
                  unreal.Vector(bx, 1700.0, 200.0),
                  (800.0, 300.0, 300.0), mats.get("Bench_Wood"))
        place_box(f"{p}_BenchR_{i}",
                  unreal.Vector(bx, -1700.0, 200.0),
                  (800.0, 300.0, 300.0), mats.get("Bench_Wood"))
        _inc(2)

    # Bright overhead spotlight on ring
    place_light(f"{p}_Spotlight",
                unreal.Vector(car_x, 0.0, CAR_HEIGHT - 300.0),
                (255, 250, 240), 6000.0, radius=5000.0)
    _inc()
    unreal.log(f"  Car 03 The Pit: ring floor, posts, benches, spotlight")


def build_car04_nursery(car_x, mats):
    """Car 4: Nursery -- partitions, soft lighting, lockable storage."""
    p = "Z1_Car04_Nursery"
    nursery_mat = mats.get("Nursery_Soft")
    half_l = CAR_LENGTH / 2.0

    # 3 partition walls creating small rooms
    for i in range(3):
        px = car_x - half_l + 3000.0 + i * 3000.0
        place_box(f"{p}_Partition_{i}",
                  unreal.Vector(px, 0.0, CAR_HEIGHT * 0.4),
                  (50.0, CAR_WIDTH * 0.5, CAR_HEIGHT * 0.8), nursery_mat)
        _inc()

    # Small cribs/sleeping areas in each partition
    for i in range(4):
        cx = car_x - half_l + 1500.0 + i * 2800.0
        cy = 800.0 if (i % 2 == 0) else -800.0
        place_box(f"{p}_Crib_{i}",
                  unreal.Vector(cx, cy, 200.0),
                  (800.0, 500.0, 300.0), mats.get("Bunk_Wood"))
        _inc()

    # Lockable storage area (quest item) -- reinforced box at far end
    place_box(f"{p}_LockedStorage",
              unreal.Vector(car_x + half_l - 1000.0, 0.0, 400.0),
              (600.0, 600.0, 600.0), mats.get("Barricade_Rust"))
    place_target_point(f"Spawn_{p}_QuestItem",
                       unreal.Vector(car_x + half_l - 1000.0, 0.0, 800.0))
    _inc(2)

    # Soft warm lights
    for i in range(3):
        lx = car_x - 3000.0 + i * 3000.0
        place_light(f"{p}_SoftLight_{i}",
                    unreal.Vector(lx, 0.0, CAR_HEIGHT - 500.0),
                    (255, 220, 180), 1500.0, radius=4000.0)
        _inc()
    unreal.log(f"  Car 04 Nursery: partitions, cribs, locked storage, soft lights")


def build_car05_elders(car_x, mats):
    """Car 5: Elders' Car -- meeting area, benches, quest giver spawn."""
    p = "Z1_Car05_Elders"
    bench_mat = mats.get("Bench_Wood")
    half_l = CAR_LENGTH / 2.0

    # Large meeting table
    place_box(f"{p}_Table",
              unreal.Vector(car_x, 0.0, 400.0),
              (3000.0, 1200.0, 100.0), bench_mat)
    _inc()

    # Benches around table
    for i, (dx, dy) in enumerate([
        (-1200, 800), (0, 800), (1200, 800),
        (-1200, -800), (0, -800), (1200, -800)
    ]):
        place_box(f"{p}_Bench_{i}",
                  unreal.Vector(car_x + dx, dy, 200.0),
                  (600.0, 200.0, 300.0), bench_mat)
        _inc()

    # Map on wall (flat box)
    place_box(f"{p}_WallMap",
              unreal.Vector(car_x - 1000.0, CAR_WIDTH / 2.0 - 50.0, 1700.0),
              (1200.0, 20.0, 800.0), mats.get("Memorial_Dark"))
    _inc()

    # Quest giver NPC spawn (Gilliam)
    place_target_point(f"Spawn_{p}_Gilliam",
                       unreal.Vector(car_x, 0.0, 500.0))
    _inc()

    # Warm lighting (candles, brazier feel)
    place_light(f"{p}_WarmLight_A",
                unreal.Vector(car_x - 2000.0, 0.0, CAR_HEIGHT - 500.0),
                (240, 200, 140), 2000.0, radius=5000.0)
    place_light(f"{p}_WarmLight_B",
                unreal.Vector(car_x + 2000.0, 0.0, CAR_HEIGHT - 500.0),
                (240, 200, 140), 2000.0, radius=5000.0)
    _inc(2)
    unreal.log(f"  Car 05 Elders: meeting table, benches, map, Gilliam spawn")


def build_car06_sickbay(car_x, mats):
    """Car 6: Sickbay -- beds along walls, medical station, green light."""
    p = "Z1_Car06_Sickbay"
    bunk_mat = mats.get("Bunk_Wood")
    sickbay_mat = mats.get("Sickbay_Green")
    half_l = CAR_LENGTH / 2.0

    # 4 beds along each wall
    for i in range(4):
        bx = car_x - half_l + 1500.0 + i * 2500.0
        place_box(f"{p}_BedL_{i}",
                  unreal.Vector(bx, 1400.0, 250.0),
                  (1600.0, 700.0, 400.0), bunk_mat)
        place_box(f"{p}_BedR_{i}",
                  unreal.Vector(bx, -1400.0, 250.0),
                  (1600.0, 700.0, 400.0), bunk_mat)
        _inc(2)

    # Medical station -- table with supplies
    place_box(f"{p}_MedTable",
              unreal.Vector(car_x + 3000.0, 0.0, 450.0),
              (1200.0, 800.0, 100.0), sickbay_mat)
    place_box(f"{p}_MedCabinet",
              unreal.Vector(car_x + 3000.0, 0.0, 1000.0),
              (600.0, 400.0, 800.0), sickbay_mat)
    _inc(2)

    # Doctor NPC spawn
    place_target_point(f"Spawn_{p}_DrAsha",
                       unreal.Vector(car_x + 3000.0, 400.0, 500.0))
    _inc()

    # Green-tinted lights
    for i in range(3):
        lx = car_x - 3000.0 + i * 3000.0
        place_light(f"{p}_GreenLight_{i}",
                    unreal.Vector(lx, 0.0, CAR_HEIGHT - 400.0),
                    (140, 220, 160), 2800.0, radius=5000.0)
        _inc()
    unreal.log(f"  Car 06 Sickbay: 8 beds, medical station, Dr. Asha spawn, green lights")


def build_car07_workshop(car_x, mats):
    """Car 7: Workshop -- workbench, tool racks, crafting station, industrial light."""
    p = "Z1_Car07_Workshop"
    metal_mat = mats.get("Workshop_Metal")
    bench_mat = mats.get("Bench_Wood")
    half_l = CAR_LENGTH / 2.0

    # False wall partition at entrance (decoy living space)
    place_box(f"{p}_FalseWall",
              unreal.Vector(car_x - half_l + 2500.0, 0.0, CAR_HEIGHT * 0.45),
              (80.0, CAR_WIDTH * 0.8, CAR_HEIGHT * 0.9), mats.get("Tail_Wall"))
    _inc()

    # Main workbench (large)
    place_box(f"{p}_Workbench",
              unreal.Vector(car_x, -1000.0, 450.0),
              (2500.0, 800.0, 100.0), bench_mat)
    _inc()

    # Anvil / forge station
    place_box(f"{p}_Anvil",
              unreal.Vector(car_x - 1000.0, 1000.0, 350.0),
              (500.0, 300.0, 600.0), metal_mat)
    _inc()

    # Tool racks on left wall (flat boxes)
    for i in range(3):
        place_box(f"{p}_ToolRack_{i}",
                  unreal.Vector(car_x + 2000.0, CAR_WIDTH / 2.0 - 80.0, 1300.0 + i * 400.0),
                  (1200.0, 50.0, 300.0), metal_mat)
        _inc()

    # Crafting station marker
    place_target_point(f"Spawn_{p}_CraftStation",
                       unreal.Vector(car_x, -1000.0, 600.0))
    _inc()

    # Forge NPC spawn
    place_target_point(f"Spawn_{p}_Forge",
                       unreal.Vector(car_x - 1000.0, 1000.0, 500.0))
    _inc()

    # Testing dummy
    place_box(f"{p}_TestDummy",
              unreal.Vector(car_x + 3500.0, 0.0, 800.0),
              (200.0, 200.0, 1400.0), mats.get("Bunk_Wood"))
    _inc()

    # Industrial overhead lights (bright, harsh)
    for i in range(3):
        lx = car_x - 2000.0 + i * 2000.0
        place_light(f"{p}_IndustLight_{i}",
                    unreal.Vector(lx, 0.0, CAR_HEIGHT - 300.0),
                    (255, 220, 160), 4000.0, radius=6000.0)
        _inc()
    unreal.log(f"  Car 07 Workshop: false wall, workbench, anvil, tool racks, test dummy")


def build_car08_listening_post(car_x, mats):
    """Car 8: Listening Post -- comms equipment, intel spawn points."""
    p = "Z1_Car08_ListenPost"
    comms_mat = mats.get("Comms_Metal")
    half_l = CAR_LENGTH / 2.0

    # Communications console (large table with equipment boxes)
    place_box(f"{p}_ConsoleTable",
              unreal.Vector(car_x - 1000.0, 0.0, 450.0),
              (2000.0, 1000.0, 100.0), comms_mat)
    _inc()

    # Equipment boxes on table
    for i in range(3):
        place_box(f"{p}_Equipment_{i}",
                  unreal.Vector(car_x - 1800.0 + i * 800.0, 0.0, 700.0),
                  (500.0, 400.0, 400.0), comms_mat)
        _inc()

    # Antenna/wire assembly on ceiling (tall thin box)
    place_box(f"{p}_Antenna",
              unreal.Vector(car_x, 0.0, CAR_HEIGHT - 300.0),
              (100.0, 100.0, 500.0), comms_mat)
    _inc()

    # Intel collectible spawn points (4 locations)
    intel_positions = [
        (car_x - 3000.0, 1000.0),
        (car_x - 1000.0, -800.0),
        (car_x + 1500.0, 600.0),
        (car_x + 4000.0, -1000.0),
    ]
    for i, (ix, iy) in enumerate(intel_positions):
        place_target_point(f"Spawn_{p}_Intel_{i}",
                           unreal.Vector(ix, iy, 500.0))
        _inc()

    # Whisper NPC spawn
    place_target_point(f"Spawn_{p}_Whisper",
                       unreal.Vector(car_x - 1000.0, 500.0, 500.0))
    _inc()

    # Blue-ish overhead lights
    for i in range(2):
        lx = car_x - 2000.0 + i * 4000.0
        place_light(f"{p}_CommsLight_{i}",
                    unreal.Vector(lx, 0.0, CAR_HEIGHT - 400.0),
                    (160, 180, 220), 2200.0, radius=5000.0)
        _inc()
    unreal.log(f"  Car 08 Listening Post: console, equipment, 4 intel spawns, Whisper spawn")


def build_car09_blockade(car_x, mats):
    """Car 9: Blockade -- barricade, combat arena, 8 enemy spawns."""
    p = "Z1_Car09_Blockade"
    barricade_mat = mats.get("Barricade_Rust")
    metal_mat = mats.get("Platform_Metal")
    half_l = CAR_LENGTH / 2.0

    # Main barricade across center
    place_box(f"{p}_Barricade",
              unreal.Vector(car_x, 0.0, 500.0),
              (300.0, CAR_WIDTH * 0.85, 1000.0), barricade_mat)
    _inc()

    # Sandbag positions (3)
    for i, dy in enumerate([-1000.0, 0.0, 1000.0]):
        place_box(f"{p}_Sandbag_{i}",
                  unreal.Vector(car_x + 1000.0, dy, 250.0),
                  (400.0, 600.0, 400.0), barricade_mat)
        _inc()

    # Guard booth
    place_box(f"{p}_GuardBooth",
              unreal.Vector(car_x + 3500.0, 1200.0, 700.0),
              (800.0, 600.0, 1200.0), metal_mat)
    _inc()

    # Overhead catwalk
    place_box(f"{p}_Catwalk",
              unreal.Vector(car_x, CAR_WIDTH / 2.0 - 400.0, 1800.0),
              (6000.0, 400.0, 50.0), metal_mat)
    _inc()

    # Steam pipe valve (interactable marker)
    place_box(f"{p}_SteamValve",
              unreal.Vector(car_x - 2000.0, CAR_WIDTH / 2.0 - 150.0, 1200.0),
              (150.0, 150.0, 150.0), mats.get("Comms_Metal"))
    place_target_point(f"Spawn_{p}_SteamValve",
                       unreal.Vector(car_x - 2000.0, CAR_WIDTH / 2.0 - 150.0, 1200.0))
    _inc(2)

    # Electrical panel
    place_box(f"{p}_ElecPanel",
              unreal.Vector(car_x + 1000.0, -(CAR_WIDTH / 2.0 - 100.0), 1400.0),
              (300.0, 50.0, 500.0), mats.get("Comms_Metal"))
    _inc()

    # 8 enemy spawn points
    enemy_positions = [
        (-3000.0,  800.0), (-3000.0, -800.0),   # approach patrol
        ( 500.0,  1000.0), ( 500.0, -1000.0),   # behind barricade
        ( 1500.0,  500.0), ( 1500.0, -500.0),   # sandbag positions
        ( 3000.0,  0.0),                          # gate area
        ( 3500.0, 1200.0),                        # guard booth
    ]
    for i, (dx, dy) in enumerate(enemy_positions):
        place_target_point(f"Spawn_{p}_Enemy_{i}",
                           unreal.Vector(car_x + dx, dy, 500.0))
        _inc()

    # Harsh overhead lights on Jackboot side, dim on Tail side
    place_light(f"{p}_Light_TailSide",
                unreal.Vector(car_x - 3000.0, 0.0, CAR_HEIGHT - 300.0),
                (180, 140, 100), 1500.0, radius=4000.0)
    place_light(f"{p}_Light_JackbootSide",
                unreal.Vector(car_x + 2000.0, 0.0, CAR_HEIGHT - 300.0),
                (255, 240, 220), 4500.0, radius=6000.0)
    _inc(2)
    unreal.log(f"  Car 09 Blockade: barricade, sandbags, guard booth, catwalk, 8 enemy spawns")


def build_car10_dark_car(car_x, mats):
    """Car 10: Dark Car -- minimal lighting, fog volume, hiding spots."""
    p = "Z1_Car10_DarkCar"
    half_l = CAR_LENGTH / 2.0

    # Overturned bunk debris (maze obstacles)
    debris_mat = mats.get("Bunk_Wood")
    debris_positions = [
        (-3500.0,  800.0, 500.0, 1500.0, 600.0, 800.0, 25.0),
        (-2000.0, -600.0, 400.0, 1000.0, 800.0, 700.0, -15.0),
        ( -500.0,  400.0, 600.0, 1200.0, 500.0, 900.0, 10.0),
        ( 1000.0, -900.0, 300.0, 800.0, 700.0, 600.0, 40.0),
        ( 2500.0,  700.0, 500.0, 1400.0, 400.0, 700.0, -20.0),
        ( 4000.0, -400.0, 450.0, 1100.0, 600.0, 800.0, 5.0),
    ]
    for i, (dx, dy, dz, sx, sy, sz, _) in enumerate(debris_positions):
        place_box(f"{p}_Debris_{i}",
                  unreal.Vector(car_x + dx, dy, dz),
                  (sx, sy, sz), debris_mat)
        _inc()

    # Hiding spots (recessed areas between debris) -- marked with target points
    for i, (dx, dy) in enumerate([(-2800.0, -1200.0), (300.0, 1300.0), (3500.0, -1300.0)]):
        place_target_point(f"Spawn_{p}_HidingSpot_{i}",
                           unreal.Vector(car_x + dx, dy, 300.0))
        _inc()

    # Creature nest markers
    place_target_point(f"Spawn_{p}_Nest_Juvenile",
                       unreal.Vector(car_x - 1000.0, -800.0, 300.0))
    place_target_point(f"Spawn_{p}_Nest_Alpha",
                       unreal.Vector(car_x + 2000.0, 0.0, 300.0))
    _inc(2)

    # Atmospheric fog actor for this car
    fog = level_lib.spawn_actor_from_class(
        unreal.ExponentialHeightFog,
        unreal.Vector(car_x, 0.0, 0.0)
    )
    if fog:
        fog.set_actor_label(f"Fog_Z1_DarkCar")
        fc = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
        if fc:
            fc.set_editor_property("fog_density", 0.005)
            fc.set_editor_property("fog_inscattering_color",
                                   unreal.LinearColor(0.03, 0.03, 0.05, 1.0))
        _inc()

    # Only a single very dim entry light
    place_light(f"{p}_EntryGlow",
                unreal.Vector(car_x - half_l + 800.0, 0.0, CAR_HEIGHT - 500.0),
                (80, 80, 100), 300.0, radius=2000.0)
    _inc()

    # Faint exit glow (light under far door)
    place_light(f"{p}_ExitGlow",
                unreal.Vector(car_x + half_l - 300.0, 0.0, 100.0),
                (200, 180, 120), 200.0, radius=1500.0)
    _inc()
    unreal.log(f"  Car 10 Dark Car: 6 debris pieces, hiding spots, nests, fog, minimal light")


def build_car11_freezer(car_x, mats):
    """Car 11: Freezer Breach -- ice geometry, blue-white light, cold zone marker."""
    p = "Z1_Car11_Freezer"
    ice_mat = mats.get("Ice_Blue")
    half_l = CAR_LENGTH / 2.0
    half_w = CAR_WIDTH / 2.0

    # Ice formations on walls (irregular boxes)
    ice_placements = [
        (-4000.0,  half_w - 200.0, 800.0,  600.0, 300.0, 1200.0),
        (-2500.0, -(half_w - 200.0), 1000.0, 400.0, 250.0, 1400.0),
        (-1000.0,  half_w - 150.0, 600.0,  800.0, 200.0, 1000.0),
        ( 500.0, -(half_w - 250.0), 1200.0, 500.0, 350.0, 1600.0),
        ( 2000.0,  half_w - 180.0, 900.0,  700.0, 220.0, 1100.0),
        ( 3500.0, -(half_w - 200.0), 700.0,  450.0, 280.0, 900.0),
    ]
    for i, (dx, dy, dz, sx, sy, sz) in enumerate(ice_placements):
        place_box(f"{p}_Ice_{i}",
                  unreal.Vector(car_x + dx, dy, dz),
                  (sx, sy, sz), ice_mat)
        _inc()

    # Ice stalactites from ceiling
    for i in range(4):
        sx = car_x - 3000.0 + i * 2000.0
        place_box(f"{p}_Stalactite_{i}",
                  unreal.Vector(sx, -400.0 + i * 300.0, CAR_HEIGHT - 400.0),
                  (120.0, 120.0, 700.0), ice_mat)
        _inc()

    # Breach in right wall (gap represented by removing wall section -- thin box for visual)
    place_box(f"{p}_BreachFrame",
              unreal.Vector(car_x + 1000.0, -(half_w + WALL_THICK / 2.0), 1200.0),
              (800.0, WALL_THICK + 50.0, 600.0), ice_mat)
    _inc()

    # Warm pipe safe spots (3 steam pipes)
    for i in range(3):
        wx = car_x - 3000.0 + i * 3000.0
        place_box(f"{p}_WarmPipe_{i}",
                  unreal.Vector(wx, half_w - 300.0, 500.0),
                  (400.0, 120.0, 120.0), mats.get("Workshop_Metal"))
        place_target_point(f"Spawn_{p}_WarmSpot_{i}",
                           unreal.Vector(wx, half_w - 500.0, 500.0))
        _inc(2)

    # Cold zone volume marker
    place_target_point(f"Volume_{p}_ColdZone",
                       unreal.Vector(car_x, 0.0, CAR_HEIGHT / 2.0))
    _inc()

    # Blue-white lighting
    for i in range(3):
        lx = car_x - 3000.0 + i * 3000.0
        place_light(f"{p}_IceLight_{i}",
                    unreal.Vector(lx, 0.0, CAR_HEIGHT - 300.0),
                    (160, 200, 255), 3500.0, radius=5000.0)
        _inc()
    unreal.log(f"  Car 11 Freezer: ice formations, stalactites, breach, warm pipes, cold zone")


def build_car12_kronole_den(car_x, mats):
    """Car 12: Kronole Den -- purple/sickly lighting, makeshift furniture, dealer NPC."""
    p = "Z1_Car12_KronoleDen"
    purple_mat = mats.get("Kronole_Purple")
    bunk_mat = mats.get("Bunk_Wood")
    half_l = CAR_LENGTH / 2.0

    # Makeshift mattresses/cushions on floor
    for i in range(5):
        mx = car_x - 3500.0 + i * 1800.0
        my = 1000.0 if (i % 2 == 0) else -1000.0
        place_box(f"{p}_Mattress_{i}",
                  unreal.Vector(mx, my, 80.0),
                  (1200.0, 600.0, 150.0), bunk_mat)
        _inc()

    # Low tables
    for i in range(2):
        tx = car_x - 1500.0 + i * 3000.0
        place_box(f"{p}_Table_{i}",
                  unreal.Vector(tx, 0.0, 200.0),
                  (800.0, 600.0, 80.0), bunk_mat)
        _inc()

    # Curtain partitions (hanging cloth = thin tall boxes)
    for i in range(2):
        cx = car_x - 2000.0 + i * 4000.0
        place_box(f"{p}_Curtain_{i}",
                  unreal.Vector(cx, 0.0, CAR_HEIGHT * 0.4),
                  (30.0, CAR_WIDTH * 0.6, CAR_HEIGHT * 0.7), purple_mat)
        _inc()

    # Kronole Kim dealer NPC spawn
    place_target_point(f"Spawn_{p}_KronoleKim",
                       unreal.Vector(car_x + 2000.0, -500.0, 500.0))
    _inc()

    # Purple/sickly lights
    for i in range(3):
        lx = car_x - 3000.0 + i * 3000.0
        place_light(f"{p}_PurpleLight_{i}",
                    unreal.Vector(lx, 0.0, CAR_HEIGHT - 500.0),
                    (180, 100, 200), 1800.0, radius=4000.0)
        _inc()
    unreal.log(f"  Car 12 Kronole Den: mattresses, tables, curtains, Kim spawn, purple lights")


def build_car13_smugglers_cache(car_x, mats):
    """Car 13: Smuggler's Cache -- hidden compartment, trade goods, merchant NPC."""
    p = "Z1_Car13_SmugglerCache"
    crate_mat = mats.get("Cache_Crate")
    metal_mat = mats.get("Platform_Metal")
    half_l = CAR_LENGTH / 2.0
    half_w = CAR_WIDTH / 2.0

    # Stacked crates (trade goods display)
    crate_positions = [
        (-3000.0, 1200.0, 250.0, 600.0, 500.0, 500.0),
        (-3000.0, 1200.0, 700.0, 500.0, 400.0, 400.0),
        (-1500.0, -1300.0, 250.0, 700.0, 500.0, 500.0),
        ( 0.0,   1300.0, 250.0, 500.0, 500.0, 500.0),
        ( 1500.0, -1200.0, 250.0, 600.0, 600.0, 500.0),
        ( 1500.0, -1200.0, 700.0, 400.0, 400.0, 400.0),
    ]
    for i, (dx, dy, dz, sx, sy, sz) in enumerate(crate_positions):
        place_box(f"{p}_Crate_{i}",
                  unreal.Vector(car_x + dx, dy, dz),
                  (sx, sy, sz), crate_mat)
        _inc()

    # Hidden compartment (floor panel)
    place_box(f"{p}_HiddenFloor",
              unreal.Vector(car_x + 3000.0, 0.0, -20.0),
              (1000.0, 800.0, 50.0), metal_mat)
    place_target_point(f"Spawn_{p}_HiddenCompartment",
                       unreal.Vector(car_x + 3000.0, 0.0, 100.0))
    _inc(2)

    # Trade goods display shelf
    place_box(f"{p}_Shelf",
              unreal.Vector(car_x, half_w - 150.0, 1000.0),
              (3000.0, 80.0, 50.0), metal_mat)
    _inc()

    # Merchant NPC spawn
    place_target_point(f"Spawn_{p}_Merchant",
                       unreal.Vector(car_x - 500.0, 0.0, 500.0))
    _inc()

    # Moderate warm lighting
    for i in range(2):
        lx = car_x - 2000.0 + i * 4000.0
        place_light(f"{p}_Light_{i}",
                    unreal.Vector(lx, 0.0, CAR_HEIGHT - 400.0),
                    (200, 180, 140), 2200.0, radius=5000.0)
        _inc()
    unreal.log(f"  Car 13 Smuggler's Cache: crates, hidden compartment, shelf, merchant spawn")


def build_car14_martyrs_gate(car_x, mats):
    """Car 14: Martyr's Gate -- grand entrance, memorial, boss arena, dramatic lighting."""
    p = "Z1_Car14_MartyrsGate"
    stone_mat = mats.get("Martyrs_Stone")
    memorial_mat = mats.get("Memorial_Dark")
    metal_mat = mats.get("Platform_Metal")
    half_l = CAR_LENGTH / 2.0
    half_w = CAR_WIDTH / 2.0

    # Memorial cairn in center
    place_box(f"{p}_Cairn_Base",
              unreal.Vector(car_x - 1000.0, 0.0, 300.0),
              (800.0, 800.0, 500.0), stone_mat)
    place_box(f"{p}_Cairn_Mid",
              unreal.Vector(car_x - 1000.0, 0.0, 650.0),
              (600.0, 600.0, 300.0), stone_mat)
    place_box(f"{p}_Cairn_Top",
              unreal.Vector(car_x - 1000.0, 0.0, 900.0),
              (400.0, 400.0, 200.0), stone_mat)
    _inc(3)

    # Makeshift shrines along walls (12 total, 6 per side)
    for i in range(6):
        sx = car_x - 4000.0 + i * 1300.0
        # Left wall shrines
        place_box(f"{p}_ShrineL_{i}",
                  unreal.Vector(sx, half_w - 250.0, 500.0),
                  (300.0, 150.0, 400.0), memorial_mat)
        # Right wall shrines
        place_box(f"{p}_ShrineR_{i}",
                  unreal.Vector(sx, -(half_w - 250.0), 500.0),
                  (300.0, 150.0, 400.0), memorial_mat)
        _inc(2)

    # Boss arena at far end -- raised platform area
    place_box(f"{p}_BossArena",
              unreal.Vector(car_x + 3500.0, 0.0, 100.0),
              (2500.0, CAR_WIDTH * 0.8, 200.0), metal_mat)
    _inc()

    # Gate frame (grand entrance feel -- tall pillars)
    for dy in [1000.0, -1000.0]:
        place_box(f"{p}_GatePillar_{'L' if dy > 0 else 'R'}",
                  unreal.Vector(car_x + 3500.0, dy, CAR_HEIGHT / 2.0),
                  (200.0, 200.0, CAR_HEIGHT), metal_mat)
        _inc()

    # Historical marker plaques (flat boxes on walls)
    for i in range(3):
        mx = car_x - 3000.0 + i * 2500.0
        place_box(f"{p}_Plaque_{i}",
                  unreal.Vector(mx, half_w - 50.0, 1600.0),
                  (800.0, 20.0, 400.0), memorial_mat)
        _inc()

    # Boss spawn point
    place_target_point(f"Spawn_{p}_Boss_CmdrGrey",
                       unreal.Vector(car_x + 4000.0, 0.0, 500.0))
    _inc()

    # Mourner NPC spawn
    place_target_point(f"Spawn_{p}_Mourner",
                       unreal.Vector(car_x - 1000.0, 400.0, 500.0))
    _inc()

    # Dramatic lighting -- warm in memorial area, bright at boss arena
    place_light(f"{p}_MemorialLight",
                unreal.Vector(car_x - 2000.0, 0.0, CAR_HEIGHT - 300.0),
                (220, 180, 120), 2000.0, radius=5000.0)
    place_light(f"{p}_CairnLight",
                unreal.Vector(car_x - 1000.0, 0.0, 1200.0),
                (255, 200, 100), 1000.0, radius=3000.0)
    place_light(f"{p}_BossArenaLight_A",
                unreal.Vector(car_x + 3000.0, 800.0, CAR_HEIGHT - 200.0),
                (255, 240, 200), 5000.0, radius=6000.0)
    place_light(f"{p}_BossArenaLight_B",
                unreal.Vector(car_x + 3000.0, -800.0, CAR_HEIGHT - 200.0),
                (255, 240, 200), 5000.0, radius=6000.0)
    _inc(4)
    unreal.log(f"  Car 14 Martyr's Gate: cairn, 12 shrines, boss arena, pillars, plaques, dramatic lights")


# ---------------------------------------------------------------------------
# Car interior dispatch table
# ---------------------------------------------------------------------------

CAR_BUILDERS = {
    0:  build_car00_caboose,
    1:  build_car01_quarters_a,
    2:  build_car02_quarters_b,
    3:  build_car03_pit,
    4:  build_car04_nursery,
    5:  build_car05_elders,
    6:  build_car06_sickbay,
    7:  build_car07_workshop,
    8:  build_car08_listening_post,
    9:  build_car09_blockade,
    10: build_car10_dark_car,
    11: build_car11_freezer,
    12: build_car12_kronole_den,
    13: build_car13_smugglers_cache,
    14: build_car14_martyrs_gate,
}


# ---------------------------------------------------------------------------
# Sublevel management
# ---------------------------------------------------------------------------

def create_or_load_sublevel(car_idx):
    """Create or load a streaming sublevel for a single car.

    Returns the ULevelStreaming object added to the persistent world.
    The sublevel is made current so subsequent spawn calls go into it.
    """
    sl_path = sublevel_path(car_idx)

    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    world = level_lib.get_editor_world()

    # Ensure the Zone1 subdirectory exists
    if not editor_util.does_directory_exist(SUBLEVEL_DIR):
        editor_util.make_directory(SUBLEVEL_DIR)

    if editor_util.does_asset_exist(sl_path):
        # Add existing sublevel to the persistent world
        streaming = level_utils.add_level_to_world(
            world, sl_path, unreal.LevelStreamingDynamic)
    else:
        # Create a new empty sublevel and add it
        streaming = level_utils.create_new_streaming_level(
            unreal.LevelStreamingDynamic, sl_path, False)

    if streaming:
        # Make this sublevel the current editing target
        level = streaming.get_loaded_level()
        if level:
            level_utils.make_level_current(level)

    return streaming


def make_persistent_current():
    """Switch the current editing level back to the persistent level."""
    world = level_lib.get_editor_world()
    persistent_level = world.get_editor_property("persistent_level") if world else None
    if persistent_level:
        level_utils.make_level_current(persistent_level)


# ---------------------------------------------------------------------------
# Build a single car sublevel
# ---------------------------------------------------------------------------

def build_single_car(car_idx, mats):
    """Build all geometry for one car inside its own streaming sublevel."""
    total_span = CAR_LENGTH + CAR_GAP
    car_x = car_idx * total_span
    name = CAR_DEFS[car_idx][0]

    unreal.log(f"--- Building sublevel: Car {car_idx:02d} ({name}) ---")

    # Create/load the sublevel and make it current for editing
    streaming = create_or_load_sublevel(car_idx)
    if not streaming:
        unreal.log_warning(f"  FAILED to create sublevel for car {car_idx}")
        return None

    # Shell (floor, ceiling, walls, doors, connector gangway)
    build_car_shell(car_idx, car_x, mats)

    # Themed interior lights
    build_car_lights(car_idx, car_x)

    # Pipe meshes along ceiling
    build_car_pipes(car_idx, car_x)

    # Directional sign
    build_car_sign(car_idx, car_x)

    # Car-specific interior details
    builder = CAR_BUILDERS.get(car_idx)
    if builder:
        builder(car_x, mats)

    unreal.log(f"  [{car_idx:02d}/{NUM_CARS - 1}] {name} complete ({_actor_count} actors total)")
    return streaming


# ---------------------------------------------------------------------------
# Persistent level: atmosphere + PlayerStart
# ---------------------------------------------------------------------------

def setup_atmosphere():
    """Global atmosphere for the Tail -- cold, oppressive.
    Placed in the persistent level (not a sublevel)."""
    unreal.log("=== Setting up Zone 1 atmosphere (persistent level) ===")
    total_length = NUM_CARS * (CAR_LENGTH + CAR_GAP)
    mid_x = total_length / 2.0

    # Directional light -- cold exterior
    dl = level_lib.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(mid_x, 0.0, 15000.0),
        unreal.Rotator(-35.0, 20.0, 0.0)
    )
    if dl:
        dl.set_actor_label("SunLight_Zone1_Frozen")
        lc = dl.get_component_by_class(unreal.DirectionalLightComponent)
        if lc:
            lc.set_editor_property("intensity", 1.0)
            lc.set_editor_property("light_color", unreal.Color(150, 170, 210, 255))

    # Sky light
    sl = level_lib.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(mid_x, 0.0, 8000.0)
    )
    if sl:
        sl.set_actor_label("SkyLight_Zone1")
        sc = sl.get_component_by_class(unreal.SkyLightComponent)
        if sc:
            sc.set_editor_property("intensity", 0.2)

    # Sky atmosphere
    sa = level_lib.spawn_actor_from_class(
        unreal.SkyAtmosphere,
        unreal.Vector(mid_x, 0.0, 0.0)
    )
    if sa:
        sa.set_actor_label("SkyAtmosphere_Zone1")

    # Post-process -- dark, cold
    ppv = level_lib.spawn_actor_from_class(
        unreal.PostProcessVolume,
        unreal.Vector(mid_x, 0.0, CAR_HEIGHT / 2.0)
    )
    if ppv:
        ppv.set_actor_label("PostProcess_Z1_Tail")
        ppv.set_editor_property("unbound", True)
        settings = ppv.get_editor_property("settings")
        settings.set_editor_property("override_auto_exposure_bias", True)
        settings.set_editor_property("auto_exposure_bias", -0.5)
        ppv.set_editor_property("settings", settings)

    # PlayerStart in Car 0 position (persistent so it's always loaded)
    ps = level_lib.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(0.0, 0.0, 500.0),
        unreal.Rotator(0.0, 0.0, 0.0)
    )
    if ps:
        ps.set_actor_label("PlayerStart_Zone1")

    unreal.log("  Atmosphere + PlayerStart configured (cold, oppressive)")


# ---------------------------------------------------------------------------
# Create / load persistent map
# ---------------------------------------------------------------------------

def create_or_load_map():
    """Create or load the Zone1_Tail persistent map."""
    maps_dir = "/Game/Maps"

    if not editor_util.does_directory_exist(maps_dir):
        editor_util.make_directory(maps_dir)

    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

    if editor_util.does_asset_exist(PERSISTENT_MAP):
        unreal.log(f"  Loading existing map: {PERSISTENT_MAP}")
        level_subsystem.load_level(PERSISTENT_MAP)
    else:
        unreal.log(f"  Creating new map: {PERSISTENT_MAP}")
        level_subsystem.new_level(PERSISTENT_MAP)

    return PERSISTENT_MAP


def set_startup_map(map_path):
    """Set the Zone1_Tail map as editor startup and default game map."""
    settings = unreal.GameMapsSettings.get_game_maps_settings()
    if settings:
        soft_path = unreal.SoftObjectPath(f"{map_path}.Zone1_Tail")
        settings.set_editor_property("editor_startup_map", soft_path)
        settings.set_editor_property("game_default_map", soft_path)
        unreal.log(f"  Set {map_path} as editor startup map")


# ---------------------------------------------------------------------------
# Generate streaming registration data (for runtime use)
# ---------------------------------------------------------------------------

def generate_streaming_registry():
    """Write a JSON file that maps car indices to sublevel names.

    At runtime, a Blueprint or C++ initializer reads this file and calls
    USEECarStreamingSubsystem::RegisterCarLevel() for each entry.
    """
    import json

    registry = {}
    for car_idx in range(NUM_CARS):
        registry[str(car_idx)] = sublevel_name(car_idx)

    registry_path = "/Game/DataTables/Zone1_CarStreamingRegistry"
    out_dir = unreal.Paths.project_content_dir() + "DataTables/"

    # Also write a simple JSON file alongside the DataTables
    json_path = unreal.Paths.project_dir() + "DataTables/Zone1_CarStreamingRegistry.json"
    with open(json_path, "w") as f:
        json.dump(registry, f, indent=2)
    unreal.log(f"  Wrote streaming registry: {json_path}")

    return registry


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def run():
    global _actor_count
    _actor_count = 0

    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  Zone 1 -- The Tail (15 Cars) [10x SCALE]")
    unreal.log("  STREAMING SUBLEVEL BUILD")
    unreal.log("=" * 64)
    unreal.log("")

    # 1. Create/load the persistent level
    map_path = create_or_load_map()
    cleanup()

    # 2. Pre-create all materials (shared across sublevels)
    mats = _load_surface_mats()

    # 3. Build each car as its own streaming sublevel
    unreal.log("=== Building 15 car sublevels ===")
    streaming_levels = {}
    for car_idx in range(NUM_CARS):
        streaming = build_single_car(car_idx, mats)
        if streaming:
            streaming_levels[car_idx] = streaming

    # 4. Switch back to persistent level for atmosphere/PlayerStart
    make_persistent_current()
    setup_atmosphere()

    # 5. Generate streaming registry for runtime subsystem
    registry = generate_streaming_registry()

    # 6. Save all levels
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    level_subsystem.save_all_dirty_levels()

    set_startup_map(map_path)

    total_length_m = (NUM_CARS * (CAR_LENGTH + CAR_GAP)) / 100.0
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  ZONE 1 BUILD COMPLETE (STREAMING SUBLEVELS)")
    unreal.log("=" * 64)
    unreal.log(f"  Persistent map: {PERSISTENT_MAP}")
    unreal.log(f"  Sublevel dir:   {SUBLEVEL_DIR}/")
    unreal.log(f"  Cars: {NUM_CARS} ({CAR_LENGTH/100:.0f}m x {CAR_WIDTH/100:.0f}m x {CAR_HEIGHT/100:.0f}m each)")
    unreal.log(f"  Total length: {total_length_m:.0f}m")
    unreal.log(f"  Actors placed: {_actor_count}")
    unreal.log("")
    unreal.log("  STREAMING SUBLEVELS:")
    for idx in range(NUM_CARS):
        name, _, _, desc = CAR_DEFS[idx]
        unreal.log(f"    [{idx:02d}] {sublevel_name(idx):30s} -- {desc}")
    unreal.log("")
    unreal.log("  RUNTIME INTEGRATION:")
    unreal.log("    USEECarStreamingSubsystem::RegisterCarLevel() bindings")
    unreal.log("    in Zone1_CarStreamingRegistry.json")
    unreal.log("")
    unreal.log("  PlayerStart is in the persistent level (Car 00 position).")
    unreal.log("  3-car streaming window: current + adjacent cars loaded.")
    unreal.log("  Press Play to begin.")
    unreal.log("")


run()
