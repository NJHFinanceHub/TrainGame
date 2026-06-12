"""
Snowpiercer: Eternal Engine -- Zone 1 TRAIN FEEL PASS
Run in editor (Tools > Execute Python Script) or headless via
-ExecutePythonScript="...Scripts/train_feel_pass.py".

Makes /Game/Maps/Zone1_Tail feel like a moving train:

  1. AUDIO    -- synthesizes 5 foley WAVs with pure stdlib (wave/struct/
                 math/random, 44100 Hz 16-bit mono) into a temp dir, then
                 imports them as SoundWaves under /Game/Audio/Foley:
                   SFX_Footstep_01..04  metal-deck footsteps (~0.22 s):
                                        noise transient (tau ~0.03 s) +
                                        80-120 Hz thump (tau ~0.06 s) +
                                        faint 600-900 Hz metallic ring
                                        (tau ~0.05 s); per-variant seeds.
                                        Normalized to 0.7 peak.
                   SFX_RailClack        seamless 1.4 s da-dum...da-dum
                                        rail-joint loop: paired 90 Hz
                                        thuds at t=0.0/0.12 + softer echo
                                        pair at 0.70/0.82 over a 30-60 Hz
                                        detuned-sine rumble bed; the bed
                                        is synthesized 0.1 s long and its
                                        tail crossfaded into the first
                                        0.1 s (sample-continuous wrap);
                                        thud tails wrap modulo length so
                                        the loop point is inaudible.
                                        Normalized to 0.6.
                                        looping=True set on this one only.
  2. MATERIALS -- M_TF_FrozenWindow: pale ice-blue (0.55,0.75,0.9)
                  emissive at ~1.2 intensity; if a suitable existing
                  texture is found (Environment / Metals), its R channel
                  drives subtle large-scale emissive variation via
                  Multiply/Add; flat emissive fallback otherwise.
                  M_TF_WindowFrame: reuses
                  /Game/Materials/Textured/M_Tex_trim_brushed_metal if it
                  exists, else a flat dark metal.
  3. WINDOWS  -- 4 windows per side wall on every car except Car 10
                 (Dark Car).  Each window: 4 frame strips (170x130 outer,
                 12 cm deep, protruding 8 cm into the room) + an inset
                 140x100 frozen-glass pane just inside the wall plane.
                 Centered z=170, x offsets -3600/-1200/+1200/+3600 from
                 car center (door zones at the +-6000 ends avoided).
                 Car 11 (Freezer Breach) gets one extra BROKEN window
                 (frame + pane rolled 12 deg, 6 emissive ice shards on
                 the floor beneath).  All window actors get collision
                 disabled -- the solid walls behind them keep blocking.

Idempotent: every placed actor's label starts with TF_ and all such
actors are destroyed at the start of each run; audio is re-synthesized
and re-imported with replace_existing; materials are rebuilt in place.

Ends with save_all_dirty_levels() AND save_current_level().
"""

import math
import os
import random
import shutil
import struct
import tempfile
import wave as wave_mod

import unreal

# ---------------------------------------------------------------------------
# API shortcuts (style of build_zone1.py / detail_pass_film.py)
# ---------------------------------------------------------------------------

editor_util = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mat_lib = unreal.MaterialEditingLibrary
level_lib = unreal.EditorLevelLibrary  # deprecated-but-functional in 5.7

# ---------------------------------------------------------------------------
# Constants (mirror build_zone1.py)
# ---------------------------------------------------------------------------

PERSISTENT_MAP = "/Game/Maps/Zone1_Tail"
CAR_LENGTH = 12000.0
CAR_WIDTH = 4000.0
CAR_HEIGHT = 3000.0
WALL_THICK = 150.0
CAR_STRIDE = 13000.0          # CAR_LENGTH + CAR_GAP
NUM_CARS = 15
HALF_L = CAR_LENGTH / 2.0     # 6000
HALF_W = CAR_WIDTH / 2.0      # 2000 = interior wall face |y|
                              # (walls span |y| 2000..2150, build_zone1)

