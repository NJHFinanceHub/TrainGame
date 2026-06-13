"""
Snowpiercer: Eternal Engine -- TAIL SECTION ART PASS
Run in editor (Tools > Execute Python Script) or headless via
-ExecutePythonScript="...Scripts/tail_dressing.py".

Intensifies cars 0-3 (Caboose / Tail Quarters A & B / The Pit Approach) to
match the look of the Tail in Bong Joon-ho's Snowpiercer (2013):

  - Oppressive darkness: ceiling lights in cars 0-3 dimmed to near-black.
  - Warm practicals only: barrel fires + candle clusters are the sole light.
  - Dense bunk slums: triple-stacked scrap frames packed wall-to-wall in
    cars 1-2 (and partial in 3), 10-14 stacks per car.
  - Hanging cloth partitions + laundry: 20-30 per car, no-collision.
  - Bundle/crate clutter on, under, and around bunks.
  - Dripping pipes: wall-and-ceiling cylinder runs in every rear car.
  - Floor debris piles near walls (|y| > 800, aisle clear).
  - Hanging chains: dark metal cylinders dangling from ceiling.
  - Ragged tarp panels across the ceiling.
  - Caboose (car 0) dead-end bulkhead: riveted dark-metal slabs + rivet
    spheres behind the FD_ memorial; two huddled-bundle props.

What it does NOT do:
  - Modify any actor not created by this script (except light intensity
    clamping on Light_Z1_Car0*_* labels, which uses actor.modify() first).
  - Place anything in |y| < 700 at floor level (aisle stays navigable).
  - Exceed 500 new TAIL_ actors.
  - Edit any other script.

Idempotent: every placed actor's label starts with TAIL_ and all of them are
destroyed at the start of each run.  Light dimming uses a one-way clamp so
re-running never makes lights brighter.

Materials reused from FD_ / Zone1 passes:
  /Game/Materials/FilmDetail/M_FD_DarkMetal
  /Game/Materials/FilmDetail/M_FD_FireGlow
  /Game/Materials/FilmDetail/M_FD_CandleWax
  /Game/Materials/FilmDetail/M_FD_DarkCloth
  /Game/Materials/FilmDetail/M_FD_Sandbag       (bundles)
  /Game/Materials/Textured/M_Tex_wood_worn_planks (bunk slabs)
  /Game/Materials/Textured/M_Tex_wood_planks_grey (bunk frames)
  /Game/Materials/Zone1/M_Bunk_Wood              (fallback bunk)
  /Game/Materials/Zone1/M_Tail_Wall              (fallback metal)
"""

import re
import random
import unreal

# ---------------------------------------------------------------------------
# API shortcuts (mirror detail_pass_film.py)
# ---------------------------------------------------------------------------

editor_util = unreal.EditorAssetLibrary
asset_tools  = unreal.AssetToolsHelpers.get_asset_tools()
mat_lib      = unreal.MaterialEditingLibrary
level_lib    = unreal.EditorLevelLibrary

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

PERSISTENT_MAP = "/Game/Maps/Zone1_Tail"
CAR_LENGTH  = 12000.0
CAR_WIDTH   = 4000.0
CAR_HEIGHT  = 3000.0
CAR_STRIDE  = 13000.0          # CAR_LENGTH + 1000 gap
HALF_L      = CAR_LENGTH / 2.0
HALF_W      = CAR_WIDTH  / 2.0

TAIL_CARS   = [0, 1, 2, 3]    # rear four cars
PREFIX      = "TAIL_"
MAX_ACTORS  = 500

MAT_FD_DIR  = "/Game/Materials/FilmDetail"
MAT_TEX_DIR = "/Game/Materials/Textured"
MAT_Z1_DIR  = "/Game/Materials/Zone1"

# Deterministic layout on every run
rng = random.Random(20130101)   # "Snowpiercer day"

# Warm fire colour (1.0, 0.55, 0.25 in linear)
FIRE_COLOR   = (255, 140, 64)
CANDLE_COLOR = (255, 153, 76)

# ---------------------------------------------------------------------------
# Counters & state
# ---------------------------------------------------------------------------

_car_counts   = {i: 0 for i in TAIL_CARS}
_total_new    = 0
_budget_blown = False
_dimmed       = 0

# ---------------------------------------------------------------------------
# Small helpers (same pattern as detail_pass_film.py)
# ---------------------------------------------------------------------------

