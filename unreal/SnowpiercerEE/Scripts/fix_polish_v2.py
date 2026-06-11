"""
Snowpiercer: Eternal Engine -- Polish Pass v2
Run in editor: Tools > Execute Python Script (or headless via -ExecutePythonScript)

Fixes the two issues reported from the second playtest of /Game/Maps/Zone1_Tail,
plus adds navmesh coverage for the incoming enemy AI work:

  1. GANGWAYS LOOK LIKE STACKED LAYERS
     fix_world_polish.seal_gangways() (GW_*) laid a 1400x4400 floor slab that
     overlaps 200cm into BOTH neighbouring car floors -- coplanar top surfaces
     at z=0 that z-fight ("weird stacked layers").  The old build_zone1.py
     connector pieces (Conn_##_*) ALSO still sit in the same gap, so up to
     three slabs coexist.  This script destroys every GW_* and Conn_* actor
     and rebuilds each of the 14 gaps as ONE clean corridor (label prefix
     GW2_) whose pieces ABUT the car geometry exactly -- no overlap, no
     coplanar faces:

       Geometry derived from build_zone1.py (car_x = idx * 13000):
         car floor:            x in [car_x-6000, car_x+6000], top at z=0
         front wall (car i):   x in [car_x+6000, car_x+6150], door |y|<=750
         back wall (car i+1):  x in [car_x+6850, car_x+7000], door |y|<=750
       So per gap:
         floor:    x [gap+6000, gap+7000]  (abuts both car floor edges,
                   also closes the floorless 150cm slots under each doorway)
         walls:    x [gap+6150, gap+6850]  (abut both wall outer faces)
         ceiling:  x [gap+6150, gap+6850]  at door-top height (abuts headers)
       Hidden (hidden_in_game) collision blockers seal the rest of the gap
       cross-section + end caps behind Car 0 / past Car 14.

  2. TRAIN SOUND AT START IS WAY TOO LOUD
     populate_zone1.py spawned 15 AmbientSound actors (Sound_Z1_Car##_Ambient)
     at full volume with NO attenuation = 15 overlapping full-volume loops.
     Fix: volume_multiplier 0.25 + spatialization + per-actor attenuation
     override (sphere radius 1500, falloff 6000) so each loop is only audible
     near its own car.  If attenuation cannot be set from Python, fall back
     to volume 0.15 and keep only the Car 00/05/10 emitters.

  3. NAVMESH
     The level has no NavMeshBoundsVolume, so AI cannot pathfind.  Spawn one
     scaled over the whole train, verify its brush bounds (Python-spawned
     volumes often have no brush geometry), fall back to per-car
     NavModifierVolumes, and as a last resort log loudly for editor-side
     placement.  Also force RecastNavMesh runtime generation and request a
     nav rebuild.

Idempotent: removes its own GW2_* / NAV2_* actors before re-adding; audio
property sets are naturally re-runnable.  Never raises on missing assets --
logs and continues.  CRITICAL: every mutated pre-existing actor/component
gets modify() called first so the headless save actually persists.
"""

import re
import unreal

# ---------------------------------------------------------------------------
# API shortcuts
# ---------------------------------------------------------------------------

level_lib = unreal.EditorLevelLibrary
editor_util = unreal.EditorAssetLibrary

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
GW2_PREFIX = "GW2_"
NAV_PREFIX = "NAV2_"

# Prefixes of the OLD overlapping gangway geometry we replace
OLD_PREFIXES = ("GW_", "Conn_")

# Gangway material (created by upgrade_textures.py), with fallbacks
GANGWAY_MAT_CANDIDATES = [
    "/Game/Materials/Textured/M_Tex_gangway_scratchy_metal",
    "/Game/Materials/Textured/M_Tex_gangway_dirty_metal",
    "/Game/Materials/Zones/Tail/M_Tail_GrimeMetal",
    "/Game/Materials/Zone1/M_Connector",
    "/Game/Materials/Zone1/M_Workshop_Metal",
    "/Game/Materials/ModularPipes/M_BareMetal",
    "/Game/Materials/Zone1/M_Platform_Metal",
]

_stats = {
    "old_destroyed": 0,
    "gangways_built": 0,
    "blockers_placed": 0,
    "sounds_fixed": 0,
    "sounds_removed": 0,
    "sound_strategy": "none",
    "navmesh_status": "not attempted",
}

