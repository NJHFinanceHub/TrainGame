"""
Snowpiercer: Eternal Engine -- World Polish & Safety Pass
Run in editor: Tools > Execute Python Script (or headless via -ExecutePythonScript)

Fixes the three issues reported from the first playtest of /Game/Maps/Zone1_Tail:

  1. PLAYER FELL OFF THE MAP
     build_zone1.py leaves a 150cm-wide floorless slot under every front
     doorway: the car floor ends at car_x + 6000 (CAR_LENGTH/2) but the
     connector gangway floor only starts at car_x + 6150 (after the front
     wall's 150cm thickness).  The doorway opening (|y| <= 750, z 0..2300)
     sits directly over that slot, so a player walking through any door can
     drop through the world.  This script seals every inter-car gap with an
     overlapping full-width floor slab, enclosed connector walls/ceiling,
     and hidden collision blockers filling the rest of the gap
     cross-section, plus end-cap blockers behind Car 0 and past Car 14.

  2. ENEMIES / NPCs ARE INVISIBLE
     setup_game_content.py only assigns a skeletal mesh to BP_NPC_* CDOs if
     UE5 Mannequin content exists under /Game/Characters -- it does not in
     this project, so every NPC blueprint has an empty SkeletalMeshComponent
     (an invisible capsule).  This script gives every NPC blueprint AND
     every already-placed NPC/Boss instance a clearly visible body
     (skeletal mesh if one can be found, otherwise cube body + sphere head
     placeholders) with faction-colored materials:
       Jackboot/Boss = dark blue / red (enemies), Civilian = grey-brown,
       Merchant = green, Breachman = amber, FirstClass = cream.

  3. WORLD NEEDS TEXTURES
     Any StaticMeshActor still using the engine default material (grey
     checker / BasicShapeMaterial) gets an appropriate zone material based
     on its actor label (Floor/Wall/Ceiling/Door/etc.), preferring the
     persisted /Game/Materials/Zones/Tail set, falling back to
     /Game/Materials/Zone1, then a metal fallback.

Idempotent: removes its own previously placed actors (label prefixes
"GW_" and "NPCVis_") before re-adding.  Never raises on missing assets --
logs and continues.
"""

import unreal

# ---------------------------------------------------------------------------
# API shortcuts
# ---------------------------------------------------------------------------

level_lib = unreal.EditorLevelLibrary
editor_util = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mat_lib = unreal.MaterialEditingLibrary

# ---------------------------------------------------------------------------
# Dimensions (cm) -- MUST mirror build_zone1.py
# ---------------------------------------------------------------------------

CAR_LENGTH  = 12000.0
CAR_WIDTH   = 4000.0
CAR_HEIGHT  = 3000.0
WALL_THICK  = 150.0
DOOR_WIDTH  = 1500.0
DOOR_HEIGHT = 2300.0
CAR_GAP     = 1000.0
NUM_CARS    = 15
CAR_STRIDE  = CAR_LENGTH + CAR_GAP   # 13000

PERSISTENT_MAP = "/Game/Maps/Zone1_Tail"

# Label prefixes owned by THIS script (cleaned up on re-run)
GW_PREFIX     = "GW_"        # gangway seal geometry + blockers
NPCVIS_PREFIX = "NPCVis_"    # per-instance NPC visual placeholders

# ---------------------------------------------------------------------------
# Stats
# ---------------------------------------------------------------------------

_stats = {
    "gaps_sealed": 0,
    "blockers_placed": 0,
    "actors_retextured": 0,
    "npc_bps_fixed": 0,
    "npc_instances_fixed": 0,
    "assets_saved": 0,
}

# ---------------------------------------------------------------------------
# Generic helpers
# ---------------------------------------------------------------------------


def ensure_dir(path):
    try:
        if not editor_util.does_directory_exist(path):
            editor_util.make_directory(path)
    except Exception as e:
        unreal.log_warning(f"  Could not create directory {path}: {e}")


def safe_save_asset(asset_path):
    try:
        editor_util.save_asset(asset_path, only_if_is_dirty=False)
        _stats["assets_saved"] += 1
        return True
    except Exception as e:
        unreal.log_warning(f"  Could not save asset {asset_path}: {e}")
        return False


def load_first_existing(paths, label="asset"):
    """Return (asset, path) for the first existing asset in `paths`."""
    for p in paths:
        try:
            if editor_util.does_asset_exist(p):
                a = editor_util.load_asset(p)
                if a:
                    return a, p
        except Exception:
            continue
    unreal.log_warning(f"  No {label} found among: {paths}")
    return None, None


_cube_mesh = None
_sphere_mesh = None


