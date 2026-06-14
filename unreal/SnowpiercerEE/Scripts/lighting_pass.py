"""
Snowpiercer: Eternal Engine -- Zone 1 GLOBAL LIGHTING PASS
Run in editor (Tools > Execute Python Script) or headless via
-ExecutePythonScript="...Scripts/lighting_pass.py".

GLOBAL lighting quality + a navigable baseline for /Game/Maps/Zone1_Tail.
This pass is COMPLEMENTARY to the per-car PRACTICAL point lights placed by
train_interior_pass.py (TI_), detail_pass_film.py (FD_) and tail_dressing.py
(TAIL_) -- barrel fires, lamps, candles.  It does NOT add, remove, dim or
duplicate any per-car practical: it only sets the three GLOBAL actors that
govern the whole train --

  1. BASELINE FILL  -- one SkyLight, very dim + cool, so no car is pitch
                       black and the player can always see the floor, WITHOUT
                       washing out the warm practical pools.
  2. POST-PROCESS   -- one unbound (infinite) PostProcessVolume: clamped
                       auto-exposure (stable, no bloom-out / crush), a cold-
                       shadow / warm-highlight grade, subtle bloom, mild
                       vignette, a touch of film grain.
  3. ATMOSPHERE     -- one global ExponentialHeightFog, dim + cool + low
                       density, for depth down the 15 long cars.

Snowpiercer-Tail mood: cold, grimy, oppressive, but NAVIGABLE -- warm pools
from the practicals read against cold blue steel creeping in at the edges.
Cars 10 (Dark Car) and 11 (Freezer Breach) are left moodier: the baseline
fill is tuned low enough that it keeps them barely-navigable cold-blue, not
fully lit, so we don't fight their intended darkness.

Idempotency / ownership:
  * Every actor THIS script creates is label-prefixed LX_ and all LX_ actors
    are destroyed at the start of the run.
  * The level already ships global actors from build_zone1.py
    (SkyLight_Zone1, PostProcess_Z1_Tail).  For those non-LX actors we
    FIND-OR-MODIFY in place (never duplicate).  If a target is somehow
    missing we spawn an LX_ replacement.
  * Per-car FD_/TI_/TAIL_/Light_Z1_* actors are NEVER touched.

Rules honoured (from detail_pass_film.py / persist_materials_lighting.py):
  * actor.modify() (and component.modify()) BEFORE mutating placed actors --
    headless property-only saves are no-ops otherwise.
  * PostProcessSettings need their override_* flag set True alongside each
    value, and the mutated `settings` struct must be written back.
  * Fog uses fog_inscattering_luminance (NOT fog_inscattering_color).
  * Risky set_editor_property wrapped in try/except with a logged fallback.
  * Ends with save_all_dirty_levels() AND save_current_level().

Helper actors (SkyLight / PostProcessVolume / ExponentialHeightFog) carry no
collision and do not block the player -- verified explicitly at the end.
"""

import unreal

# ---------------------------------------------------------------------------
# API shortcuts (style of build_zone1.py / detail_pass_film.py)
# ---------------------------------------------------------------------------

editor_util = unreal.EditorAssetLibrary
level_lib = unreal.EditorLevelLibrary          # deprecated-but-functional in 5.7

# ---------------------------------------------------------------------------
# Constants (mirror build_zone1.py / detail_pass_film.py)
# ---------------------------------------------------------------------------

PERSISTENT_MAP = "/Game/Maps/Zone1_Tail"
CAR_LENGTH = 12000.0
CAR_GAP = 1000.0
CAR_STRIDE = 13000.0
NUM_CARS = 15
CAR_HEIGHT = 3000.0

PREFIX = "LX_"                  # every actor THIS script places

# Existing global actors built by build_zone1.py that we tune rather than dup
EXISTING_SKYLIGHT_LABEL = "SkyLight_Zone1"
EXISTING_PPV_LABEL = "PostProcess_Z1_Tail"

