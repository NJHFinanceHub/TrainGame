"""
Snowpiercer: Eternal Engine -- Zone 1 TEXTURE DENSITY FIX
Run in editor (Tools > Execute Python Script) or headless via
-ExecutePythonScript="...Scripts/fix_texture_density.py".

THE PROBLEM ("textures look janky"): every textured role material under
/Game/Materials/Textured (built by upgrade_textures.py / detail_pass_film.py)
bakes a FIXED TextureCoordinate tiling (x4 walls/floors, x2 trim, x1 props)
into the material, but actors are /Engine/BasicShapes/Cube scaled anywhere
from ~20 cm trinkets to 120 m floor slabs.  Result: a 120 m floor stretches
4 tiles into blurry smears while a 1 m crate crams the same 4 tiles into
dense noise.  Spec-derived roughness also reads too glossy.

THE FIX -- world-size-consistent texel density via per-size material
instances:
  1. PARAMETERIZE -- every /Game/Materials/Textured/M_Tex_* material is
     harvested (which textures feed which pin, constants, spec-vs-rough
     chain) THEN rebuilt in place with:
       TexCoord(1,1) --A--> Multiply <--B-- ScalarParameter "TileScale"
                                |             (default 1.0)
                                +--> every TextureSample's UVs
       roughness chain ... --> Add <-- ScalarParameter "RoughnessBoost"
                                |        (default 0.15)
                                +--> Clamp(0..1) --> Roughness
     Normals stay SAMPLERTYPE_NORMAL, spec/rough/metallic/AO stay
     SAMPLERTYPE_MASKS (Linear Color samplers on masks-compressed textures
     fail the whole compile in 5.7).
  2. RETARGET -- every StaticMeshActor in /Game/Maps/Zone1_Tail (Z1_, FD_,
     GW2_, everything) whose slot-0 material resolves to a parameterized
     base gets a cached MaterialInstanceConstant with TileScale picked from
     the actor's world bounds:
        dims  = 2 * get_actor_bounds() extent          (world-space, cm)
        face  = the largest face = the two largest axes; its driving
                dimension is the larger of those two (== the max dim)
        raw   = face / 250.0          (target: one tile per ~250 cm)
        TileScale = nearest bucket to raw in log space, from
                    {0.5, 1, 2, 4, 8, 16, 24}
     So the 12000 cm car floor gets raw 48 -> bucket 24 (vs the old 4),
     and a 100 cm crate gets raw 0.4 -> bucket 0.5 (vs the old 1-4).
     MICs are cached one-per-(material, bucket) under
     /Game/Materials/Textured/Instances/MIC_<mat>_b<bucket>.
  3. SKIP -- flat-color materials (M_FD_FireGlow, Zone1 flats, M_Ice_Blue,
     ModularPipes baked mats, NPC visuals...) are skipped automatically:
     only bases that actually got a TileScale parameter qualify.
  4. SAVE -- save_all_dirty_levels() AND save_current_level(), plus
     /Game/Materials/Textured recursively (covers Instances/).

Idempotent: materials are re-harvested (the harvester walks through a
previously-installed Multiply/Add/Clamp chain) and rebuilt to the same
graph; MICs are fetched-or-created and re-pinned to the same values;
actors land on the same MIC every run (bounds don't change).

Does NOT touch: existing scripts' actors/labels, lights, C++ -- material
assets and slot-0 assignments only.
"""

import math

import unreal

# ---------------------------------------------------------------------------
# API shortcuts (style of build_zone1.py / upgrade_textures.py)
# ---------------------------------------------------------------------------

editor_util = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mat_lib = unreal.MaterialEditingLibrary
level_lib = unreal.EditorLevelLibrary  # deprecated-but-functional in 5.7

PERSISTENT_MAP = "/Game/Maps/Zone1_Tail"
MAT_DIR = "/Game/Materials/Textured"
INST_DIR = "/Game/Materials/Textured/Instances"

# Material roots whose actors we are allowed to retarget (final gate is
# still "does the base material have a TileScale param")
ALLOWED_ROOTS = ("/Game/Materials/Textured/", "/Game/Materials/Zone1/")
ALLOWED_NAME_PREFIXES = ("M_FD_",)