def get_cube():
    global _cube_mesh
    if _cube_mesh is None:
        _cube_mesh, _ = load_first_existing(["/Engine/BasicShapes/Cube"], "cube mesh")
    return _cube_mesh


def get_sphere():
    global _sphere_mesh
    if _sphere_mesh is None:
        _sphere_mesh, _ = load_first_existing(["/Engine/BasicShapes/Sphere"], "sphere mesh")
    return _sphere_mesh


def place_box(label, location, size, material=None, hidden_in_game=False):
    """Spawn a scaled cube StaticMeshActor (same approach as build_zone1.py).
    size = (x, y, z) in cm.  hidden_in_game=True makes an invisible blocker
    that still has full collision."""
    try:
        actor = level_lib.spawn_actor_from_class(
            unreal.StaticMeshActor, location, unreal.Rotator(0.0, 0.0, 0.0))
    except Exception as e:
        unreal.log_warning(f"  Spawn failed for {label}: {e}")
        return None
    if not actor:
        return None
    actor.set_actor_label(label)
    sm = actor.get_component_by_class(unreal.StaticMeshComponent)
    if sm:
        cube = get_cube()
        if cube:
            sm.set_static_mesh(cube)
            actor.set_actor_scale3d(unreal.Vector(
                size[0] / 100.0, size[1] / 100.0, size[2] / 100.0))
            if material:
                try:
                    sm.set_material(0, material)
                except Exception as e:
                    unreal.log_warning(f"  set_material failed on {label}: {e}")
        # Blockers must never move and must block everything
        try:
            sm.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
        except Exception:
            pass
    if hidden_in_game:
        try:
            actor.set_actor_hidden_in_game(True)
        except Exception as e:
            unreal.log_warning(f"  Could not hide {label}: {e}")
    return actor


def place_blocker(label, location, size, material=None):
    """Place an invisible collision blocker.

    First tries unreal.BlockingVolume -- but volumes spawned from Python get
    no brush geometry (zero bounds), so we validate and fall back to a
    hidden cube StaticMeshActor, which is guaranteed to collide."""
    try:
        bv = level_lib.spawn_actor_from_class(
            unreal.BlockingVolume, location, unreal.Rotator(0.0, 0.0, 0.0))
        if bv:
            valid = False
            try:
                origin, extent = bv.get_actor_bounds(False)
                if extent.x > 1.0 and extent.y > 1.0 and extent.z > 1.0:
                    valid = True
            except Exception:
                valid = False
            if valid:
                bv.set_actor_label(label)
                bv.set_actor_scale3d(unreal.Vector(
                    size[0] / max(extent.x * 2.0, 1.0),
                    size[1] / max(extent.y * 2.0, 1.0),
                    size[2] / max(extent.z * 2.0, 1.0)))
                _stats["blockers_placed"] += 1
                return bv
            # No usable brush geometry -- discard and fall back
            level_lib.destroy_actor(bv)
    except Exception:
        pass

    actor = place_box(label, location, size, material, hidden_in_game=True)
    if actor:
        _stats["blockers_placed"] += 1
    return actor


# ---------------------------------------------------------------------------
# Simple colored material creation (same pattern as build_zone1.get_material)
# ---------------------------------------------------------------------------

_mat_cache = {}