PREFIX = "TF_"                # every actor this script places

AUDIO_DIR = "/Game/Audio/Foley"
MAT_TF_DIR = "/Game/Materials/TrainFeel"

SR = 44100                    # sample rate, 16-bit mono

# ---- Window geometry (cm) ----
WIN_Z = 170.0                 # window center height
FRAME_W, FRAME_H = 170.0, 130.0   # frame outer
FRAME_DEPTH = 12.0
FRAME_PROUD = 8.0             # how far the frame sticks into the room
PANE_W, PANE_H = 140.0, 100.0
PANE_DEPTH = 6.0
BORDER = 15.0                 # (170-140)/2 == (130-100)/2

# Frame spans y |1992 .. 2004| -> center 1998 (8 cm proud of the 2000 face)
FRAME_Y = HALF_W - FRAME_PROUD + FRAME_DEPTH / 2.0    # 1998
# Pane spans y |1998 .. 2004| -> just inside the wall plane, recessed
# 6 cm behind the frame's room face, visible through the frame opening
PANE_Y = HALF_W + 1.0                                  # 2001

WIN_X_OFFSETS = [-3600.0, -1200.0, 1200.0, 3600.0]   # |dx| <= 4200, even
SKIP_CARS = {10}              # Dark Car keeps its windowless dread
BROKEN_CAR = 11               # Freezer Breach
BROKEN_X_OFFSET = 2400.0      # right wall, between the regular windows

CAR_NAMES = {
    0: "Caboose", 1: "Tail Quarters A", 2: "Tail Quarters B", 3: "The Pit",
    4: "Nursery", 5: "Elders' Car", 6: "Sickbay", 7: "Workshop",
    8: "Listening Post", 9: "Blockade", 10: "Dark Car", 11: "Freezer Breach",
    12: "Kronole Den", 13: "Smuggler's Cache", 14: "Martyr's Gate",
}

rng = random.Random(1031)     # deterministic shard scatter

_win_counts = {i: 0 for i in range(NUM_CARS)}
_actors_placed = 0
_audio_imported = []
_audio_failed = []
_materials_built = []


# ---------------------------------------------------------------------------
# Small helpers
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
    except Exception as e:
        unreal.log_warning(f"  Could not save asset {asset_path}: {e}")


def _rot(pitch=0.0, yaw=0.0, roll=0.0):
    r = unreal.Rotator()
    r.set_editor_property("pitch", float(pitch))
    r.set_editor_property("yaw", float(yaw))
    r.set_editor_property("roll", float(roll))
    return r


def car_x(idx):
    return idx * CAR_STRIDE


_cube_mesh = None


def get_cube():
    global _cube_mesh
    if _cube_mesh is None:
        _cube_mesh = editor_util.load_asset("/Engine/BasicShapes/Cube")
    return _cube_mesh


def spawn_box(label, x, y, z, sx, sy, sz, mat=None,
              pitch=0.0, yaw=0.0, roll=0.0, collide=False):
    """Scaled cube StaticMeshActor; sizes in cm (engine cube is 100^3).
    Collision OFF by default -- the solid car walls stay authoritative."""
    global _actors_placed
    try:
        actor = level_lib.spawn_actor_from_class(
            unreal.StaticMeshActor, unreal.Vector(x, y, z),
            _rot(pitch, yaw, roll))
    except Exception as e:
        unreal.log_warning(f"  Spawn failed for {label}: {e}")
        return None
    if not actor:
        return None
    try:
        actor.modify()                 # mark for undo/dirty before mutating
    except Exception:
        pass
    actor.set_actor_label(f"{PREFIX}{label}")
    sm = actor.get_component_by_class(unreal.StaticMeshComponent)
    if sm:
        mesh = get_cube()
        if mesh:
            sm.set_static_mesh(mesh)
        actor.set_actor_scale3d(unreal.Vector(sx / 100.0, sy / 100.0,
                                              sz / 100.0))
        if mat:
            try:
                sm.set_material(0, mat)
            except Exception as e:
                unreal.log_warning(f"  Material assign failed on {label}: {e}")
        if not collide:
            try:
                sm.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
            except Exception:
                pass
    if not collide:
        try:
            actor.set_actor_enable_collision(False)
        except Exception:
            pass
    _actors_placed += 1
    return actor