TILE_WORLD_CM = 250.0                       # ~one texture tile per 2.5 m
BUCKETS = [0.5, 1.0, 2.0, 4.0, 8.0, 16.0, 24.0]

TILE_PARAM = "TileScale"
ROUGH_PARAM = "RoughnessBoost"
ROUGH_BOOST_DEFAULT = 0.15

_stats = {
    "mats_seen": 0,
    "mats_parameterized": 0,
    "mats_failed": 0,
    "mics_created": 0,
    "mics_reused": 0,
    "actors_checked": 0,
    "actors_retargeted": 0,
    "actors_already_ok": 0,
    "skipped_no_slot0": 0,
    "skipped_foreign": 0,
    "skipped_no_param": 0,
}
_parameterized = set()       # base material paths that carry TileScale
_failed_mats = []            # (name, reason) -- couldn't be parameterized
_bucket_hist = {}            # bucket -> actor count (retargeted + already ok)
_mic_cache = {}              # (base_path, bucket) -> MIC asset


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


def load_zone1():
    """Make sure /Game/Maps/Zone1_Tail is the loaded editor level."""
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        world = level_lib.get_editor_world()
        if not world or "Zone1_Tail" not in world.get_path_name():
            unreal.log(f"  Loading {PERSISTENT_MAP} ...")
            if not editor_util.does_asset_exist(PERSISTENT_MAP):
                unreal.log_error(f"  Map not found: {PERSISTENT_MAP} "
                                 f"-- run build_zone1.py first")
                return False
            les.load_level(PERSISTENT_MAP)
        return True
    except Exception as e:
        unreal.log_error(f"  Could not load level: {e}")
        return False


# ===========================================================================
# 1a. HARVEST -- read each M_Tex_* material's existing graph
# ===========================================================================

def _prop_input(mat, prop):
    """Expression node feeding a material output property, or None."""
    try:
        return mat_lib.get_material_property_input_node(mat, prop)
    except Exception:
        return None


def _expr_inputs(mat, expr):
    """Expressions feeding `expr`, or []."""
    try:
        return list(mat_lib.get_inputs_for_material_expression(mat, expr))
    except Exception:
        return []


def _tex_of(node):
    """Texture asset of a TextureSample node, or None."""
    if node and isinstance(node, unreal.MaterialExpressionTextureSample):
        try:
            return node.get_editor_property("texture")
        except Exception:
            return None
    return None


def _trace_to_sample_or_const(mat, node, max_depth=8):
    """Walk DOWN a chain of pass-through nodes (Clamp / Add / OneMinus /
    Multiply -- i.e. the chain a previous run of this script installed, or
    upgrade_textures' OneMinus) until hitting a TextureSample or Constant.
    ScalarParameter branches are skipped (they're our own params).

    Returns (texture_or_None, saw_one_minus, const_value_or_None).
    """
    saw_one_minus = False
    cur = node
    for _ in range(max_depth):
        if cur is None:
            break
        if isinstance(cur, unreal.MaterialExpressionTextureSample):
            return _tex_of(cur), saw_one_minus, None
        if isinstance(cur, unreal.MaterialExpressionConstant):
            try:
                return None, saw_one_minus, float(cur.get_editor_property("r"))
            except Exception:
                return None, saw_one_minus, None
        if isinstance(cur, unreal.MaterialExpressionOneMinus):
            saw_one_minus = True
        nxt = None
        for inp in _expr_inputs(mat, cur):
            if isinstance(inp, unreal.MaterialExpressionScalarParameter):
                continue  # our TileScale / RoughnessBoost params
            if isinstance(inp, unreal.MaterialExpressionTextureCoordinate):
                continue  # UV plumbing, not a value source
            nxt = inp
            break
        cur = nxt
    return None, saw_one_minus, None