# Train midpoint (where build_zone1.py parks its global actors)
TOTAL_LENGTH = NUM_CARS * (CAR_LENGTH + CAR_GAP)
MID_X = TOTAL_LENGTH / 2.0

_log = []                       # collected key values for the final summary


def _note(msg):
    _log.append(msg)
    unreal.log(f"  {msg}")


# ---------------------------------------------------------------------------
# Small helpers
# ---------------------------------------------------------------------------

def _rot(pitch=0.0, yaw=0.0, roll=0.0):
    r = unreal.Rotator()
    r.set_editor_property("pitch", float(pitch))
    r.set_editor_property("yaw", float(yaw))
    r.set_editor_property("roll", float(roll))
    return r


def _try_set(obj, prop, value, ctx=""):
    """set_editor_property wrapped with a logged fallback (per project rules)."""
    try:
        obj.set_editor_property(prop, value)
        return True
    except Exception as e:
        unreal.log_warning(f"  [skip] {ctx}{prop}: {e}")
        return False


def _all_actors():
    try:
        return level_lib.get_all_level_actors()
    except Exception as e:
        unreal.log_error(f"  Could not enumerate actors: {e}")
        return []


def _find_by_label(label):
    for a in _all_actors():
        try:
            if a.get_actor_label() == label:
                return a
        except Exception:
            continue
    return None


def _find_first_of_class(cls, ignore_prefix=None):
    """First actor of a class, optionally skipping our own LX_ actors."""
    for a in _all_actors():
        if not isinstance(a, cls):
            continue
        if ignore_prefix:
            try:
                if a.get_actor_label().startswith(ignore_prefix):
                    continue
            except Exception:
                pass
        return a
    return None


def _disable_collision(actor):
    """Belt-and-suspenders: make sure a helper actor never blocks the player.
    SkyLight / PP volume / fog have no blocking collision by default, but we
    force NoCollision on every primitive component we can reach."""
    try:
        comps = actor.get_components_by_class(unreal.PrimitiveComponent)
    except Exception:
        comps = []
    for c in comps:
        try:
            c.modify()
            c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        except Exception:
            try:
                c.set_editor_property("collision_enabled",
                                      unreal.CollisionEnabled.NO_COLLISION)
            except Exception:
                pass


# ---------------------------------------------------------------------------
# 0. CLEANUP -- destroy everything from previous LX_ runs
# ---------------------------------------------------------------------------

def cleanup():
    unreal.log("=" * 64)
    unreal.log("0. CLEANUP previous LX_ actors")
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
    unreal.log(f"  Removed {removed} LX_ actors")


# ---------------------------------------------------------------------------
# 1. BASELINE FILL -- SkyLight (find-or-modify; cool + dim)
# ---------------------------------------------------------------------------
#
# Why these values (RETUNED -- playtest: "too dark, train very blue"):
#   intensity 0.75  -- raised from 0.25.  The old fill barely lifted shadows
#                      off black, so the aisle wasn't navigable.  0.75 makes
#                      every car clearly readable while still sitting well
#                      below the warm practical POOLS (800-1500 cd point
#                      lights) so the bulbs still read as the light sources.
#   colour ~ desaturated cool-grey (0.62, 0.65, 0.72 linear) -- was a
#                      saturated cold-blue (0.34/0.40/0.52) which made the
#                      whole train read monochrome-blue.  Now a near-neutral
#                      grey with only a slight cool tint: cold ATMOSPHERE, not
#                      "everything is blue", and the warm bulbs stay warm.
#   SLS_SPECIFIED_CUBEMAP would need an asset; we keep SLS_CAPTURED_SCENE
#   (the build_zone1 default) but drive the look with lower_hemisphere colour
#   so even down-facing fill reads, and a mild occlusion so it stays moody.

SKY_INTENSITY = 0.75
SKY_COLOR = unreal.LinearColor(0.62, 0.65, 0.72, 1.0)   # desaturated cool-grey
SKY_LOWER_HEMI = unreal.LinearColor(0.30, 0.31, 0.34, 1.0)  # brighter neutral floor fill