# ---------------------------------------------------------------------------
# Generic helpers
# ---------------------------------------------------------------------------

_cube_mesh = None


def get_cube():
    global _cube_mesh
    if _cube_mesh is None:
        try:
            _cube_mesh = editor_util.load_asset("/Engine/BasicShapes/Cube")
        except Exception as e:
            unreal.log_warning(f"  Could not load cube mesh: {e}")
    return _cube_mesh


_gangway_mat = None
_gangway_mat_resolved = False


def get_gangway_material():
    """Resolve the gangway material once, walking the fallback list."""
    global _gangway_mat, _gangway_mat_resolved
    if _gangway_mat_resolved:
        return _gangway_mat
    _gangway_mat_resolved = True
    for path in GANGWAY_MAT_CANDIDATES:
        try:
            if editor_util.does_asset_exist(path):
                mat = editor_util.load_asset(path)
                if mat:
                    unreal.log(f"  Gangway material: {path}")
                    _gangway_mat = mat
                    return mat
        except Exception:
            continue
    unreal.log_warning("  No gangway material found -- pieces keep cube default")
    return None


def place_box(label, location, size, material=None, hidden_in_game=False):
    """Spawn a scaled cube StaticMeshActor (pattern from build_zone1.place_box).
    size = (x, y, z) in cm.  hidden_in_game=True makes an invisible blocker
    that still has full collision."""
    try:
        actor = level_lib.spawn_actor_from_class(
            unreal.StaticMeshActor, location, unreal.Rotator(0.0, 0.0, 0.0))
    except Exception as e:
        unreal.log_warning(f"  Spawn failed for {label}: {e}")
        return None
    if not actor:
        unreal.log_warning(f"  Spawn returned None for {label}")
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


def place_blocker(label, location, size):
    """Hidden full-collision blocker (invisible cube; may overlap anything)."""
    actor = place_box(label, location, size, material=None, hidden_in_game=True)
    if actor:
        _stats["blockers_placed"] += 1
    return actor


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
                unreal.log_error(
                    f"  Map not found: {PERSISTENT_MAP} -- run build_zone1.py first")
                return False
            les.load_level(PERSISTENT_MAP)
        return True
    except Exception as e:
        unreal.log_error(f"  Could not load level: {e}")
        return False


def cleanup_gangway_actors():
    """Destroy ALL old gangway geometry (GW_*, Conn_*) and our own GW2_*/NAV2_*
    from a previous run (idempotency)."""
    unreal.log("=== Removing old gangway geometry (GW_, Conn_, GW2_, NAV2_) ===")
    removed_old = 0
    removed_own = 0
    try:
        for actor in list(level_lib.get_all_level_actors()):
            if isinstance(actor, unreal.WorldSettings):
                continue
            try:
                label = actor.get_actor_label()
            except Exception:
                continue
            try:
                if label.startswith(GW2_PREFIX) or label.startswith(NAV_PREFIX):
                    level_lib.destroy_actor(actor)
                    removed_own += 1
                elif any(label.startswith(p) for p in OLD_PREFIXES):
                    level_lib.destroy_actor(actor)
                    removed_old += 1
            except Exception as e:
                unreal.log_warning(f"  Could not destroy {label}: {e}")
    except Exception as e:
        unreal.log_warning(f"  Cleanup error: {e}")
    _stats["old_destroyed"] = removed_old
    unreal.log(f"  Destroyed {removed_old} old gangway actors, "
               f"{removed_own} previous v2 actors")


# ---------------------------------------------------------------------------
# 1. GANGWAY REBUILD -- one clean abutting corridor per gap
# ---------------------------------------------------------------------------
#
# Exact bounds derived from build_zone1.build_car_shell (car_x = i * 13000):
#   car i floor edge / front wall inner face:  x = car_x + 6000
#   front wall outer face:                     x = car_x + 6150
#   car i+1 back wall outer face:              x = car_x + 6850
#   car i+1 back wall inner face / floor edge: x = car_x + 7000
#   doorway opening in both walls:             |y| <= 750, z in [0, 2300]
#
# Floor spans the FULL 6000..7000 (it must also cover the floorless 150cm
# doorway slots under both walls).  It lives entirely at z in [-150, 0] --
# the car floors end at exactly x=6000 / x=7000, so the surfaces ABUT and
# never overlap (the cause of the old z-fighting).
# Walls/ceiling span only the open air gap 6150..6850, butting flush against
# the outer faces of both cars' walls.


