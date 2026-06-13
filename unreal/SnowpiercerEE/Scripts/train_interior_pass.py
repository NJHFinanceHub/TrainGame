"""
Snowpiercer: Eternal Engine -- ZONE 1 TRAIN INTERIOR PASS
Run in editor (Tools > Execute Python Script) or headless via
-ExecutePythonScript="...Scripts/train_interior_pass.py".

PLAYER COMPLAINT addressed: "the rooms are too empty, it's not really like
the Snowpiercer train -- make it more train-like."

This pass densely dresses cars 4-14 of /Game/Maps/Zone1_Tail so each reads
as a real, CRAMMED, lived-in train compartment with a distinct PURPOSE, and
adds the GENERAL TRAIN FEEL everywhere (cars 4-14): wall conduit/pipe runs,
ceiling ducts, structural support ribs every few meters, riveted wall plates,
floor grating, gauges/valve wheels, and hanging work-lights -- so it reads as
INSIDE A TRAIN, not a generic room.  Cars 1-3 get light reinforcement only
(prior TAIL_/FD_ passes already filled them); cars 0 is left to those passes.

Car identities (per design/zone1 + narrative/zone1/car-manifest.md):
  0  Caboose/memorial   1-2 Tail quarters   3 The Pit
  4  Nursery            5 Elders            6 Sickbay        7 Workshop
  8  Listening Post     9 Blockade          10 Dark Car      11 Freezer Breach
  12 Kronole Den        13 Smuggler's Cache 14 Martyr's Gate

DESIGN RULES honored:
  * Central walking corridor stays clear: NO collision-bearing props in
    |y| < AISLE_HALF (650).  Dense dressing lives at |y| > 700.
  * Thin / overhead / hanging / decorative props get collision OFF so they
    cannot snag the player (icicles, ducts, ribs, banners, chains, cables,
    rivets, gauges, hanging lamps, grating, wall plates, conduit).
  * Reuse existing materials (M_FD_*, M_Tex_*, M_*); create <= 4 new ones.
  * Budget <= 900 new TI_ actors total.
  * Per-car mood lighting; vary per car.

Idempotent: every placed actor's label starts with TI_ and all of them are
destroyed at the start of each run.  Ends with save_all_dirty_levels() AND
save_current_level() (modify-then-save; the dirty flag can be lost on headless
property-only mutations).

FILE OWNERSHIP: this is the only file this pass touches.  It does NOT edit or
re-run any other script and does NOT mutate non-TI_ actors.
"""

import random
import unreal

# ---------------------------------------------------------------------------
# API shortcuts (mirror detail_pass_film.py / tail_dressing.py)
# ---------------------------------------------------------------------------

editor_util = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mat_lib = unreal.MaterialEditingLibrary
level_lib = unreal.EditorLevelLibrary           # deprecated-but-functional in 5.7

# ---------------------------------------------------------------------------
# Constants (mirror build_zone1.py geometry)
# ---------------------------------------------------------------------------

PERSISTENT_MAP = "/Game/Maps/Zone1_Tail"
CAR_LENGTH = 12000.0
CAR_WIDTH = 4000.0
CAR_HEIGHT = 3000.0
CAR_STRIDE = 13000.0          # CAR_LENGTH + 1000 gap; car i center x = i*13000
HALF_L = CAR_LENGTH / 2.0     # 6000
HALF_W = CAR_WIDTH / 2.0      # 2000
WALL_INNER = HALF_W - 80.0    # inner wall face for wall-mounted dressing

AISLE_HALF = 650.0            # |y| < this stays free of collision props
WALL_SIDE = 700.0            # collision dressing only at |y| > this

PREFIX = "TI_"
MAX_ACTORS = 900

# Cars this pass is responsible for dressing densely.
PURPOSE_CARS = list(range(4, 15))     # 4..14
# Cars to lightly reinforce only if thin (prior passes own them).
LIGHT_REINFORCE_CARS = [1, 2, 3]
ALL_CARS = list(range(1, 15))         # train-feel + builders run 1..14

# Material directories
MAT_FD_DIR = "/Game/Materials/FilmDetail"
MAT_TEX_DIR = "/Game/Materials/Textured"
MAT_Z1_DIR = "/Game/Materials/Zone1"
MAT_TI_DIR = "/Game/Materials/TrainInterior"

# Deterministic layout on every run
rng = random.Random(18)        # 18 years trapped in the Tail

# ---------------------------------------------------------------------------
# Counters & state
# ---------------------------------------------------------------------------

_car_counts = {i: 0 for i in range(15)}
_total_new = 0
_budget_blown = False
_materials_used = []           # names, for the summary

# ---------------------------------------------------------------------------
# Small helpers
# ---------------------------------------------------------------------------


def _rot(pitch=0.0, yaw=0.0, roll=0.0):
    r = unreal.Rotator()
    r.set_editor_property("pitch", float(pitch))
    r.set_editor_property("yaw", float(yaw))
    r.set_editor_property("roll", float(roll))
    return r


def car_x(idx):
    return float(idx) * CAR_STRIDE


def _car_idx_for_x(x):
    return max(0, min(14, int((float(x) + CAR_STRIDE * 0.5) / CAR_STRIDE)))


def _count(x):
    global _total_new
    _total_new += 1
    _car_counts[_car_idx_for_x(x)] += 1


def _ok(x):
    global _budget_blown
    if _total_new >= MAX_ACTORS:
        if not _budget_blown:
            unreal.log_warning(
                f"  TI_ budget ({MAX_ACTORS}) reached -- skipping further spawns")
            _budget_blown = True
        return False
    return True


def ensure_dir(path):
    try:
        if not editor_util.does_directory_exist(path):
            editor_util.make_directory(path)
    except Exception as e:
        unreal.log_warning(f"  Could not create directory {path}: {e}")


def safe_save_asset(asset_path):
    try:
        editor_util.save_asset(asset_path, only_if_is_dirty=False)
    except Exception as e:
        unreal.log_warning(f"  Could not save asset {asset_path}: {e}")


# ---------------------------------------------------------------------------
# Shape cache & spawn helpers (tail_dressing.py pattern, with no_coll)
# ---------------------------------------------------------------------------

_shape_cache = {}


def _shape(name):
    if name not in _shape_cache:
        _shape_cache[name] = editor_util.load_asset(f"/Engine/BasicShapes/{name}")
    return _shape_cache[name]


def _spawn_shape(shape, label, x, y, z, sx, sy, sz, mat=None,
                 pitch=0.0, yaw=0.0, roll=0.0, no_coll=False):
    """Spawn a scaled BasicShape StaticMeshActor with TI_ prefix.
    Engine basic shapes are 100x100x100, so sizes (cm) are /100 for scale."""
    if not _ok(x):
        return None
    try:
        actor = level_lib.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(float(x), float(y), float(z)),
            _rot(pitch, yaw, roll))
    except Exception as e:
        unreal.log_warning(f"  spawn failed ({label}): {e}")
        return None
    if not actor:
        return None

    actor.set_actor_label(f"{PREFIX}{label}")
    sm = actor.get_component_by_class(unreal.StaticMeshComponent)
    if sm:
        mesh = _shape(shape)
        if mesh:
            sm.set_static_mesh(mesh)
        actor.set_actor_scale3d(
            unreal.Vector(sx / 100.0, sy / 100.0, sz / 100.0))
        if mat:
            sm.set_material(0, mat)
        if no_coll:
            try:
                sm.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
            except Exception:
                try:
                    sm.set_editor_property(
                        "collision_enabled",
                        unreal.CollisionEnabled.NO_COLLISION)
                except Exception:
                    pass
    _count(x)
    return actor


def box(label, x, y, z, sx, sy, sz, mat=None,
        pitch=0.0, yaw=0.0, roll=0.0, no_coll=False):
    return _spawn_shape("Cube", label, x, y, z, sx, sy, sz, mat,
                        pitch, yaw, roll, no_coll)


def cyl(label, x, y, z, dia, height, mat=None,
        pitch=0.0, yaw=0.0, roll=0.0, no_coll=False):
    return _spawn_shape("Cylinder", label, x, y, z, dia, dia, height, mat,
                        pitch, yaw, roll, no_coll)


def sphere(label, x, y, z, r, mat=None, no_coll=False):
    return _spawn_shape("Sphere", label, x, y, z, r, r, r, mat,
                        no_coll=no_coll)


def cone(label, x, y, z, dia, height, mat=None,
         pitch=0.0, yaw=0.0, roll=0.0, no_coll=False):
    return _spawn_shape("Cone", label, x, y, z, dia, dia, height, mat,
                        pitch, yaw, roll, no_coll)


def light(label, x, y, z, color_rgb, intensity, radius):
    """Warm/cold practical point light (no shadows -- perf)."""
    if not _ok(x):
        return None
    try:
        pl = level_lib.spawn_actor_from_class(
            unreal.PointLight,
            unreal.Vector(float(x), float(y), float(z)),
            _rot())
    except Exception as e:
        unreal.log_warning(f"  light spawn failed ({label}): {e}")
        return None
    if not pl:
        return None
    pl.set_actor_label(f"{PREFIX}{label}")
    plc = pl.get_component_by_class(unreal.PointLightComponent)
    if plc:
        plc.set_editor_property("intensity", float(intensity))
        plc.set_editor_property("attenuation_radius", float(radius))
        plc.set_editor_property("light_color", unreal.Color(
            int(color_rgb[0]), int(color_rgb[1]), int(color_rgb[2]), 255))
        try:
            plc.set_editor_property("cast_shadows", False)
        except Exception:
            pass
    _count(x)
    return pl


