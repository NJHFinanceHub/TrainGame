"""
Snowpiercer: Eternal Engine -- ZONE 1 TRAIN CEILING-LIGHT LAYER
Run in editor (Tools > Execute Python Script) or headless via
-ExecutePythonScript="...Scripts/train_ceiling_lights.py".

PLAYER COMPLAINT addressed (TWICE): "it's too dark -- make it more train light."

This pass adds a DEDICATED, BRIGHT, EVENLY-SPACED OVERHEAD TRAIN-LIGHT LAYER
running the full length of all 15 cars (plus a light over every gangway gap
between cars), so every car reads as a properly lit passenger/freight train at
night -- NO dark stretches, NO black transitions.

It is purely ADDITIVE on top of the existing per-car PRACTICALS placed by other
passes (train_interior_pass.py TI_, detail_pass_film.py FD_, tail_dressing.py
TAIL_, build_zone1.py Light_Z1_*) and the global tune in lighting_pass.py (LX_).
It does NOT add, remove, dim, move or duplicate any of those -- it only places
its own LIT_Ceiling_ actors.

GEOMETRY (confirmed from build_zone1.py / train_interior_pass.py -- NOT edited):
  CAR_LENGTH 12000, CAR_WIDTH 4000, CAR_HEIGHT 3000, WALL_THICK 150.
  CAR_STRIDE = CAR_LENGTH + CAR_GAP(1000) = 13000; car i center x = i*13000.
  Interior: floor underside at z=0, ceiling SLAB centered at
  CAR_HEIGHT + WALL_THICK/2 -> interior ceiling face is at z = CAR_HEIGHT = 3000.
  Gangway connector between car i and i+1 sits around x = i*13000 + 6500, with
  a lower connector ceiling at DOOR_HEIGHT = 2300.

DESIGN:
  * Fixture: unreal.PointLight (cheap, omnidirectional pools that overlap into
    continuous light -- robust + easy to read as a string of ceiling fixtures).
  * Per car: a row of fixtures down the CENTERLINE (y=0) plus, because the car
    is wide (4000), TWO offset rows at y = +/-900, so floor AND walls light.
  * Spacing: ~2660 cm along X (5 fixtures per row across the 12000 length),
    so pools overlap with NO dark gaps -- ~15 fixtures per car.
  * Gangway: one extra fixture over each Conn/gangway gap (14 gaps) so the
    transitions between cars are never black.
  * Ceiling Z: CEILING_Z = CAR_HEIGHT - 150 = 2850 (just below the 3000 ceiling
    face). Gangway lights sit lower at GANGWAY_Z = DOOR_HEIGHT - 150 = 2150.
  * Intensity: bright but not blown out -- centerline 6500 cd, side rows 5500
    cd, gangway 6000 cd (PointLight candelas).
  * Attenuation radius: 2200 cm so neighbouring pools overlap into a continuous
    wash floor-to-wall.
  * Colour: warm-neutral train light ~4200K (slightly warm white).
  * Shadows: OFF on all of these (cheap, even fill) -- prioritise BRIGHT + EVEN.

Idempotent: every placed actor's label starts with LIT_Ceiling_ and all of them
are destroyed at the start of each run, then recreated. Per-car spawns are
guarded so one car failing cannot abort the whole run. Ends with
save_all_dirty_levels() AND save_current_level() (modify-then-save; the dirty
flag can be lost on headless property-only mutations).

FILE OWNERSHIP: this is the only file this pass touches. It does NOT edit or
re-run any other script and does NOT mutate non-LIT_Ceiling_ actors.
"""

import unreal

# ---------------------------------------------------------------------------
# API shortcuts (mirror build_zone1.py / train_interior_pass.py / lighting_pass.py)
# ---------------------------------------------------------------------------

editor_util = unreal.EditorAssetLibrary
level_lib = unreal.EditorLevelLibrary           # deprecated-but-functional in 5.7