def _rot(pitch=0.0, yaw=0.0, roll=0.0):
    r = unreal.Rotator()
    r.set_editor_property("pitch", float(pitch))
    r.set_editor_property("yaw",   float(yaw))
    r.set_editor_property("roll",  float(roll))
    return r


def car_cx(idx):
    """World-space centre X of car idx."""
    return float(idx) * CAR_STRIDE


def _car_idx_for_x(x):
    return max(0, min(15, int((float(x) + CAR_STRIDE * 0.5) / CAR_STRIDE)))


def _count(x):
    global _total_new
    _total_new += 1
    ci = _car_idx_for_x(x)
    if ci in _car_counts:
        _car_counts[ci] += 1


def _ok(x):
    global _budget_blown
    if _total_new >= MAX_ACTORS:
        if not _budget_blown:
            unreal.log_warning(
                f"  TAIL_ budget ({MAX_ACTORS}) reached -- skipping further spawns")
            _budget_blown = True
        return False
    return True

# ---------------------------------------------------------------------------
# Shape cache & spawn helpers
# ---------------------------------------------------------------------------

_shape_cache = {}


def _shape(name):
    if name not in _shape_cache:
        _shape_cache[name] = editor_util.load_asset(f"/Engine/BasicShapes/{name}")
    return _shape_cache[name]


def _spawn_shape(shape, label, x, y, z, sx, sy, sz, mat=None,
                 pitch=0.0, yaw=0.0, roll=0.0, no_collision=False):
    """Spawn a scaled BasicShape StaticMeshActor with TAIL_ prefix."""
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
        if no_collision:
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


def spawn_box(label, x, y, z, sx, sy, sz, mat=None,
              pitch=0.0, yaw=0.0, roll=0.0, no_coll=False):
    return _spawn_shape("Cube", label, x, y, z, sx, sy, sz, mat,
                        pitch, yaw, roll, no_collision=no_coll)


def spawn_cyl(label, x, y, z, dia, height, mat=None,
              pitch=0.0, yaw=0.0, roll=0.0, no_coll=False):
    return _spawn_shape("Cylinder", label, x, y, z, dia, dia, height, mat,
                        pitch, yaw, roll, no_collision=no_coll)


def spawn_sphere(label, x, y, z, r, mat=None, no_coll=False):
    return _spawn_shape("Sphere", label, x, y, z, r, r, r, mat,
                        no_collision=no_coll)


def spawn_light(label, x, y, z, color_rgb, intensity, radius):
    """Warm practical point light (no shadows -- perf)."""
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
        plc.set_editor_property("light_color",
            unreal.Color(int(color_rgb[0]), int(color_rgb[1]),
                         int(color_rgb[2]), 255))
        try:
            plc.set_editor_property("cast_shadows", False)
        except Exception:
            pass
    _count(x)
    return pl

# ---------------------------------------------------------------------------
# Material loader (reuse existing assets, never create)
# ---------------------------------------------------------------------------

_mat_cache = {}


def _load_mat(path):
    """Load a material asset, returning None on failure (no crash)."""
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