# ---------------------------------------------------------------------------
# Material palette -- reuse existing assets; create <= 4 new TI_ materials.
# ---------------------------------------------------------------------------

_mat_cache = {}


def _load_mat(path):
    if path in _mat_cache:
        return _mat_cache[path]
    try:
        if editor_util.does_asset_exist(path):
            m = editor_util.load_asset(path)
            _mat_cache[path] = m
            return m
    except Exception as e:
        unreal.log_warning(f"  Could not load material {path}: {e}")
    _mat_cache[path] = None
    return None


def _make_flat_material(name, base, roughness=0.7, metallic=0.0, emissive=None):
    """Create a small flat/emissive PBR material under MAT_TI_DIR
    (build_zone1.get_material / get_fd_material pattern).  Idempotent: if it
    already exists, load and reuse.  emissive=(r,g,b) HDR (>1 glows)."""
    ensure_dir(MAT_TI_DIR)
    mat_path = f"{MAT_TI_DIR}/{name}"
    if editor_util.does_asset_exist(mat_path):
        return editor_util.load_asset(mat_path)
    mat = asset_tools.create_asset(name, MAT_TI_DIR,
                                   unreal.Material, unreal.MaterialFactoryNew())
    if not mat:
        unreal.log_warning(f"  Failed to create material: {name}")
        return None
    try:
        color_node = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionConstant3Vector, -300, -200)
        color_node.set_editor_property(
            "constant", unreal.LinearColor(base[0], base[1], base[2], 1.0))
        mat_lib.connect_material_property(
            color_node, "", unreal.MaterialProperty.MP_BASE_COLOR)

        rough_node = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionConstant, -300, 0)
        rough_node.set_editor_property("r", float(roughness))
        mat_lib.connect_material_property(
            rough_node, "", unreal.MaterialProperty.MP_ROUGHNESS)

        metal_node = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionConstant, -300, 150)
        metal_node.set_editor_property("r", float(metallic))
        mat_lib.connect_material_property(
            metal_node, "", unreal.MaterialProperty.MP_METALLIC)

        if emissive:
            em_node = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionConstant3Vector, -300, 320)
            em_node.set_editor_property(
                "constant",
                unreal.LinearColor(emissive[0], emissive[1], emissive[2], 1.0))
            mat_lib.connect_material_property(
                em_node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    except Exception as e:
        unreal.log_warning(f"  Expression wiring failed for {name}: {e}")
    try:
        mat_lib.recompile_material(mat)
    except Exception as e:
        unreal.log_warning(f"  Recompile failed for {name}: {e}")
    safe_save_asset(mat_path)
    unreal.log(f"  Created {mat_path}")
    return mat


def load_palette():
    """Reuse the FD_ / M_Tex_ / Zone1 palette; add at most 4 new TI_ flats.
    Returns a dict of role -> material, with a fallback cascade so a missing
    asset never crashes a builder."""
    unreal.log("=" * 64)
    unreal.log("MATERIALS -- reuse FD_/M_Tex_/Zone1; create <= 4 new TI_ flats")
    unreal.log("=" * 64)
    m = {}

    # --- Reused FilmDetail flats/emissives (detail_pass_film.py) ---
    m["dark_metal"] = _load_mat(f"{MAT_FD_DIR}/M_FD_DarkMetal")
    m["fire_glow"] = _load_mat(f"{MAT_FD_DIR}/M_FD_FireGlow")
    m["kronole_glow"] = _load_mat(f"{MAT_FD_DIR}/M_FD_KronoleGlow")
    m["blood"] = _load_mat(f"{MAT_FD_DIR}/M_FD_BloodStain")
    m["sandbag"] = _load_mat(f"{MAT_FD_DIR}/M_FD_Sandbag")
    m["wax"] = _load_mat(f"{MAT_FD_DIR}/M_FD_CandleWax")
    m["dark_cloth"] = _load_mat(f"{MAT_FD_DIR}/M_FD_DarkCloth")
    m["stripes"] = (_load_mat(f"{MAT_TEX_DIR}/M_Tex_stripes_warning")
                    or _load_mat(f"{MAT_FD_DIR}/M_FD_HazardYellow"))

    # --- Reused textured wood/fabric (detail_pass_film.py) ---
    m["wood_worn"] = _load_mat(f"{MAT_TEX_DIR}/M_Tex_wood_worn_planks")
    m["wood_grey"] = _load_mat(f"{MAT_TEX_DIR}/M_Tex_wood_planks_grey")
    m["fabric"] = _load_mat(f"{MAT_TEX_DIR}/M_Tex_fabric_pattern_07")

    # --- Reused Zone1 surface/accent materials (build_zone1.py) ---
    m["bunk_wood"] = _load_mat(f"{MAT_Z1_DIR}/M_Bunk_Wood")
    m["tail_wall"] = _load_mat(f"{MAT_Z1_DIR}/M_Tail_Wall")
    m["ice"] = (_load_mat(f"{MAT_Z1_DIR}/M_Ice_Blue")
                or _load_mat(f"{MAT_FD_DIR}/M_FD_DarkMetal"))
    m["sickbay_green"] = _load_mat(f"{MAT_Z1_DIR}/M_Sickbay_Green")
    m["crate"] = _load_mat(f"{MAT_Z1_DIR}/M_Cache_Crate")

    # --- New TI_ materials (max 4) for looks the palette lacks ---
    # 1. Frost: pale translucent-ish ice for shards/icicles (slight emissive).
    m["frost"] = _make_flat_material(
        "M_TI_Frost", (0.62, 0.78, 0.92), roughness=0.18, metallic=0.0,
        emissive=(0.25, 0.45, 0.65))
    # 2. Bright bulb glass for hanging work-lights / lamps.
    m["bulb"] = _make_flat_material(
        "M_TI_BulbGlow", (1.0, 0.92, 0.7), roughness=0.1, metallic=0.0,
        emissive=(6.0, 4.6, 2.0))
    # 3. Sickly med-green glass for IV/medicine bottles & sickbay glow.
    m["med_glow"] = _make_flat_material(
        "M_TI_MedGlow", (0.18, 0.55, 0.32), roughness=0.2, metallic=0.0,
        emissive=(0.4, 1.6, 0.7))
    # 4. Brass/bronze for gauges, valve wheels, scales, kettles, rivets.
    m["brass"] = _make_flat_material(
        "M_TI_Brass", (0.32, 0.24, 0.10), roughness=0.45, metallic=0.95)

    for n in ("M_TI_Frost", "M_TI_BulbGlow", "M_TI_MedGlow", "M_TI_Brass"):
        _materials_used.append(n + " (new)")

    # Fallback cascade so a missing asset never breaks a builder.
    fb = [
        ("dark_metal", "tail_wall", "brass"),
        ("fire_glow", "bulb", "dark_metal"),
        ("kronole_glow", "med_glow", "fire_glow"),
        ("blood", "dark_cloth", "dark_metal"),
        ("sandbag", "wood_worn", "fabric"),
        ("wax", "fabric", "dark_cloth"),
        ("dark_cloth", "fabric", "sandbag"),
        ("stripes", "brass", "dark_metal"),
        ("wood_worn", "bunk_wood", "wood_grey", "tail_wall"),
        ("wood_grey", "bunk_wood", "wood_worn", "tail_wall"),
        ("fabric", "dark_cloth", "sandbag"),
        ("bunk_wood", "wood_worn", "tail_wall"),
        ("tail_wall", "dark_metal", "brass"),
        ("ice", "frost", "dark_metal"),
        ("sickbay_green", "med_glow", "tail_wall"),
        ("crate", "wood_worn", "bunk_wood", "tail_wall"),
    ]
    for row in fb:
        key = row[0]
        if m.get(key) is None:
            for alt in row[1:]:
                if m.get(alt) is not None:
                    unreal.log_warning(f"  Material '{key}' missing -- using '{alt}'")
                    m[key] = m[alt]
                    break

    # Record reused (non-new) names for the summary.
    reused_names = [
        "M_FD_DarkMetal", "M_FD_FireGlow", "M_FD_KronoleGlow", "M_FD_BloodStain",
        "M_FD_Sandbag", "M_FD_CandleWax", "M_FD_DarkCloth", "M_Tex_stripes_warning",
        "M_Tex_wood_worn_planks", "M_Tex_wood_planks_grey", "M_Tex_fabric_pattern_07",
        "M_Bunk_Wood", "M_Tail_Wall", "M_Ice_Blue", "M_Sickbay_Green", "M_Cache_Crate",
    ]
    for n in reused_names:
        _materials_used.append(n)

    have = [k for k, v in m.items() if v is not None]
    miss = [k for k, v in m.items() if v is None]
    unreal.log(f"  Palette ready: {len(have)} roles populated")
    if miss:
        unreal.log_warning(f"  Roles still empty (final fallback to dark_metal): {miss}")
        for k in miss:
            m[k] = m.get("dark_metal")
    return m


# ---------------------------------------------------------------------------
# Reusable dressing primitives
# ---------------------------------------------------------------------------

def hanging_worklight(tag, x, y, mats, drop=600.0, color=(255, 196, 130),
                      intensity=1100.0, radius=900.0):
    """Caged work-lamp on a flex cable dropping from the ceiling.
    Cable + cage + emissive bulb (all no-collision) + a point light."""
    cyl(f"{tag}_cable", x, y, CAR_HEIGHT - drop * 0.25, 16.0, drop * 0.5,
        mats["dark_metal"], no_coll=True)
    sphere(f"{tag}_bulb", x, y, CAR_HEIGHT - drop, 90.0, mats["bulb"], no_coll=True)
    light(f"{tag}_glow", x, y, CAR_HEIGHT - drop - 60.0, color, intensity, radius)


def gauge(tag, x, y, z, mats, side=1.0):
    """Round pressure gauge mounted on a wall (no collision, thin)."""
    cyl(f"{tag}_face", x, side * WALL_INNER, z, 220.0, 30.0, mats["brass"],
        roll=90.0, no_coll=True)
    cyl(f"{tag}_rim", x, side * (WALL_INNER - 18.0), z, 260.0, 24.0,
        mats["dark_metal"], roll=90.0, no_coll=True)


def valve_wheel(tag, x, y, z, mats, side=1.0):
    """Valve hand-wheel on a stub pipe against a wall (no collision)."""
    cyl(f"{tag}_stub", x, side * (WALL_INNER - 60.0), z, 110.0, 240.0,
        mats["dark_metal"], roll=90.0, no_coll=True)
    cyl(f"{tag}_wheel", x, side * (WALL_INNER - 180.0), z, 360.0, 50.0,
        mats["brass"], roll=90.0, no_coll=True)
    for s in range(2):
        box(f"{tag}_spoke{s}", x, side * (WALL_INNER - 180.0), z,
            330.0, 30.0, 30.0, mats["brass"], roll=90.0,
            yaw=s * 90.0, no_coll=True)


def crate_stack(tag, x, y, mats, n=2, base=520.0):
    """Stack of n crates against a wall (collision ON; they are solid props,
    but only ever placed at |y| > WALL_SIDE)."""
    z = 0.0
    for i in range(n):
        s = base * (1.0 - i * 0.16)
        box(f"{tag}_{i}", x, y, z + s * 0.5, s, s * rng.uniform(0.85, 1.1),
            s * rng.uniform(0.7, 0.95), mats["crate"],
            yaw=rng.uniform(-8, 8))
        z += s * 0.85


def cloth_hang(tag, x, y, z, w, h, mats, yaw=0.0, along_x=False, dark=False):
    """Hanging cloth slab: thin, no collision (laundry/partition/curtain)."""
    yaw = yaw + rng.uniform(-12.0, 12.0)
    mat = mats["dark_cloth"] if dark else mats["fabric"]
    if along_x:
        box(tag, x, y, z, w, 16.0, h, mat, yaw=yaw, no_coll=True)
    else:
        box(tag, x, y, z, 16.0, w, h, mat, yaw=yaw, no_coll=True)


def brazier(tag, x, y, mats, intensity=1400.0, radius=2600.0):
    """Oil-drum brazier: rusty drum + emissive embers + warm light."""
    cyl(f"{tag}_drum", x, y, 420.0, 460.0, 840.0, mats["dark_metal"])
    cyl(f"{tag}_embers", x, y, 850.0, 380.0, 60.0, mats["fire_glow"], no_coll=True)
    light(f"{tag}_glow", x, y, 1180.0, (255, 140, 50), intensity, radius)


# ---------------------------------------------------------------------------
# GENERAL TRAIN FEEL -- structural skeleton in every purpose car (4-14)
# ---------------------------------------------------------------------------

def train_feel(idx, mats, ribs=True, full_pipes=True, n_ribs=3, n_plates=2,
               n_worklights=2):
    """Wall conduit/pipe runs, ceiling ducts, structural support ribs every
    few meters, riveted wall plates, floor grating, gauges/valve wheels, and
    hanging work-lights -- so the car reads as INSIDE A TRAIN.  Kept lean
    (~35 actors) so 10 cars stay well inside the 900 budget.
    Everything thin/overhead is no-collision; all wall dressing |y| > WALL."""
    cx = car_x(idx)

    # --- Structural support ribs: a ceiling cross-beam + two wall posts every
    # few meters (the train's structural ribs).  No collision. ---
    if ribs:
        for r in range(n_ribs):
            rx = cx - HALF_L + 1400.0 + r * (CAR_LENGTH - 2800.0) / max(n_ribs - 1, 1)
            box(f"C{idx:02d}_rib{r}_top", rx, 0.0, CAR_HEIGHT - 120.0,
                160.0, CAR_WIDTH - 60.0, 220.0, mats["dark_metal"], no_coll=True)
            for s in (1.0, -1.0):
                box(f"C{idx:02d}_rib{r}_post{int(s)}", rx, s * WALL_INNER,
                    CAR_HEIGHT * 0.5, 150.0, 130.0, CAR_HEIGHT,
                    mats["dark_metal"], no_coll=True)

    # --- Ceiling pipe/conduit runs the length of the car (both sides) +
    # one electrical conduit bundle. ---
    if full_pipes:
        for s, dia, z in ((1.0, 150.0, CAR_HEIGHT - 200.0),
                          (-1.0, 110.0, CAR_HEIGHT - 300.0)):
            cyl(f"C{idx:02d}_pipe{int(s)}", cx, s * (WALL_INNER - 120.0), z,
                dia, CAR_LENGTH - 300.0, mats["dark_metal"], pitch=90.0,
                no_coll=True)
        # one electrical conduit bundle mid-wall
        side = 1.0 if idx % 2 == 0 else -1.0
        cyl(f"C{idx:02d}_conduit", cx, side * (WALL_INNER - 40.0), 1880.0,
            70.0, CAR_LENGTH - 800.0, mats["dark_metal"], pitch=90.0,
            no_coll=True)

    # --- Mid-wall HVAC duct run (one side). ---
    duct_side = -1.0 if idx % 2 == 0 else 1.0
    box(f"C{idx:02d}_duct", cx, duct_side * (WALL_INNER - 30.0), 2200.0,
        CAR_LENGTH - 1400.0, 230.0, 300.0, mats["dark_metal"], no_coll=True)

    # --- Riveted wall plates along both walls (panel + 4 corner rivet dots). ---
    for p in range(n_plates):
        px = cx - HALF_L + 1700.0 + p * (CAR_LENGTH - 3400.0) / max(n_plates - 1, 1)
        for s in (1.0, -1.0):
            box(f"C{idx:02d}_plate{p}_{int(s)}", px, s * (WALL_INNER + 10.0),
                1300.0, 1100.0, 30.0, 1500.0, mats["tail_wall"], no_coll=True)
            # rivet rows top+bottom on the panel (single thin box each)
            box(f"C{idx:02d}_plrvT{p}_{int(s)}", px, s * (WALL_INNER - 8.0),
                1980.0, 1000.0, 20.0, 40.0, mats["dark_metal"], no_coll=True)
            box(f"C{idx:02d}_plrvB{p}_{int(s)}", px, s * (WALL_INNER - 8.0),
                620.0, 1000.0, 20.0, 40.0, mats["dark_metal"], no_coll=True)

    # --- Floor grating strip along one aisle edge (flat, no collision). ---
    box(f"C{idx:02d}_grate", cx, 760.0, 8.0,
        CAR_LENGTH - 600.0, 360.0, 14.0, mats["dark_metal"], no_coll=True)

    # --- Gauges + a valve wheel on the walls. ---
    gauge(f"C{idx:02d}_gaugeA", cx - 2600.0, 0.0, 1700.0, mats, side=1.0)
    valve_wheel(f"C{idx:02d}_valve", cx + 1800.0, 0.0, 1400.0, mats, side=1.0)

    # --- Hanging work-lights down the car (off-aisle, no collision). ---
    for k in range(n_worklights):
        dx = -3400.0 + k * (6800.0 / max(n_worklights - 1, 1))
        s = 1.0 if k % 2 == 0 else -1.0
        hanging_worklight(f"C{idx:02d}_worklight{k}", cx + dx, s * 1100.0,
                          mats, drop=rng.uniform(550.0, 750.0))


# ---------------------------------------------------------------------------
# PER-CAR PURPOSE BUILDERS (cars 4-14)
# ---------------------------------------------------------------------------

def car04_nursery(mats):
    """NURSERY: low cots/cribs, a teaching corner (crate seats + chalk panel),
    hanging cloth dividers, soft toys, a chalk-scrawled wall panel."""
    cx = car_x(4)
    # Low cots/cribs along both walls (small frames, child-scale).
    for i in range(4):
        bx = cx - 4200.0 + i * 2800.0
        for s in (1.0, -1.0):
            box(f"C04_crib{i}_{int(s)}", bx, s * 1500.0, 180.0,
                900.0, 560.0, 340.0, mats["wood_worn"], yaw=rng.uniform(-4, 4))
            # crib side rails (thin)
            box(f"C04_cribrail{i}_{int(s)}", bx, s * 1500.0, 380.0,
                880.0, 540.0, 40.0, mats["wood_grey"], no_coll=True)
            # a soft toy (tiny cube) on each cot
            sphere(f"C04_toy{i}_{int(s)}", bx + rng.uniform(-200, 200),
                   s * 1500.0, 420.0, 70.0, mats["fabric"], no_coll=True)
    # Hanging cloth dividers between crib bays.
    for i in range(4):
        mx = cx - 3300.0 + i * 2200.0
        for s in (1.0, -1.0):
            cloth_hang(f"C04_divider{i}_{int(s)}", mx, s * 1500.0, 950.0,
                       720.0, 1250.0, mats)
    # Teaching corner at the front: crate seats in a ring + a chalk wall panel.
    tcx = cx + 3800.0
    for j, (dx, dy) in enumerate(((-500, 1300), (500, 1300), (-500, 800),
                                  (500, 800))):
        box(f"C04_seat{j}", tcx + dx, dy, 130.0, 260.0, 260.0, 260.0,
            mats["crate"], yaw=rng.uniform(0, 30))
    # Chalk-scrawled wall panel (dark slate board on the wall).
    box("C04_chalkpanel", tcx, WALL_INNER, 1500.0, 1600.0, 30.0, 1100.0,
        mats["dark_cloth"], no_coll=True)
    for c in range(6):  # chalk scrawls (pale thin marks)
        box(f"C04_chalk{c}", tcx + rng.uniform(-600, 600), WALL_INNER - 18.0,
            1500.0 + rng.uniform(-380, 380), rng.uniform(120, 340), 6.0,
            22.0, mats["wax"], no_coll=True, roll=rng.uniform(-20, 20))
    # Soft toys / bundles scattered on the floor.
    for j in range(4):
        sphere(f"C04_softtoy{j}", cx + rng.uniform(-4500, 4500),
               rng.choice([-1, 1]) * rng.uniform(1000, 1700), 90.0,
               rng.uniform(70, 130), mats["fabric"], no_coll=True)
    # Soft warm lighting.
    for k, dx in enumerate((-3500.0, 0.0, 3500.0)):
        light(f"C04_soft{k}", cx + dx, rng.choice([-700.0, 700.0]), 2150.0,
              (255, 224, 188), 1300.0, 3200.0)


def car05_elders(mats):
    """ELDERS: communal table with benches, oil lamps, shelves of salvaged
    keepsakes, blankets, a tea/brew station (kettle = cylinder + spout)."""
    cx = car_x(5)
    # Communal long table down one side of center (clear of aisle: top edge
    # at y=+/-, kept off the lane).
    box("C05_table", cx - 800.0, 1150.0, 430.0, 4200.0, 900.0, 110.0,
        mats["wood_worn"])
    box("C05_tabletop", cx - 800.0, 1150.0, 500.0, 4350.0, 1000.0, 50.0,
        mats["wood_grey"], no_coll=True)
    # Benches along the table (both long sides, off-aisle).
    for j, by in ((0, 1750.0), (1, 720.0)):
        box(f"C05_bench{j}", cx - 800.0, by, 200.0, 4000.0, 280.0, 300.0,
            mats["wood_worn"])
    # Oil lamps on the table (brass body + emissive flame + warm light).
    for k, dx in enumerate((-2200.0, 0.0, 2200.0)):
        lx = cx - 800.0 + dx
        cyl(f"C05_lamp{k}_body", lx, 1150.0, 600.0, 130.0, 200.0, mats["brass"],
            no_coll=True)
        sphere(f"C05_lamp{k}_flame", lx, 1150.0, 740.0, 60.0, mats["fire_glow"],
               no_coll=True)
        light(f"C05_lamp{k}_glow", lx, 1150.0, 820.0, (255, 168, 78),
              rng.uniform(700, 950), 1500.0)
    # Shelves of salvaged keepsakes on the opposite wall.
    for r, rz in enumerate((900.0, 1400.0, 1900.0)):
        box(f"C05_shelf{r}", cx, -WALL_INNER, rz, 5000.0, 240.0, 40.0,
            mats["wood_grey"], no_coll=True)
        for k in range(5):
            kx = cx - 2400.0 + k * 1100.0
            s = rng.uniform(110, 230)
            box(f"C05_keepsake{r}_{k}", kx, -(WALL_INNER - 60.0), rz + s * 0.5 + 20.0,
                s, s * rng.uniform(0.7, 1.2), s * rng.uniform(0.8, 1.5),
                mats["brass"] if k % 2 else mats["crate"],
                yaw=rng.uniform(0, 360), no_coll=True)
    # Tea/brew station near the front: kettle (cyl) + spout (thin cyl) + cups.
    bsx = cx + 3600.0
    box("C05_brewbench", bsx, -1300.0, 420.0, 1100.0, 700.0, 120.0,
        mats["dark_metal"])
    cyl("C05_kettle", bsx, -1300.0, 640.0, 360.0, 300.0, mats["brass"])
    cyl("C05_kettle_spout", bsx + 220.0, -1300.0, 700.0, 60.0, 240.0,
        mats["brass"], pitch=55.0, no_coll=True)
    cyl("C05_kettle_lid", bsx, -1300.0, 800.0, 200.0, 50.0, mats["brass"],
        no_coll=True)
    for c in range(4):
        cyl(f"C05_cup{c}", bsx + rng.uniform(-400, 400), -1300.0 + rng.uniform(-250, 250),
            520.0, 90.0, 110.0, mats["crate"], no_coll=True)
    # Blankets draped over benches / table (cloth).
    for j in range(4):
        cloth_hang(f"C05_blanket{j}", cx - 2200.0 + j * 1600.0, 1700.0, 380.0,
                   900.0, 520.0, mats, along_x=True, dark=(j % 2 == 0))
    # Warm dignified council lighting.
    light("C05_warmA", cx - 1500.0, 0.0, 2200.0, (244, 204, 146), 1500.0, 3600.0)
    light("C05_warmB", cx + 2200.0, 0.0, 2200.0, (244, 204, 146), 1300.0, 3400.0)


def car06_sickbay(mats):
    """SICKBAY: cots in rows, a medical bench with bottles, IV-pole stands,
    a privacy curtain, basins; sickly green-tinted light."""
    cx = car_x(6)
    # Cots in rows along both walls.
    for i in range(3):
        bx = cx - 4000.0 + i * 3600.0
        for s in (1.0, -1.0):
            box(f"C06_cot{i}_{int(s)}", bx, s * 1450.0, 230.0,
                1500.0, 680.0, 360.0, mats["wood_grey"])
            # thin mattress pad
            box(f"C06_pad{i}_{int(s)}", bx, s * 1450.0, 430.0,
                1480.0, 660.0, 60.0, mats["fabric"], no_coll=True)
            # IV-pole stand beside each cot (pole + drip bag + hook).
            ivx = bx + 700.0
            cyl(f"C06_ivpole{i}_{int(s)}", ivx, s * 1750.0, 900.0, 50.0, 1800.0,
                mats["dark_metal"], no_coll=True)
            box(f"C06_ivbag{i}_{int(s)}", ivx, s * 1750.0, 1650.0,
                160.0, 90.0, 260.0, mats["med_glow"], no_coll=True)
    # Medical bench with bottles (small cylinders) near the front.
    mbx = cx + 3700.0
    box("C06_medbench", mbx, 0.0, 450.0, 1400.0, 760.0, 120.0, mats["dark_metal"])
    box("C06_medbench_top", mbx, 0.0, 520.0, 1500.0, 860.0, 40.0,
        mats["sickbay_green"], no_coll=True)
    for b in range(6):
        cyl(f"C06_bottle{b}", mbx + rng.uniform(-600, 600),
            rng.uniform(-300, 300), 600.0, rng.uniform(60, 110),
            rng.uniform(160, 280), mats["med_glow"], no_coll=True)
    # Medicine cabinet behind the bench.
    box("C06_cabinet", mbx, -WALL_INNER, 1300.0, 1300.0, 60.0, 1100.0,
        mats["dark_metal"], no_coll=True)
    # Basins (two squat cylinders against a wall).
    for b in range(2):
        cyl(f"C06_basin{b}", cx - 1500.0 + b * 800.0, WALL_INNER - 250.0,
            520.0, 460.0, 200.0, mats["dark_metal"])
    # Privacy curtains pulled around some cots.
    for j, (bx, s) in enumerate(((cx - 4200.0, 1.0), (cx + 1000.0, -1.0),
                                 (cx - 1600.0, 1.0))):
        cloth_hang(f"C06_curtain{j}", bx, s * 1050.0, 1150.0, 1700.0, 1500.0,
                   mats, along_x=True)
    # Sickly green-tinted light (with one warm bench lamp for contrast).
    for k, dx in enumerate((-3500.0, 0.0, 3500.0)):
        light(f"C06_green{k}", cx + dx, rng.choice([-800.0, 800.0]), 2250.0,
              (150, 224, 168), 2400.0, 3400.0)
    light("C06_benchlamp", mbx, 0.0, 1500.0, (255, 220, 170), 900.0, 1400.0)


def car07_workshop(mats):
    """WORKSHOP: workbenches, tool racks (pegboard wall + tool shapes), a vise,
    scrap-metal piles, sparks-area forge (emissive), hanging chains,
    gear/cog props."""
    cx = car_x(7)
    # Two heavy workbenches (off-aisle, one per side).
    for j, (s, by) in enumerate(((1.0, 1300.0), (-1.0, -1300.0))):
        bx = cx - 1000.0 + j * 3000.0
        box(f"C07_bench{j}", bx, by, 450.0, 2400.0, 800.0, 120.0,
            mats["dark_metal"])
        box(f"C07_benchtop{j}", bx, by, 520.0, 2500.0, 900.0, 40.0,
            mats["wood_worn"], no_coll=True)
        # a vise clamped to the bench edge
        box(f"C07_vise{j}", bx + 900.0, by, 620.0, 220.0, 160.0, 200.0,
            mats["brass"], no_coll=True)
    # Pegboard tool-rack walls with hung tool shapes (both walls).
    for s in (1.0, -1.0):
        box(f"C07_pegboard{int(s)}", cx - 1500.0, s * WALL_INNER, 1700.0,
            3000.0, 30.0, 1200.0, mats["tail_wall"], no_coll=True)
        for t in range(4):
            tx = cx - 2700.0 + t * 720.0
            # hammer/file/wrench stand-ins: thin boxes + a cyl handle
            box(f"C07_tool{int(s)}_{t}", tx, s * (WALL_INNER - 20.0),
                1700.0 + rng.uniform(-300, 300), 90.0, 20.0,
                rng.uniform(300, 520), mats["dark_metal"], no_coll=True,
                roll=rng.uniform(-15, 15))
    # Forge / sparks area (emissive heart + warm orange light + anvil).
    fx, fy = cx + 3500.0, 1350.0
    cyl("C07_forge_body", fx, fy, 450.0, 900.0, 900.0, mats["dark_metal"])
    cyl("C07_forge_coals", fx, fy, 920.0, 760.0, 90.0, mats["fire_glow"],
        no_coll=True)
    box("C07_anvil", fx - 1000.0, fy, 600.0, 700.0, 300.0, 400.0,
        mats["dark_metal"])
    light("C07_forge_glow", fx, fy, 1100.0, (255, 130, 40), 2000.0, 3200.0)
    # Scrap-metal piles near the walls.
    for j in range(4):
        s = rng.uniform(340, 620)
        box(f"C07_scrap{j}", cx + rng.uniform(-3500, 2500),
            rng.choice([-1, 1]) * rng.uniform(1500, 1800), s * 0.35,
            s, s * rng.uniform(0.7, 1.2), s * 0.7, mats["dark_metal"],
            yaw=rng.uniform(0, 360))
    # Gear / cog props (flat cylinders) leaned against scrap.
    for j in range(4):
        cyl(f"C07_cog{j}", cx + rng.uniform(-3000, 3000),
            rng.choice([-1, 1]) * rng.uniform(1550, 1750), 260.0,
            rng.uniform(340, 560), 90.0, mats["brass"], roll=90.0, no_coll=True)
    # Hanging chains from the ceiling (no collision).
    for j in range(4):
        chx = cx - 4000.0 + j * 2600.0
        h = rng.uniform(500, 1100)
        cyl(f"C07_chain{j}", chx, rng.choice([-1, 1]) * rng.uniform(900, 1500),
            CAR_HEIGHT - h * 0.5, 32.0, h, mats["dark_metal"], no_coll=True)


def car08_listening_post(mats):
    """LISTENING POST: salvaged comms gear (stacked boxes with dials), wire
    bundles, a map wall, headphones on a hook, a lookout stool by a vent."""
    cx = car_x(8)
    # Comms console wall: stacked equipment boxes with dial faces.
    for j in range(3):
        ex = cx - 3000.0 + j * 1400.0
        box(f"C08_rack{j}", ex, -1400.0, 700.0, 1100.0, 700.0, 1400.0,
            mats["dark_metal"])
        for d in range(3):  # dials/gauges on the face
            cyl(f"C08_dial{j}_{d}", ex - 350.0 + d * 350.0, -1050.0,
                900.0 + (d % 2) * 400.0, 150.0, 30.0, mats["brass"],
                pitch=90.0, no_coll=True)
        # blinking indicator (emissive)
        sphere(f"C08_led{j}", ex, -1050.0, 1300.0, 50.0, mats["fire_glow"],
               no_coll=True)
    # Desk in front of the racks with a stool.
    box("C08_desk", cx - 2000.0, -800.0, 430.0, 1600.0, 600.0, 100.0,
        mats["wood_worn"])
    cyl("C08_stool", cx - 2000.0, -250.0, 250.0, 360.0, 500.0, mats["wood_grey"])
    # Headphones on a hook (band cyl + two ear cups) by the desk.
    cyl("C08_hp_band", cx - 2700.0, -(WALL_INNER - 40.0), 1500.0, 280.0, 40.0,
        mats["dark_metal"], roll=90.0, no_coll=True)
    for s in (1.0, -1.0):
        sphere(f"C08_hp_cup{int(s)}", cx - 2700.0 + s * 130.0,
               -(WALL_INNER - 60.0), 1380.0, 110.0, mats["dark_cloth"],
               no_coll=True)
    # Wire bundles slung along the ceiling and dropping to the racks.
    for j in range(4):
        wx = cx - 3000.0 + j * 1900.0
        cyl(f"C08_wire{j}", wx, -(WALL_INNER - 200.0), CAR_HEIGHT - 500.0,
            60.0, rng.uniform(600, 1100), mats["dark_cloth"], no_coll=True)
    # Map wall: large panel + pinned route slips on the opposite wall.
    box("C08_mapwall", cx + 500.0, WALL_INNER, 1600.0, 4000.0, 30.0, 1500.0,
        mats["wood_grey"], no_coll=True)
    for p in range(7):
        box(f"C08_mapslip{p}", cx - 1300.0 + p * 540.0, WALL_INNER - 18.0,
            1300.0 + rng.uniform(-400, 500), rng.uniform(160, 300), 6.0,
            rng.uniform(180, 280), mats["fabric"], no_coll=True,
            roll=rng.uniform(-8, 8))
    # red string between pins (thin angled cylinders)
    for p in range(5):
        cyl(f"C08_string{p}", cx - 800.0 + p * 600.0, WALL_INNER - 22.0,
            1500.0, 18.0, rng.uniform(500, 900), mats["blood"], pitch=90.0,
            yaw=rng.uniform(-40, 40), no_coll=True)
    # Lookout stool by a wall vent at the front.
    box("C08_vent", cx + 4200.0, WALL_INNER, 1400.0, 700.0, 30.0, 700.0,
        mats["dark_metal"], no_coll=True)
    cyl("C08_lookout_stool", cx + 4200.0, 1300.0, 280.0, 360.0, 560.0,
        mats["wood_worn"])
    # Dim, conspiratorial lighting -- one desk lamp + one bluish glow.
    light("C08_desklamp", cx - 2000.0, -800.0, 1400.0, (255, 200, 140),
          1000.0, 1500.0)
    light("C08_coldglow", cx + 1500.0, 0.0, 2200.0, (150, 175, 220), 1100.0,
          3000.0)


def car09_blockade(mats):
    """BLOCKADE (jackboot checkpoint): riot barricades (angled slabs),
    sandbag stacks, a guard desk with a lamp, a weapons rack against the wall,
    warning-stripe panels, a spotlight."""
    cx = car_x(9)
    # Angled riot barricade slabs flanking the lane (off-aisle).
    for i, (dx, s) in enumerate(((-1200.0, 1.0), (-1200.0, -1.0),
                                 (600.0, 1.0), (600.0, -1.0))):
        box(f"C09_riot{i}", cx + dx, s * 1150.0, 470.0, 110.0, 1300.0, 900.0,
            mats["dark_metal"], yaw=s * 22.0, pitch=-10.0)
    # Sandbag stacks (squashed spheres) at firing positions.
    bag = 0
    for arc_x, ys in ((500.0, (1000.0, 1450.0, 1800.0)),
                      (1700.0, (-1000.0, -1450.0, -1800.0))):
        for sy in ys:
            sphere(f"C09_sandbag{bag}", cx + arc_x + rng.uniform(-90, 90),
                   sy, 130.0, 460.0, mats["sandbag"])
            bag += 1
    # Guard desk with a lamp (jackboot side).
    gdx = cx + 3400.0
    box("C09_guarddesk", gdx, -1300.0, 450.0, 1500.0, 800.0, 120.0,
        mats["dark_metal"])
    cyl("C09_guardstool", gdx, -650.0, 280.0, 360.0, 560.0, mats["dark_metal"])
    cyl("C09_desklamp_arm", gdx + 500.0, -1300.0, 700.0, 40.0, 500.0,
        mats["dark_metal"], no_coll=True)
    sphere("C09_desklamp_bulb", gdx + 500.0, -1300.0, 960.0, 90.0, mats["bulb"],
           no_coll=True)
    light("C09_desklamp_glow", gdx + 500.0, -1300.0, 960.0, (255, 235, 200),
          1200.0, 1400.0)
    # Weapons rack against the wall (vertical bars = batons/rifles).
    box("C09_weaprack", cx + 2400.0, WALL_INNER, 1100.0, 2200.0, 60.0, 1400.0,
        mats["dark_metal"], no_coll=True)
    for w in range(8):
        cyl(f"C09_weapon{w}", cx + 1500.0 + w * 240.0, WALL_INNER - 60.0,
            1100.0, 70.0, 1100.0, mats["dark_metal"], no_coll=True)
    # Warning-stripe panels on the barricades / walls.
    for i, sy in enumerate((-1200.0, 0.0, 1200.0)):
        box(f"C09_stripes{i}", cx - 50.0, sy, 800.0, 20.0, 900.0, 280.0,
            mats["stripes"], no_coll=True)
    for s in (1.0, -1.0):
        box(f"C09_wallstripe{int(s)}", cx + 4400.0, s * WALL_INNER, 1300.0,
            1400.0, 20.0, 400.0, mats["stripes"], no_coll=True)
    # Harsh checkpoint spotlight + a second cold floodlight (jackboot grammar).
    light("C09_spotlight", cx + 1000.0, 0.0, CAR_HEIGHT - 200.0,
          (255, 250, 240), 5000.0, 5500.0)
    light("C09_floodlight", cx + 4200.0, 0.0, CAR_HEIGHT - 300.0,
          (230, 235, 255), 2600.0, 4500.0)


def car10_dark_car(mats):
    """DARK CAR: minimal -- overturned furniture, debris piles, a few cold-blue
    shafts of light, cobweb-tarps, an eerie emptiness.  Deliberately sparse.
    (No train_feel skeleton here; the void is the point.)"""
    cx = car_x(10)
    # Overturned furniture / silhouette hulks (tilted, near walls).
    for j in range(5):
        s = rng.uniform(450, 800)
        box(f"C10_hulk{j}", cx + rng.uniform(-4500, 4500),
            rng.choice([-1, 1]) * rng.uniform(1100, 1750), s * 0.4,
            s, s * rng.uniform(0.5, 1.0), s * 0.85,
            mats["dark_cloth"] if j % 2 else mats["dark_metal"],
            yaw=rng.uniform(0, 360), roll=rng.uniform(15, 55))
    # Debris piles (low, scattered).
    for j in range(4):
        s = rng.uniform(260, 460)
        box(f"C10_debris{j}", cx + rng.uniform(-4000, 4000),
            rng.choice([-1, 1]) * rng.uniform(1200, 1800), s * 0.3,
            s, s * rng.uniform(0.7, 1.1), s * 0.5, mats["dark_metal"],
            yaw=rng.uniform(0, 360))
    # Cobweb tarps strung overhead (no collision, sagging).
    for j in range(3):
        box(f"C10_cobweb{j}", cx - 3000.0 + j * 3000.0,
            rng.uniform(-700, 700), CAR_HEIGHT - 120.0,
            rng.uniform(1800, 2800), rng.uniform(900, 1500), 18.0,
            mats["dark_cloth"], yaw=rng.uniform(-15, 15),
            roll=rng.uniform(-5, 5), no_coll=True)
    # A few cold-blue shafts of light only -- eerie, dim, deliberate.
    for k, dx in enumerate((-3500.0, 800.0, 4000.0)):
        light(f"C10_coldshaft{k}", cx + dx, rng.uniform(-500, 500),
              CAR_HEIGHT - 400.0, (90, 110, 150), 350.0, 2200.0)


def car11_freezer(mats):
    """FREEZER BREACH: ice formations (pale-blue shards/columns), frost-coated
    crates, a breached wall panel with cold-fog feel (dim blue lights),
    icicle clusters from the ceiling (no collision)."""
    cx = car_x(11)
    # Ice columns / formations along the walls.
    ice_spots = [(-4200.0, 1.0), (-2600.0, -1.0), (-900.0, 1.0),
                 (700.0, -1.0), (2400.0, 1.0), (3900.0, -1.0)]
    for i, (dx, s) in enumerate(ice_spots):
        h = rng.uniform(900, 1700)
        cyl(f"C11_icecol{i}", cx + dx, s * (WALL_INNER - 200.0), h * 0.5,
            rng.uniform(350, 600), h, mats["ice"], yaw=rng.uniform(0, 360))
        # a jagged shard splitting off the top
        cone(f"C11_iceshard{i}", cx + dx + rng.uniform(-200, 200),
             s * (WALL_INNER - 200.0), h, rng.uniform(200, 360),
             rng.uniform(400, 700), mats["frost"], no_coll=True,
             roll=rng.uniform(-20, 20))
    # Icicle clusters hanging from the ceiling (cones, no collision).
    for j in range(10):
        ix = cx - 4500.0 + j * 1000.0
        iy = rng.uniform(-1500, 1500)
        h = rng.uniform(350, 800)
        cone(f"C11_icicle{j}", ix, iy, CAR_HEIGHT - h * 0.5, rng.uniform(90, 180),
             h, mats["frost"], pitch=180.0, no_coll=True)
    # Frost-coated crates (frost overcube over a crate).
    for j in range(4):
        ckx = cx + rng.uniform(-3500, 3500)
        cky = rng.choice([-1, 1]) * rng.uniform(1200, 1700)
        box(f"C11_frostcrate{j}", ckx, cky, 260.0, 520.0, 520.0, 520.0,
            mats["crate"])
        box(f"C11_frostcoat{j}", ckx, cky, 270.0, 560.0, 560.0, 540.0,
            mats["frost"], no_coll=True)
    # Breached wall panel: a torn-open dark plate with cold light pouring in.
    bx = cx + 1200.0
    box("C11_breach_frame", bx, WALL_INNER, 1500.0, 1400.0, 60.0, 1700.0,
        mats["dark_metal"], no_coll=True)
    box("C11_breach_gap", bx, WALL_INNER - 40.0, 1500.0, 900.0, 40.0, 1200.0,
        mats["frost"], no_coll=True, roll=8.0)
    for j in range(4):  # ice rubble spilling from the breach onto the floor
        s = rng.uniform(300, 520)
        box(f"C11_rubble{j}", bx + rng.uniform(-700, 700),
            (WALL_INNER - 500.0) + rng.uniform(-200, 200), s * 0.4,
            s, s * rng.uniform(0.6, 1.0), s * 0.7, mats["ice"],
            yaw=rng.uniform(0, 360))
    # Dim cold-blue lighting (fog feel via low, broad blue lights).
    light("C11_breachglow", bx, 1000.0, 1400.0, (150, 195, 255), 1400.0, 3000.0)
    for k, dx in enumerate((-3500.0, 0.0, 3500.0)):
        light(f"C11_coldwash{k}", cx + dx, rng.choice([-700.0, 700.0]), 2200.0,
              (150, 190, 255), 1600.0, 3200.0)


def car12_kronole_den(mats):
    """KRONOLE DEN: low mattresses/cushions, drug-paraphernalia clutter (small
    vials, pipes), hazy purple-green lighting, a dealer's table, hanging
    bead/cloth curtains."""
    cx = car_x(12)
    # Low mattresses + cushions along the walls.
    spots = [(-4000.0, 1.0), (-2400.0, -1.0), (-800.0, 1.0), (800.0, -1.0),
             (2800.0, 1.0)]
    for i, (dx, s) in enumerate(spots):
        box(f"C12_mattress{i}", cx + dx, s * 1300.0, 75.0, 1300.0, 700.0, 150.0,
            mats["fabric"], yaw=rng.uniform(-10, 10))
        box(f"C12_cushion{i}", cx + dx - 480.0, s * 1300.0, 190.0,
            340.0, 420.0, 110.0, mats["dark_cloth"], yaw=rng.uniform(0, 30),
            no_coll=True)
        # a glowing kronole vial + a pipe on each mattress
        cyl(f"C12_vial{i}", cx + dx + rng.uniform(-300, 300), s * 1300.0, 230.0,
            70.0, 160.0, mats["kronole_glow"], no_coll=True)
        cyl(f"C12_pipe{i}", cx + dx + rng.uniform(-300, 300),
            s * 1300.0 + rng.uniform(-150, 150), 200.0, 36.0, 360.0,
            mats["dark_metal"], pitch=80.0, no_coll=True)
    # Dealer's table near the front: low table + a brick of glowing kronole.
    dtx = cx + 3600.0
    box("C12_dealertable", dtx, 0.0, 220.0, 1100.0, 800.0, 80.0,
        mats["wood_worn"])
    for k in range(4):
        box(f"C12_brick{k}", dtx + rng.uniform(-350, 350),
            rng.uniform(-250, 250), 320.0, 150.0, 150.0, 100.0,
            mats["kronole_glow"], yaw=rng.uniform(0, 360), no_coll=True)
    # Scales on the dealer's table (brass pans on a beam).
    cyl("C12_scale_post", dtx, 0.0, 420.0, 50.0, 280.0, mats["brass"],
        no_coll=True)
    for s in (1.0, -1.0):
        box(f"C12_scale_pan{int(s)}", dtx + s * 260.0, 0.0, 540.0, 220.0, 220.0,
            30.0, mats["brass"], no_coll=True)
    # Hanging bead/cloth curtains partitioning the lounges (no collision).
    for j in range(4):
        cx_div = cx - 3000.0 + j * 2000.0
        for s in (1.0, -1.0):
            cloth_hang(f"C12_beadcurtain{j}_{int(s)}", cx_div, s * 1100.0,
                       1300.0, 900.0, 1900.0, mats, dark=(j % 2 == 0))
    # Dark cloth canopies sagging over the lounges.
    for j in range(3):
        box(f"C12_canopy{j}", cx - 2500.0 + j * 2500.0,
            rng.choice([-1100.0, 1100.0]), 2380.0, 1700.0, 1300.0, 24.0,
            mats["dark_cloth"], yaw=rng.uniform(-8, 8), roll=rng.uniform(-6, 6),
            no_coll=True)
    # Hazy purple-green lighting (alternating sickly hues).
    hues = [(150, 60, 175), (95, 175, 110), (150, 60, 175)]
    for k, dx in enumerate((-3000.0, 0.0, 3000.0)):
        light(f"C12_haze{k}", cx + dx, rng.choice([-900.0, 900.0]), 1900.0,
              hues[k], rng.uniform(700, 1000), 2600.0)


def car13_smugglers(mats):
    """SMUGGLER'S CACHE: stacked contraband crates, a hidden-compartment wall,
    a trading counter, hanging scales, locked chests, lantern light."""
    cx = car_x(13)
    # Stacked contraband crates along both walls (deterministic stack height
    # so the global actor budget is never overshot by a random draw).
    for i in range(4):
        bx = cx - 4000.0 + i * 2600.0
        for s in (1.0, -1.0):
            crate_stack(f"C13_crates{i}_{int(s)}", bx, s * 1450.0, mats,
                        n=2 + (i % 2))
    # Hidden-compartment wall: a wall panel ajar revealing crates behind.
    hx = cx - 1000.0
    box("C13_hidden_panel", hx, WALL_INNER, 1300.0, 1500.0, 50.0, 1800.0,
        mats["wood_grey"], no_coll=True, yaw=6.0)
    box("C13_hidden_stash", hx, WALL_INNER - 220.0, 600.0, 700.0, 360.0, 700.0,
        mats["brass"])
    # Trading counter near the front, with a merchant stool.
    tcx = cx + 3500.0
    box("C13_counter", tcx, -1100.0, 480.0, 1600.0, 800.0, 120.0,
        mats["wood_worn"])
    box("C13_counter_top", tcx, -1100.0, 560.0, 1700.0, 900.0, 50.0,
        mats["dark_metal"], no_coll=True)
    cyl("C13_merchant_stool", tcx, -450.0, 280.0, 360.0, 560.0, mats["wood_grey"])
    # Hanging scales above the counter (beam + two pans, no collision).
    cyl("C13_scalebeam", tcx, -1100.0, 1700.0, 40.0, 700.0, mats["brass"],
        pitch=90.0, no_coll=True)
    for s in (1.0, -1.0):
        cyl(f"C13_scale_chain{int(s)}", tcx + s * 320.0, -1100.0, 1450.0, 16.0,
            500.0, mats["dark_metal"], no_coll=True)
        cyl(f"C13_scale_pan{int(s)}", tcx + s * 320.0, -1100.0, 1180.0, 280.0,
            40.0, mats["brass"], no_coll=True)
    # Locked chests scattered near the cache (with a brass lock plate).
    for j in range(4):
        chx = cx + rng.uniform(-2500, 2500)
        chy = rng.choice([-1, 1]) * rng.uniform(1100, 1600)
        box(f"C13_chest{j}", chx, chy, 230.0, 640.0, 420.0, 420.0,
            mats["wood_worn"], yaw=rng.uniform(0, 360))
        box(f"C13_lock{j}", chx, chy, 280.0, 120.0, 80.0, 120.0, mats["brass"],
            no_coll=True)
    # Warm lantern light (smuggler's furtive glow).
    for k, dx in enumerate((-3000.0, 0.0, 3500.0)):
        light(f"C13_lantern{k}", cx + dx, rng.choice([-1000.0, 1000.0]), 1600.0,
              (255, 184, 110), rng.uniform(1000, 1300), 2200.0)


def car14_martyrs_gate(mats):
    """MARTYR'S GATE: a grand riveted bulkhead gate (layered dark slabs +
    rivet spheres), memorial banners, braziers flanking, a raised guard
    platform -- the imposing end of the Tail."""
    cx = car_x(14)
    face = cx + HALF_L   # inner face of the sealed front wall

    # Layered gate slabs (proud of the wall, increasingly massive).
    box("C14_gate_L1", face - 70.0, 0.0, 1400.0, 120.0, 3600.0, 2700.0,
        mats["dark_metal"], no_coll=True)
    box("C14_gate_L2", face - 170.0, 0.0, 1350.0, 90.0, 2800.0, 2300.0,
        mats["tail_wall"], no_coll=True)
    box("C14_gate_L3", face - 250.0, 0.0, 1300.0, 70.0, 1900.0, 1900.0,
        mats["dark_metal"], no_coll=True)
    # Horizontal strapping beams.
    for r, rz in enumerate((560.0, 1350.0, 2100.0)):
        box(f"C14_gate_beam{r}", face - 290.0, 0.0, rz, 60.0, 3100.0, 230.0,
            mats["dark_metal"], no_coll=True)
    # Flanking columns.
    for s in (1.0, -1.0):
        box(f"C14_gate_col{int(s)}", face - 130.0, s * 1560.0, 1400.0,
            160.0, 280.0, 2700.0, mats["dark_metal"], no_coll=True)
    # Rivet grid on the L2 face (spheres, 6 cols x 3 rows -- imposing but lean).
    for r, rz in enumerate((480.0, 1350.0, 2200.0)):
        for c in range(6):
            ry = -1100.0 + c * 440.0
            sphere(f"C14_rivet{r}_{c}", face - 296.0, ry, rz, 80.0,
                   mats["dark_metal"], no_coll=True)
    # A huge central riveted handwheel on the gate.
    cyl("C14_gatewheel", face - 320.0, 0.0, 1350.0, 1000.0, 90.0, mats["brass"],
        pitch=90.0, no_coll=True)
    for s in range(6):
        box(f"C14_gatewheel_spoke{s}", face - 320.0, 0.0, 1350.0, 920.0, 60.0,
            60.0, mats["brass"], pitch=90.0, yaw=s * 30.0, no_coll=True)
    # Memorial banners hung the length of the approach (tattered cloth).
    for j in range(3):
        bx = cx - 3800.0 + j * 2900.0
        for s in (1.0, -1.0):
            cloth_hang(f"C14_banner{j}_{int(s)}", bx, s * (WALL_INNER - 40.0),
                       1700.0, 1000.0, 1700.0, mats, along_x=True,
                       dark=(j % 2 == 0))
    # Braziers flanking the approach to the gate (warm, ominous).
    for tag, sy in (("C14_brazier_L", 1500.0), ("C14_brazier_R", -1500.0)):
        brazier(tag, cx + 4300.0, sy, mats, intensity=1600.0, radius=3200.0)
    # Raised guard platform overlooking the gate (deck + legs + rail).
    pxc, pyc = cx + 4400.0, 0.0
    for s in ((1, 1), (1, -1), (-1, 1), (-1, -1)):
        box(f"C14_platleg{s[0]}{s[1]}", pxc + s[0] * 700.0, pyc + s[1] * 900.0,
            700.0, 150.0, 150.0, 1400.0, mats["dark_metal"])
    box("C14_platdeck", pxc, pyc, 1440.0, 1700.0, 2100.0, 80.0,
        mats["dark_metal"])
    for s in (1.0, -1.0):
        box(f"C14_platrail{int(s)}", pxc, s * 1000.0, 1680.0, 1700.0, 40.0,
            380.0, mats["dark_metal"], no_coll=True)
    # Dramatic gate uplight + warm memorial wash.
    light("C14_gatelight", face - 600.0, 0.0, 600.0, (255, 180, 110), 2600.0,
          4000.0)
    light("C14_gateuplight", face - 900.0, 0.0, 2400.0, (255, 200, 150), 2200.0,
          3600.0)
    light("C14_memorialwash", cx - 1500.0, 0.0, 2200.0, (230, 190, 140), 1500.0,
          3600.0)


# --- Light reinforcement for cars 1-3 (only thin spots; prior passes own them).

def car01_reinforce(mats):
    """Tail Quarters A: a couple of market stalls + a water barrel + a
    laundry line.  Just enough to thicken any gaps left by FD_/TAIL_."""
    cx = car_x(1)
    # Two small market stalls against the walls.
    for j, s in ((0, 1.0), (1, -1.0)):
        sx = cx - 2500.0 + j * 5000.0
        box(f"C01_stall{j}", sx, s * 1450.0, 450.0, 1300.0, 700.0, 120.0,
            mats["wood_worn"])
        cloth_hang(f"C01_stallawning{j}", sx, s * 1100.0, 1300.0, 1300.0, 700.0,
                   mats, along_x=True, dark=(j == 0))
        crate_stack(f"C01_stallgoods{j}", sx + 600.0, s * 1500.0, mats, n=2,
                    base=380.0)
    # A water barrel + a communal cook-pot.
    cyl("C01_waterbarrel", cx + 3500.0, 1500.0, 450.0, 560.0, 900.0,
        mats["dark_metal"])
    cyl("C01_cookpot", cx - 3500.0, -1500.0, 350.0, 520.0, 500.0,
        mats["dark_metal"])
    cyl("C01_cookpot_fire", cx - 3500.0, -1500.0, 120.0, 420.0, 120.0,
        mats["fire_glow"], no_coll=True)
    light("C01_cookglow", cx - 3500.0, -1500.0, 700.0, (255, 150, 60), 900.0,
          1800.0)
    # A high laundry line across the car (no collision).
    for j in range(4):
        cloth_hang(f"C01_laundry{j}", cx - 3000.0 + j * 2000.0,
                   rng.uniform(-900, 900), 2500.0, rng.uniform(500, 760),
                   rng.uniform(340, 520), mats, along_x=True)


def car02_reinforce(mats):
    """Tail Quarters B: a couple more market stalls + water barrels +
    laundry, blended with the existing protein kitchen."""
    cx = car_x(2)
    for j, s in ((0, -1.0), (1, 1.0)):
        sx = cx - 3500.0 + j * 3000.0
        box(f"C02_stall{j}", sx, s * 1480.0, 430.0, 1100.0, 640.0, 110.0,
            mats["wood_worn"])
        crate_stack(f"C02_stallgoods{j}", sx + 500.0, s * 1500.0, mats, n=2,
                    base=360.0)
    for j in range(2):
        cyl(f"C02_waterbarrel{j}", cx - 4500.0 + j * 9000.0,
            rng.choice([-1, 1]) * 1500.0, 430.0, 540.0, 860.0,
            mats["dark_metal"])
    for j in range(4):
        cloth_hang(f"C02_laundry{j}", cx - 3000.0 + j * 2000.0,
                   rng.uniform(-900, 900), 2500.0, rng.uniform(480, 720),
                   rng.uniform(320, 500), mats, along_x=True, dark=(j % 2 == 0))


def car03_reinforce(mats):
    """The Pit: crowd barriers + a blood-stained ring center + torch posts.
    (FD_/TAIL_ already placed the rope ring & benches; this thickens the
    spectacle without blocking the lane.)"""
    cx = car_x(3)
    # Extra crowd barriers behind the existing benches (off-aisle).
    for i, dx in enumerate((-3800.0, -1300.0, 1300.0, 3800.0)):
        for s in (1.0, -1.0):
            box(f"C03_crowdbar{i}_{int(s)}", cx + dx, s * 1880.0, 480.0,
                1100.0, 70.0, 920.0, mats["dark_metal"],
                yaw=rng.uniform(-6, 6))
    # Blood-stained ring center (flat decal-like patches; set-piece, lane
    # exempt but flat + no collision so it never trips the player).
    for i in range(4):
        box(f"C03_bloodstain{i}", cx + rng.uniform(-1600, 1600),
            rng.uniform(-900, 900), 205.0, rng.uniform(500, 850),
            rng.uniform(420, 700), 6.0, mats["blood"], yaw=rng.uniform(0, 360),
            no_coll=True)
    # Torch posts at the ring corners (post + emissive flame + warm light).
    for i, (sx, sy) in enumerate(((1, 1), (1, -1), (-1, 1), (-1, -1))):
        tx, ty = cx + sx * 2700.0, sy * 1720.0
        cyl(f"C03_torchpost{i}", tx, ty, 850.0, 110.0, 1700.0, mats["dark_metal"])
        cyl(f"C03_torchhead{i}", tx, ty, 1720.0, 240.0, 200.0, mats["fire_glow"],
            no_coll=True)
        light(f"C03_torchglow{i}", tx, ty, 1850.0, (255, 150, 60),
              rng.uniform(900, 1200), 2200.0)


# ---------------------------------------------------------------------------
# Dispatch tables
# ---------------------------------------------------------------------------

PURPOSE_BUILDERS = {
    4: car04_nursery,
    5: car05_elders,
    6: car06_sickbay,
    7: car07_workshop,
    8: car08_listening_post,
    9: car09_blockade,
    10: car10_dark_car,
    11: car11_freezer,
    12: car12_kronole_den,
    13: car13_smugglers,
    14: car14_martyrs_gate,
}

REINFORCE_BUILDERS = {
    1: car01_reinforce,
    2: car02_reinforce,
    3: car03_reinforce,
}

CAR_NAMES = {
    1: "Tail Quarters A (reinforce)", 2: "Tail Quarters B (reinforce)",
    3: "The Pit (reinforce)", 4: "Nursery", 5: "Elders", 6: "Sickbay",
    7: "Workshop", 8: "Listening Post", 9: "Blockade", 10: "Dark Car",
    11: "Freezer Breach", 12: "Kronole Den", 13: "Smuggler's Cache",
    14: "Martyr's Gate",
}


# ---------------------------------------------------------------------------
# Cleanup / load / save
# ---------------------------------------------------------------------------

def cleanup():
    unreal.log("=" * 64)
    unreal.log("0. CLEANUP -- removing previous TI_ actors")
    unreal.log("=" * 64)
    removed = 0
    try:
        actors = level_lib.get_all_level_actors()
    except Exception as e:
        unreal.log_error(f"  Could not enumerate actors: {e}")
        return
    for actor in actors:
        if isinstance(actor, unreal.WorldSettings):
            continue
        try:
            if actor.get_actor_label().startswith(PREFIX):
                level_lib.destroy_actor(actor)
                removed += 1
        except Exception:
            continue
    unreal.log(f"  Removed {removed} TI_ actors")


def load_zone1():
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        world = level_lib.get_editor_world()
        if world and "Zone1_Tail" in world.get_path_name():
            return True
        if not editor_util.does_asset_exist(PERSISTENT_MAP):
            unreal.log_error(
                f"  Map not found: {PERSISTENT_MAP} -- run build_zone1.py first")
            return False
        unreal.log(f"  Loading {PERSISTENT_MAP} ...")
        les.load_level(PERSISTENT_MAP)
        return True
    except Exception as e:
        unreal.log_error(f"  Could not load level: {e}")
        return False


def save_everything():
    unreal.log("=" * 64)
    unreal.log("SAVE")
    unreal.log("=" * 64)
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        les.save_all_dirty_levels()
        # Belt-and-suspenders: the dirty flag can be lost on headless
        # property-only mutations -- force the current level too.
        try:
            les.save_current_level()
        except Exception:
            pass
        unreal.log("  save_all_dirty_levels() + save_current_level() done")
    except Exception as e:
        unreal.log_warning(f"  Level save failed: {e}")
    try:
        if editor_util.does_directory_exist(MAT_TI_DIR):
            editor_util.save_directory(MAT_TI_DIR, only_if_is_dirty=True,
                                       recursive=True)
    except Exception as e:
        unreal.log_warning(f"  save_directory failed for {MAT_TI_DIR}: {e}")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def run():
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  ZONE 1 TRAIN INTERIOR PASS  |  prefix TI_  |  budget 900")
    unreal.log("=" * 64)
    unreal.log("")

    if not load_zone1():
        return

    cleanup()
    mats = load_palette()

    unreal.log("=" * 64)
    unreal.log("DRESSING -- purpose builders (4-14) + train feel + reinforce 1-3")
    unreal.log("=" * 64)

    # Cars 4-14: purpose builders + general train-feel skeleton.
    for idx in PURPOSE_CARS:
        before = _total_new
        try:
            PURPOSE_BUILDERS[idx](mats)
        except Exception as e:
            unreal.log_error(f"  Car {idx:02d} purpose builder failed: {e}")
        # Dark Car (10) is deliberately a void -- no structural skeleton.
        if idx != 10:
            try:
                train_feel(idx, mats)
            except Exception as e:
                unreal.log_error(f"  Car {idx:02d} train_feel failed: {e}")
        else:
            # Minimal cold structural hint only: two long ceiling pipes, no ribs.
            try:
                train_feel(idx, mats, ribs=False, full_pipes=True)
            except Exception as e:
                unreal.log_error(f"  Car 10 minimal train_feel failed: {e}")
        unreal.log(f"  Car {idx:02d} {CAR_NAMES[idx]:30s} +{_total_new - before} actors")

    # Cars 1-3: light reinforcement only (no train_feel; prior passes own them).
    for idx in LIGHT_REINFORCE_CARS:
        before = _total_new
        try:
            REINFORCE_BUILDERS[idx](mats)
        except Exception as e:
            unreal.log_error(f"  Car {idx:02d} reinforce failed: {e}")
        unreal.log(f"  Car {idx:02d} {CAR_NAMES[idx]:30s} +{_total_new - before} actors")

    save_everything()

    # --- Summary ---
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  TRAIN INTERIOR PASS COMPLETE")
    unreal.log("=" * 64)
    unreal.log(f"  Total TI_ actors placed: {_total_new} / {MAX_ACTORS} budget")
    unreal.log("  Per-car actor counts:")
    for idx in range(1, 15):
        if _car_counts[idx]:
            unreal.log(f"    [{idx:02d}] {CAR_NAMES.get(idx, ''):30s} "
                       f"{_car_counts[idx]:4d}")
    unreal.log("")
    new_mats = [n for n in _materials_used if n.endswith("(new)")]
    reused = [n for n in _materials_used if not n.endswith("(new)")]
    unreal.log(f"  New materials created ({len(new_mats)}):")
    for n in new_mats:
        unreal.log(f"    {n}")
    unreal.log(f"  Materials reused ({len(reused)}):")
    for n in reused:
        unreal.log(f"    {n}")
    unreal.log("")
    unreal.log(f"  Corridor lane |y| < {AISLE_HALF:.0f} kept clear of collision props.")
    unreal.log("  Thin/overhead/hanging/decorative props: collision OFF.")
    unreal.log("  Dark Car (10) left deliberately sparse (no rib skeleton).")
    unreal.log("")


run()