# ---------------------------------------------------------------------------
# Constants (mirror build_zone1.py geometry -- DO NOT diverge)
# ---------------------------------------------------------------------------

PERSISTENT_MAP = "/Game/Maps/Zone1_Tail"

CAR_LENGTH = 12000.0
CAR_WIDTH = 4000.0
CAR_HEIGHT = 3000.0
WALL_THICK = 150.0
DOOR_HEIGHT = 2300.0
CAR_GAP = 1000.0
CAR_STRIDE = 13000.0          # CAR_LENGTH + CAR_GAP; car i center x = i*13000
HALF_L = CAR_LENGTH / 2.0     # 6000
NUM_CARS = 15

PREFIX = "LIT_Ceiling_"

# --- Placement / tuning --------------------------------------------------
# Interior ceiling face is at z = CAR_HEIGHT (3000). Sit fixtures just below it.
CEILING_Z = CAR_HEIGHT - 150.0          # 2850 -- just under the ceiling
# Gangway connector ceiling is lower (DOOR_HEIGHT). Sit gangway lights below it.
GANGWAY_Z = DOOR_HEIGHT - 150.0         # 2150

ROW_OFFSET_Y = 900.0                     # +/- side rows for wide-car coverage
FIXTURES_PER_ROW = 5                     # 5 along the 12000 length -> ~2660 spacing
ATTEN_RADIUS = 2200.0                    # pools overlap into continuous light

# Warm-neutral train light ~4200K (slightly warm white).
COLOR_CENTER = (255, 236, 210)           # centerline -- brightest, warm-neutral
COLOR_SIDE = (255, 232, 202)             # side rows
COLOR_GANGWAY = (255, 230, 198)          # gangway -- slightly warmer

INTENSITY_CENTER = 6500.0                # cd
INTENSITY_SIDE = 5500.0                  # cd
INTENSITY_GANGWAY = 6000.0               # cd

CAST_SHADOWS = False                     # cheap, even bright fill

# ---------------------------------------------------------------------------
# Counters
# ---------------------------------------------------------------------------

_per_car = {i: 0 for i in range(NUM_CARS)}
_gangway_total = 0
_total = 0


def car_x(idx):
    return float(idx) * CAR_STRIDE


def _rot(pitch=0.0, yaw=0.0, roll=0.0):
    r = unreal.Rotator()
    r.set_editor_property("pitch", float(pitch))
    r.set_editor_property("yaw", float(yaw))
    r.set_editor_property("roll", float(roll))
    return r


def _all_actors():
    try:
        return level_lib.get_all_level_actors()
    except Exception as e:
        unreal.log_error(f"  Could not enumerate actors: {e}")
        return []


# ---------------------------------------------------------------------------
# 0. CLEANUP -- destroy everything from previous LIT_Ceiling_ runs (idempotent)
# ---------------------------------------------------------------------------

def cleanup():
    unreal.log("=" * 64)
    unreal.log("0. CLEANUP previous LIT_Ceiling_ actors")
    unreal.log("=" * 64)
    removed = 0
    for actor in _all_actors():
        if isinstance(actor, unreal.WorldSettings):
            continue
        try:
            if actor.get_actor_label().startswith(PREFIX):
                level_lib.destroy_actor(actor)
                removed += 1
        except Exception:
            continue
    unreal.log(f"  Removed {removed} LIT_Ceiling_ actors")


# ---------------------------------------------------------------------------
# Light spawn helper
# ---------------------------------------------------------------------------