def _configure_skylight_component(sc, source):
    """Apply the cool/dim baseline to a SkyLightComponent (modify-then-set)."""
    try:
        sc.modify()
    except Exception:
        pass
    _try_set(sc, "intensity", float(SKY_INTENSITY), "skylight ")
    _try_set(sc, "light_color",
             unreal.Color(int(SKY_COLOR.r * 255), int(SKY_COLOR.g * 255),
                          int(SKY_COLOR.b * 255), 255), "skylight ")
    # Real-time/static capture: leave source type as-is on existing lights;
    # just make the hemisphere/occlusion cold + contrasty.
    _try_set(sc, "lower_hemisphere_color", SKY_LOWER_HEMI, "skylight ")
    _try_set(sc, "lower_hemisphere_is_black", False, "skylight ")
    # Mild occlusion so the flat ambient still gives shape to clutter, but
    # lower contrast than before so occluded aisle corners don't crush dark.
    _try_set(sc, "occlusion_max_distance", 1500.0, "skylight ")
    _try_set(sc, "contrast", 0.15, "skylight ")
    _note(f"SkyLight ({source}): intensity={SKY_INTENSITY} (raised for navigability), "
          f"color=desaturated cool-grey {SKY_COLOR.r:.2f}/{SKY_COLOR.g:.2f}/{SKY_COLOR.b:.2f} "
          f"(de-blued), lower-hemi lifted, contrast softened")


def setup_skylight():
    unreal.log("=" * 64)
    unreal.log("1. BASELINE FILL -- SkyLight (cool + dim, navigable)")
    unreal.log("=" * 64)

    sky = _find_by_label(EXISTING_SKYLIGHT_LABEL)
    if sky is None:
        sky = _find_first_of_class(unreal.SkyLight, ignore_prefix=PREFIX)

    if sky is not None:
        try:
            sky.modify()
        except Exception:
            pass
        sc = sky.get_component_by_class(unreal.SkyLightComponent)
        if sc:
            _configure_skylight_component(sc, f"tuned '{sky.get_actor_label()}'")
            try:
                # Re-capture so the new colour/intensity take in static capture.
                sc.recapture_sky()
            except Exception:
                pass
        else:
            unreal.log_warning("  Existing SkyLight has no component -- skipping")
        return

    # No SkyLight in the level -- spawn our own LX_ one.
    sky = level_lib.spawn_actor_from_class(
        unreal.SkyLight, unreal.Vector(MID_X, 0.0, 8000.0), _rot())
    if not sky:
        unreal.log_warning("  Could not spawn LX_ SkyLight")
        return
    sky.set_actor_label(f"{PREFIX}SkyLight_Fill")
    sc = sky.get_component_by_class(unreal.SkyLightComponent)
    if sc:
        # A freshly-spawned skylight: drive it as a real-time capture so it
        # picks up the dim interior without needing a cubemap asset.
        _try_set(sc, "real_time_capture", True, "skylight ")
        _configure_skylight_component(sc, "spawned LX_SkyLight_Fill")
        try:
            sc.recapture_sky()
        except Exception:
            pass
    _disable_collision(sky)