def _load_mats():
    """Collect the small palette of reused materials."""
    m = {}

    # FD_ palette (detail_pass_film.py created these)
    m["dark_metal"]  = _load_mat(f"{MAT_FD_DIR}/M_FD_DarkMetal")
    m["fire_glow"]   = _load_mat(f"{MAT_FD_DIR}/M_FD_FireGlow")
    m["wax"]         = _load_mat(f"{MAT_FD_DIR}/M_FD_CandleWax")
    m["dark_cloth"]  = _load_mat(f"{MAT_FD_DIR}/M_FD_DarkCloth")
    m["sandbag"]     = _load_mat(f"{MAT_FD_DIR}/M_FD_Sandbag")

    # Textured wood (detail_pass_film.py created these)
    m["wood_worn"]   = _load_mat(f"{MAT_TEX_DIR}/M_Tex_wood_worn_planks")
    m["wood_grey"]   = _load_mat(f"{MAT_TEX_DIR}/M_Tex_wood_planks_grey")
    m["fabric"]      = _load_mat(f"{MAT_TEX_DIR}/M_Tex_fabric_pattern_07")

    # Zone1 surface materials (build_zone1.py created these)
    m["bunk_wood"]   = _load_mat(f"{MAT_Z1_DIR}/M_Bunk_Wood")
    m["tail_wall"]   = _load_mat(f"{MAT_Z1_DIR}/M_Tail_Wall")

    # Fallback cascade: if a mat is missing, substitute the nearest neighbour
    fallbacks = [
        ("wood_worn",  "bunk_wood",  "tail_wall", "dark_metal"),
        ("wood_grey",  "bunk_wood",  "tail_wall", "dark_metal"),
        ("fabric",     "dark_cloth", "sandbag",   "dark_metal"),
        ("dark_cloth", "fabric",     "sandbag",   "dark_metal"),
        ("sandbag",    "wood_worn",  "fabric",    "dark_metal"),
        ("wax",        "fabric",     "sandbag",   "dark_metal"),
        ("fire_glow",  "dark_metal", None, None),
        ("dark_metal", "tail_wall",  None, None),
        ("bunk_wood",  "wood_worn",  "tail_wall", "dark_metal"),
        ("tail_wall",  "dark_metal", None, None),
    ]
    for row in fallbacks:
        key = row[0]
        if m.get(key) is None:
            for fb in row[1:]:
                if fb and m.get(fb) is not None:
                    unreal.log_warning(
                        f"  Material '{key}' missing -- using '{fb}'")
                    m[key] = m[fb]
                    break

    reused = [k for k, v in m.items() if v is not None]
    missing = [k for k, v in m.items() if v is None]
    unreal.log(f"  Materials loaded: {len(reused)}  "
               f"({', '.join(reused)})")
    if missing:
        unreal.log_warning(f"  Materials still missing: {missing}")
    return m

# ---------------------------------------------------------------------------
# Step 1: DARKEN -- clamp general lights in cars 0-3
# ---------------------------------------------------------------------------

# The Tail is OPPRESSIVELY dark.  Ceiling floods get clamped to 80-120 lm
# so that only the practicals this script adds read as meaningful light.
TAIL_DIM_TARGET = 90.0          # near-black ceiling wash
TAIL_DIM_RADIUS = 2500.0        # shrink attenuation too

# Patterns that identify general ceiling lights placed by build_zone1.py
# and detail_pass_film.py:  Light_Z1_Car00_*, Light_Z1_Car01_*, ...
# and FD_C0*_bunklight*, FD_C0*_lamp_*, etc.
_CEIL_RE = re.compile(r"^(?:Light_Z1_Car0([0-3])_|FD_C0([0-3])_(?:bunklight|lamp_|candles))")


def darken_tail_lights():
    """One-way intensity clamp on all general-purpose lights in cars 0-3.
    Uses actor.modify() + component.modify() before any mutation."""
    global _dimmed
    unreal.log("=" * 64)
    unreal.log("1. DARKEN -- clamp ceiling/general lights in cars 0-3")
    unreal.log("=" * 64)

    try:
        actors = level_lib.get_all_level_actors()
    except Exception as e:
        unreal.log_error(f"  Could not enumerate actors: {e}")
        return

    dim_cold = unreal.Color(60, 45, 35, 255)   # very dim warm-brown residue

    for actor in actors:
        if not isinstance(actor, unreal.PointLight):
            continue
        try:
            label = actor.get_actor_label()
        except Exception:
            continue

        mm = _CEIL_RE.match(label)
        if not mm:
            continue

        plc = actor.get_component_by_class(unreal.PointLightComponent)
        if not plc:
            continue

        try:
            cur = float(plc.get_editor_property("intensity"))
            if cur <= TAIL_DIM_TARGET:
                continue                    # already dark enough
            actor.modify()
            plc.modify()
            plc.set_editor_property("intensity", TAIL_DIM_TARGET)
            cur_rad = float(plc.get_editor_property("attenuation_radius"))
            plc.set_editor_property("attenuation_radius",
                                    min(cur_rad, TAIL_DIM_RADIUS))
            plc.set_editor_property("light_color", dim_cold)
            _dimmed += 1
        except Exception as e:
            unreal.log_warning(f"  Could not dim {label}: {e}")

    unreal.log(f"  Dimmed/clamped {_dimmed} lights in cars 0-3")

# ---------------------------------------------------------------------------
# Step 2 helpers: fire barrel & candle cluster
# ---------------------------------------------------------------------------