def _place_ceiling_light(label, x, y, z, color_rgb, intensity,
                         radius=ATTEN_RADIUS):
    """Spawn one warm overhead PointLight (no shadows by default).
    Returns the actor or None.  Wrapped so a single failure is non-fatal."""
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
    try:
        pl.set_actor_label(f"{PREFIX}{label}")
        try:
            pl.modify()
        except Exception:
            pass
        plc = pl.get_component_by_class(unreal.PointLightComponent)
        if plc:
            try:
                plc.modify()
            except Exception:
                pass
            plc.set_editor_property("intensity", float(intensity))
            plc.set_editor_property("attenuation_radius", float(radius))
            plc.set_editor_property("light_color", unreal.Color(
                int(color_rgb[0]), int(color_rgb[1]), int(color_rgb[2]), 255))
            # Warm-neutral ~4200K train light.
            try:
                plc.set_editor_property("use_temperature", True)
                plc.set_editor_property("temperature", 4200.0)
            except Exception:
                pass
            try:
                plc.set_editor_property("cast_shadows", CAST_SHADOWS)
            except Exception:
                pass
            # Bigger source so the pool is soft + reads as a fixture, not a dot.
            try:
                plc.set_editor_property("source_radius", 60.0)
            except Exception:
                pass
    except Exception as e:
        unreal.log_warning(f"  light config failed ({label}): {e}")
    return pl


# ---------------------------------------------------------------------------
# Per-car overhead light layer (centerline + two side rows)
# ---------------------------------------------------------------------------

def build_car_ceiling(idx):
    """Place the overhead train-light layer for one car.  Guarded so a single
    car failure can't abort the whole run.  Returns count placed."""
    cx = car_x(idx)
    placed = 0
    try:
        x0 = cx - HALF_L
        step = CAR_LENGTH / float(FIXTURES_PER_ROW)   # 2400 spacing inside car
        rows = (
            (0.0, COLOR_CENTER, INTENSITY_CENTER, "C"),
            (ROW_OFFSET_Y, COLOR_SIDE, INTENSITY_SIDE, "L"),
            (-ROW_OFFSET_Y, COLOR_SIDE, INTENSITY_SIDE, "R"),
        )
        for ry, rcolor, rintensity, rtag in rows:
            for k in range(FIXTURES_PER_ROW):
                lx = x0 + (k + 0.5) * step       # evenly spaced, no edge gaps
                a = _place_ceiling_light(
                    f"Car{idx:02d}_{rtag}{k}", lx, ry, CEILING_Z,
                    rcolor, rintensity)
                if a is not None:
                    placed += 1
    except Exception as e:
        unreal.log_warning(f"  Car {idx:02d} ceiling-light build failed: {e}")

    _per_car[idx] = placed
    _total_add(placed)
    unreal.log(f"  Car {idx:02d}: placed {placed} ceiling lights "
               f"({FIXTURES_PER_ROW} x 3 rows)")
    return placed


def build_gangway_lights():
    """One bright light over every gangway gap between adjacent cars so the
    car-to-car transitions are never black.  14 gaps for 15 cars."""
    global _gangway_total
    unreal.log("=" * 64)
    unreal.log("GANGWAY lights -- one over every car-to-car gap")
    unreal.log("=" * 64)
    placed = 0
    for i in range(NUM_CARS - 1):
        # Midpoint between car i and car i+1 centers.
        gx = (car_x(i) + car_x(i + 1)) * 0.5
        a = _place_ceiling_light(
            f"Gangway{i:02d}", gx, 0.0, GANGWAY_Z,
            COLOR_GANGWAY, INTENSITY_GANGWAY, radius=ATTEN_RADIUS)
        if a is not None:
            placed += 1
    _gangway_total = placed
    _total_add(placed)
    unreal.log(f"  Placed {placed} gangway lights")
    return placed


def _total_add(n):
    global _total
    _total += n


# ---------------------------------------------------------------------------
# SAVE (modify-then-save; train_interior_pass.py / lighting_pass.py pattern)
# ---------------------------------------------------------------------------

