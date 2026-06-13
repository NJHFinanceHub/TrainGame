"""
Snowpiercer: Eternal Engine -- Poly Haven PBR Material Overhaul
Run in editor (Tools > Execute Python Script) or headless via
-ExecutePythonScript="...Scripts/polyhaven_materials.py".

Replaces the legacy texture sets feeding the role materials under
/Game/Materials/Textured with professional CC0 2K PBR sets from
Poly Haven (downloaded to Assets/Textures/PolyHaven/<slug>/ by the
asset-prep pass).

What it does:
  1. IMPORT   -- AssetImportTask-imports each <slug>_<map>_2k.jpg into
     /Game/Textures/PolyHaven/<slug>/ and enforces correct texture
     settings (idempotent: existing assets are not re-imported, but
     settings are re-enforced every run):
       *_diff_2k    : default (TC_Default, sRGB on)
       *_nor_gl_2k  : TC_NORMALMAP, sRGB off
       *_rough_2k / *_ao_2k / *_metal_2k : TC_Default, sRGB off
     (TC_Default + sRGB-off for masks, NOT TC_Masks -- Masks-compressed
     textures under Linear Color samplers fail the whole material
     compile in 5.7; see upgrade_textures.fix_modularpipes fix.)
  2. REBUILD  -- rebuilds the existing role materials IN PLACE
     (delete_all_material_expressions + rewire), keeping the exact
     parameter/wiring contract installed by fix_texture_density.py so
     the ~48 MaterialInstanceConstants under
     /Game/Materials/Textured/Instances keep working untouched:
       TexCoord(1,1) --A--> Multiply <--B-- ScalarParameter "TileScale"
                               |              (default 1.0)
                               +--> every TextureSample's UVs
       diff  RGB -> BaseColor                       (default sRGB sampler)
       nor_gl RGB -> Normal                         (SAMPLERTYPE_NORMAL)
       rough R -> Add <- ScalarParameter "RoughnessBoost" (default 0.15)
                   +--> Clamp(0..1) -> Roughness    (SAMPLERTYPE_MASKS)
       (NO OneMinus -- Poly Haven rough maps are true roughness)
       ao    R -> AmbientOcclusion                  (SAMPLERTYPE_MASKS)
       metal R -> Metallic when the set has one     (SAMPLERTYPE_MASKS)
                  else Constant (0.6 metal roles, 0.0 concrete)
     Recompiles + saves each material.
  3. LOG      -- which slug went to which role material + summary.

Role -> Poly Haven slug:
  M_Tex_floor_metal_ground        <- metal_plate_02     (riveted plates)
  M_Tex_floor_plate_metal_plates  <- metal_plate_02     (riveted plates)
  M_Tex_wall_a_concrete_wall      <- concrete_layers_02 (rough concrete)
  M_Tex_wall_b_dirty_metal        <- corrugated_iron_02 (corrugated wall)
  M_Tex_wall_c_metal1             <- blue_metal_plate   (painted plate)
  M_Tex_ceiling_metal_stuff       <- rusty_metal_03     (dark rusted)
  M_Tex_door_red_metal            <- rusted_shutter     (worn painted)
  M_Tex_gangway_scratchy_metal    <- rusty_metal_03     (dark rusted)

Does NOT touch: the level, the MICs under Instances/, any other
M_Tex_* material (trim/pipe/furniture/crate/barricade/metal_misc/wood/
fabric/stripes), or any other script's assets.

Idempotent: re-running skips already-imported textures (settings are
re-enforced), clears and rewires the same materials to the same graph.
Never raises on a missing file/asset -- logs and continues.
"""

import os

import unreal

# ---------------------------------------------------------------------------
# API shortcuts (style of upgrade_textures.py / fix_texture_density.py)
# ---------------------------------------------------------------------------

editor_util = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mat_lib = unreal.MaterialEditingLibrary

# Source jpgs live at <project-root-repo>/Assets/Textures/PolyHaven.
# This script lives at <repo>/unreal/SnowpiercerEE/Scripts/, so walk up 3.
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SRC_ROOT = os.path.normpath(os.path.join(
    _SCRIPT_DIR, "..", "..", "..", "Assets", "Textures", "PolyHaven"))

TEX_ROOT = "/Game/Textures/PolyHaven"
MAT_DIR = "/Game/Materials/Textured"

TILE_PARAM = "TileScale"
ROUGH_PARAM = "RoughnessBoost"
ROUGH_BOOST_DEFAULT = 0.15