# ===========================================================================
# PART 1 -- AUDIO SYNTHESIS (pure stdlib)
# ===========================================================================

def _normalize(samples, peak):
    m = max(abs(s) for s in samples) or 1.0
    k = peak / m
    return [s * k for s in samples]


def _write_wav(path, samples):
    """44100 Hz, 16-bit, mono."""
    frames = bytearray()
    for s in samples:
        v = int(max(-1.0, min(1.0, s)) * 32767.0)
        frames += struct.pack("<h", v)
    w = wave_mod.open(path, "wb")
    try:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(bytes(frames))
    finally:
        w.close()


def synth_footstep(seed):
    """Metal-deck footstep, ~0.22 s:
      - sharp transient: decaying noise burst, exp envelope tau ~0.03 s
      - low thump: 80-120 Hz damped sine, tau ~0.06 s
      - faint metallic ring: random 600-900 Hz damped sine, tau ~0.05 s
    Seeded per variant so the four differ.  Peak-normalized to 0.7."""
    r = random.Random(seed)
    dur = 0.22
    n = int(SR * dur)
    noise_tau = r.uniform(0.025, 0.035)
    noise_amp = r.uniform(0.8, 1.0)
    thump_f = r.uniform(80.0, 120.0)
    ring_f = r.uniform(600.0, 900.0)
    ring_amp = r.uniform(0.08, 0.16)
    ring_ph = r.uniform(0.0, math.pi)
    out = []
    for i in range(n):
        t = i / SR
        s = (r.uniform(-1.0, 1.0) * noise_amp * math.exp(-t / noise_tau)
             + 0.8 * math.sin(2.0 * math.pi * thump_f * t)
             * math.exp(-t / 0.06)
             + ring_amp * math.sin(2.0 * math.pi * ring_f * t + ring_ph)
             * math.exp(-t / 0.05))
        out.append(s)
    # 10 ms fade-out so the tail never clicks
    fade = int(SR * 0.01)
    for i in range(fade):
        out[n - 1 - i] *= i / float(fade)
    return _normalize(out, 0.7)


def synth_railclack():
    """Seamless 1.4 s da-dum...da-dum rail-joint loop.
    Rumble bed: 6 detuned 30-60 Hz sines, each with a slow (0.25-1.2 Hz)
    amplitude wobble.  Seamless construction: the bed is synthesized 0.1 s
    LONG (1.5 s) and its extra tail is crossfaded into the first 0.1 s,
    so bed[0] is the exact continuation of bed[n-1] -- the wrap is
    sample-continuous.  Thuds get a 2 ms attack ramp and their tails wrap
    modulo length, so the loop point itself is silent (the t=0 "da"
    landing there is the rhythm, not an artifact).  Peak-normalized 0.6."""
    r = random.Random(4242)
    dur = 1.4
    n = int(SR * dur)
    x = int(SR * 0.1)             # crossfade window

    # --- continuous very-low rumble bed (n + x samples: 0.1 s overrun) ---
    comps = []
    for _ in range(6):
        comps.append((r.uniform(30.0, 60.0),            # freq
                      r.uniform(0.0, 2.0 * math.pi),    # phase
                      r.uniform(0.25, 1.2),             # wobble rate
                      r.uniform(0.0, 2.0 * math.pi),    # wobble phase
                      r.uniform(0.05, 0.11)))           # amplitude
    raw = []
    for i in range(n + x):
        t = i / SR
        v = 0.0
        for f, ph, wf, wph, a in comps:
            wob = 0.75 + 0.25 * math.sin(2.0 * math.pi * wf * t + wph)
            v += a * wob * math.sin(2.0 * math.pi * f * t + ph)
        raw.append(v)

    # crossfade the overrun tail (last 0.1 s, samples n..n+x) into the
    # first 0.1 s, then truncate to n: bed[0] == raw[n] continues bed[n-1]
    bed = raw[:n]
    for i in range(x):
        w = i / float(x)
        bed[i] = bed[i] * w + raw[n + i] * (1.0 - w)

    # --- da-dum thud pairs (90 Hz damped sine + slight noise) ---
    def add_thud(t0, amp):
        tr = random.Random(int(t0 * 1000) + 7)
        start = int(t0 * SR)
        length = int(0.25 * SR)
        attack = int(0.002 * SR)     # 2 ms ramp: punchy but click-free
        for i in range(length):
            j = (start + i) % n          # wrap tails across the loop point
            t = i / SR
            env = math.exp(-t / 0.05)
            if i < attack:
                env *= i / float(attack)
            bed[j] += amp * env * (math.sin(2.0 * math.pi * 90.0 * t)
                                   + 0.15 * tr.uniform(-1.0, 1.0))

    add_thud(0.00, 0.55)     # da
    add_thud(0.12, 0.45)     # dum   (pair spacing 0.12 s)
    add_thud(0.70, 0.30)     # softer echo pair at t=0.7
    add_thud(0.82, 0.25)

    return _normalize(bed, 0.6)


