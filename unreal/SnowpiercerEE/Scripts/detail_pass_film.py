"""
Snowpiercer: Eternal Engine -- Zone 1 FILM DETAIL PASS
Run in editor (Tools > Execute Python Script) or headless via
-ExecutePythonScript="...Scripts/detail_pass_film.py".

Art-directs /Game/Maps/Zone1_Tail to match the visual language of
Bong Joon-ho's Snowpiercer (2013).  The Tail is dark, monochromatic,
claustrophobically cluttered (Hong Kong slum density); light comes from
scattered warm practicals -- candle / barrel glow, work lamps -- never
ceiling floods.  Cold blue ONLY in the Freezer Breach (car 11).

What it does:
  1. IMPORT     -- new raw wood / fabric textures from Assets/Textures
                   into /Game/Textures/{Wood,Fabric} (AssetImportTask,
                   import_assets.py pattern; normals TC_NORMALMAP,
                   roughness TC_MASKS, sRGB off).
  2. MATERIALS  -- M_Tex_wood_planks_grey / M_Tex_wood_worn_planks /
                   M_Tex_fabric_pattern_07 under /Game/Materials/Textured
                   (upgrade_textures.py wiring: normals SAMPLERTYPE_NORMAL,
                   rough maps SAMPLERTYPE_MASKS -- Linear Color samplers on
                   masks-compressed textures fail the whole compile).
                   Plus a small set of flat/emissive dressing materials
                   under /Game/Materials/FilmDetail (fire glow, kronole
                   glow, protein block, blood, sandbag, rivet metal...).
  3. LIGHTING   -- dims the generic ceiling lights (Light_Z1_Car##_*)
                   in cars 0-2, 4-5, 10 via modify()+set (idempotent:
                   clamps to a fixed target, never multiplies), warms
                   their color, then adds per-car warm practicals.
  4. DRESSING   -- per-car film set pieces, all label-prefixed FD_:
                   bunk slums + cloth (1,2,4), protein kitchen (2),
                   pit ring ropes (3), memorial (0), elder candles (5),
                   checkpoint riot dressing (8,9), kronole den (12),
                   Martyr's Gate (14), pipes/valves/debris everywhere.
  5. SAVE       -- save_all_dirty_levels() AND save_current_level(),
                   plus the material/texture directories.

Idempotent: every placed actor's label starts with FD_ and all of them
are destroyed at the start of each run.  Light dimming clamps to fixed
targets.  Textures are imported only if missing; materials are rebuilt
in place (expressions cleared and rewired).

Keeps the player corridor (y in [-800, 800]) walkable -- no new
floor-level collision in the lane except designated set pieces that
already own their car (pit ring surface decals, blockade dressing).

Budget: total new actors capped at 900.
"""

import os
import random
import re

import unreal

# ---------------------------------------------------------------------------
# API shortcuts (style of build_zone1.py / upgrade_textures.py)
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
CAR_STRIDE = 13000.0          # CAR_LENGTH + CAR_GAP
NUM_CARS = 15
HALF_L = CAR_LENGTH / 2.0     # 6000
HALF_W = CAR_WIDTH / 2.0      # 2000

PREFIX = "FD_"                # every actor this script places
MAX_NEW_ACTORS = 900

MAT_TEX_DIR = "/Game/Materials/Textured"
MAT_FD_DIR = "/Game/Materials/FilmDetail"
TEX_WOOD_DIR = "/Game/Textures/Wood"
TEX_FABRIC_DIR = "/Game/Textures/Fabric"
TEX_METALS_DIR = "/Game/Textures/Metals"

# Repo root: Scripts/ -> SnowpiercerEE/ -> unreal/ -> repo root
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)
REPO_ROOT = os.path.dirname(os.path.dirname(PROJECT_DIR))
ASSETS_DIR = os.path.join(REPO_ROOT, "Assets")

# Deterministic jitter -> identical layout on every run
rng = random.Random(2013)     # Snowpiercer (2013)

# Warm practical color (film: candle / barrel / work-lamp glow)
WARM = (255, 153, 76)         # 1.0, 0.6, 0.3

_car_counts = {i: 0 for i in range(NUM_CARS)}
_total_new = 0
_budget_blown = False
_dimmed = 0
_materials_created = []       # names, for the summary

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
    """Build a Rotator without relying on constructor argument order."""
    r = unreal.Rotator()
    r.set_editor_property("pitch", float(pitch))
    r.set_editor_property("yaw", float(yaw))
    r.set_editor_property("roll", float(roll))
    return r


def car_x(idx):
    return idx * CAR_STRIDE


def _car_idx_for_x(x):
    return max(0, min(NUM_CARS - 1, int((x + CAR_STRIDE / 2.0) // CAR_STRIDE)))


def _count_new(x):
    global _total_new
    _total_new += 1
    _car_counts[_car_idx_for_x(x)] += 1


def _budget_ok(x):
    global _budget_blown
    if _total_new >= MAX_NEW_ACTORS:
        if not _budget_blown:
            unreal.log_warning(f"  ACTOR BUDGET ({MAX_NEW_ACTORS}) reached -- "
                               f"skipping further spawns")
            _budget_blown = True
        return False
    return True


# ---------------------------------------------------------------------------
# Shape / spawn helpers
# ---------------------------------------------------------------------------

_shape_cache = {}


def _shape(name):
    if name not in _shape_cache:
        _shape_cache[name] = editor_util.load_asset(f"/Engine/BasicShapes/{name}")
    return _shape_cache[name]


def spawn_shape(shape, label, x, y, z, sx, sy, sz, mat=None,
                pitch=0.0, yaw=0.0, roll=0.0):
    """Spawn a scaled basic-shape StaticMeshActor.  Sizes in cm
    (engine basic shapes are 100x100x100)."""
    if not _budget_ok(x):
        return None
    actor = level_lib.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, z), _rot(pitch, yaw, roll))
    if not actor:
        return None
    actor.set_actor_label(f"{PREFIX}{label}")
    sm = actor.get_component_by_class(unreal.StaticMeshComponent)
    if sm:
        mesh = _shape(shape)
        if mesh:
            sm.set_static_mesh(mesh)
        actor.set_actor_scale3d(unreal.Vector(sx / 100.0, sy / 100.0, sz / 100.0))
        if mat:
            sm.set_material(0, mat)
    _count_new(x)
    return actor


def spawn_box(label, x, y, z, sx, sy, sz, mat=None, pitch=0.0, yaw=0.0, roll=0.0):
    return spawn_shape("Cube", label, x, y, z, sx, sy, sz, mat, pitch, yaw, roll)


def spawn_cyl(label, x, y, z, dia, height, mat=None, pitch=0.0, yaw=0.0, roll=0.0):
    return spawn_shape("Cylinder", label, x, y, z, dia, dia, height, mat,
                       pitch, yaw, roll)


def spawn_sphere(label, x, y, z, sx, sy, sz, mat=None, yaw=0.0):
    return spawn_shape("Sphere", label, x, y, z, sx, sy, sz, mat, 0.0, yaw, 0.0)


def spawn_light(label, x, y, z, color_rgb, intensity, radius):
    """Warm practical point light."""
    if not _budget_ok(x):
        return None
    pl = level_lib.spawn_actor_from_class(
        unreal.PointLight, unreal.Vector(x, y, z), _rot())
    if not pl:
        return None
    pl.set_actor_label(f"{PREFIX}{label}")
    plc = pl.get_component_by_class(unreal.PointLightComponent)
    if plc:
        plc.set_editor_property("intensity", float(intensity))
        plc.set_editor_property("attenuation_radius", float(radius))
        plc.set_editor_property("light_color", unreal.Color(
            int(color_rgb[0]), int(color_rgb[1]), int(color_rgb[2]), 255))
        # soft shadows are wasted on tiny practicals
        try:
            plc.set_editor_property("cast_shadows", False)
        except Exception:
            pass
    _count_new(x)
    return pl


def practical_intensity():
    """Per-spec 800-1500, varied so the run of practicals doesn't read flat
    (flicker needs a BP -- intensity variation is the static stand-in)."""
    return rng.uniform(800.0, 1500.0)


# ---------------------------------------------------------------------------
# 0. CLEANUP -- destroy everything from previous FD_ runs
# ---------------------------------------------------------------------------

def cleanup():
    unreal.log("=" * 64)
    unreal.log("0. CLEANUP previous FD_ actors")
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
    unreal.log(f"  Removed {removed} FD_ actors")


# ---------------------------------------------------------------------------
# 1. TEXTURE IMPORT (import_assets.py AssetImportTask pattern)
# ---------------------------------------------------------------------------

NEW_TEXTURE_FILES = [
    # (disk subdir, content dir, filename)
    ("Wood",   TEX_WOOD_DIR,   "wood_planks_grey_diff_2k.jpg"),
    ("Wood",   TEX_WOOD_DIR,   "wood_planks_grey_nor_gl_2k.jpg"),
    ("Wood",   TEX_WOOD_DIR,   "wood_planks_grey_rough_2k.jpg"),
    ("Wood",   TEX_WOOD_DIR,   "worn_planks_diff_2k.jpg"),
    ("Wood",   TEX_WOOD_DIR,   "worn_planks_nor_gl_2k.jpg"),
    ("Wood",   TEX_WOOD_DIR,   "worn_planks_rough_2k.jpg"),
    ("Fabric", TEX_FABRIC_DIR, "fabric_pattern_07_diff_2k.jpg"),
    ("Fabric", TEX_FABRIC_DIR, "fabric_pattern_07_nor_gl_2k.jpg"),
    ("Fabric", TEX_FABRIC_DIR, "fabric_pattern_07_rough_2k.jpg"),
]


def _classify_new_tex(filename):
    low = filename.lower()
    if "_nor_gl_" in low or "_nor_" in low:
        return "normal"
    if "_rough_" in low:
        return "roughness"
    return "diffuse"


def _set_tex_compression(asset_path, tex_type):
    """Normals -> TC_NORMALMAP, roughness -> TC_MASKS, both sRGB off.
    (Mismatched sampler/compression fails the whole material compile.)"""
    try:
        tex = editor_util.load_asset(asset_path)
        if not tex:
            return
        tex.modify()
        if tex_type == "normal":
            tex.set_editor_property(
                "compression_settings",
                unreal.TextureCompressionSettings.TC_NORMALMAP)
            tex.set_editor_property("srgb", False)
        elif tex_type == "roughness":
            tex.set_editor_property(
                "compression_settings",
                unreal.TextureCompressionSettings.TC_MASKS)
            tex.set_editor_property("srgb", False)
        editor_util.save_loaded_asset(tex)
    except Exception as e:
        unreal.log_warning(f"  Compression setup failed for {asset_path}: {e}")


def _import_one_texture(source_path, dest_dir):
    ensure_dir(dest_dir)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source_path)
    task.set_editor_property("destination_path", dest_dir)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    asset_tools.import_asset_tasks([task])
    imported = task.get_editor_property("imported_object_paths")
    if imported:
        return str(imported[0]).split(".")[0]
    unreal.log_warning(f"  Texture import returned no paths: {source_path}")
    return None