def harvest_material(mat):
    """Read which textures/constants feed which pins.  Returns dict or None
    if there is nothing tileable (no texture sample on any pin)."""
    info = {
        "diffuse": None, "normal": None,
        "rough_tex": None, "rough_is_spec": False, "rough_const": None,
        "metal_tex": None, "metal_const": None,
        "ao": None,
    }

    # BaseColor / Normal / Metallic / AO: sample (or constant) directly,
    # but trace defensively in case of pass-through nodes.
    node = _prop_input(mat, unreal.MaterialProperty.MP_BASE_COLOR)
    tex, _om, _c = _trace_to_sample_or_const(mat, node)
    info["diffuse"] = tex

    node = _prop_input(mat, unreal.MaterialProperty.MP_NORMAL)
    tex, _om, _c = _trace_to_sample_or_const(mat, node)
    info["normal"] = tex

    node = _prop_input(mat, unreal.MaterialProperty.MP_METALLIC)
    tex, _om, const = _trace_to_sample_or_const(mat, node)
    info["metal_tex"] = tex
    info["metal_const"] = const

    node = _prop_input(mat, unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
    tex, _om, _c = _trace_to_sample_or_const(mat, node)
    info["ao"] = tex

    # Roughness: TextureSample (direct rough map), OneMinus<-spec map,
    # Constant fallback, or a previous run's Clamp<-Add<-... chain.
    node = _prop_input(mat, unreal.MaterialProperty.MP_ROUGHNESS)
    tex, one_minus, const = _trace_to_sample_or_const(mat, node)
    info["rough_tex"] = tex
    info["rough_is_spec"] = bool(tex and one_minus)
    info["rough_const"] = const

    if not (info["diffuse"] or info["normal"] or info["rough_tex"]
            or info["metal_tex"] or info["ao"]):
        return None  # flat-color material: tiling is meaningless
    return info


# ===========================================================================
# 1b. REBUILD -- rewire with TileScale / RoughnessBoost parameters
# ===========================================================================

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


def rebuild_parameterized(mat, mat_path, info):
    """Clear and rewire `mat` with the TileScale / RoughnessBoost params.
    Returns True on success."""
    name = mat_path.split("/")[-1]
    try:
        mat_lib.delete_all_material_expressions(mat)
    except Exception as e:
        _failed_mats.append((name, f"could not clear expressions: {e}"))
        return False

    # --- Shared UV chain: TexCoord * TileScale ---
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
        unreal.log_warning(f"  UV param chain failed for {name}: {e}")
        uv_node = None

    if uv_node is None:
        _failed_mats.append((name, "TileScale UV chain could not be built"))
        # leave the material cleared-but-unparameterized? No: bail before
        # touching pins so a recompile keeps it visibly broken in logs.
        return False

    wired = 0

    # --- BaseColor ---
    if info["diffuse"]:
        node = _add_sample(mat, info["diffuse"], -550, -300, uv_node=uv_node)
        if node:
            try:
                mat_lib.connect_material_property(
                    node, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
                wired += 1
            except Exception as e:
                unreal.log_warning(f"    BaseColor connect failed: {e}")

    # --- Normal ---
    if info["normal"]:
        node = _add_sample(
            mat, info["normal"], -550, -50,
            sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL,
            uv_node=uv_node)
        if node:
            try:
                mat_lib.connect_material_property(
                    node, "RGB", unreal.MaterialProperty.MP_NORMAL)
                wired += 1
            except Exception as e:
                unreal.log_warning(f"    Normal connect failed: {e}")

    # --- Roughness: (map | 1-spec | const) + RoughnessBoost, clamped 0..1 ---
    rough_src = None      # (node, output_pin)
    if info["rough_tex"]:
        node = _add_sample(
            mat, info["rough_tex"], -550, 200,
            sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_MASKS,
            uv_node=uv_node)
        if node:
            if info["rough_is_spec"]:
                try:
                    one_minus = mat_lib.create_material_expression(
                        mat, unreal.MaterialExpressionOneMinus, -380, 200)
                    mat_lib.connect_material_expressions(
                        node, "R", one_minus, "")
                    rough_src = (one_minus, "")
                except Exception as e:
                    unreal.log_warning(f"    OneMinus failed: {e}")
            else:
                rough_src = (node, "R")
    if rough_src is None:
        # constant fallback (preserve harvested value, else grimy 0.85)
        try:
            const = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionConstant, -550, 200)
            fallback = info["rough_const"]
            const.set_editor_property(
                "r", float(fallback) if fallback is not None else 0.85)
            rough_src = (const, "")
        except Exception as e:
            unreal.log_warning(f"    Roughness constant failed: {e}")

    if rough_src:
        try:
            boost = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionScalarParameter, -380, 340)
            boost.set_editor_property("parameter_name", ROUGH_PARAM)
            boost.set_editor_property("default_value", ROUGH_BOOST_DEFAULT)

            add = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionAdd, -230, 240)
            mat_lib.connect_material_expressions(
                rough_src[0], rough_src[1], add, "A")
            mat_lib.connect_material_expressions(boost, "", add, "B")

            clamp = mat_lib.create_material_expression(
                mat, unreal.MaterialExpressionClamp, -100, 240)
            mat_lib.connect_material_expressions(add, "", clamp, "")
            mat_lib.connect_material_property(
                clamp, "", unreal.MaterialProperty.MP_ROUGHNESS)
            wired += 1
        except Exception as e:
            unreal.log_warning(f"    Roughness boost chain failed: {e}")

    # --- Metallic ---
    if info["metal_tex"]:
        node = _add_sample(
            mat, info["metal_tex"], -550, 480,
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
            mc = info["metal_const"]
            const.set_editor_property(
                "r", float(mc) if mc is not None else 0.5)
            mat_lib.connect_material_property(
                const, "", unreal.MaterialProperty.MP_METALLIC)
        except Exception as e:
            unreal.log_warning(f"    Metallic constant failed: {e}")

    # --- Ambient Occlusion ---
    if info["ao"]:
        node = _add_sample(
            mat, info["ao"], -550, 720,
            sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_MASKS,
            uv_node=uv_node)
        if node:
            try:
                mat_lib.connect_material_property(
                    node, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
                wired += 1
            except Exception as e:
                unreal.log_warning(f"    AO connect failed: {e}")

    if wired == 0:
        _failed_mats.append((name, "no channel could be rewired"))
        return False

    try:
        mat_lib.recompile_material(mat)
    except Exception as e:
        unreal.log_warning(f"  Recompile failed for {name}: {e}")
    safe_save_asset(mat_path)
    unreal.log(f"  Parameterized {mat_path} ({wired} channels)")
    return True


def parameterize_all_materials():
    unreal.log("=" * 64)
    unreal.log("1. PARAMETERIZE /Game/Materials/Textured/M_Tex_*")
    unreal.log("=" * 64)
    try:
        paths = editor_util.list_assets(MAT_DIR, recursive=False,
                                        include_folder=False)
    except Exception as e:
        unreal.log_error(f"  Could not list {MAT_DIR}: {e}")
        return
    for raw in paths:
        clean = str(raw).split(".")[0]
        name = clean.split("/")[-1]
        if not name.startswith("M_Tex_"):
            continue
        mat = editor_util.load_asset(clean)
        if not mat or not isinstance(mat, unreal.Material):
            continue  # MICs etc. live elsewhere; ignore non-Materials
        _stats["mats_seen"] += 1

        info = harvest_material(mat)
        if info is None:
            _stats["mats_failed"] += 1
            _failed_mats.append((name, "no texture samples found "
                                       "(flat material) -- left untouched"))
            continue
        if rebuild_parameterized(mat, clean, info):
            _stats["mats_parameterized"] += 1
            _parameterized.add(clean)
        else:
            _stats["mats_failed"] += 1

    unreal.log(f"  Parameterized {_stats['mats_parameterized']}/"
               f"{_stats['mats_seen']} M_Tex_ materials")


# ===========================================================================
# 2. RETARGET -- per-actor size buckets -> cached MICs
# ===========================================================================

def _base_has_tile_param(base_mat):
    """Fallback gate for bases not (re)parameterized in this run."""
    try:
        names = mat_lib.get_scalar_parameter_names(base_mat)
        return any(str(n) == TILE_PARAM for n in names)
    except Exception:
        return False


def _bucket_for_actor(actor):
    """TileScale bucket from world bounds: one tile per ~250 cm along the
    largest face's driving dimension, snapped to the nearest bucket in
    log space (buckets are geometric, so log distance is the fair metric)."""
    try:
        _origin, extent = actor.get_actor_bounds(False)
    except Exception:
        return None
    dims = sorted([abs(extent.x) * 2.0, abs(extent.y) * 2.0,
                   abs(extent.z) * 2.0], reverse=True)
    face_dim = max(dims[0], dims[1])    # two largest axes span the big face
    raw = max(face_dim / TILE_WORLD_CM, 0.01)
    return min(BUCKETS, key=lambda b: abs(math.log(raw) - math.log(b)))


def _bucket_str(b):
    return ("%g" % b).replace(".", "_")    # 0.5 -> "0_5", 8.0 -> "8"


def get_or_create_mic(base_mat, base_path, bucket):
    """One MaterialInstanceConstant per (base material, bucket), cached at
    /Game/Materials/Textured/Instances/MIC_<mat>_b<bucket>."""
    key = (base_path, bucket)
    if key in _mic_cache:
        return _mic_cache[key]

    ensure_dir(INST_DIR)
    base_name = base_path.split("/")[-1]
    mic_name = f"MIC_{base_name}_b{_bucket_str(bucket)}"
    mic_path = f"{INST_DIR}/{mic_name}"

    mic = None
    created = False
    if editor_util.does_asset_exist(mic_path):
        mic = editor_util.load_asset(mic_path)
        if mic and not isinstance(mic, unreal.MaterialInstanceConstant):
            unreal.log_warning(f"  {mic_path} exists but is not a MIC -- "
                               f"skipping this bucket")
            _mic_cache[key] = None
            return None
    if mic is None:
        try:
            mic = asset_tools.create_asset(
                mic_name, INST_DIR, unreal.MaterialInstanceConstant,
                unreal.MaterialInstanceConstantFactoryNew())
            created = True
        except Exception as e:
            unreal.log_warning(f"  Could not create {mic_path}: {e}")
            mic = None
    if mic is None:
        _mic_cache[key] = None
        return None

    # (Re)pin parent + TileScale every run -- idempotent, survives the
    # parent rebuild in step 1.
    try:
        cur_parent = mic.get_editor_property("parent")
        if (not cur_parent
                or cur_parent.get_path_name().split(".")[0] != base_path):
            mat_lib.set_material_instance_parent(mic, base_mat)
        mat_lib.set_material_instance_scalar_parameter_value(
            mic, TILE_PARAM, float(bucket))
        # RoughnessBoost stays inherited from the parent default (0.15)
        try:
            mat_lib.update_material_instance(mic)
        except Exception:
            pass
    except Exception as e:
        unreal.log_warning(f"  Could not configure {mic_path}: {e}")
        _mic_cache[key] = None
        return None

    safe_save_asset(mic_path)
    if created:
        _stats["mics_created"] += 1
        unreal.log(f"  Created {mic_path} ({TILE_PARAM}={bucket})")
    else:
        _stats["mics_reused"] += 1
    _mic_cache[key] = mic
    return mic


def _resolve_base(slot0_mat):
    """(base_material, base_path) for an actor's slot-0 material: walk up
    through MaterialInstances (e.g. our own MICs on a re-run)."""
    mat = slot0_mat
    for _ in range(4):  # parent chains are shallow; guard against cycles
        if mat is None:
            return None, None
        if isinstance(mat, unreal.MaterialInstance):
            try:
                mat = mat.get_editor_property("parent")
                continue
            except Exception:
                return None, None
        break
    if mat is None:
        return None, None
    return mat, mat.get_path_name().split(".")[0]


def retarget_level():
    unreal.log("=" * 64)
    unreal.log("2. RETARGET actors -> size-bucketed MICs")
    unreal.log("=" * 64)
    try:
        actors = level_lib.get_all_level_actors()
    except Exception as e:
        unreal.log_error(f"  Could not enumerate actors: {e}")
        return

    for actor in actors:
        if not isinstance(actor, unreal.StaticMeshActor):
            continue
        _stats["actors_checked"] += 1
        try:
            sm = actor.get_component_by_class(unreal.StaticMeshComponent)
            if not sm or not sm.static_mesh:
                _stats["skipped_no_slot0"] += 1
                continue
            cur = sm.get_material(0)
        except Exception:
            _stats["skipped_no_slot0"] += 1
            continue
        if cur is None:
            _stats["skipped_no_slot0"] += 1
            continue

        base_mat, base_path = _resolve_base(cur)
        if not base_mat:
            _stats["skipped_foreign"] += 1
            continue
        base_name = base_path.split("/")[-1]
        if not (base_path.startswith(ALLOWED_ROOTS)
                or base_name.startswith(ALLOWED_NAME_PREFIXES)):
            _stats["skipped_foreign"] += 1
            continue

        # Final gate: the base must actually carry TileScale (flat-color
        # M_FD_ / Zone1 specials and anything that failed step 1 drop out)
        if base_path not in _parameterized:
            if not _base_has_tile_param(base_mat):
                _stats["skipped_no_param"] += 1
                continue

        bucket = _bucket_for_actor(actor)
        if bucket is None:
            _stats["skipped_no_slot0"] += 1
            continue

        mic = get_or_create_mic(base_mat, base_path, bucket)
        if mic is None:
            _stats["skipped_no_param"] += 1
            continue

        _bucket_hist[bucket] = _bucket_hist.get(bucket, 0) + 1

        cur_path = cur.get_path_name().split(".")[0]
        if cur_path == mic.get_path_name().split(".")[0]:
            _stats["actors_already_ok"] += 1
            continue  # re-run: already on the right MIC

        try:
            # modify() BEFORE mutating, or headless saves are no-ops
            actor.modify()
            sm.modify()
            sm.set_material(0, mic)
            _stats["actors_retargeted"] += 1
        except Exception as e:
            try:
                label = actor.get_actor_label()
            except Exception:
                label = "<?>"
            unreal.log_warning(f"  set_material failed on {label}: {e}")

    unreal.log(f"  Checked {_stats['actors_checked']} StaticMeshActors, "
               f"retargeted {_stats['actors_retargeted']} "
               f"(already correct: {_stats['actors_already_ok']})")


# ===========================================================================
# 3. SAVE
# ===========================================================================

def save_everything():
    unreal.log("=" * 64)
    unreal.log("3. SAVE")
    unreal.log("=" * 64)
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        les.save_all_dirty_levels()
        # Belt and suspenders: dirty flag has been observed to drop on
        # headless set_material-only runs.
        try:
            les.save_current_level()
        except Exception:
            pass
        unreal.log("  Saved all dirty levels")
    except Exception as e:
        unreal.log_warning(f"  Level save failed: {e}")
    try:
        if editor_util.does_directory_exist(MAT_DIR):
            editor_util.save_directory(MAT_DIR, only_if_is_dirty=True,
                                       recursive=True)
            unreal.log(f"  Saved {MAT_DIR} (recursive, incl. Instances)")
    except Exception as e:
        unreal.log_warning(f"  save_directory failed for {MAT_DIR}: {e}")


# ===========================================================================
# Entry point
# ===========================================================================

def run():
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  Texture density fix (size-bucketed material instances)")
    unreal.log("=" * 64)
    unreal.log("")

    if not load_zone1():
        return

    parameterize_all_materials()
    if not _parameterized:
        unreal.log_error("No material could be parameterized -- aborting "
                         "before touching the level")
        return

    retarget_level()
    save_everything()

    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  TEXTURE DENSITY FIX COMPLETE")
    unreal.log("=" * 64)
    unreal.log(f"  Materials parameterized:  {_stats['mats_parameterized']}"
               f"/{_stats['mats_seen']} "
               f"({TILE_PARAM} + {ROUGH_PARAM})")
    if _failed_mats:
        unreal.log("  Not parameterized:")
        for name, reason in _failed_mats:
            unreal.log(f"    {name}: {reason}")
    unreal.log(f"  MICs created:             {_stats['mics_created']} "
               f"(reused from disk: {_stats['mics_reused']})")
    unreal.log(f"  Actors checked:           {_stats['actors_checked']}")
    unreal.log(f"  Actors retargeted:        {_stats['actors_retargeted']} "
               f"(already on MIC: {_stats['actors_already_ok']})")
    unreal.log("  Bucket histogram (TileScale -> actors):")
    for b in BUCKETS:
        if b in _bucket_hist:
            tiles = b * TILE_WORLD_CM / 100.0
            unreal.log(f"    x{('%g' % b):>4s} (~{tiles:.1f} m face) "
                       f"{_bucket_hist[b]:5d} actors")
    unreal.log(f"  Skipped -- no/empty slot0: {_stats['skipped_no_slot0']}")
    unreal.log(f"  Skipped -- foreign mats:   {_stats['skipped_foreign']} "
               f"(ModularPipes/character/engine)")
    unreal.log(f"  Skipped -- no {TILE_PARAM}:  {_stats['skipped_no_param']} "
               f"(flat M_FD_/Zone1 specials)")
    unreal.log("")


run()