def import_sound(src_path, name):
    """AssetImportTask import into /Game/Audio/Foley (import_music.py
    pattern).  Returns True on success."""
    dest = f"{AUDIO_DIR}/{name}"
    try:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", src_path)
        task.set_editor_property("destination_path", AUDIO_DIR)
        task.set_editor_property("destination_name", name)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)
        asset_tools.import_asset_tasks([task])
    except Exception as e:
        unreal.log_warning(f"  Import task failed for {name}: {e}")
        return False
    return editor_util.does_asset_exist(dest)


def build_audio():
    unreal.log("=" * 64)
    unreal.log("1. AUDIO -- synthesize + import foley")
    unreal.log("=" * 64)
    ensure_dir(AUDIO_DIR)

    tmp = tempfile.mkdtemp(prefix="tf_foley_")
    try:
        jobs = []
        for i in range(4):
            name = f"SFX_Footstep_{i + 1:02d}"
            jobs.append((name, synth_footstep(seed=101 + i), False))
        jobs.append(("SFX_RailClack", synth_railclack(), True))

        for name, samples, looping in jobs:
            wav_path = os.path.join(tmp, name + ".wav")
            try:
                _write_wav(wav_path, samples)
            except Exception as e:
                unreal.log_warning(f"  WAV write failed for {name}: {e}")
                _audio_failed.append(name)
                continue
            if import_sound(wav_path, name):
                _audio_imported.append(name)
                unreal.log(f"  Imported {AUDIO_DIR}/{name} "
                           f"({len(samples) / float(SR):.2f}s"
                           f"{', looping' if looping else ''})")
                if looping:
                    try:
                        snd = editor_util.load_asset(f"{AUDIO_DIR}/{name}")
                        if snd:
                            snd.modify()
                            snd.set_editor_property("looping", True)
                            editor_util.save_loaded_asset(snd)
                    except Exception as e:
                        unreal.log_warning(
                            f"  Could not set looping on {name}: {e}")
            else:
                _audio_failed.append(name)
                unreal.log_warning(f"  FAILED import: {name}")
    finally:
        try:
            shutil.rmtree(tmp, ignore_errors=True)
        except Exception:
            pass
    unreal.log(f"  Audio: {len(_audio_imported)} imported, "
               f"{len(_audio_failed)} failed")


# ===========================================================================
# PART 2a -- MATERIALS
# ===========================================================================

# Existing textures (imported by import_assets.py / detail passes) that
# could drive the subtle emissive variation -- tried in order.
NOISE_TEX_CANDIDATES = [
    "/Game/Textures/Environment/crepe_satin_diff_4k",
    "/Game/Textures/Metals/brushed_metal",
    "/Game/Textures/Metals/dirty_metal",
    "/Game/Textures/Metals/metals_norm",
]