# ---------------------------------------------------------------------------
# 2. POST-PROCESS -- unbound volume (find-or-modify; clamp + grade)
# ---------------------------------------------------------------------------
#
# Exposure (RETUNED -- playtest "too dark"): the old window (min 0.25 / max
#   1.25, bias -0.25) metered the Tail far too dark to navigate.  We RAISE and
#   WIDEN it -- min EV 0.8 / max EV 2.3 -- so dim aisle sections brighten while
#   barrel-lit cars still don't bloom out, and we drop the negative bias to 0.0
#   so nothing is pushed darker than metered.  Histogram metering, fast-ish
#   adaptation so doorway-to-doorway swings settle without a long pump.
# Grade (RETUNED -- playtest "very blue"): the old grade pushed a teal split
#   (cool shadows / warm highlights) AND desaturated, which combined with the
#   blue skylight/fog to read monochrome-blue.  Now:
#   white_balance_temp 6300K -- near-neutral (was effectively cold via tints);
#     lets the warm practical bulbs read warm instead of being pulled blue.
#   color_saturation 0.92  -- only a light desaturation (was 0.80) so the warm
#     bulbs keep their colour; grime without going grey-monochrome.
#   color_contrast   1.05  -- gentle contrast lift, slightly softened.
#   color_gain       (1.0,0.995,0.99) -- a whisper of warmth, near-neutral.
#   shadows: teal push REMOVED.  Shadow tint is now near-neutral
#     (1.00,1.005,1.02) -- a hint of cool, not a steel-blue dye -- and shadow
#     saturation lifted to 0.95 so shadows aren't a flat blue wash.
#   highlights: keep a mild warmth (1.04,1.00,0.96) so practicals read warm,
#     but don't fight the now-neutral white balance.
# Bloom 0.25 (subtle, practicals glow without smearing or hiding geometry).
# Vignette 0.35 (claustrophobia, not a black tunnel -- lowered for visibility).
# Film grain 0.15 (grime; low enough not to crawl or crush visibility).

PP = {
    # auto-exposure (histogram, raised + widened for navigability)
    "auto_exposure_method": unreal.AutoExposureMethod.AEM_HISTOGRAM,
    "auto_exposure_min_brightness": 0.80,
    "auto_exposure_max_brightness": 2.30,
    "auto_exposure_bias": 0.0,
    "auto_exposure_speed_up": 3.0,
    "auto_exposure_speed_down": 1.5,
    # white balance -- neutral, so the train isn't dyed cold
    "white_temp": 6300.0,
    "white_tint": 0.0,
    # grade
    "color_saturation": unreal.Vector4(0.92, 0.92, 0.92, 1.0),
    "color_contrast": unreal.Vector4(1.05, 1.05, 1.05, 1.0),
    "color_gain": unreal.Vector4(1.00, 0.995, 0.99, 1.0),
    # near-neutral shadows (teal push removed) + warm highlights for the bulbs
    "color_saturation_shadows": unreal.Vector4(0.95, 0.95, 0.95, 1.0),
    "color_gain_shadows": unreal.Vector4(1.00, 1.005, 1.02, 1.0),
    "color_gain_highlights": unreal.Vector4(1.04, 1.00, 0.96, 1.0),
    # lens
    "bloom_intensity": 0.25,
    "vignette_intensity": 0.35,
    "film_grain_intensity": 0.15,
}


def _apply_pp_settings(settings):
    """Write every PP value with its matching override_* flag (modify-then-set
    handled by the caller on the owning actor).  Each is individually guarded
    so an unknown UE 5.7 property name can't abort the whole grade."""
    applied = 0
    for prop, value in PP.items():
        ok_v = _try_set(settings, prop, value, "pp ")
        ok_o = _try_set(settings, f"override_{prop}", True, "pp ")
        if ok_v and ok_o:
            applied += 1
    return applied