def save_everything():
    unreal.log("=" * 64)
    unreal.log("SAVE")
    unreal.log("=" * 64)
    # Primary: LevelEditorSubsystem (matches lighting_pass.py).
    saved = False
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        les.save_all_dirty_levels()
        try:
            les.save_current_level()
        except Exception:
            pass
        saved = True
        unreal.log("  save_all_dirty_levels() + save_current_level() done")
    except Exception as e:
        unreal.log_warning(f"  LevelEditorSubsystem save failed: {e}")
    # Belt-and-suspenders: EditorLevelLibrary (matches build_zone1.py spec).
    try:
        unreal.EditorLevelLibrary.save_all_dirty_levels()
        try:
            unreal.EditorLevelLibrary.save_current_level()
        except Exception:
            pass
        saved = True
    except Exception as e:
        unreal.log_warning(f"  EditorLevelLibrary save failed: {e}")
    if not saved:
        unreal.log_warning("  WARNING: level may not have been saved")


# ---------------------------------------------------------------------------
# MAIN
# ---------------------------------------------------------------------------

def run():
    unreal.log("")
    unreal.log("*" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE -- CEILING TRAIN-LIGHT LAYER (LIT_Ceiling_)")
    unreal.log("*" * 64)

    # Make sure the persistent map is the active level.
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        cur = ""
        try:
            cur = unreal.EditorLevelLibrary.get_editor_world().get_name()
        except Exception:
            cur = ""
        if PERSISTENT_MAP.split("/")[-1] not in cur:
            unreal.log(f"  Loading {PERSISTENT_MAP} ...")
            les.load_level(PERSISTENT_MAP)
    except Exception as e:
        unreal.log_warning(f"  Could not confirm/load level: {e}")

    with unreal.ScopedSlowTask(NUM_CARS + 2, "Train ceiling lights...") as task:
        task.make_dialog(True)

        task.enter_progress_frame(1, "Cleanup previous LIT_Ceiling_ ...")
        try:
            cleanup()
        except Exception as e:
            unreal.log_warning(f"Cleanup stage failed: {e}")

        unreal.log("=" * 64)
        unreal.log("PLACE per-car overhead light layer (centerline + 2 side rows)")
        unreal.log("=" * 64)
        for idx in range(NUM_CARS):
            task.enter_progress_frame(1, f"Car {idx:02d} ceiling lights...")
            try:
                build_car_ceiling(idx)
            except Exception as e:
                unreal.log_warning(f"  Car {idx:02d} failed (continuing): {e}")

        task.enter_progress_frame(1, "Gangway lights...")
        try:
            build_gangway_lights()
        except Exception as e:
            unreal.log_warning(f"Gangway stage failed: {e}")

        task.enter_progress_frame(1, "Save...")
        save_everything()

    # Summary
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  CEILING TRAIN-LIGHT LAYER COMPLETE")
    unreal.log("=" * 64)
    for idx in range(NUM_CARS):
        unreal.log(f"  - Car {idx:02d}: {_per_car[idx]} lights")
    unreal.log(f"  - Gangway lights: {_gangway_total}")
    unreal.log(f"  - TOTAL LIT_Ceiling_ lights placed: {_total}")
    unreal.log("")
    unreal.log(f"  Fixture: PointLight | rows: centerline y=0 + sides y=+/-{int(ROW_OFFSET_Y)}")
    unreal.log(f"  {FIXTURES_PER_ROW} fixtures/row x 3 rows = {FIXTURES_PER_ROW * 3}/car"
               f" | ceiling Z={int(CEILING_Z)} (interior ceiling {int(CAR_HEIGHT)})")
    unreal.log(f"  Intensity: center {int(INTENSITY_CENTER)}cd / sides "
               f"{int(INTENSITY_SIDE)}cd / gangway {int(INTENSITY_GANGWAY)}cd")
    unreal.log(f"  Color ~4200K warm-neutral | attenuation {int(ATTEN_RADIUS)} "
               f"| shadows={'ON' if CAST_SHADOWS else 'OFF'}")
    unreal.log("  Additive on top of TI_/FD_/TAIL_/Light_Z1_* practicals (untouched).")
    unreal.log("")


if __name__ == "__main__":
    run()
else:
    run()