ICE_BLUE = (0.55, 0.75, 0.9)
EMISSIVE_INTENSITY = 1.2


def _find_noise_texture():
    for path in NOISE_TEX_CANDIDATES:
        try:
            if editor_util.does_asset_exist(path):
                tex = editor_util.load_asset(path)
                if tex and isinstance(tex, unreal.Texture):
                    return tex, path
        except Exception:
            continue
    return None, None


def _fresh_material(name):
    """Create or clear-and-reuse a material under /Game/Materials/TrainFeel.
    Returns (material, was_cleared_ok)."""
    ensure_dir(MAT_TF_DIR)
    full = f"{MAT_TF_DIR}/{name}"
    if editor_util.does_asset_exist(full):
        mat = editor_util.load_asset(full)
        if not mat:
            return None, False
        try:
            mat.modify()
            mat_lib.delete_all_material_expressions(mat)
        except Exception as e:
            unreal.log_warning(f"  Could not clear {name} ({e}) -- "
                               f"reusing as-is")
            return mat, False
        return mat, True
    mat = asset_tools.create_asset(name, MAT_TF_DIR, unreal.Material,
                                   unreal.MaterialFactoryNew())
    if not mat:
        unreal.log_warning(f"  Failed to create material: {name}")
        return None, False
    try:
        mat.modify()
    except Exception:
        pass
    return mat, True