def rebuild_gangways():
    unreal.log("=== Rebuilding gangways (single-layer corridors) ===")
    mat = get_gangway_material()

    floor_x0_off = CAR_LENGTH / 2.0                       # +6000
    floor_x1_off = CAR_LENGTH / 2.0 + CAR_GAP             # +7000
    open_x0_off = CAR_LENGTH / 2.0 + WALL_THICK           # +6150
    open_x1_off = CAR_LENGTH / 2.0 + CAR_GAP - WALL_THICK # +6850

    floor_len = floor_x1_off - floor_x0_off               # 1000
    open_len = open_x1_off - open_x0_off                  # 700
    floor_cx_off = (floor_x0_off + floor_x1_off) / 2.0    # +6500
    open_cx_off = (open_x0_off + open_x1_off) / 2.0       # +6500

    corridor_w = DOOR_WIDTH + 2.0 * WALL_THICK            # 1800 (walls incl.)
    wall_cy = DOOR_WIDTH / 2.0 + WALL_THICK / 2.0         # 825

    for i in range(NUM_CARS - 1):
        car_x = i * CAR_STRIDE
        pfx = f"{GW2_PREFIX}{i:02d}"

        # ONE floor slab: top surface exactly at z=0, abutting both car
        # floor edges in X.  Never overlaps the car floors in XY.
        place_box(f"{pfx}_Floor",
                  unreal.Vector(car_x + floor_cx_off, 0.0, -WALL_THICK / 2.0),
                  (floor_len, corridor_w, WALL_THICK), mat)

        # Two side walls, flush against both cars' wall outer faces.
        place_box(f"{pfx}_WallL",
                  unreal.Vector(car_x + open_cx_off, wall_cy, DOOR_HEIGHT / 2.0),
                  (open_len, WALL_THICK, DOOR_HEIGHT), mat)
        place_box(f"{pfx}_WallR",
                  unreal.Vector(car_x + open_cx_off, -wall_cy, DOOR_HEIGHT / 2.0),
                  (open_len, WALL_THICK, DOOR_HEIGHT), mat)

        # One ceiling slab resting on the walls (z 2300..2450), abutting the
        # door headers of both cars.
        place_box(f"{pfx}_Ceil",
                  unreal.Vector(car_x + open_cx_off, 0.0,
                                DOOR_HEIGHT + WALL_THICK / 2.0),
                  (open_len, corridor_w, WALL_THICK), mat)

        # Hidden collision blockers OUTSIDE the visible corridor, sealing the
        # rest of the gap cross-section (overlap freely -- never rendered).
        side_y0 = corridor_w / 2.0                         # 900
        side_y1 = CAR_WIDTH / 2.0 + WALL_THICK + 200.0     # 2350
        side_cy = (side_y0 + side_y1) / 2.0
        place_blocker(f"{pfx}_BlockL",
                      unreal.Vector(car_x + open_cx_off, side_cy,
                                    CAR_HEIGHT / 2.0),
                      (open_len, side_y1 - side_y0, CAR_HEIGHT + 800.0))
        place_blocker(f"{pfx}_BlockR",
                      unreal.Vector(car_x + open_cx_off, -side_cy,
                                    CAR_HEIGHT / 2.0),
                      (open_len, side_y1 - side_y0, CAR_HEIGHT + 800.0))

        # Top blocker: from corridor ceiling top up past the car roof line.
        top_z0 = DOOR_HEIGHT + WALL_THICK                  # 2450
        top_z1 = CAR_HEIGHT + WALL_THICK + 300.0           # 3450
        place_blocker(f"{pfx}_BlockTop",
                      unreal.Vector(car_x + open_cx_off, 0.0,
                                    (top_z0 + top_z1) / 2.0),
                      (open_len, CAR_WIDTH + 800.0, top_z1 - top_z0))

        # Bottom blocker: safety net below the floor slab.
        place_blocker(f"{pfx}_BlockBottom",
                      unreal.Vector(car_x + floor_cx_off, 0.0,
                                    -WALL_THICK - 150.0),
                      (floor_len + 200.0, CAR_WIDTH + 800.0, 300.0))

        _stats["gangways_built"] += 1

    # End caps (hidden blockers only) behind Car 0 and past Car 14.
    rear_x = -(CAR_LENGTH / 2.0 + WALL_THICK + 200.0)
    place_blocker(f"{GW2_PREFIX}EndCap_Rear",
                  unreal.Vector(rear_x, 0.0, CAR_HEIGHT / 2.0),
                  (300.0, CAR_WIDTH + 1000.0, CAR_HEIGHT + 1000.0))
    front_x = (NUM_CARS - 1) * CAR_STRIDE + CAR_LENGTH / 2.0 + WALL_THICK + 200.0
    place_blocker(f"{GW2_PREFIX}EndCap_Front",
                  unreal.Vector(front_x, 0.0, CAR_HEIGHT / 2.0),
                  (300.0, CAR_WIDTH + 1000.0, CAR_HEIGHT + 1000.0))

    unreal.log(f"  Rebuilt {_stats['gangways_built']} gangways "
               f"({_stats['blockers_placed']} hidden blockers incl. 2 end caps)")