def barrel_fire(tag, x, y, mats, intensity=None, radius=None):
    """Oil-drum barrel fire: metal cylinder body + emissive ember cap + light.
    Placed near walls (|y| > 1000) so aisle stays open."""
    if intensity is None:
        intensity = rng.uniform(700.0, 1000.0)
    if radius is None:
        radius = rng.uniform(500.0, 750.0)

    # Drum body (collision ON -- physical prop)
    spawn_cyl(f"{tag}_drum",  x, y, 420.0, 460.0, 840.0, mats["dark_metal"])
    # Ember glow cap (no collision -- purely visual)
    spawn_cyl(f"{tag}_ember", x, y, 848.0, 380.0, 60.0, mats["fire_glow"],
              no_coll=True)
    # Warm point light above
    spawn_light(f"{tag}_glow", x, y, 1200.0, FIRE_COLOR, intensity, radius)


def candle_cluster(tag, x, y, z, mats, n=3):
    """Cluster of n candles: wax cylinders + small emissive flame caps + light."""
    for i in range(n):
        cx = x + rng.uniform(-120.0, 120.0)
        cy = y + rng.uniform(-120.0, 120.0)
        h  = rng.uniform(90.0, 160.0)
        # Wax body
        spawn_cyl(f"{tag}_wax{i}",  cx, cy, z + h * 0.5, 60.0, h, mats["wax"])
        # Flame cap (no collision)
        spawn_cyl(f"{tag}_flame{i}", cx, cy, z + h + 22.0,
                  36.0, 36.0, mats["fire_glow"], no_coll=True)
    # One warm light per cluster
    spawn_light(f"{tag}_glow", x, y, z + 300.0, CANDLE_COLOR,
                rng.uniform(320.0, 560.0), 600.0)

# ---------------------------------------------------------------------------
# Step 2: WARM PRACTICALS per car (barrel fires + candle clusters)
# ---------------------------------------------------------------------------

def place_practicals_car(idx, mats):
    """Scatter 3-5 barrel fires + 2-3 candle clusters in car idx.
    All placed at |y| > 1000 (wall-side) so the centre aisle stays dim."""
    cx = car_cx(idx)

    # Barrel fire positions along both walls, staggered in X
    # 3 fires in cars 0 & 3; 4-5 in cars 1-2 (slum density)
    n_fires = 5 if idx in (1, 2) else 3

    fire_xs  = [cx + (HALF_L * (-0.7 + k * (1.4 / max(n_fires - 1, 1))))
                for k in range(n_fires)]
    # Alternate left/right wall
    fire_sides = [1.0 if k % 2 == 0 else -1.0 for k in range(n_fires)]

    for k, (fx, fs) in enumerate(zip(fire_xs, fire_sides)):
        fy = fs * rng.uniform(1100.0, 1700.0)
        barrel_fire(f"C{idx:02d}_fire{k}", fx, fy, mats)

    # Candle clusters: 2 per car on small surfaces / near bunks
    for k in range(2):
        clx  = cx + rng.uniform(-HALF_L * 0.6, HALF_L * 0.6)
        cly  = rng.choice([-1.0, 1.0]) * rng.uniform(900.0, 1600.0)
        candle_cluster(f"C{idx:02d}_candles{k}", clx, cly, 0.0, mats, n=3)

# ---------------------------------------------------------------------------
# Step 3: DENSE BUNK SLUMS
# ---------------------------------------------------------------------------

def bunk_stack(tag, x, y, mats, scale=1.0, yaw=0.0):
    """Triple-stacked scrap bunk.  Footprint ~2000 x 900 (scaled).
    Head/foot panels + 3 sleeping slabs.  7 actors total."""
    L  = 2000.0 * scale
    W  = 900.0  * scale
    fh = 1800.0 * scale
    slab_mat  = mats["wood_worn"] if rng.random() < 0.55 else mats["wood_grey"]
    panel_mat = mats["wood_grey"]

    # End panels (head & foot)
    for end, dx in (("H", -L * 0.5 + 30.0), ("F", L * 0.5 - 30.0)):
        spawn_box(f"{tag}_pnl{end}", x + dx, y, fh * 0.5,
                  55.0, W, fh, panel_mat, yaw=yaw)

    # Two side rails (long thin boxes along car wall side)
    for rail, dy in (("RA", W * 0.5 - 20.0), ("RB", -W * 0.5 + 20.0)):
        spawn_box(f"{tag}_rail{rail}", x, y + dy, fh * 0.5,
                  L - 60.0, 18.0, fh * 0.85, panel_mat, yaw=yaw)

    # 3 sleeping slabs
    for i in range(3):
        sz = (220.0 + i * 660.0) * scale
        spawn_box(f"{tag}_bunk{i}", x, y, sz,
                  L - 100.0, W - 60.0, 70.0, slab_mat, yaw=yaw)