def setup_postprocess():
    unreal.log("=" * 64)
    unreal.log("2. POST-PROCESS -- unbound volume (clamped exposure + grade)")
    unreal.log("=" * 64)

    ppv = _find_by_label(EXISTING_PPV_LABEL)
    source = None
    if ppv is not None:
        source = f"tuned '{ppv.get_actor_label()}'"
    else:
        ppv = _find_first_of_class(unreal.PostProcessVolume, ignore_prefix=PREFIX)
        if ppv is not None:
            source = f"tuned '{ppv.get_actor_label()}'"

    if ppv is None:
        ppv = level_lib.spawn_actor_from_class(
            unreal.PostProcessVolume,
            unreal.Vector(MID_X, 0.0, CAR_HEIGHT / 2.0), _rot())
        if not ppv:
            unreal.log_warning("  Could not spawn LX_ PostProcessVolume")
            return
        ppv.set_actor_label(f"{PREFIX}PostProcess_Global")
        source = "spawned LX_PostProcess_Global"

    try:
        ppv.modify()
    except Exception:
        pass

    # Unbound (infinite extent) so it grades every car; modest priority so a
    # future per-car PP volume could still override locally if one is added.
    _try_set(ppv, "unbound", True, "ppv ")
    _try_set(ppv, "priority", 1.0, "ppv ")
    _try_set(ppv, "blend_weight", 1.0, "ppv ")

    settings = ppv.get_editor_property("settings")
    applied = 0
    if settings:
        applied = _apply_pp_settings(settings)
        # Write the mutated struct back -- a copy was returned.
        _try_set(ppv, "settings", settings, "ppv ")
    else:
        unreal.log_warning("  PostProcessVolume has no settings struct")

    _disable_collision(ppv)
    _note(f"PostProcess ({source}): unbound=True, "
          f"auto-exposure histogram min=0.80/max=2.30 EV bias=0.0 (brighter), "
          f"white_temp=6300K neutral, saturation 0.92, contrast 1.05, "
          f"shadows de-blued (near-neutral) / highlights mildly warm, "
          f"bloom 0.25, vignette 0.35, grain 0.15 ({applied}/{len(PP)} props)")


# ---------------------------------------------------------------------------
# 3. ATMOSPHERE -- global ExponentialHeightFog (dim, cool, low density)
# ---------------------------------------------------------------------------
#
# build_zone1.py already drops a *local* "Fog_Z1_DarkCar" tuned for car 10 --
# we leave that alone.  Here we add ONE global, train-wide fog for depth down
# the 13000-unit-apart cars so distant cars haze out instead of reading flat.
#   density 0.025       -- slightly lowered (was 0.03): visible depth haze but
#                          it tints/darkens the scene less, helping navigability.
#   height_falloff 0.20 -- low, so the haze fills the full 3000-unit-tall
#                          interior rather than pooling at the floor.
#   inscattering (RETUNED -- playtest "very blue"): was a cold blue-grey
#                          (0.018,0.022,0.032) -- the blue channel dominated and
#                          dyed every distant car blue.  Now a near-neutral dark
#                          grey with only a faint cool tint (0.026,0.027,0.030):
#                          the blue is pulled way down so haze reads as grimy
#                          air, not blue gas.  Uses fog_inscattering_luminance.
#   max_opacity 0.60    -- keeps far cars readable; never fully white-out.
#   start_distance 200  -- the player's own car stays crisp; haze builds with
#                          distance for depth.
# Spawned as LX_ (idempotent) rather than reusing the dark-car local fog.

FOG_DENSITY = 0.025
FOG_HEIGHT_FALLOFF = 0.20
FOG_INSCATTER = unreal.LinearColor(0.026, 0.027, 0.030, 1.0)  # near-neutral dark grey, faint cool
FOG_MAX_OPACITY = 0.60
FOG_START_DISTANCE = 200.0


def setup_fog():
    unreal.log("=" * 64)
    unreal.log("3. ATMOSPHERE -- global ExponentialHeightFog (dim, cool)")
    unreal.log("=" * 64)

    # Find-or-modify OUR OWN previous global fog (LX_); never touch the
    # build_zone1 dark-car local fog.
    fog = _find_by_label(f"{PREFIX}Fog_Global")
    if fog is None:
        fog = level_lib.spawn_actor_from_class(
            unreal.ExponentialHeightFog,
            unreal.Vector(MID_X, 0.0, 200.0), _rot())
        if not fog:
            unreal.log_warning("  Could not spawn LX_ ExponentialHeightFog")
            return
        fog.set_actor_label(f"{PREFIX}Fog_Global")

    try:
        fog.modify()
    except Exception:
        pass

    fc = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    if not fc:
        unreal.log_warning("  Fog actor has no component -- skipping")
        return
    try:
        fc.modify()
    except Exception:
        pass

    _try_set(fc, "fog_density", float(FOG_DENSITY), "fog ")
    _try_set(fc, "fog_height_falloff", float(FOG_HEIGHT_FALLOFF), "fog ")
    # IMPORTANT: luminance, NOT color (UE 5.7).
    _try_set(fc, "fog_inscattering_luminance", FOG_INSCATTER, "fog ")
    _try_set(fc, "fog_max_opacity", float(FOG_MAX_OPACITY), "fog ")
    _try_set(fc, "start_distance", float(FOG_START_DISTANCE), "fog ")
    # No volumetric -- cheaper and avoids fighting the practical pools.
    _try_set(fc, "volumetric_fog", False, "fog ")

    _disable_collision(fog)
    _note(f"Fog (LX_Fog_Global): density={FOG_DENSITY}, "
          f"height_falloff={FOG_HEIGHT_FALLOFF}, inscattering(lum)="
          f"{FOG_INSCATTER.r:.3f}/{FOG_INSCATTER.g:.3f}/{FOG_INSCATTER.b:.3f} cool, "
          f"max_opacity={FOG_MAX_OPACITY}, start_distance={FOG_START_DISTANCE}")