# ---------------------------------------------------------------------------
# 2. AUDIO FIX -- quiet + spatialize the 15 ambient train loops
# ---------------------------------------------------------------------------

ATTEN_INNER_RADIUS = 1500.0
ATTEN_FALLOFF = 6000.0
TARGET_VOLUME = 0.25
FALLBACK_VOLUME = 0.15
FALLBACK_KEEP_CARS = (0, 5, 10)

_car_idx_re = re.compile(r"Car(\d+)")


def _build_attenuation_settings():
    """Build an FSoundAttenuationSettings (sphere r=1500, falloff 6000)."""
    att = unreal.SoundAttenuationSettings()
    try:
        att.set_editor_property("attenuation_shape",
                                unreal.AttenuationShape.SPHERE)
    except Exception as e:
        unreal.log_warning(f"  attenuation_shape not settable: {e}")
    try:
        att.set_editor_property(
            "attenuation_shape_extents",
            unreal.Vector(ATTEN_INNER_RADIUS, 0.0, 0.0))
    except Exception as e:
        unreal.log_warning(f"  attenuation_shape_extents not settable: {e}")
        raise
    try:
        att.set_editor_property("falloff_distance", ATTEN_FALLOFF)
    except Exception as e:
        unreal.log_warning(f"  falloff_distance not settable: {e}")
        raise
    return att


def _try_set(obj, prop_names, value):
    """Try set_editor_property under several candidate names; True on success."""
    for name in prop_names:
        try:
            obj.set_editor_property(name, value)
            return True
        except Exception:
            continue
    return False