def build_frozen_window_material():
    """Pale ice-blue emissive at ~1.2 intensity.  Subtle large-scale
    variation from an existing texture's R channel via Multiply/Add when
    one is available; flat emissive fallback.  Unlit-looking."""
    name = "M_TF_FrozenWindow"
    mat, fresh = _fresh_material(name)
    if not mat:
        return None
    if not fresh:                       # could not rewire; reuse as-is
        return mat

    er = ICE_BLUE[0] * EMISSIVE_INTENSITY
    eg = ICE_BLUE[1] * EMISSIVE_INTENSITY
    eb = ICE_BLUE[2] * EMISSIVE_INTENSITY

    em_node = None
    try:
        em_node = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionConstant3Vector, -700, -200)
        em_node.set_editor_property(
            "constant", unreal.LinearColor(er, eg, eb, 1.0))
    except Exception as e:
        unreal.log_warning(f"  Emissive constant failed: {e}")

    wired_emissive = False
    tex, tex_path = _find_noise_texture()
    if em_node and tex:
        # emissive = color * (0.75 + 0.5 * tex.R) with large-scale UVs
        try:
            texcoord = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionTextureCoordinate, -1000, 100)
            texcoord.set_editor_property("u_tiling", 0.25)
            texcoord.set_editor_property("v_tiling", 0.25)
            sample = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionTextureSample, -800, 100)
            sample.set_editor_property("texture", tex)
            # If the candidate is a normal map, sample it as a normal so
            # the sampler/compression types agree (mismatch fails compile)
            if "_norm" in tex_path.lower():
                sample.set_editor_property(
                    "sampler_type",
                    unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
            mat_lib.connect_material_expressions(texcoord, "", sample, "UVs")

            mul1 = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionMultiply, -550, 100)
            mat_lib.connect_material_expressions(sample, "R", mul1, "A")
            mul1.set_editor_property("const_b", 0.5)

            add1 = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionAdd, -400, 100)
            mat_lib.connect_material_expressions(mul1, "", add1, "A")
            add1.set_editor_property("const_b", 0.75)

            mul2 = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionMultiply, -250, -50)
            mat_lib.connect_material_expressions(em_node, "", mul2, "A")
            mat_lib.connect_material_expressions(add1, "", mul2, "B")
            mat_lib.connect_material_property(
                mul2, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            wired_emissive = True
            unreal.log(f"  FrozenWindow emissive varied by {tex_path}")
        except Exception as e:
            unreal.log_warning(f"  Texture-noise wiring failed ({e}) -- "
                               f"flat emissive fallback")
            wired_emissive = False

    if not wired_emissive and em_node:
        try:
            mat_lib.connect_material_property(
                em_node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            wired_emissive = True
            unreal.log("  FrozenWindow: flat emissive")
        except Exception as e:
            unreal.log_warning(f"  Flat emissive connect failed: {e}")

    # Base color (ignored if unlit sticks) + zero roughness influence
    try:
        base = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionConstant3Vector, -700, 250)
        base.set_editor_property(
            "constant", unreal.LinearColor(ICE_BLUE[0] * 0.2,
                                           ICE_BLUE[1] * 0.2,
                                           ICE_BLUE[2] * 0.2, 1.0))
        mat_lib.connect_material_property(
            base, "", unreal.MaterialProperty.MP_BASE_COLOR)
        rough = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionConstant, -700, 450)
        rough.set_editor_property("r", 1.0)
        mat_lib.connect_material_property(
            rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    except Exception as e:
        unreal.log_warning(f"  Base/roughness wiring failed: {e}")

    # Unlit so the panes glow flatly regardless of lighting (defensive;
    # a lit material with emissive still reads fine if this fails)
    if wired_emissive:
        try:
            mat.set_editor_property(
                "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        except Exception as e:
            unreal.log_warning(f"  Could not set unlit shading model: {e}")

    try:
        mat_lib.recompile_material(mat)
    except Exception as e:
        unreal.log_warning(f"  Recompile failed for {name}: {e}")
    safe_save_asset(f"{MAT_TF_DIR}/{name}")
    _materials_built.append(name)
    return mat


def build_frame_material():
    """Dark metal for the window frames.  Reuses the existing brushed
    trim material when it is on disk."""
    reuse = "/Game/Materials/Textured/M_Tex_trim_brushed_metal"
    try:
        if editor_util.does_asset_exist(reuse):
            mat = editor_util.load_asset(reuse)
            if mat:
                _materials_built.append("M_Tex_trim_brushed_metal (reused)")
                unreal.log(f"  Frame material: reusing {reuse}")
                return mat
    except Exception:
        pass

    name = "M_TF_WindowFrame"
    mat, fresh = _fresh_material(name)
    if not mat:
        return None
    if fresh:
        try:
            base = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionConstant3Vector, -300, -200)
            base.set_editor_property(
                "constant", unreal.LinearColor(0.04, 0.04, 0.045, 1.0))
            mat_lib.connect_material_property(
                base, "", unreal.MaterialProperty.MP_BASE_COLOR)
            rough = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionConstant, -300, 0)
            rough.set_editor_property("r", 0.45)
            mat_lib.connect_material_property(
                rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
            metal = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionConstant, -300, 150)
            metal.set_editor_property("r", 0.9)
            mat_lib.connect_material_property(
                metal, "", unreal.MaterialProperty.MP_METALLIC)
        except Exception as e:
            unreal.log_warning(f"  Frame material wiring failed: {e}")
        try:
            mat_lib.recompile_material(mat)
        except Exception as e:
            unreal.log_warning(f"  Recompile failed for {name}: {e}")
        safe_save_asset(f"{MAT_TF_DIR}/{name}")
    _materials_built.append(name)
    unreal.log(f"  Frame material: {MAT_TF_DIR}/{name}")
    return mat


# ===========================================================================
# PART 2b -- WINDOWS
# ===========================================================================