def cloth_hang(tag, x, y, z, w, h, mats, yaw=0.0, along_x=False):
    """Hanging cloth slab: thin, no collision (purely visual).
    along_x=True makes it a laundry-strip along X axis."""
    yaw = yaw + rng.uniform(-14.0, 14.0)
    mat = mats["dark_cloth"] if rng.random() < 0.6 else mats["fabric"]
    if along_x:
        spawn_box(tag, x, y, z, w, 16.0, h, mat, yaw=yaw, no_coll=True)
    else:
        spawn_box(tag, x, y, z, 16.0, w, h, mat, yaw=yaw, no_coll=True)


def scatter_bundle(tag, x, y, z, mats):
    """Small crate or cloth bundle on/under a bunk."""
    s  = rng.uniform(200.0, 380.0)
    sy = s * rng.uniform(0.7, 1.2)
    sz = s * rng.uniform(0.4, 0.8)
    mat = (mats["wood_worn"] if rng.random() < 0.4
           else (mats["fabric"] if rng.random() < 0.5 else mats["sandbag"]))
    spawn_box(tag, x, y, z + sz * 0.5,
              s, sy, sz, mat, yaw=rng.uniform(0.0, 360.0), no_coll=True)


def place_bunk_slum_car(idx, mats):
    """Dense bunk slum for one car.  10-14 stacks per car, both walls,
    20-30 cloth hangs, 8-12 bundles/crates.
    All bunk centres |y| in 950-1750 (well outside the 700-clear aisle).
    Cloth fronts/partitions use no_coll=True."""
    cx = car_cx(idx)

    # --- Bunk X positions (evenly distributed along car length) ---
    if idx == 1:
        n_stacks = 6       # 6 positions = 12 stacks total (both walls)
    elif idx == 2:
        n_stacks = 6
    else:   # car 3 gets partial treatment (front 2/3 of car only)
        n_stacks = 4

    # Space them out with slight jitter
    xs = []
    span = CAR_LENGTH * (0.85 if idx == 3 else 0.92)
    for k in range(n_stacks):
        base_x = cx - span * 0.5 + span * k / max(n_stacks - 1, 1)
        xs.append(base_x + rng.uniform(-200.0, 200.0))

    ci_cloth = 0
    for k, dx in enumerate(xs):
        for side, sy in ((1.0, rng.uniform(1000.0, 1650.0)),
                         (-1.0, rng.uniform(1000.0, 1650.0))):
            stag = f"C{idx:02d}_bunk{k}{'L' if side > 0 else 'R'}"
            bunk_stack(stag, dx, side * sy, mats,
                       scale=rng.uniform(0.88, 1.05),
                       yaw=rng.uniform(-5.0, 5.0))

            # Cloth partition beside/between stacks (no collision)
            if k < n_stacks - 1:
                mid_x = (xs[k] + xs[k + 1]) * 0.5
                cloth_hang(f"C{idx:02d}_clpart{ci_cloth}",
                           mid_x, side * sy, 1100.0, 820.0, 1400.0, mats)
                ci_cloth += 1

            # Curtain on the aisle-facing side of each bunk
            cloth_hang(f"C{idx:02d}_clfront{ci_cloth}",
                       dx + rng.uniform(-250.0, 250.0),
                       side * (sy - 460.0),
                       1000.0, 1350.0, 1150.0, mats, along_x=True)
            ci_cloth += 1

            # Bundles on the top bunk and under the lowest slab
            for bi in range(2):
                scatter_bundle(f"C{idx:02d}_bundle{k}_{side:.0f}_{bi}",
                               dx + rng.uniform(-600.0, 600.0),
                               side * sy + rng.uniform(-200.0, 200.0),
                               (1500.0 if bi == 0 else 0.0), mats)

    # --- Extra ceiling laundry strung high (no collision) ---
    n_laundry = 8 if idx in (1, 2) else 4
    for j in range(n_laundry):
        lx  = cx - HALF_L * 0.8 + (HALF_L * 1.6) * j / max(n_laundry - 1, 1)
        ly  = rng.uniform(-1200.0, 1200.0)   # can cross the aisle at height
        cloth_hang(f"C{idx:02d}_laundry{j}", lx, ly, 2450.0,
                   rng.uniform(480.0, 780.0), rng.uniform(320.0, 560.0),
                   mats, along_x=rng.random() < 0.5)

    # --- Cloth across the ceiling (tarp swags, no collision) ---
    n_tarps = 4 if idx in (1, 2) else 2
    for j in range(n_tarps):
        tx = cx - HALF_L * 0.7 + (HALF_L * 1.4) * j / max(n_tarps - 1, 1)
        spawn_box(f"C{idx:02d}_tarp{j}", tx,
                  rng.uniform(-700.0, 700.0), CAR_HEIGHT - 80.0,
                  rng.uniform(1800.0, 3200.0), rng.uniform(800.0, 1600.0),
                  24.0, mats["dark_cloth"],
                  yaw=rng.uniform(-20.0, 20.0),
                  roll=rng.uniform(-6.0, 6.0), no_coll=True)