def fix_ambient_sounds():
    unreal.log("=== Fixing ambient train sounds ===")
    sound_actors = []
    try:
        for actor in level_lib.get_all_level_actors():
            try:
                label = actor.get_actor_label()
            except Exception:
                continue
            if label.startswith("Sound_Z1_"):
                sound_actors.append((label, actor))
    except Exception as e:
        unreal.log_warning(f"  Could not enumerate actors: {e}")
        return

    if not sound_actors:
        unreal.log_warning("  No Sound_Z1_* actors found -- nothing to fix")
        _stats["sound_strategy"] = "no sound actors found"
        return

    attenuation_ok_count = 0
    for label, actor in sound_actors:
        try:
            actor.modify()
        except Exception:
            pass
        comp = None
        try:
            comp = actor.get_component_by_class(unreal.AudioComponent)
        except Exception:
            pass
        if not comp:
            unreal.log_warning(f"  {label}: no AudioComponent")
            continue
        try:
            comp.modify()
        except Exception:
            pass

        # --- volume (must always apply) ---
        vol_ok = _try_set(comp, ("volume_multiplier",), TARGET_VOLUME)
        if not vol_ok:
            try:
                comp.set_volume_multiplier(TARGET_VOLUME)
                vol_ok = True
            except Exception as e:
                unreal.log_warning(f"  {label}: volume set FAILED: {e}")
        if vol_ok:
            _stats["sounds_fixed"] += 1

        # --- attenuation override ---
        att_ok = False
        try:
            if not _try_set(comp, ("allow_spatialization",), True):
                unreal.log_warning(f"  {label}: allow_spatialization not settable")
            override_ok = _try_set(
                comp, ("b_override_attenuation", "override_attenuation"), True)
            if not override_ok:
                unreal.log_warning(f"  {label}: override_attenuation not settable")
            att = _build_attenuation_settings()
            settings_ok = _try_set(comp, ("attenuation_overrides",), att)
            if not settings_ok:
                unreal.log_warning(f"  {label}: attenuation_overrides not settable")
            att_ok = override_ok and settings_ok
        except Exception as e:
            unreal.log_warning(f"  {label}: attenuation setup failed: {e}")
        if att_ok:
            attenuation_ok_count += 1

    if attenuation_ok_count == len(sound_actors):
        _stats["sound_strategy"] = (
            f"volume {TARGET_VOLUME} + attenuation override "
            f"(inner {ATTEN_INNER_RADIUS:.0f}, falloff {ATTEN_FALLOFF:.0f}) "
            f"on all {len(sound_actors)} emitters")
        unreal.log(f"  Attenuation applied to all {len(sound_actors)} emitters")
        return

    # ---- FALLBACK: attenuation not (fully) settable from Python ----
    unreal.log_warning(
        f"  Attenuation only applied to {attenuation_ok_count}/"
        f"{len(sound_actors)} emitters -- falling back: volume "
        f"{FALLBACK_VOLUME}, keep only cars {FALLBACK_KEEP_CARS}")
    kept = 0
    for label, actor in sound_actors:
        m = _car_idx_re.search(label)
        car_idx = int(m.group(1)) if m else -1
        try:
            if car_idx in FALLBACK_KEEP_CARS:
                actor.modify()
                comp = actor.get_component_by_class(unreal.AudioComponent)
                if comp:
                    comp.modify()
                    if not _try_set(comp, ("volume_multiplier",), FALLBACK_VOLUME):
                        try:
                            comp.set_volume_multiplier(FALLBACK_VOLUME)
                        except Exception as e:
                            unreal.log_warning(
                                f"  {label}: fallback volume FAILED: {e}")
                kept += 1
            else:
                level_lib.destroy_actor(actor)
                _stats["sounds_removed"] += 1
        except Exception as e:
            unreal.log_warning(f"  {label}: fallback handling failed: {e}")
    _stats["sound_strategy"] = (
        f"FALLBACK: attenuation unavailable -- volume {FALLBACK_VOLUME}, "
        f"kept {kept} emitters (cars {FALLBACK_KEEP_CARS}), "
        f"removed {_stats['sounds_removed']}")


# ---------------------------------------------------------------------------
# 3. NAVMESH -- bounds volume over the whole train + runtime generation
# ---------------------------------------------------------------------------

NAV_MIN = (-8000.0, -2500.0, -200.0)
NAV_MAX = (NUM_CARS * CAR_STRIDE + 2000.0, 2500.0, 3200.0)  # 197000


def _bounds_valid(actor, min_extent=1.0):
    """True if actor reports non-degenerate world bounds (and return extent)."""
    try:
        origin, extent = actor.get_actor_bounds(False)
        ok = (extent.x > min_extent and extent.y > min_extent
              and extent.z > min_extent)
        return ok, extent
    except Exception as e:
        unreal.log_warning(f"  get_actor_bounds failed: {e}")
        return False, None


def _spawn_scaled_volume(volume_class, label, center, size):
    """Spawn a brush volume and scale it to `size`; validate brush bounds.
    Returns the actor on success, None (actor destroyed) on degenerate brush."""
    try:
        actor = level_lib.spawn_actor_from_class(
            volume_class, unreal.Vector(*center), unreal.Rotator(0.0, 0.0, 0.0))
    except Exception as e:
        unreal.log_warning(f"  Spawn of {volume_class.get_name()} failed: {e}")
        return None
    if not actor:
        unreal.log_warning(f"  Spawn of {label} returned None")
        return None
    actor.set_actor_label(label)

    ok, extent = _bounds_valid(actor)
    unreal.log(f"  {label}: post-spawn bounds extent = "
               f"{extent if extent else 'unavailable'}")
    if ok:
        # Scale the existing brush to the requested size.
        actor.set_actor_scale3d(unreal.Vector(
            size[0] / max(extent.x * 2.0, 1.0),
            size[1] / max(extent.y * 2.0, 1.0),
            size[2] / max(extent.z * 2.0, 1.0)))
    else:
        # No measurable brush -- try the conventional 200uu default brush.
        unreal.log_warning(f"  {label}: degenerate bounds -- trying blind "
                           f"scale assuming 200uu default brush")
        actor.set_actor_scale3d(unreal.Vector(
            size[0] / 200.0, size[1] / 200.0, size[2] / 200.0))

    # Re-verify after scaling -- this is the real pass/fail signal.
    ok2, extent2 = _bounds_valid(actor, min_extent=100.0)
    unreal.log(f"  {label}: post-scale bounds extent = "
               f"{extent2 if extent2 else 'unavailable'}")
    if not ok2:
        unreal.log_warning(f"  {label}: bounds still degenerate -- discarding")
        try:
            level_lib.destroy_actor(actor)
        except Exception:
            pass
        return None
    return actor