def build_window(car_idx, side, dx, frozen_mat, frame_mat, win_tag):
    """One window: 4 frame strips (so the pane shows through the opening)
    + the inset frozen pane.  side = +1 left wall (+y), -1 right wall.
    Geometry (left wall, mirrored for right):
      frame strips  y 1992..2004 (center 1998) -- 8 cm proud of the
                    interior wall face at y=2000, 4 cm into the wall
      pane          y 1998..2004 (center 2001) -- just inside the wall
                    plane, recessed behind the frame's room face
    Opening is 140 x 100; border strips are 15 cm wide."""
    wx = car_x(car_idx) + dx
    fy = side * FRAME_Y
    py = side * PANE_Y
    s = "L" if side > 0 else "R"
    tag = f"Win_C{car_idx:02d}_{s}{win_tag}"

    half_open_h = PANE_H / 2.0          # 50
    half_open_w = PANE_W / 2.0          # 70
    strip_zc = half_open_h + BORDER / 2.0   # 57.5

    # Top / bottom strips (full frame width)
    spawn_box(f"{tag}_FrameT", wx, fy, WIN_Z + strip_zc,
              FRAME_W, FRAME_DEPTH, BORDER, frame_mat)
    spawn_box(f"{tag}_FrameB", wx, fy, WIN_Z - strip_zc,
              FRAME_W, FRAME_DEPTH, BORDER, frame_mat)
    # Left / right strips (opening height)
    strip_xc = half_open_w + BORDER / 2.0   # 77.5
    spawn_box(f"{tag}_FrameL", wx - strip_xc, fy, WIN_Z,
              BORDER, FRAME_DEPTH, PANE_H, frame_mat)
    spawn_box(f"{tag}_FrameR", wx + strip_xc, fy, WIN_Z,
              BORDER, FRAME_DEPTH, PANE_H, frame_mat)
    # Frozen-glass pane
    spawn_box(f"{tag}_Pane", wx, py, WIN_Z,
              PANE_W, PANE_DEPTH, PANE_H, frozen_mat)

    _win_counts[car_idx] += 1


def build_broken_window(frozen_mat, frame_mat):
    """Car 11 Freezer Breach: one window knocked askew -- slab frame +
    pane rolled 12 deg, popped proud of the wall, with 6 small emissive
    ice shards scattered on the floor beneath."""
    idx = BROKEN_CAR
    wx = car_x(idx) + BROKEN_X_OFFSET
    side = -1.0                          # right wall, same side as breach
    tag = f"Win_C{idx:02d}_R_BROKEN"

    # Slab frame, tilted (broken: pane in front of the slab reads as
    # glass knocked out of its seat)
    spawn_box(f"{tag}_Frame", wx, side * FRAME_Y, WIN_Z,
              FRAME_W, FRAME_DEPTH, FRAME_H, frame_mat, roll=12.0)
    spawn_box(f"{tag}_Pane", wx, side * (HALF_W - 12.0), WIN_Z - 8.0,
              PANE_W, PANE_DEPTH, PANE_H, frozen_mat, roll=12.0, yaw=-4.0)

    # Ice shard scatter on the floor beneath
    n_shards = rng.choice([5, 6])
    for i in range(n_shards):
        s = rng.uniform(10.0, 22.0)
        spawn_box(f"{tag}_Shard{i}",
                  wx + rng.uniform(-160.0, 160.0),
                  side * (HALF_W - rng.uniform(60.0, 320.0)),
                  s / 2.0,
                  s, s * rng.uniform(0.6, 1.0), s * rng.uniform(0.3, 0.7),
                  frozen_mat,
                  pitch=rng.uniform(-20.0, 20.0),
                  yaw=rng.uniform(0.0, 360.0),
                  roll=rng.uniform(-20.0, 20.0))

    _win_counts[idx] += 1
    unreal.log(f"  Car 11 broken window at x={wx:.0f} "
               f"(+{n_shards} ice shards)")


def build_all_windows(frozen_mat, frame_mat):
    unreal.log("=" * 64)
    unreal.log("3. WINDOWS -- frozen world outside")
    unreal.log("=" * 64)
    for idx in range(NUM_CARS):
        if idx in SKIP_CARS:
            unreal.log(f"  Car {idx:02d} {CAR_NAMES[idx]:20s} skipped "
                       f"(windowless by design)")
            continue
        try:
            for wi, dx in enumerate(WIN_X_OFFSETS):
                build_window(idx, +1, dx, frozen_mat, frame_mat, wi)
                build_window(idx, -1, dx, frozen_mat, frame_mat, wi)
        except Exception as e:
            unreal.log_error(f"  Car {idx:02d} windows failed: {e}")
        unreal.log(f"  Car {idx:02d} {CAR_NAMES[idx]:20s} "
                   f"{_win_counts[idx]} windows")
    try:
        build_broken_window(frozen_mat, frame_mat)
    except Exception as e:
        unreal.log_error(f"  Broken window failed: {e}")