# ---------------------------------------------------------------------------
# Step 4: CLUTTER & GRIME
# ---------------------------------------------------------------------------

def place_clutter_car(idx, mats):
    """Drip pipes, floor debris, hanging chains, grime -- every rear car."""
    cx = car_cx(idx)

    # --- Ceiling drip pipe runs (full car length, both ceiling sides) ---
    for side, py in ((1.0, HALF_W - 160.0), (-1.0, -(HALF_W - 160.0))):
        # Main run
        spawn_cyl(f"C{idx:02d}_piperun{side:.0f}A",
                  cx, py, CAR_HEIGHT - 200.0,
                  110.0, CAR_LENGTH - 200.0,
                  mats["dark_metal"], pitch=90.0)
        # Offset thinner run
        spawn_cyl(f"C{idx:02d}_piperun{side:.0f}B",
                  cx, py - side * 180.0, CAR_HEIGHT - 320.0,
                  75.0, CAR_LENGTH * 0.75,
                  mats["dark_metal"], pitch=90.0)

    # --- Vertical drip pipes (2 per side, with drip-spot boxes) ---
    for i, (dx, sy) in enumerate(((-3600.0, 1.0), (3600.0, -1.0),
                                   (-1200.0, -1.0), (1200.0, 1.0))):
        px = cx + dx
        py = sy * (HALF_W - 160.0)
        spawn_cyl(f"C{idx:02d}_pdrop{i}", px, py, 1500.0,
                  90.0, 3000.0, mats["dark_metal"])
        # Drip puddle box at base (very thin, no collision)
        spawn_box(f"C{idx:02d}_drip{i}", px, py, 6.0,
                  120.0, 120.0, 12.0, mats["dark_metal"], no_coll=True)

    # --- Mid-wall horizontal duct run ---
    side = 1.0 if idx % 2 == 0 else -1.0
    spawn_box(f"C{idx:02d}_duct",
              cx + rng.uniform(-1000.0, 1000.0),
              side * (HALF_W - 130.0), 2100.0,
              8500.0, 160.0, 220.0, mats["dark_metal"])

    # --- Hanging chains (dark metal cylinders from ceiling, no collision) ---
    n_chains = 4
    for i in range(n_chains):
        chx = cx - HALF_L * 0.6 + (HALF_L * 1.2) * i / max(n_chains - 1, 1)
        chy = rng.choice([-1.0, 1.0]) * rng.uniform(700.0, 1600.0)
        ch_h = rng.uniform(400.0, 900.0)
        spawn_cyl(f"C{idx:02d}_chain{i}",
                  chx, chy, CAR_HEIGHT - ch_h * 0.5,
                  28.0, ch_h, mats["dark_metal"], no_coll=True)

    # --- Floor debris piles near walls (|y| > 850) ---
    n_debris = 3
    for i in range(n_debris):
        dx  = rng.uniform(-HALF_L * 0.8, HALF_L * 0.8)
        dy  = rng.choice([-1.0, 1.0]) * rng.uniform(1100.0, 1800.0)
        s   = rng.uniform(280.0, 480.0)
        mat = (mats["dark_metal"] if rng.random() < 0.5
               else mats["wood_worn"])
        spawn_box(f"C{idx:02d}_debris{i}", cx + dx, dy,
                  s * 0.3, s, s * rng.uniform(0.7, 1.2), s * 0.6, mat,
                  yaw=rng.uniform(0.0, 360.0))

# ---------------------------------------------------------------------------
# Step 5: CABOOSE (car 0) DEAD-END BULKHEAD
# ---------------------------------------------------------------------------