def get_or_create_material(name, rgb, roughness=0.7, metallic=0.2,
                           emissive=None, dir_path="/Game/Materials/Characters"):
    """Get or create a simple PBR material.  emissive = optional (r,g,b)."""
    mat_path = f"{dir_path}/{name}"
    if mat_path in _mat_cache:
        return _mat_cache[mat_path]

    try:
        if editor_util.does_asset_exist(mat_path):
            mat = editor_util.load_asset(mat_path)
            _mat_cache[mat_path] = mat
            return mat

        ensure_dir(dir_path)
        mat = asset_tools.create_asset(name, dir_path,
                                       unreal.Material, unreal.MaterialFactoryNew())
        if not mat:
            unreal.log_warning(f"  Failed to create material: {name}")
            return None

        color_node = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionConstant3Vector, -300, -200)
        if color_node:
            color_node.set_editor_property(
                "constant", unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0))
            mat_lib.connect_material_property(
                color_node, "", unreal.MaterialProperty.MP_BASE_COLOR)

        rough_node = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionConstant, -300, 0)
        if rough_node:
            rough_node.set_editor_property("r", roughness)
            mat_lib.connect_material_property(
                rough_node, "", unreal.MaterialProperty.MP_ROUGHNESS)

        metal_node = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionConstant, -300, 200)
        if metal_node:
            metal_node.set_editor_property("r", metallic)
            mat_lib.connect_material_property(
                metal_node, "", unreal.MaterialProperty.MP_METALLIC)

        if emissive:
            em_node = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionConstant3Vector, -300, 400)
            if em_node:
                em_node.set_editor_property(
                    "constant",
                    unreal.LinearColor(emissive[0], emissive[1], emissive[2], 1.0))
                mat_lib.connect_material_property(
                    em_node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

        mat_lib.recompile_material(mat)
        safe_save_asset(mat_path)
        _mat_cache[mat_path] = mat
        return mat
    except Exception as e:
        unreal.log_warning(f"  Material creation failed for {name}: {e}")
        return None


# ---------------------------------------------------------------------------
# Zone surface material lookup (persisted Zones/Tail set first, then Zone1)
# ---------------------------------------------------------------------------

SURFACE_MAT_CANDIDATES = {
    "floor": [
        "/Game/Materials/Zones/Tail/M_Tail_Floor",
        "/Game/Materials/Zone1/M_Tail_Floor",
    ],
    "wall": [
        "/Game/Materials/Zones/Tail/M_Tail_Wall",
        "/Game/Materials/Zone1/M_Tail_Wall",
    ],
    "ceiling": [
        "/Game/Materials/Zones/Tail/M_Tail_Ceiling",
        "/Game/Materials/Zone1/M_Tail_Ceiling",
    ],
    "door": [
        "/Game/Materials/Zones/Tail/M_Tail_Door",
        "/Game/Materials/Zone1/M_Tail_Door",
    ],
    "connector": [
        "/Game/Materials/Zone1/M_Connector",
        "/Game/Materials/Zones/Tail/M_Tail_Trim",
        "/Game/Materials/Zones/Tail/M_Tail_Wall",
    ],
    "wood": [
        "/Game/Materials/Zones/Tail/M_Tail_BunkWood",
        "/Game/Materials/Zone1/M_Bunk_Wood",
    ],
    "crate": [
        "/Game/Materials/Zone1/M_Cache_Crate",
        "/Game/Materials/Zones/Tail/M_Tail_BunkWood",
    ],
    "ice": [
        "/Game/Materials/Zone1/M_Ice_Blue",
    ],
    "metal": [
        "/Game/Materials/Zones/Tail/M_Tail_GrimeMetal",
        "/Game/Materials/Zone1/M_Workshop_Metal",
        "/Game/Materials/ModularPipes/M_BareMetal",
        "/Game/Materials/Zone1/M_Platform_Metal",
    ],
}

_surface_mats = {}


def get_surface_material(role):
    """Resolve a surface role to a loaded material, with fallback to metal."""
    if role in _surface_mats:
        return _surface_mats[role]
    mat, path = load_first_existing(
        SURFACE_MAT_CANDIDATES.get(role, []), f"{role} material")
    if not mat and role != "metal":
        mat = get_surface_material("metal")
    if not mat:
        # Last resort: create a flat dark metal so nothing stays checker-grey
        mat = get_or_create_material(
            "M_Polish_FallbackMetal", (0.06, 0.05, 0.04),
            roughness=0.8, metallic=0.8, dir_path="/Game/Materials/Zone1")
    _surface_mats[role] = mat
    return mat


# ---------------------------------------------------------------------------
# Level loading & cleanup
# ---------------------------------------------------------------------------


def load_zone1():
    """Make sure /Game/Maps/Zone1_Tail is the loaded editor level."""
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        world = level_lib.get_editor_world()
        if not world or "Zone1_Tail" not in world.get_path_name():
            unreal.log(f"  Loading {PERSISTENT_MAP} ...")
            if not editor_util.does_asset_exist(PERSISTENT_MAP):
                unreal.log_error(f"  Map not found: {PERSISTENT_MAP} -- run build_zone1.py first")
                return False
            les.load_level(PERSISTENT_MAP)
        return True
    except Exception as e:
        unreal.log_error(f"  Could not load level: {e}")
        return False


def cleanup_own_actors():
    """Remove actors this script placed on a previous run (idempotency)."""
    unreal.log("=== Cleaning previous polish actors ===")
    removed = 0
    try:
        for actor in level_lib.get_all_level_actors():
            if isinstance(actor, unreal.WorldSettings):
                continue
            try:
                label = actor.get_actor_label()
            except Exception:
                continue
            if label.startswith(GW_PREFIX) or label.startswith(NPCVIS_PREFIX):
                level_lib.destroy_actor(actor)
                removed += 1
    except Exception as e:
        unreal.log_warning(f"  Cleanup error: {e}")
    unreal.log(f"  Removed {removed} previous polish actors")


# ---------------------------------------------------------------------------
# 1. GANGWAY FIX -- seal every inter-car gap
# ---------------------------------------------------------------------------
#
# Geometry recap (from build_zone1.py):
#   Car center:            car_x = idx * 13000
#   Car floor:             spans car_x +/- 6000, top surface at z = 0
#   Front wall:            x in [car_x+6000, car_x+6150]  (door slot |y|<=750)
#   Next car back wall:    x in [car_x+6850, car_x+7000]
#   Existing Conn_ floor:  x in [car_x+6150, car_x+7150]  -> leaves the
#                          150cm doorway slot [6000..6150] FLOORLESS.
#
# We seal each gap with overlapping geometry centered at gap_cx = car_x+6500.


def seal_gangways():
    unreal.log("=== Sealing gangway gaps ===")
    mat_conn = get_surface_material("connector")
    mat_floor = get_surface_material("floor")

    for i in range(NUM_CARS - 1):
        car_x = i * CAR_STRIDE
        gcx = car_x + CAR_LENGTH / 2.0 + (CAR_STRIDE - CAR_LENGTH) / 2.0  # car_x + 6500
        pfx = f"{GW_PREFIX}{i:02d}"

        # Full-width floor slab spanning the entire gap PLUS 200cm overlap
        # into both car floors -- closes the 150cm doorway slot on each side.
        place_box(f"{pfx}_Floor",
                  unreal.Vector(gcx, 0.0, -WALL_THICK / 2.0),
                  (1400.0, CAR_WIDTH + 400.0, WALL_THICK), mat_floor)

        # Connector tube side walls (visible), flush with the 1500cm doorway
        place_box(f"{pfx}_WallL",
                  unreal.Vector(gcx, DOOR_WIDTH / 2.0 + WALL_THICK / 2.0, DOOR_HEIGHT / 2.0),
                  (1400.0, WALL_THICK, DOOR_HEIGHT), mat_conn)
        place_box(f"{pfx}_WallR",
                  unreal.Vector(gcx, -(DOOR_WIDTH / 2.0 + WALL_THICK / 2.0), DOOR_HEIGHT / 2.0),
                  (1400.0, WALL_THICK, DOOR_HEIGHT), mat_conn)

        # Connector tube ceiling
        place_box(f"{pfx}_Ceil",
                  unreal.Vector(gcx, 0.0, DOOR_HEIGHT + WALL_THICK / 2.0),
                  (1400.0, DOOR_WIDTH + WALL_THICK * 3.0, WALL_THICK), mat_conn)

        # Hidden blockers filling the rest of the gap cross-section so even a
        # physics glitch through the tube walls cannot leave the train.
        # Side blockers: |y| from tube wall to beyond car wall, full height.
        side_y0 = DOOR_WIDTH / 2.0 + WALL_THICK          # 900
        side_y1 = CAR_WIDTH / 2.0 + WALL_THICK + 100.0   # 2250
        side_cy = (side_y0 + side_y1) / 2.0
        side_w = side_y1 - side_y0
        place_blocker(f"{pfx}_BlockL",
                      unreal.Vector(gcx, side_cy, CAR_HEIGHT / 2.0),
                      (1000.0, side_w, CAR_HEIGHT + 600.0), mat_conn)
        place_blocker(f"{pfx}_BlockR",
                      unreal.Vector(gcx, -side_cy, CAR_HEIGHT / 2.0),
                      (1000.0, side_w, CAR_HEIGHT + 600.0), mat_conn)

        # Top blocker: from connector ceiling up past the car roof line.
        top_z0 = DOOR_HEIGHT + WALL_THICK                 # 2450
        top_z1 = CAR_HEIGHT + WALL_THICK * 2.0 + 100.0    # 3400
        place_blocker(f"{pfx}_BlockTop",
                      unreal.Vector(gcx, 0.0, (top_z0 + top_z1) / 2.0),
                      (1000.0, CAR_WIDTH + 400.0, top_z1 - top_z0), mat_conn)

        # Bottom blocker: a second slab below the floor as a safety net.
        place_blocker(f"{pfx}_BlockBottom",
                      unreal.Vector(gcx, 0.0, -WALL_THICK * 1.5),
                      (1400.0, CAR_WIDTH + 400.0, WALL_THICK), mat_conn)

        _stats["gaps_sealed"] += 1

    # End caps: oversize hidden blocker walls behind Car 0 and past Car 14
    # (their sealed walls exist, these guarantee no escape past either end).
    rear_x = 0.0 - CAR_LENGTH / 2.0 - WALL_THICK - 150.0
    place_blocker(f"{GW_PREFIX}EndCap_Rear",
                  unreal.Vector(rear_x, 0.0, CAR_HEIGHT / 2.0),
                  (300.0, CAR_WIDTH + 800.0, CAR_HEIGHT + 800.0),
                  get_surface_material("wall"))
    front_x = (NUM_CARS - 1) * CAR_STRIDE + CAR_LENGTH / 2.0 + WALL_THICK + 150.0
    place_blocker(f"{GW_PREFIX}EndCap_Front",
                  unreal.Vector(front_x, 0.0, CAR_HEIGHT / 2.0),
                  (300.0, CAR_WIDTH + 800.0, CAR_HEIGHT + 800.0),
                  get_surface_material("wall"))
    _stats["gaps_sealed"] += 2

    unreal.log(f"  Sealed {NUM_CARS - 1} gangway gaps + 2 end caps "
               f"({_stats['blockers_placed']} blockers)")


# ---------------------------------------------------------------------------
# 2. TEXTURE PASS -- replace default/missing materials by actor label role
# ---------------------------------------------------------------------------

# Ordered (first match wins) keyword -> surface role
LABEL_ROLE_RULES = [
    (("Conn_", "GW_"),                                            "connector"),
    (("Floor", "Catwalk", "Corridor", "BossArena", "RingFloor"),  "floor"),
    (("Ceiling", "Ceil", "Roof"),                                 "ceiling"),
    (("Door", "Header"),                                          "door"),
    (("Ice", "Stalactite"),                                       "ice"),
    (("Bunk", "Bed", "Mattress", "Table", "Bench", "Seat",
      "Shelf", "Crib", "Workbench"),                              "wood"),
    (("Crate", "Cache"),                                          "crate"),
    (("Wall", "Partition", "Curtain", "FrontL", "FrontR",
      "BackL", "BackR", "Barricade", "Sign_"),                    "wall"),
]


def _role_for_label(label):
    for keywords, role in LABEL_ROLE_RULES:
        for kw in keywords:
            if kw in label:
                return role
    return "metal"


def _is_default_material(mat):
    """True if a slot has no override or an engine default/basic material."""
    if mat is None:
        return True
    try:
        path = mat.get_path_name()
    except Exception:
        return True
    if path.startswith("/Engine/"):
        # BasicShapeMaterial, WorldGridMaterial, DefaultMaterial, etc.
        return True
    return False


def texture_pass():
    unreal.log("=== Texture pass (default-material sweep) ===")
    fixed = 0
    checked = 0
    try:
        actors = level_lib.get_all_level_actors()
    except Exception as e:
        unreal.log_warning(f"  Could not enumerate actors: {e}")
        return

    for actor in actors:
        if not isinstance(actor, unreal.StaticMeshActor):
            continue
        try:
            label = actor.get_actor_label()
            sm = actor.get_component_by_class(unreal.StaticMeshComponent)
            if not sm or not sm.static_mesh:
                continue
            checked += 1
            num_slots = max(1, sm.get_num_materials())
            actor_fixed = False
            for slot in range(num_slots):
                current = None
                try:
                    current = sm.get_material(slot)
                except Exception:
                    pass
                if _is_default_material(current):
                    role = _role_for_label(label)
                    mat = get_surface_material(role)
                    if mat:
                        sm.set_material(slot, mat)
                        actor_fixed = True
            if actor_fixed:
                fixed += 1
        except Exception as e:
            unreal.log_warning(f"  Texture pass error on actor: {e}")

    _stats["actors_retextured"] = fixed
    unreal.log(f"  Checked {checked} StaticMeshActors, retextured {fixed}")


# ---------------------------------------------------------------------------
# 3. NPC VISIBILITY FIX
# ---------------------------------------------------------------------------

# NPC type -> (label keywords, base color, roughness, metallic, emissive)
# Enemies (Jackboot, Boss) get emissive accents so they read as hostile even
# in the Dark Car.
NPC_STYLES = {
    "Jackboot":  ((0.02, 0.04, 0.10), 0.6, 0.4, (0.10, 0.00, 0.00)),  # dark blue, red glow
    "Boss":      ((0.45, 0.04, 0.04), 0.5, 0.3, (0.30, 0.02, 0.02)),  # red, strong glow
    "Civilian":  ((0.30, 0.24, 0.18), 0.9, 0.0, None),                # grey-brown
    "Merchant":  ((0.06, 0.30, 0.10), 0.8, 0.1, None),                # green
    "Breachman": ((0.40, 0.22, 0.06), 0.7, 0.3, None),                # amber
    "FirstClass":((0.45, 0.42, 0.35), 0.6, 0.1, None),                # cream
    "Player":    ((0.10, 0.45, 0.55), 0.6, 0.1, None),                # teal
}

NPC_BPS = {
    "/Game/Blueprints/NPCs/BP_NPC_Civilian":   "Civilian",
    "/Game/Blueprints/NPCs/BP_NPC_Jackboot":   "Jackboot",
    "/Game/Blueprints/NPCs/BP_NPC_Merchant":   "Merchant",
    "/Game/Blueprints/NPCs/BP_NPC_Breachman":  "Breachman",
    "/Game/Blueprints/NPCs/BP_NPC_FirstClass": "FirstClass",
    "/Game/Blueprints/Characters/BP_SEECharacter": "Player",
}

SKELETAL_MESH_PATHS = [
    "/Game/Characters/Mannequins/Meshes/SKM_Manny",
    "/Game/Characters/Mannequins/Meshes/SKM_Quinn",
    "/Game/Characters/Mannequin/Mesh/SKM_Manny",
    "/Game/ThirdPerson/Characters/Mannequins/Meshes/SKM_Manny",
    "/Engine/EngineMeshes/SkeletalCube",
]


def get_npc_material(npc_type):
    style = NPC_STYLES.get(npc_type, NPC_STYLES["Civilian"])
    rgb, rough, metal, emissive = style
    return get_or_create_material(
        f"M_NPC_{npc_type}", rgb, roughness=rough, metallic=metal,
        emissive=emissive, dir_path="/Game/Materials/Characters")


def find_skeletal_mesh():
    for path in SKELETAL_MESH_PATHS:
        try:
            if editor_util.does_asset_exist(path):
                mesh = editor_util.load_asset(path)
                if mesh and isinstance(mesh, unreal.SkeletalMesh):
                    unreal.log(f"  Using skeletal mesh: {path}")
                    return mesh
        except Exception:
            continue
    unreal.log("  No skeletal mesh available -- will use shape placeholders")
    return None


def _add_sm_component_to_bp(bp, comp_name, mesh, material,
                            rel_loc, rel_scale):
    """Add a StaticMeshComponent to a Blueprint via SubobjectDataSubsystem.
    Returns True on success.  Fully guarded -- the subsystem API varies
    between UE5 minor versions."""
    try:
        sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
        if not sds:
            return False
        handles = sds.k2_gather_subobject_data_for_blueprint(bp)
        if not handles:
            return False

        # Skip if a component with this name already exists (idempotency)
        for h in handles:
            try:
                data = sds.k2_find_subobject_data_from_handle(h)
                obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
                if obj and comp_name in obj.get_name():
                    # Already added on a previous run -- just refresh props
                    if isinstance(obj, unreal.StaticMeshComponent):
                        obj.set_static_mesh(mesh)
                        if material:
                            obj.set_material(0, material)
                    return True
            except Exception:
                continue

        params = unreal.AddNewSubobjectParams(
            parent_handle=handles[0],
            new_class=unreal.StaticMeshComponent,
            blueprint_context=bp)
        new_handle, fail_reason = sds.add_new_subobject(params)
        try:
            # is_valid() is not exposed on the handle in every 5.x build;
            # treat a check failure as "proceed and let the next step decide".
            if hasattr(new_handle, "is_valid") and not new_handle.is_valid():
                unreal.log_warning(f"    add_new_subobject failed: {fail_reason}")
                return False
        except Exception:
            pass
        try:
            sds.rename_subobject(new_handle, unreal.Text(comp_name))
        except Exception:
            pass

        data = sds.k2_find_subobject_data_from_handle(new_handle)
        comp = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
        if not comp or not isinstance(comp, unreal.StaticMeshComponent):
            return False

        comp.set_static_mesh(mesh)
        if material:
            comp.set_material(0, material)
        comp.set_editor_property("relative_location",
                                 unreal.Vector(rel_loc[0], rel_loc[1], rel_loc[2]))
        comp.set_editor_property("relative_scale3d",
                                 unreal.Vector(rel_scale[0], rel_scale[1], rel_scale[2]))
        try:
            comp.set_collision_profile_name("NoCollision")
        except Exception:
            pass
        return True
    except Exception as e:
        unreal.log_warning(f"    SubobjectDataSubsystem path failed: {e}")
        return False


def fix_npc_blueprints():
    """Give every NPC blueprint a visible body so future spawns work."""
    unreal.log("=== Fixing NPC blueprints ===")
    sk_mesh = find_skeletal_mesh()
    cube = get_cube()
    sphere = get_sphere()

    for bp_path, npc_type in NPC_BPS.items():
        try:
            if not editor_util.does_asset_exist(bp_path):
                unreal.log_warning(f"  BP not found, skipping: {bp_path}")
                continue
            bp = editor_util.load_asset(bp_path)
            if not bp:
                continue

            mat = get_npc_material(npc_type)
            fixed_any = False

            # (a) Skeletal mesh on the inherited SkeletalMeshComponent (CDO)
            try:
                gen_class = bp.generated_class()
                cdo = unreal.get_default_object(gen_class)
                if cdo:
                    mesh_comp = cdo.get_component_by_class(unreal.SkeletalMeshComponent)
                    if mesh_comp:
                        if sk_mesh:
                            mesh_comp.set_skeletal_mesh_asset(sk_mesh)
                            fixed_any = True
                        if mat:
                            try:
                                mesh_comp.set_material(0, mat)
                            except Exception:
                                pass
                        mesh_comp.set_editor_property("visible", True)
                        mesh_comp.set_editor_property("hidden_in_game", False)
            except Exception as e:
                unreal.log_warning(f"  CDO mesh setup failed for {bp_path}: {e}")

            # (b) Always also add shape components so the NPC silhouette is a
            # clear humanoid even if the skeletal mesh is missing/tiny.
            if cube and sphere:
                body_ok = _add_sm_component_to_bp(
                    bp, "VisBody", cube, mat,
                    rel_loc=(0.0, 0.0, 0.0), rel_scale=(0.45, 0.45, 1.7))
                head_ok = _add_sm_component_to_bp(
                    bp, "VisHead", sphere, mat,
                    rel_loc=(0.0, 0.0, 105.0), rel_scale=(0.35, 0.35, 0.35))
                if body_ok or head_ok:
                    fixed_any = True

            # Compile + save
            try:
                unreal.BlueprintEditorLibrary.compile_blueprint(bp)
            except Exception:
                pass
            safe_save_asset(bp_path)

            if fixed_any:
                _stats["npc_bps_fixed"] += 1
                unreal.log(f"  Fixed BP: {bp_path} ({npc_type})")
            else:
                unreal.log_warning(f"  Could not add any visual to {bp_path}")
        except Exception as e:
            unreal.log_warning(f"  Error fixing {bp_path}: {e}")

    unreal.log(f"  NPC blueprints fixed: {_stats['npc_bps_fixed']}")


def _npc_type_for_label(label):
    if label.startswith("Boss_") or "Boss" in label:
        return "Boss"
    for t in ("Jackboot", "Merchant", "Breachman", "FirstClass", "Civilian"):
        if t in label:
            return t
    return "Civilian"


def fix_npc_instances():
    """Ensure every already-placed NPC/Boss instance in the level is visible.
    Instances spawned before the BP fix may not pick up new SCS components,
    so we also attach explicit placeholder shape actors to each instance."""
    unreal.log("=== Fixing placed NPC instances ===")
    sk_mesh = find_skeletal_mesh()
    cube = get_cube()
    sphere = get_sphere()
    fixed = 0

    try:
        actors = level_lib.get_all_level_actors()
    except Exception as e:
        unreal.log_warning(f"  Could not enumerate actors: {e}")
        return

    for actor in actors:
        try:
            label = actor.get_actor_label()
        except Exception:
            continue
        if not (label.startswith("NPC_") or label.startswith("Boss_")):
            continue

        try:
            npc_type = _npc_type_for_label(label)
            mat = get_npc_material(npc_type)
            actor.set_actor_hidden_in_game(False)

            has_visible_mesh = False
            sk_comp = actor.get_component_by_class(unreal.SkeletalMeshComponent)
            if sk_comp:
                try:
                    current = sk_comp.get_editor_property("skeletal_mesh_asset")
                except Exception:
                    current = None
                if current is None and sk_mesh:
                    sk_comp.set_skeletal_mesh_asset(sk_mesh)
                    current = sk_mesh
                if current is not None:
                    has_visible_mesh = True
                    if mat:
                        try:
                            sk_comp.set_material(0, mat)
                        except Exception:
                            pass
                    sk_comp.set_editor_property("visible", True)
                    sk_comp.set_editor_property("hidden_in_game", False)

            # Belt-and-suspenders: attach shape placeholders to the instance
            # (cleaned up by label prefix on re-run).  Skip only if a real
            # (non-placeholder-cube) skeletal mesh is present.
            needs_shapes = True
            if has_visible_mesh and sk_mesh is not None:
                try:
                    mesh_path = sk_comp.get_editor_property(
                        "skeletal_mesh_asset").get_path_name()
                    if not mesh_path.startswith("/Engine/"):
                        needs_shapes = False  # real character mesh, good enough
                except Exception:
                    pass

            if needs_shapes and cube and sphere:
                loc = actor.get_actor_location()
                rot = actor.get_actor_rotation()
                scale = 1.3 if npc_type == "Boss" else 1.0

                body = place_box(
                    f"{NPCVIS_PREFIX}{label}_Body",
                    unreal.Vector(loc.x, loc.y, loc.z),
                    (45.0 * scale, 45.0 * scale, 170.0 * scale), mat)
                head = None
                hsp = level_lib.spawn_actor_from_class(
                    unreal.StaticMeshActor,
                    unreal.Vector(loc.x, loc.y, loc.z + 105.0 * scale),
                    rot)
                if hsp:
                    hsp.set_actor_label(f"{NPCVIS_PREFIX}{label}_Head")
                    hsm = hsp.get_component_by_class(unreal.StaticMeshComponent)
                    if hsm:
                        hsm.set_static_mesh(sphere)
                        hsp.set_actor_scale3d(unreal.Vector(
                            0.35 * scale, 0.35 * scale, 0.35 * scale))
                        if mat:
                            hsm.set_material(0, mat)
                    head = hsp

                # Disable collision on placeholders (purely visual) and try
                # to attach them so they follow the NPC if it moves.
                for vis in (body, head):
                    if not vis:
                        continue
                    vsm = vis.get_component_by_class(unreal.StaticMeshComponent)
                    if vsm:
                        try:
                            vsm.set_collision_profile_name("NoCollision")
                        except Exception:
                            pass
                        try:
                            vsm.set_editor_property(
                                "mobility", unreal.ComponentMobility.MOVABLE)
                        except Exception:
                            pass
                    try:
                        vis.attach_to_actor(
                            actor, "",
                            unreal.AttachmentRule.KEEP_WORLD,
                            unreal.AttachmentRule.KEEP_WORLD,
                            unreal.AttachmentRule.KEEP_WORLD,
                            False)
                    except Exception:
                        pass

            fixed += 1
        except Exception as e:
            unreal.log_warning(f"  Error fixing instance {label}: {e}")

    _stats["npc_instances_fixed"] = fixed
    unreal.log(f"  NPC/Boss instances processed: {fixed}")


# ---------------------------------------------------------------------------
# Save
# ---------------------------------------------------------------------------


def save_everything():
    unreal.log("=== Saving ===")
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        les.save_all_dirty_levels()
        unreal.log("  Saved all dirty levels")
    except Exception as e:
        unreal.log_warning(f"  Level save failed: {e}")

    # Persist blueprint + material directories we may have touched
    for d in ("/Game/Blueprints/NPCs", "/Game/Blueprints/Characters",
              "/Game/Materials/Characters", "/Game/Materials/Zone1",
              "/Game/Materials/Zones/Tail"):
        try:
            if editor_util.does_directory_exist(d):
                editor_util.save_directory(d, only_if_is_dirty=True, recursive=True)
        except Exception as e:
            unreal.log_warning(f"  save_directory failed for {d}: {e}")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------


def run():
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  World Polish & Safety Pass (Zone 1)")
    unreal.log("=" * 64)
    unreal.log("")

    if not load_zone1():
        return

    cleanup_own_actors()

    try:
        seal_gangways()
    except Exception as e:
        unreal.log_warning(f"Gangway sealing failed: {e}")

    try:
        texture_pass()
    except Exception as e:
        unreal.log_warning(f"Texture pass failed: {e}")

    try:
        fix_npc_blueprints()
    except Exception as e:
        unreal.log_warning(f"NPC blueprint fix failed: {e}")

    try:
        fix_npc_instances()
    except Exception as e:
        unreal.log_warning(f"NPC instance fix failed: {e}")

    save_everything()

    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  WORLD POLISH COMPLETE")
    unreal.log("=" * 64)
    unreal.log(f"  Gangway gaps sealed:     {_stats['gaps_sealed']} "
               f"(incl. 2 end caps)")
    unreal.log(f"  Collision blockers:      {_stats['blockers_placed']}")
    unreal.log(f"  Actors retextured:       {_stats['actors_retextured']}")
    unreal.log(f"  NPC blueprints fixed:    {_stats['npc_bps_fixed']}")
    unreal.log(f"  NPC instances fixed:     {_stats['npc_instances_fixed']}")
    unreal.log(f"  Assets saved:            {_stats['assets_saved']}")
    unreal.log("")
    unreal.log("  Enemies (Jackboot/Boss) are dark-blue/red with emissive")
    unreal.log("  accents; friendlies are grey-brown/green/amber/cream.")
    unreal.log("  Press Play to verify: walk all 15 cars end to end.")
    unreal.log("")


run()