# slug -> maps that exist on disk for it (verified against the Poly Haven
# files API before download; only these were fetched)
SLUG_MAPS = {
    "metal_plate_02":     ["diff", "nor_gl", "rough", "ao", "metal"],
    "corrugated_iron_02": ["diff", "nor_gl", "rough", "ao", "metal"],
    "blue_metal_plate":   ["diff", "nor_gl", "rough", "ao"],
    "rusty_metal_03":     ["diff", "nor_gl", "rough", "ao"],
    "rusted_shutter":     ["diff", "nor_gl", "rough", "ao"],
    "concrete_layers_02": ["diff", "nor_gl", "rough", "ao"],
}

# Existing role materials (exact names verified on disk) -> (slug, metallic
# constant used only when the slug has no metal map).
ROLE_ASSIGNMENTS = [
    ("M_Tex_floor_metal_ground",       "metal_plate_02",     0.6),
    ("M_Tex_floor_plate_metal_plates", "metal_plate_02",     0.6),
    ("M_Tex_wall_a_concrete_wall",     "concrete_layers_02", 0.0),
    ("M_Tex_wall_b_dirty_metal",       "corrugated_iron_02", 0.6),
    ("M_Tex_wall_c_metal1",            "blue_metal_plate",   0.6),
    ("M_Tex_ceiling_metal_stuff",      "rusty_metal_03",     0.6),
    ("M_Tex_door_red_metal",           "rusted_shutter",     0.6),
    ("M_Tex_gangway_scratchy_metal",   "rusty_metal_03",     0.6),
]

_stats = {
    "tex_imported": 0,
    "tex_reused": 0,
    "tex_failed": 0,
    "mats_rebuilt": 0,
    "mats_failed": 0,
}
_role_log = []   # (material_name, slug, channels_wired)


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
        return True
    except Exception as e:
        unreal.log_warning(f"  Could not save asset {asset_path}: {e}")
        return False


# ===========================================================================
# 1. IMPORT Poly Haven jpgs -> /Game/Textures/PolyHaven/<slug>/
# ===========================================================================

def _texture_settings_for(map_name):
    """(compression_settings, srgb) for a Poly Haven map suffix."""
    if map_name == "nor_gl":
        return unreal.TextureCompressionSettings.TC_NORMALMAP, False
    if map_name in ("rough", "ao", "metal"):
        # TC_Default + linear: keeps Linear Color samplers (MASKS sampler
        # type in the material) compiling in 5.7 -- TC_Masks would not.
        return unreal.TextureCompressionSettings.TC_DEFAULT, False
    return None, None  # diff: leave importer defaults (TC_Default, sRGB on)


def _apply_texture_settings(tex, map_name):
    comp, srgb = _texture_settings_for(map_name)
    if comp is None:
        return
    try:
        tex.modify()
        tex.set_editor_property("compression_settings", comp)
        tex.set_editor_property("srgb", srgb)
    except Exception as e:
        unreal.log_warning(f"    Could not set texture settings: {e}")