def _ensure_recast_runtime_generation():
    """Force RecastNavMesh runtime generation (dynamic) so nav builds in PIE."""
    try:
        recast = None
        for actor in level_lib.get_all_level_actors():
            if isinstance(actor, unreal.RecastNavMesh):
                recast = actor
                break
        if not recast:
            try:
                recast = level_lib.spawn_actor_from_class(
                    unreal.RecastNavMesh, unreal.Vector(0.0, 0.0, 0.0),
                    unreal.Rotator(0.0, 0.0, 0.0))
                if recast:
                    recast.set_actor_label(f"{NAV_PREFIX}RecastNavMesh")
                    unreal.log("  Spawned RecastNavMesh actor")
            except Exception as e:
                unreal.log_warning(f"  Could not spawn RecastNavMesh: {e}")
        if recast:
            try:
                recast.modify()
            except Exception:
                pass
            if _try_set(recast, ("runtime_generation",),
                        unreal.RuntimeGenerationType.DYNAMIC):
                unreal.log("  RecastNavMesh.runtime_generation = DYNAMIC  [OK]")
            else:
                unreal.log_warning(
                    "  Could not set runtime_generation on RecastNavMesh")
        else:
            unreal.log_warning("  No RecastNavMesh actor available")
    except Exception as e:
        unreal.log_warning(f"  RecastNavMesh setup failed: {e}")


def _request_nav_rebuild():
    """Ask the navigation system to rebuild, via API then console command."""
    world = None
    try:
        world = level_lib.get_editor_world()
    except Exception as e:
        unreal.log_warning(f"  get_editor_world failed: {e}")

    rebuilt = False
    try:
        nav_sys = unreal.NavigationSystemV1.get_navigation_system(world)
        if nav_sys:
            for method in ("build", "rebuild_all", "rebuild_dirty_areas"):
                try:
                    getattr(nav_sys, method)()
                    unreal.log(f"  NavigationSystemV1.{method}()  [OK]")
                    rebuilt = True
                    break
                except Exception:
                    continue
    except Exception as e:
        unreal.log(f"  NavigationSystemV1 API unavailable: {e}")

    if not rebuilt and world:
        for cmd in ("RebuildNavigation",):
            try:
                unreal.SystemLibrary.execute_console_command(world, cmd)
                unreal.log(f"  Console command '{cmd}' issued  [OK]")
                rebuilt = True
            except Exception as e:
                unreal.log_warning(f"  Console command '{cmd}' failed: {e}")
    if not rebuilt:
        unreal.log_warning("  Could not trigger a nav rebuild from Python -- "
                           "navmesh will build on next editor load / PIE "
                           "(runtime_generation DYNAMIC)")