def import_new_textures():
    unreal.log("=" * 64)
    unreal.log("1. IMPORT new wood / fabric textures")
    unreal.log("=" * 64)
    imported = 0
    skipped = 0
    for subdir, dest_dir, filename in NEW_TEXTURE_FILES:
        stem = os.path.splitext(filename)[0]
        asset_path = f"{dest_dir}/{stem}"
        tex_type = _classify_new_tex(filename)
        if editor_util.does_asset_exist(asset_path):
            skipped += 1
            continue
        src = os.path.join(ASSETS_DIR, "Textures", subdir, filename)
        if not os.path.exists(src):
            unreal.log_warning(f"  Source texture missing on disk: {src}")
            continue
        result = _import_one_texture(src, dest_dir)
        if result:
            _set_tex_compression(result, tex_type)
            imported += 1
            unreal.log(f"  Imported {result} ({tex_type})")
    unreal.log(f"  Textures: {imported} imported, {skipped} already present")


def ensure_stripes_texture():
    """Warning-stripe texture for checkpoint dressing.  Try /Game/Textures
    first; the raw JPG exists on disk but was never in import_assets.py's
    list, so import it if absent.  Returns asset path or None."""
    candidates = [f"{TEX_METALS_DIR}/stripes", "/Game/Textures/stripes"]
    for c in candidates:
        if editor_util.does_asset_exist(c):
            return c
    src = os.path.join(ASSETS_DIR, "Textures", "Metals", "stripes.JPG")
    if os.path.exists(src):
        return _import_one_texture(src, TEX_METALS_DIR)
    return None


# ---------------------------------------------------------------------------
# 2a. TEXTURED MATERIALS (upgrade_textures.py wiring pattern)
# ---------------------------------------------------------------------------

def _add_sample(mat, tex, x, y, sampler_type=None, texcoord_node=None):
    node = mat_lib.create_material_expression(
        mat, unreal.MaterialExpressionTextureSample, x, y)
    if not node:
        return None
    try:
        node.set_editor_property("texture", tex)
    except Exception as e:
        unreal.log_warning(f"    Could not set texture on sample: {e}")
        return None
    if sampler_type is not None:
        try:
            node.set_editor_property("sampler_type", sampler_type)
        except Exception as e:
            unreal.log_warning(f"    Could not set sampler type: {e}")
    if texcoord_node:
        try:
            mat_lib.connect_material_expressions(texcoord_node, "", node, "UVs")
        except Exception as e:
            unreal.log_warning(f"    Could not connect TexCoord: {e}")
    return node


def _load_texture(path):
    try:
        if path and editor_util.does_asset_exist(path):
            tex = editor_util.load_asset(path)
            if tex and isinstance(tex, unreal.Texture):
                return tex
    except Exception:
        pass
    return None


def build_textured_material(mat_name, channels, tiling=1.0, metallic=0.0,
                            roughness_fallback=0.85):
    """channels = {"diffuse": path, "normal": path, "roughness": path}.
    Normals sample as SAMPLERTYPE_NORMAL, roughness as SAMPLERTYPE_MASKS
    (Linear Color samplers on masks-compressed textures fail the compile).
    Idempotent: existing material is cleared and rewired."""
    ensure_dir(MAT_TEX_DIR)
    mat_full = f"{MAT_TEX_DIR}/{mat_name}"

    existed = editor_util.does_asset_exist(mat_full)
    if existed:
        mat = editor_util.load_asset(mat_full)
        if not mat:
            unreal.log_warning(f"  Could not load existing {mat_full}")
            return None
        try:
            mat_lib.delete_all_material_expressions(mat)
        except Exception as e:
            unreal.log_warning(f"  Could not clear {mat_name} ({e}) -- "
                               f"reusing as-is")
            return mat
    else:
        mat = asset_tools.create_asset(
            mat_name, MAT_TEX_DIR, unreal.Material, unreal.MaterialFactoryNew())
        if not mat:
            unreal.log_warning(f"  Failed to create material: {mat_name}")
            return None

    texcoord = None
    try:
        texcoord = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionTextureCoordinate, -800, 0)
        if texcoord:
            texcoord.set_editor_property("u_tiling", float(tiling))
            texcoord.set_editor_property("v_tiling", float(tiling))
    except Exception as e:
        unreal.log_warning(f"  TexCoord setup failed for {mat_name}: {e}")
        texcoord = None

    wired = 0

    diff_tex = _load_texture(channels.get("diffuse"))
    if diff_tex:
        node = _add_sample(mat, diff_tex, -450, -250, texcoord_node=texcoord)
        if node:
            try:
                mat_lib.connect_material_property(
                    node, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
                wired += 1
            except Exception as e:
                unreal.log_warning(f"    BaseColor connect failed: {e}")

    norm_tex = _load_texture(channels.get("normal"))
    if norm_tex:
        node = _add_sample(
            mat, norm_tex, -450, 0,
            sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL,
            texcoord_node=texcoord)
        if node:
            try:
                mat_lib.connect_material_property(
                    node, "RGB", unreal.MaterialProperty.MP_NORMAL)
                wired += 1
            except Exception as e:
                unreal.log_warning(f"    Normal connect failed: {e}")

    rough_tex = _load_texture(channels.get("roughness"))
    if rough_tex:
        node = _add_sample(
            mat, rough_tex, -450, 250,
            sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_MASKS,
            texcoord_node=texcoord)
        if node:
            try:
                mat_lib.connect_material_property(
                    node, "R", unreal.MaterialProperty.MP_ROUGHNESS)
                wired += 1
            except Exception as e:
                unreal.log_warning(f"    Roughness connect failed: {e}")
    else:
        try:
            const = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionConstant, -450, 250)
            if const:
                const.set_editor_property("r", float(roughness_fallback))
                mat_lib.connect_material_property(
                    const, "", unreal.MaterialProperty.MP_ROUGHNESS)
        except Exception as e:
            unreal.log_warning(f"    Roughness constant failed: {e}")

    try:
        const = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionConstant, -450, 500)
        if const:
            const.set_editor_property("r", float(metallic))
            mat_lib.connect_material_property(
                const, "", unreal.MaterialProperty.MP_METALLIC)
    except Exception as e:
        unreal.log_warning(f"    Metallic constant failed: {e}")

    if wired == 0:
        unreal.log_warning(f"  {mat_name}: nothing wired -- check texture paths")
    try:
        mat_lib.recompile_material(mat)
    except Exception as e:
        unreal.log_warning(f"  Recompile failed for {mat_name}: {e}")
    safe_save_asset(mat_full)
    _materials_created.append(mat_name + ("" if not existed else " (rewired)"))
    unreal.log(f"  {'Rewired' if existed else 'Created'} {mat_full} "
               f"({wired} texture channels, tiling x{tiling})")
    return mat