def _import_one(src_file, dest_path, asset_name):
    """AssetImportTask import of one jpg. Returns the texture or None."""
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", src_file)
    task.set_editor_property("destination_path", dest_path)
    task.set_editor_property("destination_name", asset_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", False)   # we save after settings pass
    task.set_editor_property("replace_existing", True)
    try:
        asset_tools.import_asset_tasks([task])
    except Exception as e:
        unreal.log_warning(f"    Import task failed for {asset_name}: {e}")
        return None
    asset_path = f"{dest_path}/{asset_name}"
    if editor_util.does_asset_exist(asset_path):
        tex = editor_util.load_asset(asset_path)
        if isinstance(tex, unreal.Texture):
            return tex
    return None


def import_polyhaven_textures():
    """Import every slug's maps. Returns slug -> {map: asset_path}."""
    unreal.log("=" * 64)
    unreal.log("1. IMPORT Poly Haven textures")
    unreal.log("=" * 64)
    unreal.log(f"  Source: {SRC_ROOT}")

    imported = {}
    for slug, maps in SLUG_MAPS.items():
        dest_path = f"{TEX_ROOT}/{slug}"
        ensure_dir(dest_path)
        imported[slug] = {}
        for map_name in maps:
            asset_name = f"{slug}_{map_name}_2k"
            asset_path = f"{dest_path}/{asset_name}"
            src_file = os.path.join(SRC_ROOT, slug, asset_name + ".jpg")

            tex = None
            if editor_util.does_asset_exist(asset_path):
                # Idempotent: keep existing import, just re-enforce settings
                tex = editor_util.load_asset(asset_path)
                if isinstance(tex, unreal.Texture):
                    _stats["tex_reused"] += 1
                else:
                    tex = None
            if tex is None:
                if not os.path.isfile(src_file):
                    unreal.log_warning(f"  MISSING on disk: {src_file}")
                    _stats["tex_failed"] += 1
                    continue
                tex = _import_one(src_file, dest_path, asset_name)
                if tex is None:
                    unreal.log_warning(f"  Import failed: {asset_name}")
                    _stats["tex_failed"] += 1
                    continue
                _stats["tex_imported"] += 1

            _apply_texture_settings(tex, map_name)
            safe_save_asset(asset_path)
            imported[slug][map_name] = asset_path

        got = sorted(imported[slug].keys())
        unreal.log(f"  {slug}: {got}")
    unreal.log(f"  Imported {_stats['tex_imported']} new, "
               f"reused {_stats['tex_reused']}, "
               f"failed {_stats['tex_failed']}")
    return imported


# ===========================================================================
# 2. REBUILD role materials in place (TileScale/RoughnessBoost contract)
# ===========================================================================

def _load_texture(path):
    try:
        if path and editor_util.does_asset_exist(path):
            tex = editor_util.load_asset(path)
            if isinstance(tex, unreal.Texture):
                return tex
    except Exception:
        pass
    return None


def _add_sample(mat, tex, x, y, sampler_type=None, uv_node=None):
    """TextureSample wired to the shared (TexCoord * TileScale) UVs."""
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
    if uv_node:
        try:
            mat_lib.connect_material_expressions(uv_node, "", node, "UVs")
        except Exception as e:
            unreal.log_warning(f"    Could not connect UVs: {e}")
    return node


def rebuild_role_material(mat_name, channels, metallic_const):
    """Clear + rewire one existing /Game/Materials/Textured material with
    the new Poly Haven textures, preserving the TileScale / RoughnessBoost
    parameter contract the MICs rely on.  Returns channels wired or -1."""
    mat_path = f"{MAT_DIR}/{mat_name}"
    if not editor_util.does_asset_exist(mat_path):
        unreal.log_warning(f"  {mat_path} does not exist -- skipped "
                           f"(run upgrade_textures.py first)")
        return -1
    mat = editor_util.load_asset(mat_path)
    if not isinstance(mat, unreal.Material):
        unreal.log_warning(f"  {mat_path} is not a Material -- skipped")
        return -1

    diff_tex = _load_texture(channels.get("diff"))
    norm_tex = _load_texture(channels.get("nor_gl"))
    rough_tex = _load_texture(channels.get("rough"))
    ao_tex = _load_texture(channels.get("ao"))
    metal_tex = _load_texture(channels.get("metal"))
    if not (diff_tex and norm_tex and rough_tex):
        unreal.log_warning(f"  {mat_name}: diff/nor_gl/rough not all loaded "
                           f"-- material left untouched")
        return -1

    try:
        mat.modify()
        mat_lib.delete_all_material_expressions(mat)
    except Exception as e:
        unreal.log_warning(f"  Could not clear expressions on {mat_name}: "
                           f"{e} -- left untouched")
        return -1

    # --- Shared UV chain: TexCoord(1,1) * TileScale (contract!) ---
    uv_node = None
    try:
        texcoord = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionTextureCoordinate, -1100, -50)
        texcoord.set_editor_property("u_tiling", 1.0)
        texcoord.set_editor_property("v_tiling", 1.0)

        tile_param = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionScalarParameter, -1100, 120)
        tile_param.set_editor_property("parameter_name", TILE_PARAM)
        tile_param.set_editor_property("default_value", 1.0)

        mult = mat_lib.create_material_expression(
            mat, unreal.MaterialExpressionMultiply, -900, 0)
        mat_lib.connect_material_expressions(texcoord, "", mult, "A")
        mat_lib.connect_material_expressions(tile_param, "", mult, "B")
        uv_node = mult
    except Exception as e:
        unreal.log_warning(f"  UV param chain failed for {mat_name}: {e}")
    if uv_node is None:
        return -1

    wired = 0

    # --- BaseColor (default sRGB sampler) ---
    node = _add_sample(mat, diff_tex, -550, -300, uv_node=uv_node)
    if node:
        try:
            mat_lib.connect_material_property(
                node, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
            wired += 1
        except Exception as e:
            unreal.log_warning(f"    BaseColor connect failed: {e}")

    # --- Normal ---
    node = _add_sample(
        mat, norm_tex, -550, -50,
        sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL,
        uv_node=uv_node)
    if node:
        try:
            mat_lib.connect_material_property(
                node, "RGB", unreal.MaterialProperty.MP_NORMAL)
            wired += 1
        except Exception as e:
            unreal.log_warning(f"    Normal connect failed: {e}")

    # --- Roughness: rough.R -> Add(+RoughnessBoost) -> Clamp -> pin ---
    # True roughness maps: NO OneMinus (that was for legacy spec maps).
    node = _add_sample(
        mat, rough_tex, -550, 200,
        sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_MASKS,
        uv_node=uv_node)
    if node:
        try:
            boost = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionScalarParameter, -380, 340)
            boost.set_editor_property("parameter_name", ROUGH_PARAM)
            boost.set_editor_property("default_value", ROUGH_BOOST_DEFAULT)

            add = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionAdd, -230, 240)
            mat_lib.connect_material_expressions(node, "R", add, "A")
            mat_lib.connect_material_expressions(boost, "", add, "B")

            clamp = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionClamp, -100, 240)
            mat_lib.connect_material_expressions(add, "", clamp, "")
            mat_lib.connect_material_property(
                clamp, "", unreal.MaterialProperty.MP_ROUGHNESS)
            wired += 1
        except Exception as e:
            unreal.log_warning(f"    Roughness boost chain failed: {e}")

    # --- Metallic: map when the set has one, else role constant ---
    if metal_tex:
        node = _add_sample(
            mat, metal_tex, -550, 480,
            sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_MASKS,
            uv_node=uv_node)
        if node:
            try:
                mat_lib.connect_material_property(
                    node, "R", unreal.MaterialProperty.MP_METALLIC)
                wired += 1
            except Exception as e:
                unreal.log_warning(f"    Metallic connect failed: {e}")
    else:
        try:
            const = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionConstant, -550, 480)
            const.set_editor_property("r", float(metallic_const))
            mat_lib.connect_material_property(
                const, "", unreal.MaterialProperty.MP_METALLIC)
        except Exception as e:
            unreal.log_warning(f"    Metallic constant failed: {e}")

    # --- Ambient Occlusion ---
    if ao_tex:
        node = _add_sample(
            mat, ao_tex, -550, 720,
            sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_MASKS,
            uv_node=uv_node)
        if node:
            try:
                mat_lib.connect_material_property(
                    node, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
                wired += 1
            except Exception as e:
                unreal.log_warning(f"    AO connect failed: {e}")

    try:
        mat_lib.recompile_material(mat)
    except Exception as e:
        unreal.log_warning(f"  Recompile failed for {mat_name}: {e}")
    safe_save_asset(mat_path)
    return wired


def rebuild_all_role_materials(imported):
    unreal.log("=" * 64)
    unreal.log("2. REBUILD role materials with Poly Haven sets")
    unreal.log("=" * 64)
    for mat_name, slug, metallic_const in ROLE_ASSIGNMENTS:
        channels = imported.get(slug, {})
        wired = rebuild_role_material(mat_name, channels, metallic_const)
        if wired > 0:
            _stats["mats_rebuilt"] += 1
            _role_log.append((mat_name, slug, wired))
            unreal.log(f"  {mat_name}  <-  {slug}  ({wired} channels)")
        else:
            _stats["mats_failed"] += 1
            _role_log.append((mat_name, slug, 0))


# ===========================================================================
# 3. SAVE + SUMMARY
# ===========================================================================

def save_everything():
    unreal.log("=" * 64)
    unreal.log("3. SAVE")
    unreal.log("=" * 64)
    for root in (TEX_ROOT, MAT_DIR):
        try:
            if editor_util.does_directory_exist(root):
                editor_util.save_directory(root, only_if_is_dirty=True,
                                           recursive=True)
                unreal.log(f"  Saved {root}")
        except Exception as e:
            unreal.log_warning(f"  save_directory failed for {root}: {e}")


def run():
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  Poly Haven PBR material overhaul")
    unreal.log("=" * 64)
    unreal.log("")

    imported = import_polyhaven_textures()
    if not any(imported.values()):
        unreal.log_error("No Poly Haven textures available -- check "
                         f"{SRC_ROOT} -- aborting before touching materials")
        return

    rebuild_all_role_materials(imported)
    save_everything()

    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  POLY HAVEN OVERHAUL COMPLETE")
    unreal.log("=" * 64)
    unreal.log(f"  Textures: {_stats['tex_imported']} imported, "
               f"{_stats['tex_reused']} reused, {_stats['tex_failed']} failed")
    unreal.log(f"  Materials rebuilt: {_stats['mats_rebuilt']}/"
               f"{len(ROLE_ASSIGNMENTS)} (failed: {_stats['mats_failed']})")
    unreal.log("  Role -> slug map:")
    for mat_name, slug, wired in _role_log:
        status = f"{wired} channels" if wired > 0 else "FAILED/SKIPPED"
        unreal.log(f"    {mat_name:34s} <- {slug:20s} [{status}]")
    unreal.log("  MICs under /Game/Materials/Textured/Instances were NOT "
               "touched -- they inherit the new look via their parents.")
    unreal.log("")


run()