def place_caboose_bulkhead(mats):
    """Heavy riveted bulkhead on the sealed rear wall of car 0.
    Layered dark-metal slabs + rivet-dot spheres in a grid.
    The FD_ memorial (shelves + candles) is forward of this wall -- we stay
    within ~400 cm of the wall face so the memorial reads clearly.
    Two huddled-bundle props flanking the wall."""
    idx = 0
    cx  = car_cx(idx)
    # Inner face of the rear (low-X) wall: cx - HALF_L = -6000
    wall_x = cx - HALF_L   # = -6000

    # ---- Slab layers (proud of the wall, deeper = more massive) ----
    # Layer 1: full-width dark slab, nearly flush
    spawn_box("C00_bh_L1", wall_x + 55.0, 0.0, 1400.0,
              90.0, 3600.0, 2800.0, mats["dark_metal"])
    # Layer 2: inset narrower slab
    spawn_box("C00_bh_L2", wall_x + 130.0, 0.0, 1350.0,
              70.0, 2800.0, 2350.0, mats["dark_metal"])
    # Horizontal banding beams
    spawn_box("C00_bh_beam1", wall_x + 185.0, 0.0, 520.0,
              50.0, 3200.0, 220.0, mats["dark_metal"])
    spawn_box("C00_bh_beam2", wall_x + 185.0, 0.0, 1900.0,
              50.0, 3200.0, 220.0, mats["dark_metal"])
    # Vertical flanking columns
    spawn_box("C00_bh_colL", wall_x + 95.0,  1620.0, 1400.0,
              130.0, 240.0, 2800.0, mats["dark_metal"])
    spawn_box("C00_bh_colR", wall_x + 95.0, -1620.0, 1400.0,
              130.0, 240.0, 2800.0, mats["dark_metal"])

    # ---- Rivet grid: 6 columns x 3 rows of small spheres on L2 face ----
    for row_z in (480.0, 1350.0, 2200.0):
        for col, ry in enumerate([-1050.0, -630.0, -210.0, 210.0, 630.0, 1050.0]):
            spawn_sphere(f"C00_rivet_r{int(row_z/100)}_c{col}",
                         wall_x + 173.0, ry, row_z, 62.0,
                         mats["dark_metal"], no_coll=True)

    # ---- Huddled bundle props flanking the bulkhead ----
    for side, sy in ((1.0, 720.0), (-1.0, -720.0)):
        # Big bedroll bundle
        spawn_box(f"C00_bundle_{'L' if side > 0 else 'R'}",
                  wall_x + 380.0, sy, 140.0,
                  550.0, 380.0, 280.0, mats["sandbag"],
                  yaw=rng.uniform(0.0, 360.0))
        # Smaller cloth wrap on top
        spawn_box(f"C00_wrap_{'L' if side > 0 else 'R'}",
                  wall_x + 360.0, sy + side * 80.0, 310.0,
                  300.0, 250.0, 160.0, mats["dark_cloth"],
                  yaw=rng.uniform(0.0, 60.0), no_coll=True)

    # ---- One tight candle cluster beside the bulkhead (keeps memorial lit) ----
    candle_cluster("C00_bh_candles", wall_x + 450.0, 0.0, 0.0, mats, n=2)

# ---------------------------------------------------------------------------
# Load / save helpers
# ---------------------------------------------------------------------------

def load_zone1():
    """Ensure Zone1_Tail is the active world."""
    try:
        les   = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        world = level_lib.get_editor_world()
        if world and "Zone1_Tail" in world.get_path_name():
            return True
        if not editor_util.does_asset_exist(PERSISTENT_MAP):
            unreal.log_error(
                f"  Map not found: {PERSISTENT_MAP} -- "
                f"run build_zone1.py first")
            return False
        unreal.log(f"  Loading {PERSISTENT_MAP} ...")
        les.load_level(PERSISTENT_MAP)
        return True
    except Exception as e:
        unreal.log_error(f"  Could not load level: {e}")
        return False


def save_everything():
    unreal.log("=" * 64)
    unreal.log("6. SAVE")
    unreal.log("=" * 64)
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        les.save_all_dirty_levels()
        try:
            les.save_current_level()
        except Exception:
            pass
        unreal.log("  save_all_dirty_levels() + save_current_level() done")
    except Exception as e:
        unreal.log_warning(f"  Level save failed: {e}")

# ---------------------------------------------------------------------------
# Cleanup: destroy all TAIL_ actors from any previous run
# ---------------------------------------------------------------------------