# ---------------------------------------------------------------------------
# Cleanup / level load / save (detail_pass_film.py patterns)
# ---------------------------------------------------------------------------

def load_zone1():
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        world = level_lib.get_editor_world()
        if not world or "Zone1_Tail" not in world.get_path_name():
            unreal.log(f"  Loading {PERSISTENT_MAP} ...")
            if not editor_util.does_asset_exist(PERSISTENT_MAP):
                unreal.log_error(f"  Map not found: {PERSISTENT_MAP} -- "
                                 f"run build_zone1.py first")
                return False
            les.load_level(PERSISTENT_MAP)
        return True
    except Exception as e:
        unreal.log_error(f"  Could not load level: {e}")
        return False


def cleanup():
    unreal.log("=" * 64)
    unreal.log("0. CLEANUP previous TF_ actors")
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
    unreal.log(f"  Removed {removed} TF_ actors")


def save_everything():
    unreal.log("=" * 64)
    unreal.log("4. SAVE")
    unreal.log("=" * 64)
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        les.save_all_dirty_levels()
        # Belt and suspenders: the dirty flag can be lost on headless
        # property-only mutations -- force-save the current level too.
        try:
            les.save_current_level()
        except Exception:
            pass
        unreal.log("  Saved all dirty levels + current level")
    except Exception as e:
        unreal.log_warning(f"  Level save failed: {e}")
    for d in (AUDIO_DIR, MAT_TF_DIR):
        try:
            if editor_util.does_directory_exist(d):
                editor_util.save_directory(d, only_if_is_dirty=True,
                                           recursive=True)
        except Exception as e:
            unreal.log_warning(f"  save_directory failed for {d}: {e}")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def run():
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  Zone 1 TRAIN FEEL PASS (foley + frozen windows)")
    unreal.log("=" * 64)
    unreal.log("")

    if not load_zone1():
        return

    cleanup()
    build_audio()

    unreal.log("=" * 64)
    unreal.log("2. MATERIALS")
    unreal.log("=" * 64)
    frozen_mat = build_frozen_window_material()
    frame_mat = build_frame_material()
    if not frozen_mat:
        unreal.log_error("  No frozen-window material -- aborting windows")
        save_everything()
        return

    build_all_windows(frozen_mat, frame_mat)
    save_everything()

    total_windows = sum(_win_counts.values())
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  TRAIN FEEL PASS COMPLETE")
    unreal.log("=" * 64)
    unreal.log(f"  Audio assets imported:  {len(_audio_imported)} "
               f"({', '.join(_audio_imported) if _audio_imported else 'none'})")
    if _audio_failed:
        unreal.log_warning(f"  Audio assets FAILED:    "
                           f"{', '.join(_audio_failed)}")
    unreal.log("  SFX_RailClack is the only looping sound.")
    unreal.log(f"  Materials built/reused: {len(_materials_built)} "
               f"({', '.join(_materials_built)})")
    unreal.log(f"  Windows placed:         {total_windows} total "
               f"(Car 11 includes 1 broken)")
    unreal.log("  Windows per car:")
    for idx in range(NUM_CARS):
        note = "  (skipped -- Dark Car)" if idx in SKIP_CARS else ""
        unreal.log(f"    [{idx:02d}] {CAR_NAMES[idx]:20s} "
                   f"{_win_counts[idx]:3d}{note}")
    unreal.log(f"  TF_ actors placed:      {_actors_placed}")
    unreal.log("  Levels + /Game/Audio/Foley + /Game/Materials/TrainFeel "
               "saved.")
    unreal.log("")


run()