def setup_navmesh():
    unreal.log("=== NavMesh setup ===")
    center = ((NAV_MIN[0] + NAV_MAX[0]) / 2.0,
              (NAV_MIN[1] + NAV_MAX[1]) / 2.0,
              (NAV_MIN[2] + NAV_MAX[2]) / 2.0)
    size = (NAV_MAX[0] - NAV_MIN[0],
            NAV_MAX[1] - NAV_MIN[1],
            NAV_MAX[2] - NAV_MIN[2])
    unreal.log(f"  Target nav bounds: center={center} size={size}")

    # --- Primary: one NavMeshBoundsVolume over the whole train ---
    volume = None
    try:
        volume = _spawn_scaled_volume(
            unreal.NavMeshBoundsVolume, f"{NAV_PREFIX}Bounds_Train",
            center, size)
    except Exception as e:
        unreal.log_warning(f"  NavMeshBoundsVolume attempt raised: {e}")
    if volume:
        _stats["navmesh_status"] = (
            "NavMeshBoundsVolume placed over whole train (verified bounds)")
        unreal.log("  NavMeshBoundsVolume placed  [OK]")
    else:
        # --- Fallback: per-car NavModifierVolumes ---
        unreal.log_warning("  NavMeshBoundsVolume unusable -- falling back to "
                           "per-car NavModifierVolumes")
        placed = 0
        try:
            for i in range(NUM_CARS):
                car_x = i * CAR_STRIDE
                v = _spawn_scaled_volume(
                    unreal.NavModifierVolume, f"{NAV_PREFIX}Modifier_Car{i:02d}",
                    (car_x, 0.0, 1500.0),
                    (CAR_STRIDE, CAR_WIDTH + 1000.0, CAR_HEIGHT + 400.0))
                if v:
                    placed += 1
        except Exception as e:
            unreal.log_warning(f"  NavModifierVolume fallback raised: {e}")
        if placed > 0:
            _stats["navmesh_status"] = (
                f"FALLBACK: {placed}/{NUM_CARS} NavModifierVolumes placed")
            unreal.log(f"  Placed {placed} NavModifierVolumes  [PARTIAL OK]")
        else:
            _stats["navmesh_status"] = (
                "NAVMESH FAILED -- needs editor-side placement")
            unreal.log_error("  ================================================")
            unreal.log_error("  NAVMESH FAILED -- needs editor-side placement")
            unreal.log_error("  Place a NavMeshBoundsVolume manually covering")
            unreal.log_error(f"    X {NAV_MIN[0]:.0f}..{NAV_MAX[0]:.0f}  "
                             f"Y {NAV_MIN[1]:.0f}..{NAV_MAX[1]:.0f}  "
                             f"Z {NAV_MIN[2]:.0f}..{NAV_MAX[2]:.0f}")
            unreal.log_error("  ================================================")

    # --- Runtime generation + rebuild (independent of volume outcome) ---
    _ensure_recast_runtime_generation()
    _request_nav_rebuild()


# ---------------------------------------------------------------------------
# Save (modify-then-save pattern; BOTH saves are required headless)
# ---------------------------------------------------------------------------


def save_everything():
    unreal.log("=== Saving ===")
    saved_any = False
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        les.save_all_dirty_levels()
        unreal.log("  save_all_dirty_levels  [OK]")
        saved_any = True
    except Exception as e:
        unreal.log_warning(f"  save_all_dirty_levels failed: {e}")
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        les.save_current_level()
        unreal.log("  save_current_level  [OK]")
        saved_any = True
    except Exception as e:
        unreal.log_warning(f"  save_current_level failed: {e}")
    if not saved_any:
        # Last resort: deprecated EditorLevelLibrary save
        try:
            level_lib.save_all_dirty_levels()
            unreal.log("  EditorLevelLibrary.save_all_dirty_levels  [OK]")
        except Exception as e:
            unreal.log_error(f"  ALL SAVE PATHS FAILED: {e}")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------


def run():
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  Polish Pass v2 (gangways / audio / navmesh)")
    unreal.log("=" * 64)
    unreal.log("")

    if not load_zone1():
        return

    cleanup_gangway_actors()

    try:
        rebuild_gangways()
    except Exception as e:
        unreal.log_warning(f"Gangway rebuild failed: {e}")

    try:
        fix_ambient_sounds()
    except Exception as e:
        unreal.log_warning(f"Ambient sound fix failed: {e}")

    try:
        setup_navmesh()
    except Exception as e:
        unreal.log_warning(f"NavMesh setup failed: {e}")

    save_everything()

    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  POLISH PASS v2 COMPLETE")
    unreal.log("=" * 64)
    unreal.log(f"  Old gangway actors destroyed:  {_stats['old_destroyed']}")
    unreal.log(f"  Gangways rebuilt (GW2_):       {_stats['gangways_built']} "
               f"({_stats['blockers_placed']} hidden blockers)")
    unreal.log(f"  Sounds volume-fixed:           {_stats['sounds_fixed']}")
    unreal.log(f"  Sounds removed (fallback):     {_stats['sounds_removed']}")
    unreal.log(f"  Sound strategy:                {_stats['sound_strategy']}")
    unreal.log(f"  NavMesh:                       {_stats['navmesh_status']}")
    unreal.log("")
    unreal.log("  Verify: walk all 15 cars -- gangway floors should be a")
    unreal.log("  single clean layer; train hum should be quiet and local.")
    unreal.log("")


run()