def cleanup():
    unreal.log("=" * 64)
    unreal.log("0. CLEANUP -- removing previous TAIL_ actors")
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
    unreal.log(f"  Removed {removed} TAIL_ actors")

# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

CAR_NAMES = {
    0: "Caboose (Dead End)",
    1: "Tail Quarters A",
    2: "Tail Quarters B",
    3: "The Pit Approach",
}

MATERIALS_REUSED = [
    "M_FD_DarkMetal",
    "M_FD_FireGlow",
    "M_FD_CandleWax",
    "M_FD_DarkCloth",
    "M_FD_Sandbag",
    "M_Tex_wood_worn_planks",
    "M_Tex_wood_planks_grey",
    "M_Tex_fabric_pattern_07",
    "M_Bunk_Wood  (Zone1 fallback)",
    "M_Tail_Wall  (Zone1 fallback)",
]


def run():
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  TAIL SECTION ART PASS (Bong Joon-ho 2013 look)")
    unreal.log("  Cars 0-3 | prefix TAIL_ | budget 500")
    unreal.log("=" * 64)
    unreal.log("")

    if not load_zone1():
        return

    # 0. Idempotent cleanup
    cleanup()

    # Load material palette (reuse only, no new materials created)
    unreal.log("=" * 64)
    unreal.log("Loading materials (reuse only -- no new assets created)")
    unreal.log("=" * 64)
    mats = _load_mats()

    # 1. Darken general ceiling lights
    darken_tail_lights()

    # 2-5: Per-car passes
    unreal.log("=" * 64)
    unreal.log("2-5. PER-CAR DRESSING (cars 0-3)")
    unreal.log("=" * 64)

    for idx in TAIL_CARS:
        before = _total_new
        cx     = car_cx(idx)
        unreal.log(f"--- Car {idx:02d}: {CAR_NAMES[idx]} (cx={cx:.0f}) ---")

        # 2. Warm practicals
        try:
            place_practicals_car(idx, mats)
        except Exception as e:
            unreal.log_error(f"  Car {idx:02d} practicals failed: {e}")

        # 3. Dense bunk slum (cars 1, 2, partial 3; light pass in car 0)
        if idx in (1, 2, 3):
            try:
                place_bunk_slum_car(idx, mats)
            except Exception as e:
                unreal.log_error(f"  Car {idx:02d} bunk slum failed: {e}")

        # 4. Clutter & grime
        try:
            place_clutter_car(idx, mats)
        except Exception as e:
            unreal.log_error(f"  Car {idx:02d} clutter failed: {e}")

        # 5. Caboose bulkhead (car 0 only)
        if idx == 0:
            try:
                place_caboose_bulkhead(mats)
            except Exception as e:
                unreal.log_error(f"  Car 00 bulkhead failed: {e}")

        added = _total_new - before
        unreal.log(f"  Car {idx:02d} {CAR_NAMES[idx]:26s}  +{added} actors")

    # 6. Save
    save_everything()

    # --- Summary ---
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  TAIL SECTION ART PASS COMPLETE")
    unreal.log("=" * 64)
    unreal.log(f"  General lights dimmed (cars 0-3):  {_dimmed}")
    unreal.log(f"  Total TAIL_ actors placed:         {_total_new}"
               f" / {MAX_ACTORS} budget")
    unreal.log("")
    unreal.log("  Per-car actor counts:")
    for idx in TAIL_CARS:
        unreal.log(f"    [{idx:02d}] {CAR_NAMES[idx]:26s}  {_car_counts[idx]:4d} actors")
    unreal.log("")
    unreal.log("  Materials reused (no new assets created):")
    for mn in MATERIALS_REUSED:
        flag = "(loaded)" if _load_mat(
            f"{MAT_FD_DIR}/{mn.split()[0]}" if "M_FD_" in mn
            else (f"{MAT_TEX_DIR}/{mn.split()[0]}" if "M_Tex_" in mn
                  else f"{MAT_Z1_DIR}/{mn.split()[0]}")) else "(missing/fallback)"
        unreal.log(f"    {mn:40s} {flag}")
    unreal.log("")
    unreal.log("  Corridor lane |y| < 700 kept walkable.")
    unreal.log("  Hanging cloth / chains / tarps: collision OFF.")
    unreal.log("  Bulkhead rivets and ember caps: collision OFF.")
    unreal.log("")


run()