# ---------------------------------------------------------------------------
# 2b. FLAT / EMISSIVE DRESSING MATERIALS (build_zone1.get_material pattern)
# ---------------------------------------------------------------------------

_fd_mat_cache = {}


def get_fd_material(name, base, roughness=0.7, metallic=0.0, emissive=None):
    """Get or create a small flat PBR material under /Game/Materials/FilmDetail.
    emissive = (r, g, b) HDR values (>1 glows)."""
    if name in _fd_mat_cache:
        return _fd_mat_cache[name]
    ensure_dir(MAT_FD_DIR)
    mat_path = f"{MAT_FD_DIR}/{name}"
    if editor_util.does_asset_exist(mat_path):
        mat = editor_util.load_asset(mat_path)
        _fd_mat_cache[name] = mat
        return mat

    mat = asset_tools.create_asset(name, MAT_FD_DIR,
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
    _materials_created.append(name)
    unreal.log(f"  Created {mat_path}")
    _fd_mat_cache[name] = mat
    return mat


def build_stripes_material():
    """Warning stripes for checkpoint dressing.  Textured if the stripes
    JPG can be found/imported, else a flat grimy hazard yellow."""
    tex_path = ensure_stripes_texture()
    if tex_path:
        mat = build_textured_material(
            "M_Tex_stripes_warning", {"diffuse": tex_path},
            tiling=2.0, metallic=0.3, roughness_fallback=0.6)
        if mat:
            return mat
    return get_fd_material("M_FD_HazardYellow", (0.45, 0.35, 0.04),
                           roughness=0.7, metallic=0.2)


def find_existing_role_material(role, fallback):
    """Reuse a role material that upgrade_textures.py already created
    (named M_Tex_<role>_<setname>).  Falls back to a FilmDetail flat."""
    try:
        paths = editor_util.list_assets(MAT_TEX_DIR, recursive=False,
                                        include_folder=False)
    except Exception:
        paths = []
    want = f"M_Tex_{role}_"
    for p in paths:
        name = str(p).split("/")[-1].split(".")[0]
        if name.startswith(want):
            mat = editor_util.load_asset(str(p).split(".")[0])
            if mat:
                return mat
    return fallback


def build_all_materials():
    unreal.log("=" * 64)
    unreal.log("2. MATERIALS (textured wood/fabric + dressing flats)")
    unreal.log("=" * 64)
    m = {}

    # --- New textured materials from the imported wood / fabric sets ---
    m["wood_grey"] = build_textured_material(
        "M_Tex_wood_planks_grey",
        {"diffuse":   f"{TEX_WOOD_DIR}/wood_planks_grey_diff_2k",
         "normal":    f"{TEX_WOOD_DIR}/wood_planks_grey_nor_gl_2k",
         "roughness": f"{TEX_WOOD_DIR}/wood_planks_grey_rough_2k"},
        tiling=2.0, metallic=0.0)
    m["wood_worn"] = build_textured_material(
        "M_Tex_wood_worn_planks",
        {"diffuse":   f"{TEX_WOOD_DIR}/worn_planks_diff_2k",
         "normal":    f"{TEX_WOOD_DIR}/worn_planks_nor_gl_2k",
         "roughness": f"{TEX_WOOD_DIR}/worn_planks_rough_2k"},
        tiling=2.0, metallic=0.0)
    m["fabric"] = build_textured_material(
        "M_Tex_fabric_pattern_07",
        {"diffuse":   f"{TEX_FABRIC_DIR}/fabric_pattern_07_diff_2k",
         "normal":    f"{TEX_FABRIC_DIR}/fabric_pattern_07_nor_gl_2k",
         "roughness": f"{TEX_FABRIC_DIR}/fabric_pattern_07_rough_2k"},
        tiling=1.5, metallic=0.0)

    # --- Small reused dressing palette (flat / emissive) ---
    m["dark_metal"] = get_fd_material(
        "M_FD_DarkMetal", (0.03, 0.03, 0.035), roughness=0.6, metallic=0.9)
    m["fire_glow"] = get_fd_material(
        "M_FD_FireGlow", (0.9, 0.35, 0.05), roughness=0.9, metallic=0.0,
        emissive=(8.0, 2.4, 0.45))
    m["kronole_glow"] = get_fd_material(
        "M_FD_KronoleGlow", (0.05, 0.6, 0.4), roughness=0.4, metallic=0.0,
        emissive=(0.5, 6.0, 3.6))
    m["protein"] = get_fd_material(
        "M_FD_ProteinBlock", (0.015, 0.035, 0.014), roughness=0.22,
        metallic=0.0)
    m["sludge"] = get_fd_material(
        "M_FD_SludgeTop", (0.02, 0.05, 0.02), roughness=0.3, metallic=0.0,
        emissive=(0.08, 0.25, 0.08))
    m["blood"] = get_fd_material(
        "M_FD_BloodStain", (0.045, 0.012, 0.008), roughness=0.95, metallic=0.0)
    m["sandbag"] = get_fd_material(
        "M_FD_Sandbag", (0.10, 0.09, 0.07), roughness=1.0, metallic=0.0)
    m["wax"] = get_fd_material(
        "M_FD_CandleWax", (0.8, 0.72, 0.58), roughness=0.45, metallic=0.0)
    m["dark_cloth"] = get_fd_material(
        "M_FD_DarkCloth", (0.04, 0.035, 0.03), roughness=1.0, metallic=0.0)

    m["stripes"] = build_stripes_material()

    # --- Reuse role materials already on disk from upgrade_textures.py ---
    m["pipe"] = find_existing_role_material("pipe", m["dark_metal"])
    m["barricade"] = find_existing_role_material("barricade", m["dark_metal"])
    m["crate"] = find_existing_role_material("crate", m["wood_worn"])
    m["metal_misc"] = find_existing_role_material("metal_misc", m["dark_metal"])

    # Freezer ice (build_zone1's intentional special material)
    ice = None
    if editor_util.does_asset_exist("/Game/Materials/Zone1/M_Ice_Blue"):
        ice = editor_util.load_asset("/Game/Materials/Zone1/M_Ice_Blue")
    m["ice"] = ice or m["dark_metal"]

    missing = [k for k, v in m.items() if v is None]
    for k in missing:
        m[k] = m.get("dark_metal")
        unreal.log_warning(f"  Material slot '{k}' missing -- using dark metal")
    return m


# ---------------------------------------------------------------------------
# 3. LIGHTING MOOD -- dim the generic ceiling lights, idempotently
# ---------------------------------------------------------------------------

# car idx -> intensity clamp for the generic Light_Z1_Car##_* ceiling lights
DIM_TARGETS = {0: 450.0, 1: 450.0, 2: 450.0, 4: 500.0, 5: 500.0, 10: 150.0}
_light_re = re.compile(r"^Light_Z1_Car(\d{2})_")


def dim_general_lights():
    global _dimmed
    unreal.log("=" * 64)
    unreal.log("3. LIGHTING MOOD -- dim general ceiling lights (cars "
               + ", ".join(str(k) for k in sorted(DIM_TARGETS)) + ")")
    unreal.log("=" * 64)
    warm = unreal.Color(255, 160, 90, 255)
    try:
        actors = level_lib.get_all_level_actors()
    except Exception as e:
        unreal.log_error(f"  Could not enumerate actors: {e}")
        return
    for actor in actors:
        if not isinstance(actor, unreal.PointLight):
            continue
        try:
            label = actor.get_actor_label()
        except Exception:
            continue
        mm = _light_re.match(label)
        if not mm:
            continue
        idx = int(mm.group(1))
        target = DIM_TARGETS.get(idx)
        if target is None:
            continue
        plc = actor.get_component_by_class(unreal.PointLightComponent)
        if not plc:
            continue
        try:
            cur = float(plc.get_editor_property("intensity"))
            # modify() BEFORE mutating, or headless saves are no-ops
            actor.modify()
            plc.modify()
            plc.set_editor_property("intensity", min(cur, target))
            cur_rad = float(plc.get_editor_property("attenuation_radius"))
            plc.set_editor_property("attenuation_radius", min(cur_rad, 4000.0))
            if idx != 10:  # Dark Car keeps its cold grey-blue, just dimmer
                plc.set_editor_property("light_color", warm)
            _dimmed += 1
        except Exception as e:
            unreal.log_warning(f"  Could not dim {label}: {e}")
    unreal.log(f"  Dimmed/clamped {_dimmed} general lights")


# ---------------------------------------------------------------------------
# 4. SET-DRESSING BUILDERS
# ---------------------------------------------------------------------------
# Player corridor: y in [-800, 800] stays free of floor-level collision.
# Bunks / props live |y| > 800 except designated set pieces.

def bunk_stack(tag, x, y, mats, scale=1.0, yaw=0.0):
    """Triple-stacked scrap bunk: 2 end panels + 3 thin wood slabs.
    Footprint 2000 x 900 (scaled). 5 actors."""
    L = 2000.0 * scale
    W = 900.0 * scale
    frame_h = 1800.0 * scale
    slab_mat = mats["wood_worn"] if rng.random() < 0.5 else mats["wood_grey"]
    panel_mat = mats["wood_grey"]
    # End panels (head / foot)
    for end, dx in (("H", -L / 2.0 + 30.0), ("F", L / 2.0 - 30.0)):
        spawn_box(f"{tag}_panel{end}", x + dx, y, frame_h / 2.0,
                  60.0, W, frame_h, panel_mat, yaw=yaw)
    # 3 bunk slabs
    for i in range(3):
        z = (250.0 + i * 700.0) * scale
        spawn_box(f"{tag}_bunk{i}", x, y, z, L - 80.0, W - 60.0, 80.0,
                  slab_mat, yaw=yaw)


def cloth_hang(tag, x, y, z, w, h, mats, yaw=0.0, along_x=False, dark=False):
    """Thin hanging cloth slab with slight random yaw (laundry/partition)."""
    yaw = yaw + rng.uniform(-12.0, 12.0)
    mat = mats["dark_cloth"] if dark else mats["fabric"]
    if along_x:
        spawn_box(tag, x, y, z, w, 18.0, h, mat, yaw=yaw)
    else:
        spawn_box(tag, x, y, z, 18.0, w, h, mat, yaw=yaw)


def scatter_crate(tag, x, y, z, mats):
    s = rng.uniform(240.0, 440.0)
    mat = mats["crate"] if rng.random() < 0.6 else mats["fabric"]
    spawn_box(tag, x, y, z + s / 2.0, s, s * rng.uniform(0.8, 1.2),
              s * rng.uniform(0.5, 0.9), mat, yaw=rng.uniform(0, 360))


def barrel_fire(tag, x, y, mats, intensity=1500.0, radius=3000.0):
    """Oil-drum fire: rusty cylinder + emissive ember disc + warm light."""
    spawn_cyl(f"{tag}_drum", x, y, 400.0, 480.0, 800.0, mats["metal_misc"])
    spawn_cyl(f"{tag}_embers", x, y, 815.0, 400.0, 50.0, mats["fire_glow"])
    spawn_light(f"{tag}_glow", x, y, 1150.0, (255, 140, 50), intensity, radius)


def candle_cluster(tag, x, y, z, mats, n=3, light=True):
    """Tiny wax cylinders + emissive flame spheres + one warm light."""
    for i in range(n):
        cx = x + rng.uniform(-140.0, 140.0)
        cy = y + rng.uniform(-140.0, 140.0)
        h = rng.uniform(100.0, 180.0)
        spawn_cyl(f"{tag}_wax{i}", cx, cy, z + h / 2.0, 70.0, h, mats["wax"])
        spawn_sphere(f"{tag}_flame{i}", cx, cy, z + h + 28.0,
                     42.0, 42.0, 60.0, mats["fire_glow"])
    if light:
        spawn_light(f"{tag}_glow", x, y, z + 320.0, (255, 150, 60),
                    rng.uniform(700.0, 1000.0), 850.0)


# --------------------------- generic clutter pass --------------------------

def clutter_pass(idx, mats):
    """Pipes / ducts / valve wheels / floor debris in every car.
    Everything outside the corridor lane."""
    cx = car_x(idx)
    side = 1.0 if idx % 2 == 0 else -1.0

    # Two ceiling pipe runs the full car length (cyl Z axis -> world X)
    for s, dia, z in ((1.0, 150.0, 2760.0), (-1.0, 110.0, 2680.0)):
        spawn_cyl(f"C{idx:02d}_pipe_run{int(s)}", cx, s * (HALF_W - 130.0), z,
                  dia, 11200.0, mats["pipe"], pitch=90.0)
    # One mid-wall duct on alternating side
    spawn_box(f"C{idx:02d}_duct", cx + rng.uniform(-800, 800),
              side * (HALF_W - 110.0), 2150.0,
              9000.0, 180.0, 260.0, mats["metal_misc"])

    # Two vertical drop pipes with valve wheels (cylinder disc, no torus)
    for i, (dx, s) in enumerate(((-3800.0, 1.0), (3800.0, -1.0))):
        px = cx + dx
        py = s * (HALF_W - 140.0)
        spawn_cyl(f"C{idx:02d}_pipe_drop{i}", px, py, 1400.0,
                  120.0, 2700.0, mats["pipe"])
        spawn_cyl(f"C{idx:02d}_valve{i}", px, py - s * 130.0, 1500.0,
                  300.0, 55.0, mats["metal_misc"], roll=90.0)

    # Floor debris piles near the walls
    for i in range(2):
        dx = rng.uniform(-4800.0, 4800.0)
        dy = rng.choice([-1.0, 1.0]) * rng.uniform(1450.0, 1800.0)
        for j in range(2):
            s = rng.uniform(260.0, 520.0)
            spawn_box(f"C{idx:02d}_debris{i}_{j}",
                      cx + dx + rng.uniform(-260, 260),
                      dy + rng.uniform(-160, 160),
                      s * 0.35, s, s * rng.uniform(0.7, 1.2), s * 0.7,
                      mats["metal_misc"] if rng.random() < 0.5 else mats["wood_worn"],
                      yaw=rng.uniform(0, 360))


# ------------------------------ per-car builders ----------------------------

def detail_car00_memorial(mats):
    """Caboose: memorial wall with trinket shelves, candle clusters,
    cloth drapes.  Sparse and reverent."""
    cx = car_x(0)
    rear = cx - HALF_L  # sealed rear wall inner face

    # Trinket shelves on the rear wall, staggered heights
    shelf_z = [1100.0, 1500.0, 1900.0]
    for i, sy in enumerate([-1500.0, -900.0, -300.0, 300.0, 900.0, 1500.0]):
        spawn_box(f"C00_shelf{i}", rear + 160.0, sy, shelf_z[i % 3],
                  60.0, 520.0, 40.0, mats["wood_grey"])
    # Small personal trinkets on the shelves
    for i in range(12):
        sy = [-1500.0, -900.0, -300.0, 300.0, 900.0, 1500.0][i % 6]
        z = shelf_z[(i % 6) % 3] + 80.0
        s = rng.uniform(70.0, 140.0)
        spawn_box(f"C00_trinket{i}", rear + 160.0,
                  sy + rng.uniform(-180.0, 180.0), z,
                  s, s, s * rng.uniform(0.8, 1.6),
                  mats["metal_misc"] if i % 3 else mats["wood_worn"],
                  yaw=rng.uniform(0, 360))

    # Floor candle clusters flanking the memorial (warm, low)
    candle_cluster("C00_candles_L", rear + 520.0, 700.0, 0.0, mats, n=3)
    candle_cluster("C00_candles_R", rear + 520.0, -700.0, 0.0, mats, n=3)

    # Cloth drapes on the side walls near the memorial
    for i, (dx, sy) in enumerate(((800.0, 1.0), (1300.0, -1.0), (2100.0, 1.0))):
        cloth_hang(f"C00_drape{i}", rear + dx, sy * (HALF_W - 60.0),
                   1500.0, 950.0, 1700.0, mats, along_x=True, dark=(i == 1))

    # Practicals over the bunk clusters build_zone1 placed
    for i, (dx, sy) in enumerate(((-3500.0, 1500.0), (-3500.0, -1500.0),
                                  (1000.0, 1500.0))):
        spawn_light(f"C00_bunklight{i}", cx + dx, sy, 1950.0,
                    WARM, practical_intensity(), 850.0)


def detail_car01_bunk_slum(mats):
    """Tail Quarters A: dense bunk slum.  Existing build bunks sit at
    x-4500/-1000/+2500 (y +-1400); new framed stacks fill the gaps."""
    cx = car_x(1)
    new_xs = [-2750.0, 750.0, 4500.0]   # gaps between existing stacks
    for i, dx in enumerate(new_xs):
        for s in (1.0, -1.0):
            bunk_stack(f"C01_stack{i}{'L' if s > 0 else 'R'}",
                       cx + dx, s * 1450.0, mats, yaw=rng.uniform(-4, 4))

    # Hanging cloth partitions between every stack (old + new)
    all_xs = sorted([-4500.0, -2750.0, -1000.0, 750.0, 2500.0, 4500.0])
    ci = 0
    for j in range(len(all_xs) - 1):
        mid = (all_xs[j] + all_xs[j + 1]) / 2.0
        for s in (1.0, -1.0):
            cloth_hang(f"C01_cloth{ci}", cx + mid, s * 1450.0, 1150.0,
                       850.0, 1500.0, mats)
            ci += 1
    # Cloth along the aisle edge (curtained bunk fronts)
    for j, dx in enumerate([-4500.0, -1000.0, 2500.0, 4500.0]):
        for s in (1.0, -1.0):
            cloth_hang(f"C01_front{ci}", cx + dx + rng.uniform(-300, 300),
                       s * 980.0, 1050.0, 1500.0, 1250.0, mats,
                       along_x=True, dark=(j % 2 == 0))
            ci += 1
    # Laundry strung high across the car (above door height, lane safe)
    for j in range(5):
        cloth_hang(f"C01_laundry{j}", cx - 4600.0 + j * 2300.0,
                   rng.uniform(-1100.0, 1100.0), 2480.0,
                   620.0, 480.0, mats, along_x=True)

    # Scattered crates / bundles on and under bunks
    for j in range(10):
        dx = rng.uniform(-5300.0, 5300.0)
        s = rng.choice([-1.0, 1.0])
        scatter_crate(f"C01_crate{j}", cx + dx,
                      s * rng.uniform(950.0, 1750.0), 0.0, mats)

    # Warm practicals over the bunk clusters + a barrel fire mid-car
    for j, dx in enumerate(all_xs[:5]):
        s = 1.0 if j % 2 == 0 else -1.0
        spawn_light(f"C01_bunklight{j}", cx + dx, s * 1300.0, 1900.0,
                    WARM, practical_intensity(), 880.0)
    barrel_fire("C01_barrel", cx - 200.0, -1500.0, mats)


def detail_car02_protein_kitchen(mats):
    """Tail Quarters B becomes the protein-block kitchen (per
    design/zone1/cars/car02-protein-kitchen.md): grinder vat, conveyor,
    block stacks, serving counter -- plus the remaining bunk slum rear."""
    cx = car_x(2)

    # ---- Rear half: bunk slum (existing build stacks at -4500 / +3500) ----
    for i, dx in enumerate([-2600.0, -400.0]):
        for s in (1.0, -1.0):
            bunk_stack(f"C02_stack{i}{'L' if s > 0 else 'R'}",
                       cx + dx, s * 1450.0, mats, yaw=rng.uniform(-4, 4))
    ci = 0
    for dx in (-3550.0, -1500.0, 600.0):
        for s in (1.0, -1.0):
            cloth_hang(f"C02_cloth{ci}", cx + dx, s * 1450.0, 1150.0,
                       850.0, 1500.0, mats)
            ci += 1
    for j, dx in enumerate([-4500.0, -2600.0, -400.0]):
        for s in (1.0, -1.0):
            cloth_hang(f"C02_front{ci}", cx + dx, s * 980.0, 1050.0,
                       1400.0, 1200.0, mats, along_x=True, dark=(j == 1))
            ci += 1
    for j in range(6):
        scatter_crate(f"C02_crate{j}", cx + rng.uniform(-5200.0, 500.0),
                      rng.choice([-1.0, 1.0]) * rng.uniform(950.0, 1750.0),
                      0.0, mats)

    # ---- Front half: THE PROTEIN KITCHEN ----
    # Grinder vat (big dirty cylinder, dark sludge-emissive top disc)
    spawn_cyl("C02_grinder_vat", cx + 2200.0, -1250.0, 550.0,
              1500.0, 1100.0, mats["dark_metal"])
    spawn_cyl("C02_grinder_sludge", cx + 2200.0, -1250.0, 1130.0,
              1380.0, 60.0, mats["sludge"])
    # Conveyor slab from vat toward the counter
    spawn_box("C02_conveyor", cx + 3600.0, -1250.0, 850.0,
              1900.0, 520.0, 80.0, mats["dark_metal"])
    spawn_box("C02_conveyor_legA", cx + 2900.0, -1250.0, 400.0,
              120.0, 420.0, 800.0, mats["dark_metal"])
    spawn_box("C02_conveyor_legB", cx + 4300.0, -1250.0, 400.0,
              120.0, 420.0, 800.0, mats["dark_metal"])

    # Prep tables with neat brick-stacks of protein blocks
    for t, tx in enumerate((2700.0, 4700.0)):
        spawn_box(f"C02_table{t}_base", cx + tx, 1250.0, 220.0,
                  1700.0, 700.0, 440.0, mats["wood_grey"])
        spawn_box(f"C02_table{t}_top", cx + tx, 1250.0, 480.0,
                  1850.0, 820.0, 70.0, mats["wood_worn"])
        # 15 blocks per table: 3 layers x 5 blocks, brick-laid
        for layer in range(3):
            for b in range(5):
                bx = cx + tx - 480.0 + b * 240.0 + (120.0 if layer % 2 else 0.0)
                spawn_box(f"C02_block{t}_{layer}_{b}", bx,
                          1250.0 + rng.uniform(-25.0, 25.0),
                          560.0 + layer * 92.0,
                          230.0, 140.0, 88.0, mats["protein"],
                          yaw=rng.uniform(-3.0, 3.0))

    # Serving counter near the front door (distribution window side)
    spawn_box("C02_counter", cx + 5350.0, 1150.0, 480.0,
              420.0, 1500.0, 960.0, mats["dark_metal"])
    spawn_box("C02_counter_top", cx + 5350.0, 1150.0, 1000.0,
              520.0, 1650.0, 60.0, mats["wood_worn"])
    # Queue rails (metal, bolted) on the ration line side
    for r, rx in enumerate((4500.0, 5000.0, 5500.0)):
        spawn_box(f"C02_queue_rail{r}", cx + rx, -1150.0, 500.0,
                  60.0, 1100.0, 950.0, mats["metal_misc"])

    # Kitchen work lamps + bunk candle practicals
    spawn_light("C02_lamp_vat", cx + 2200.0, -1250.0, 2000.0,
                WARM, 1450.0, 900.0)
    spawn_light("C02_lamp_counter", cx + 5350.0, 1150.0, 1900.0,
                WARM, 1300.0, 900.0)
    spawn_light("C02_lamp_tables", cx + 3700.0, 1250.0, 1950.0,
                WARM, practical_intensity(), 880.0)
    for j, dx in enumerate((-4500.0, -2600.0, -400.0)):
        s = 1.0 if j % 2 == 0 else -1.0
        spawn_light(f"C02_bunklight{j}", cx + dx, s * 1300.0, 1850.0,
                    WARM, practical_intensity(), 850.0)


def detail_car03_pit(mats):
    """The Pit: rope ring around the existing 5000x3000 arena platform,
    crowd barriers, blood-dark floor patches, corner stools."""
    cx = car_x(3)
    px, py = 2600.0, 1600.0   # rope post rectangle (just outside ring)

    # Rope posts
    for i, (sx, sy) in enumerate(((1, 1), (1, -1), (-1, 1), (-1, -1))):
        spawn_cyl(f"C03_ropepost{i}", cx + sx * px, sy * py, 700.0,
                  130.0, 1400.0, mats["wood_worn"])
    # Slack ropes, two heights, four sides
    for r, z in enumerate((680.0, 1130.0)):
        for s in (1.0, -1.0):
            spawn_cyl(f"C03_ropeX{r}{'L' if s > 0 else 'R'}",
                      cx, s * py, z - 30.0, 45.0, 5200.0,
                      mats["dark_cloth"], pitch=90.0)
            spawn_cyl(f"C03_ropeY{r}{'F' if s > 0 else 'B'}",
                      cx + s * px, 0.0, z - 30.0, 45.0, 3200.0,
                      mats["dark_cloth"], roll=90.0)

    # Crowd barriers behind the spectator benches
    for i, dx in enumerate((-3500.0, -500.0, 2800.0)):
        for s in (1.0, -1.0):
            spawn_box(f"C03_barrier{i}{'L' if s > 0 else 'R'}",
                      cx + dx, s * 1860.0, 450.0, 1250.0, 70.0, 880.0,
                      mats["barricade"], yaw=rng.uniform(-8.0, 8.0))

    # Blood-dark patches on the ring surface (set piece -- lane exempt)
    for i in range(3):
        spawn_box(f"C03_blood{i}", cx + rng.uniform(-1900.0, 1900.0),
                  rng.uniform(-1100.0, 1100.0), 204.0,
                  rng.uniform(550.0, 850.0), rng.uniform(420.0, 650.0), 6.0,
                  mats["blood"], yaw=rng.uniform(0, 360))

    # Two corner stools
    spawn_cyl("C03_stoolA", cx - 2950.0, -1750.0, 220.0, 360.0, 440.0,
              mats["wood_grey"])
    spawn_cyl("C03_stoolB", cx + 2950.0, 1750.0, 220.0, 360.0, 440.0,
              mats["wood_grey"])

    # Low warm practicals at two posts (the bright Pit spotlight stays)
    spawn_light("C03_postlight_A", cx - px, py, 1650.0, WARM,
                practical_intensity(), 900.0)
    spawn_light("C03_postlight_B", cx + px, -py, 1650.0, WARM,
                practical_intensity(), 900.0)


def detail_car04_nursery_bunks(mats):
    """Nursery: child-scale triple bunks in side rows (clear of the
    existing center partitions / cribs), cloth everywhere, soft warm."""
    cx = car_x(4)
    scale = 0.75
    xs = [-5000.0 + i * 1900.0 for i in range(6)]   # -5000 .. 4500
    for i, dx in enumerate(xs):
        for s in (1.0, -1.0):
            bunk_stack(f"C04_stack{i}{'L' if s > 0 else 'R'}",
                       cx + dx, s * 1520.0, mats, scale=scale,
                       yaw=rng.uniform(-5, 5))
    # Cloth partitions between stacks + curtained fronts
    ci = 0
    for j in range(len(xs) - 1):
        mid = (xs[j] + xs[j + 1]) / 2.0
        for s in (1.0, -1.0):
            cloth_hang(f"C04_cloth{ci}", cx + mid, s * 1520.0, 950.0,
                       700.0, 1250.0, mats)
            ci += 1
    for j in (0, 2, 4):
        for s in (1.0, -1.0):
            cloth_hang(f"C04_front{ci}", cx + xs[j], s * 1080.0, 900.0,
                       1150.0, 1050.0, mats, along_x=True)
            ci += 1
    # Hanging mobiles stand-in: small cloth scraps strung high
    for j in range(4):
        cloth_hang(f"C04_mobile{j}", cx - 3800.0 + j * 2500.0,
                   rng.uniform(-900.0, 900.0), 2450.0, 420.0, 380.0,
                   mats, along_x=True)
    # Bundles / toy crates
    for j in range(8):
        scatter_crate(f"C04_bundle{j}", cx + rng.uniform(-5200.0, 5200.0),
                      rng.choice([-1.0, 1.0]) * rng.uniform(1000.0, 1800.0),
                      0.0, mats)
    # Soft warm practicals (replaces the dimmed ceiling floods)
    for j, dx in enumerate((-4200.0, -1400.0, 1400.0, 4200.0)):
        s = 1.0 if j % 2 == 0 else -1.0
        spawn_light(f"C04_bunklight{j}", cx + dx, s * 1350.0, 1750.0,
                    WARM, practical_intensity() * 0.8, 800.0)


def detail_car05_elders(mats):
    """Elders' Car: candle-lit council chamber.  Candles on the war table,
    alcove drapes.  Keep it dignified."""
    cx = car_x(5)
    # Candle clusters on the existing meeting table (top z=450+50)
    for i, dx in enumerate((-1000.0, 0.0, 1000.0)):
        candle_cluster(f"C05_candles{i}", cx + dx,
                       rng.uniform(-250.0, 250.0), 500.0, mats,
                       n=2, light=(i != 1))
    # One slightly stronger center glow over the table
    spawn_light("C05_tableglow", cx, 0.0, 1100.0, (255, 160, 70),
                1200.0, 1500.0)
    # Alcove drapes on both walls
    for i, (dx, s) in enumerate(((-3600.0, 1.0), (-3600.0, -1.0),
                                 (3400.0, 1.0), (3400.0, -1.0))):
        cloth_hang(f"C05_drape{i}", cx + dx, s * (HALF_W - 70.0), 1250.0,
                   1100.0, 1900.0, mats, along_x=True, dark=(i % 2 == 0))
    # A few keepsake crates near Gilliam's end
    for j in range(3):
        scatter_crate(f"C05_keepsake{j}", cx + rng.uniform(3800.0, 5300.0),
                      rng.choice([-1.0, 1.0]) * rng.uniform(1000.0, 1700.0),
                      0.0, mats)


def detail_car06_sickbay(mats):
    """Sickbay: keep the green cast; add supply clutter + a candle by
    the beds (warm against the green)."""
    cx = car_x(6)
    for j in range(4):
        scatter_crate(f"C06_supply{j}", cx + rng.uniform(-5000.0, 2000.0),
                      rng.choice([-1.0, 1.0]) * rng.uniform(950.0, 1750.0),
                      0.0, mats)
    cloth_hang("C06_bedcurtain0", cx - 2200.0, 1050.0, 1100.0,
               1500.0, 1400.0, mats, along_x=True)
    cloth_hang("C06_bedcurtain1", cx + 300.0, -1050.0, 1100.0,
               1500.0, 1400.0, mats, along_x=True)
    candle_cluster("C06_candles", cx + 3000.0, 850.0, 500.0, mats, n=2)


def detail_car07_workshop(mats):
    """Workshop: scrap pile, hanging tool cloths, one focused work lamp."""
    cx = car_x(7)
    for j in range(4):
        s = rng.uniform(300.0, 560.0)
        spawn_box(f"C07_scrap{j}", cx + 1500.0 + rng.uniform(-450, 450),
                  -1650.0 + rng.uniform(-220, 220), s * 0.35,
                  s, s * rng.uniform(0.6, 1.1), s * 0.7,
                  mats["metal_misc"], yaw=rng.uniform(0, 360))
    cloth_hang("C07_ragwall", cx + 2000.0, HALF_W - 70.0, 900.0,
               1100.0, 900.0, mats, along_x=True, dark=True)
    # Low work lamp over the bench (bench sits at y=-1000)
    spawn_light("C07_worklamp", cx, -1000.0, 1500.0, WARM, 1400.0, 900.0)


def detail_car08_listening_post(mats):
    """Listening Post: first hints of the checkpoint ahead -- riot
    barriers and a sandbag spill near the forward door."""
    cx = car_x(8)
    # Angled riot slabs flanking the forward door
    for i, s in enumerate((1.0, -1.0)):
        spawn_box(f"C08_riot{i}", cx + 4600.0, s * 1150.0, 420.0,
                  90.0, 1250.0, 840.0, mats["barricade"],
                  yaw=s * 24.0, pitch=-10.0)
    # Sandbag rows (squashed grey spheres) along both walls
    for i in range(6):
        s = 1.0 if i < 3 else -1.0
        sy = s * (950.0 + (i % 3) * 380.0)
        spawn_sphere(f"C08_sandbag{i}", cx + 5150.0 + rng.uniform(-120, 120),
                     sy, 120.0, 460.0, 440.0, 240.0, mats["sandbag"],
                     yaw=rng.uniform(0, 180))
    # Warning stripes on the forward door frame
    for i, s in enumerate((1.0, -1.0)):
        spawn_box(f"C08_stripes{i}", cx + 5900.0, s * 960.0, 1150.0,
                  40.0, 620.0, 260.0, mats["stripes"])
    # One dim warm lamp over the comms console (intel den feel)
    spawn_light("C08_consolelamp", cx - 1000.0, 0.0, 1400.0, WARM,
                1100.0, 900.0)


def detail_car09_blockade(mats):
    """The Blockade: full guard checkpoint.  Riot barricades, sandbag
    arcs, warning stripes, watchtower platform.  (Combat set piece --
    the existing barricade already owns the lane.)"""
    cx = car_x(9)
    # Leaning riot slabs in front of the main barricade
    for i, (dx, dy) in enumerate(((-900.0, 600.0), (-900.0, -650.0),
                                  (-1100.0, 1350.0), (-1100.0, -1350.0))):
        spawn_box(f"C09_riot{i}", cx + dx, dy, 430.0,
                  90.0, 1100.0, 860.0, mats["barricade"],
                  yaw=rng.uniform(-14.0, 14.0), pitch=-12.0)
    # Sandbag arcs at the firing positions (jackboot side)
    bag = 0
    for arc_x, ys in ((300.0, (-1400.0, -1000.0, -600.0)),
                      (1700.0, (600.0, 1000.0, 1400.0))):
        for sy in ys:
            spawn_sphere(f"C09_sandbag{bag}", cx + arc_x + rng.uniform(-90, 90),
                         sy, 120.0, 470.0, 450.0, 245.0, mats["sandbag"],
                         yaw=rng.uniform(0, 180))
            bag += 1
    for i in range(4):  # spare bags dumped along the right wall
        spawn_sphere(f"C09_sandbag{bag}", cx + 2600.0 + i * 300.0,
                     -1750.0 + rng.uniform(-90, 90), 120.0,
                     460.0, 440.0, 240.0, mats["sandbag"],
                     yaw=rng.uniform(0, 180))
        bag += 1
    # Warning stripes on the barricade face (tail side)
    for i, sy in enumerate((-1200.0, 0.0, 1200.0)):
        spawn_box(f"C09_stripes{i}", cx - 170.0, sy, 800.0,
                  20.0, 900.0, 280.0, mats["stripes"])
    # Watchtower platform (jackboot side, right of the lane)
    tw_x, tw_y = cx + 4500.0, 1300.0
    for i, (sx, sy) in enumerate(((1, 1), (1, -1), (-1, 1), (-1, -1))):
        spawn_box(f"C09_towerleg{i}", tw_x + sx * 420.0, tw_y + sy * 420.0,
                  750.0, 130.0, 130.0, 1500.0, mats["dark_metal"])
    spawn_box("C09_towerdeck", tw_x, tw_y, 1540.0, 1150.0, 1150.0, 80.0,
              mats["metal_misc"])
    spawn_box("C09_towerrail_F", tw_x + 540.0, tw_y, 1760.0,
              40.0, 1150.0, 360.0, mats["dark_metal"])
    spawn_box("C09_towerrail_S", tw_x, tw_y - 540.0, 1760.0,
              1150.0, 40.0, 360.0, mats["dark_metal"])
    # Checkpoint lamps: harsher on the jackboot side, per the film grammar
    spawn_light("C09_towerlamp", tw_x, tw_y, 2150.0, (255, 200, 130),
                1500.0, 1600.0)
    spawn_light("C09_gatelamp", cx + 5500.0, 0.0, 2100.0, (255, 190, 120),
                1400.0, 1400.0)


def detail_car10_dark_car(mats):
    """Dark Car: stays a void.  Extra silhouette debris only -- the
    generic lights got clamped to 150 in the lighting pass."""
    cx = car_x(10)
    for j in range(4):
        s = rng.uniform(420.0, 760.0)
        spawn_box(f"C10_hulk{j}", cx + rng.uniform(-4500.0, 4500.0),
                  rng.choice([-1.0, 1.0]) * rng.uniform(1100.0, 1750.0),
                  s * 0.4, s, s * rng.uniform(0.6, 1.0), s * 0.8,
                  mats["dark_cloth"] if j % 2 else mats["metal_misc"],
                  yaw=rng.uniform(0, 360))


def detail_car11_freezer(mats):
    """Freezer Breach: the ONLY cold-blue car in Zone 1.  Extra ice
    rubble near the breach + two cold practicals."""
    cx = car_x(11)
    for j in range(3):
        s = rng.uniform(350.0, 650.0)
        spawn_box(f"C11_icechunk{j}", cx + 1000.0 + rng.uniform(-900, 900),
                  -1550.0 + rng.uniform(-260, 260), s * 0.4,
                  s, s * rng.uniform(0.7, 1.1), s * 0.8, mats["ice"],
                  yaw=rng.uniform(0, 360))
    spawn_light("C11_breachglow", cx + 1000.0, -1500.0, 1300.0,
                (160, 200, 255), 1000.0, 1400.0)
    spawn_light("C11_iciclelight", cx - 2500.0, 200.0, 2300.0,
                (150, 190, 255), 800.0, 1200.0)


def detail_car12_kronole_den(mats):
    """Kronole Den: low mattresses, glowing kronole bricks, dark cloth
    canopies, sickly purple-green low light.  No fog volume per-car --
    dim colored light + canopies fake the haze."""
    cx = car_x(12)
    # Extra low mattresses interleaved with the existing five
    for i, (dx, sy) in enumerate(((-2600.0, -1300.0), (-800.0, 1300.0),
                                  (1000.0, -1300.0), (2800.0, 1300.0))):
        spawn_box(f"C12_mattress{i}", cx + dx, sy, 75.0,
                  1300.0, 700.0, 150.0, mats["fabric"],
                  yaw=rng.uniform(-10.0, 10.0))
        # flattened cushion at its head
        spawn_box(f"C12_cushion{i}", cx + dx - 500.0, sy, 190.0,
                  340.0, 420.0, 90.0, mats["dark_cloth"],
                  yaw=rng.uniform(0, 30))
    # Kronole bricks: glowing cyan-green cubes on the low tables + beds
    for i in range(10):
        if i < 6:   # existing low tables at x -1500 / +1500, y 0, top z 240
            bx = cx + (-1500.0 if i < 3 else 1500.0) + (i % 3 - 1) * 220.0
            by = rng.uniform(-160.0, 160.0)
            bz = 305.0
        else:       # scattered on the new mattresses
            dx, sy = ((-2600.0, -1300.0), (-800.0, 1300.0),
                      (1000.0, -1300.0), (2800.0, 1300.0))[i - 6]
            bx, by, bz = cx + dx + rng.uniform(-300, 300), sy, 195.0
        spawn_box(f"C12_kronole{i}", bx, by, bz, 130.0, 130.0, 90.0,
                  mats["kronole_glow"], yaw=rng.uniform(0, 360))
    # Dark cloth canopies sagging from the ceiling over the lounges
    for i, (dx, sy) in enumerate(((-2400.0, 1100.0), (-400.0, -1100.0),
                                  (1600.0, 1100.0), (3300.0, -1100.0))):
        spawn_box(f"C12_canopy{i}", cx + dx, sy, 2380.0,
                  1700.0, 1300.0, 25.0, mats["dark_cloth"],
                  yaw=rng.uniform(-8, 8), roll=rng.uniform(-6.0, 6.0))
    # Sickly purple-green low light (alternating hues)
    hues = [(150, 60, 170), (110, 170, 110), (150, 60, 170)]
    for i, dx in enumerate((-2500.0, 0.0, 2500.0)):
        spawn_light(f"C12_hazeglow{i}", cx + dx,
                    rng.choice([-900.0, 900.0]), 1700.0,
                    hues[i], rng.uniform(500.0, 750.0), 1300.0)


def detail_car13_smugglers(mats):
    """Smuggler's Cache: contraband bundles tucked among the crates."""
    cx = car_x(13)
    for j in range(5):
        scatter_crate(f"C13_bundle{j}", cx + rng.uniform(-3500.0, 2500.0),
                      rng.choice([-1.0, 1.0]) * rng.uniform(950.0, 1700.0),
                      0.0, mats)
    cloth_hang("C13_stalldrape", cx - 500.0, 1050.0, 1200.0,
               1600.0, 1500.0, mats, along_x=True)
    spawn_light("C13_stalllamp", cx - 500.0, 400.0, 1500.0, WARM,
                1200.0, 950.0)


def detail_car14_martyrs_gate(mats):
    """Martyr's Gate: massive riveted bulkhead dressing on the far (sealed)
    wall -- layered dark metal slabs, rivet grids, flanking braziers.
    Boss arena platform (x +3500) stays clear."""
    cx = car_x(14)
    face = cx + HALF_L   # inner face of the sealed front wall

    # Layered gate slabs (proud of the wall, increasingly massive)
    spawn_box("C14_gate_L1", face - 70.0, 0.0, 1280.0,
              120.0, 3400.0, 2560.0, mats["dark_metal"])
    spawn_box("C14_gate_L2", face - 160.0, 0.0, 1250.0,
              80.0, 2600.0, 2150.0, mats["metal_misc"])
    # Horizontal strapping beams
    spawn_box("C14_gate_beamA", face - 210.0, 0.0, 580.0,
              60.0, 2900.0, 240.0, mats["dark_metal"])
    spawn_box("C14_gate_beamB", face - 210.0, 0.0, 1900.0,
              60.0, 2900.0, 240.0, mats["dark_metal"])
    # Flanking columns
    spawn_box("C14_gate_colL", face - 120.0, 1520.0, 1280.0,
              150.0, 260.0, 2560.0, mats["dark_metal"])
    spawn_box("C14_gate_colR", face - 120.0, -1520.0, 1280.0,
              150.0, 260.0, 2560.0, mats["dark_metal"])

    # Rivet grid: small spheres, 8 columns x 3 rows on the L2 face
    for r, rz in enumerate((420.0, 1250.0, 2080.0)):
        for c in range(8):
            ry = -1190.0 + c * 340.0
            spawn_sphere(f"C14_rivet{r}_{c}", face - 215.0, ry, rz,
                         70.0, 70.0, 70.0, mats["dark_metal"])

    # Two braziers flanking the approach to the gate (warm, ominous)
    for tag, sy in (("C14_brazier_L", 1500.0), ("C14_brazier_R", -1500.0)):
        barrel_fire(tag, cx + 4600.0, sy, mats, intensity=1500.0, radius=3200.0)

    # Tattered banners over the shrine rows
    cloth_hang("C14_banner0", cx - 2500.0, HALF_W - 80.0, 1900.0,
               900.0, 1300.0, mats, along_x=True, dark=True)
    cloth_hang("C14_banner1", cx - 500.0, -(HALF_W - 80.0), 1900.0,
               900.0, 1300.0, mats, along_x=True, dark=True)


CAR_DETAILERS = {
    0: detail_car00_memorial,
    1: detail_car01_bunk_slum,
    2: detail_car02_protein_kitchen,
    3: detail_car03_pit,
    4: detail_car04_nursery_bunks,
    5: detail_car05_elders,
    6: detail_car06_sickbay,
    7: detail_car07_workshop,
    8: detail_car08_listening_post,
    9: detail_car09_blockade,
    10: detail_car10_dark_car,
    11: detail_car11_freezer,
    12: detail_car12_kronole_den,
    13: detail_car13_smugglers,
    14: detail_car14_martyrs_gate,
}


# ---------------------------------------------------------------------------
# Level load / save (patterns from upgrade_textures.py)
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


def save_everything():
    unreal.log("=" * 64)
    unreal.log("5. SAVE")
    unreal.log("=" * 64)
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        les.save_all_dirty_levels()
        # Belt and suspenders (CRITICAL SAVE LESSON): the dirty flag can be
        # lost on headless property-only mutations -- force-save too.
        try:
            les.save_current_level()
        except Exception:
            pass
        unreal.log("  Saved all dirty levels + current level")
    except Exception as e:
        unreal.log_warning(f"  Level save failed: {e}")
    for d in (MAT_TEX_DIR, MAT_FD_DIR, TEX_WOOD_DIR, TEX_FABRIC_DIR):
        try:
            if editor_util.does_directory_exist(d):
                editor_util.save_directory(d, only_if_is_dirty=True,
                                           recursive=True)
        except Exception as e:
            unreal.log_warning(f"  save_directory failed for {d}: {e}")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

CAR_NAMES = {
    0: "Caboose / Memorial", 1: "Tail Quarters A (bunk slum)",
    2: "Tail Quarters B (protein kitchen)", 3: "The Pit", 4: "Nursery",
    5: "Elders' Car", 6: "Sickbay", 7: "Workshop", 8: "Listening Post",
    9: "Blockade", 10: "Dark Car", 11: "Freezer Breach", 12: "Kronole Den",
    13: "Smuggler's Cache", 14: "Martyr's Gate",
}


def run():
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  Zone 1 FILM DETAIL PASS (Bong Joon-ho 2013 look)")
    unreal.log("=" * 64)
    unreal.log("")

    if not load_zone1():
        return

    cleanup()
    import_new_textures()
    mats = build_all_materials()
    dim_general_lights()

    unreal.log("=" * 64)
    unreal.log("4. SET DRESSING (15 cars)")
    unreal.log("=" * 64)
    for idx in range(NUM_CARS):
        before = _total_new
        try:
            CAR_DETAILERS[idx](mats)
        except Exception as e:
            unreal.log_error(f"  Car {idx:02d} detailer failed: {e}")
        try:
            clutter_pass(idx, mats)
        except Exception as e:
            unreal.log_error(f"  Car {idx:02d} clutter failed: {e}")
        unreal.log(f"  Car {idx:02d} {CAR_NAMES[idx]:36s} "
                   f"+{_total_new - before} actors")

    save_everything()

    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  FILM DETAIL PASS COMPLETE")
    unreal.log("=" * 64)
    unreal.log(f"  General lights dimmed:   {_dimmed}")
    unreal.log(f"  Materials created/rewired: {len(_materials_created)}")
    for n in _materials_created:
        unreal.log(f"    {n}")
    unreal.log(f"  New FD_ actors placed:   {_total_new} "
               f"(budget {MAX_NEW_ACTORS})")
    unreal.log("  Per-car actor counts:")
    for idx in range(NUM_CARS):
        unreal.log(f"    [{idx:02d}] {CAR_NAMES[idx]:36s} {_car_counts[idx]:4d}")
    unreal.log("")
    unreal.log("  Corridor lane y in [-800, 800] kept clear of floor-level")
    unreal.log("  collision (set-piece cars 3 and 9 excepted by design).")
    unreal.log("")


run()