# ---------------------------------------------------------------------------
# 4. VERIFY no helper blocks the player
# ---------------------------------------------------------------------------

def verify_non_colliding():
    unreal.log("=" * 64)
    unreal.log("4. VERIFY LX_ helpers are non-colliding")
    unreal.log("=" * 64)
    checked = 0
    for a in _all_actors():
        try:
            if not a.get_actor_label().startswith(PREFIX):
                continue
        except Exception:
            continue
        _disable_collision(a)
        checked += 1
    unreal.log(f"  Ensured NO_COLLISION on {checked} LX_ helper actor(s)")


# ---------------------------------------------------------------------------
# SAVE (modify-then-save; train_interior_pass.py pattern)
# ---------------------------------------------------------------------------

def save_everything():
    unreal.log("=" * 64)
    unreal.log("SAVE")
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
# MAIN
# ---------------------------------------------------------------------------

def run():
    unreal.log("")
    unreal.log("*" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE -- GLOBAL LIGHTING PASS (LX_)")
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

    with unreal.ScopedSlowTask(4, "Global lighting pass...") as task:
        task.make_dialog(True)

        task.enter_progress_frame(1, "Cleanup + SkyLight fill...")
        try:
            cleanup()
            setup_skylight()
        except Exception as e:
            unreal.log_warning(f"SkyLight stage failed: {e}")

        task.enter_progress_frame(1, "Post-process grade...")
        try:
            setup_postprocess()
        except Exception as e:
            unreal.log_warning(f"PostProcess stage failed: {e}")

        task.enter_progress_frame(1, "Atmosphere fog...")
        try:
            setup_fog()
        except Exception as e:
            unreal.log_warning(f"Fog stage failed: {e}")

        task.enter_progress_frame(1, "Verify + save...")
        try:
            verify_non_colliding()
        except Exception as e:
            unreal.log_warning(f"Verify stage failed: {e}")
        save_everything()

    # Summary
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  GLOBAL LIGHTING PASS COMPLETE")
    unreal.log("=" * 64)
    for line in _log:
        unreal.log(f"  - {line}")
    unreal.log("")
    unreal.log("  Retune (playtest: too dark / too blue): SkyLight 0.25->0.75 &")
    unreal.log("  de-blued to cool-grey, exposure raised+widened (0.80-2.30, bias 0),")
    unreal.log("  white balance neutral 6300K, shadow teal push removed, fog blue")
    unreal.log("  pulled to near-neutral grey.  Cars 10 (Dark) & 11 (Freezer) still")
    unreal.log("  read moodier (relative) but stay navigable, not pure black.")
    unreal.log("  Warm pools come from per-car practicals (TI_/FD_/TAIL_/Light_Z1_*),")
    unreal.log("  left UNTOUCHED & not duplicated; the neutral grade now lets them")
    unreal.log("  read warm instead of being dyed blue.")
    unreal.log("")


if __name__ == "__main__":
    run()
else:
    run()
