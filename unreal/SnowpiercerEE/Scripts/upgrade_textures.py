"""
Snowpiercer: Eternal Engine -- Zone 1 Texture Upgrade Pass
Run in editor (Tools > Execute Python Script) or headless via
-ExecutePythonScript="...Scripts/upgrade_textures.py".

Upgrades the flat-color materials placed by build_zone1.py /
fix_world_polish.py to real textured PBR materials built from the
imported texture assets under /Game/Textures.

What it does:
  1. INVENTORY  -- scans /Game/Textures recursively and groups textures
     into PBR sets by naming convention:
       * Metals/etc:    X, X_norm, X_spec            (spec = inverse roughness)
       * BakedProps:    X_diff, X_norm, X_spec, X_ao
       * ModularPipes:  <Name>_BaseColor/_Normal/_Roughness/_Metallic
     Incomplete sets (missing normal or spec/roughness) are logged and
     skipped for surface duty.
  2. MATERIALS  -- creates textured materials under /Game/Materials/Textured:
       TexCoord(tiling) -> TextureSample(diff)  -> BaseColor
                           TextureSample(norm, SAMPLERTYPE_NORMAL) -> Normal
                           TextureSample(spec)  -> OneMinus -> Roughness
                           (direct Roughness / Metallic / AO when available)
     Same expression-building patterns as setup_game_content.py.
  3. RETEXTURE  -- loads /Game/Maps/Zone1_Tail, classifies every
     StaticMeshActor by its label (vocabulary from build_zone1.py and
     fix_world_polish.py) and assigns the role-appropriate textured
     material.  Walls rotate through 3 texture sets by car index so cars
     read differently.  Skips NPCVis_* placeholders, hidden GW_ collision
     blockers / end caps, and special-material actors (ice, curtains,
     memorial pieces, braziers).
  4. SAVE       -- saves all dirty levels plus /Game/Materials/Textured.

Idempotent: re-running rebuilds the same /Game/Materials/Textured assets
(expressions are cleared and rewired) and re-assigns the same materials.
Never raises on a missing asset -- logs and continues.

Does NOT touch: fix_world_polish.py's hidden blockers, NPC visuals,
lights, spawn points, or any non-StaticMeshActor.
"""

import re

import unreal

# ---------------------------------------------------------------------------
# API shortcuts (same style as build_zone1.py / setup_game_content.py)
# ---------------------------------------------------------------------------

editor_util = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mat_lib = unreal.MaterialEditingLibrary
level_lib = unreal.EditorLevelLibrary  # deprecated-but-functional in 5.7

TEXTURES_ROOT = "/Game/Textures"
MAT_DIR = "/Game/Materials/Textured"
PERSISTENT_MAP = "/Game/Maps/Zone1_Tail"

CAR_STRIDE = 13000.0  # CAR_LENGTH 12000 + CAR_GAP 1000 (mirror build_zone1.py)
NUM_CARS = 15

_stats = {
    "sets_found": 0,
    "sets_complete": 0,
    "sets_incomplete": 0,
    "materials_created": 0,
    "materials_rewired": 0,
    "actors_checked": 0,
    "actors_skipped": 0,
}
_retextured_by_role = {}   # role -> actor count
_missing_roles = []        # roles with no usable texture set


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
# 1. TEXTURE SET INVENTORY
# ===========================================================================

# Suffix -> channel, longest suffixes first so "_diff_edited" wins over "_diff"
_SUFFIX_CHANNELS = [
    ("_diff_edited", "diffuse_edited"),
    ("_BaseColor",   "diffuse"),
    ("_Normal",      "normal"),
    ("_Roughness",   "roughness"),
    ("_Metallic",    "metallic"),
    ("_Height",      "height"),
    ("_norm",        "normal"),
    ("_spec",        "spec"),
    ("_diff",        "diffuse"),
    ("_ao",          "ao"),
]


def _split_suffix(asset_name):
    """Return (set_base_name, channel) for a texture asset name."""
    for suffix, channel in _SUFFIX_CHANNELS:
        if asset_name.endswith(suffix):
            return asset_name[: -len(suffix)], channel
    return asset_name, "diffuse"  # bare name = diffuse (Metals convention)


def inventory_texture_sets():
    """Scan /Game/Textures recursively, group into PBR sets.

    Returns dict: set short name -> {
        "channels": {channel: asset_path},
        "complete": bool,   # diffuse + normal + (spec OR roughness)
        "folder":   content folder the set lives in,
    }
    Sets are keyed by short base name (e.g. "metal_ground", "container",
    "RustyMetal").  Name collisions across folders keep the first found.
    """
    unreal.log("=" * 64)
    unreal.log("1. TEXTURE SET INVENTORY")
    unreal.log("=" * 64)

    sets = {}
    try:
        asset_paths = editor_util.list_assets(
            TEXTURES_ROOT, recursive=True, include_folder=False)
    except Exception as e:
        unreal.log_error(f"  Could not list {TEXTURES_ROOT}: {e}")
        return sets

    for raw_path in asset_paths:
        # Strip UE object suffix: "/Game/Foo.Foo" -> "/Game/Foo"
        clean = str(raw_path).split(".")[0]
        name = clean.split("/")[-1]
        folder = clean.rsplit("/", 1)[0]

        base, channel = _split_suffix(name)
        if channel == "diffuse_edited":
            continue  # alt diffuse variants -- the plain _diff is canonical
        if channel == "height":
            continue  # no direct material pin

        entry = sets.setdefault(base, {"channels": {}, "folder": folder})
        # First-found wins (avoids cross-folder collisions clobbering)
        entry["channels"].setdefault(channel, clean)

    # Mark completeness
    for base, entry in sets.items():
        ch = entry["channels"]
        entry["complete"] = ("diffuse" in ch and "normal" in ch
                             and ("spec" in ch or "roughness" in ch))
        _stats["sets_found"] += 1
        if entry["complete"]:
            _stats["sets_complete"] += 1
        else:
            _stats["sets_incomplete"] += 1
            unreal.log(f"  [incomplete] {base}: has {sorted(ch.keys())} "
                       f"-- skipped for surface duty")

    unreal.log(f"  Found {_stats['sets_found']} texture sets "
               f"({_stats['sets_complete']} complete, "
               f"{_stats['sets_incomplete']} incomplete)")
    return sets


# ===========================================================================
# 2. TEXTURED MATERIAL CREATION
# ===========================================================================

def _load_texture(path):
    try:
        if editor_util.does_asset_exist(path):
            tex = editor_util.load_asset(path)
            if tex and isinstance(tex, unreal.Texture):
                return tex
    except Exception:
        pass
    return None


def _add_sample(mat, tex, x, y, sampler_type=None, texcoord_node=None):
    """Create a TextureSample expression wired to the shared TexCoord node."""
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


def build_textured_material(mat_name, set_entry, tiling=1.0, metallic=0.5):
    """Create (or rebuild) a textured PBR material under /Game/Materials/Textured.

    Wiring (patterns copied from setup_game_content.wire_pbr_material):
      diffuse  RGB -> BaseColor
      normal   RGB -> Normal      (SAMPLERTYPE_NORMAL)
      spec     R   -> OneMinus -> Roughness   (spec maps are inverse roughness)
      roughness R  -> Roughness   (direct, ModularPipes style)
      metallic R   -> Metallic    (else constant `metallic`)
      ao       R   -> AmbientOcclusion
      TexCoord(tiling) feeds every sample.
    Returns the material asset or None.
    """
    ensure_dir(MAT_DIR)
    mat_full = f"{MAT_DIR}/{mat_name}"
    channels = set_entry["channels"]

    existed = editor_util.does_asset_exist(mat_full)
    if existed:
        mat = editor_util.load_asset(mat_full)
        if not mat:
            unreal.log_warning(f"  Could not load existing {mat_full}")
            return None
        # Idempotent rebuild: clear old expressions, rewire fresh
        try:
            mat_lib.delete_all_material_expressions(mat)
        except Exception as e:
            unreal.log_warning(f"  Could not clear expressions on {mat_name} "
                               f"({e}) -- reusing as-is")
            return mat
    else:
        mat = asset_tools.create_asset(
            mat_name, MAT_DIR, unreal.Material, unreal.MaterialFactoryNew())
        if not mat:
            unreal.log_warning(f"  Failed to create material: {mat_name}")
            return None

    # --- Shared TextureCoordinate node (tiling) ---
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

    # --- BaseColor ---
    diff_tex = _load_texture(channels.get("diffuse", ""))
    if diff_tex:
        node = _add_sample(mat, diff_tex, -450, -250, texcoord_node=texcoord)
        if node:
            try:
                mat_lib.connect_material_property(
                    node, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
                wired += 1
            except Exception as e:
                unreal.log_warning(f"    BaseColor connect failed: {e}")

    # --- Normal ---
    norm_tex = _load_texture(channels.get("normal", ""))
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

    # --- Roughness ---
    if "roughness" in channels:
        # Direct roughness map (ModularPipes style)
        rough_tex = _load_texture(channels["roughness"])
        if rough_tex:
            node = _add_sample(
                mat, rough_tex, -450, 250,
                sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR,
                texcoord_node=texcoord)
            if node:
                try:
                    mat_lib.connect_material_property(
                        node, "R", unreal.MaterialProperty.MP_ROUGHNESS)
                    wired += 1
                except Exception as e:
                    unreal.log_warning(f"    Roughness connect failed: {e}")
    elif "spec" in channels:
        # Spec map = inverse roughness: spec.R -> OneMinus -> Roughness
        spec_tex = _load_texture(channels["spec"])
        if spec_tex:
            node = _add_sample(
                mat, spec_tex, -450, 250,
                sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR,
                texcoord_node=texcoord)
            if node:
                try:
                    one_minus = mat_lib.create_material_expression(
                        mat, unreal.MaterialExpressionOneMinus, -250, 250)
                    if one_minus:
                        mat_lib.connect_material_expressions(
                            node, "R", one_minus, "")
                        mat_lib.connect_material_property(
                            one_minus, "", unreal.MaterialProperty.MP_ROUGHNESS)
                        wired += 1
                except Exception as e:
                    unreal.log_warning(f"    Spec->OneMinus->Roughness failed: {e}")

    # --- Metallic ---
    if "metallic" in channels:
        metal_tex = _load_texture(channels["metallic"])
        if metal_tex:
            node = _add_sample(
                mat, metal_tex, -450, 500,
                sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR,
                texcoord_node=texcoord)
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
                mat, unreal.MaterialExpressionConstant, -450, 500)
            if const:
                const.set_editor_property("r", float(metallic))
                mat_lib.connect_material_property(
                    const, "", unreal.MaterialProperty.MP_METALLIC)
        except Exception as e:
            unreal.log_warning(f"    Metallic constant failed: {e}")

    # --- Ambient Occlusion ---
    if "ao" in channels:
        ao_tex = _load_texture(channels["ao"])
        if ao_tex:
            node = _add_sample(
                mat, ao_tex, -450, 750,
                sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR,
                texcoord_node=texcoord)
            if node:
                try:
                    mat_lib.connect_material_property(
                        node, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
                    wired += 1
                except Exception as e:
                    unreal.log_warning(f"    AO connect failed: {e}")

    if wired == 0:
        unreal.log_warning(f"  {mat_name}: nothing wired -- check texture paths")

    try:
        mat_lib.recompile_material(mat)
    except Exception as e:
        unreal.log_warning(f"  Recompile failed for {mat_name}: {e}")
    safe_save_asset(mat_full)

    if existed:
        _stats["materials_rewired"] += 1
        unreal.log(f"  Rewired  {mat_full} ({wired} channels, tiling x{tiling})")
    else:
        _stats["materials_created"] += 1
        unreal.log(f"  Created  {mat_full} ({wired} channels, tiling x{tiling})")
    return mat


# ===========================================================================
# 3. ROLE -> TEXTURE SET MAPPING (grimy train interior)
# ===========================================================================
#
# role: (ordered candidate set names, tiling, fallback metallic constant)
# First candidate that exists as a COMPLETE set wins.
# NOTE: no "structwall" or wood set exists in this project (verified on
# disk) -- walls rotate concrete_wall / dirty_metal / metal1 by car index,
# and bunks/benches/crates use the BakedProps "container" set (wood-ish
# painted panels).

ROLE_DEFS = {
    # surfaces (large -- tiling 4)
    "floor":       (["metal_ground", "metal_plates"],       4.0, 0.6),
    "floor_plate": (["metal_plates", "metal_ground"],       4.0, 0.7),
    "wall_a":      (["concrete_wall", "concrete"],          4.0, 0.05),
    "wall_b":      (["dirty_metal", "metal1"],              4.0, 0.5),
    "wall_c":      (["metal1", "dirty_metal"],              4.0, 0.5),
    "ceiling":     (["metal_stuff", "dirty_metal"],         4.0, 0.5),
    # medium surfaces (tiling 2)
    "door":        (["red_metal", "brushed_metal"],         2.0, 0.6),
    "trim":        (["brushed_metal", "metal1"],            2.0, 0.8),
    "gangway":     (["scratchy_metal", "dirty_metal"],      2.0, 0.6),
    "barricade":   (["scratchy_metal", "dirty_metal"],      2.0, 0.4),
    "pipe":        (["metal_pipe", "RustyMetal"],           2.0, 0.7),
    # props (tiling 1)
    "furniture":   (["container", "metal_planks"],          1.0, 0.15),
    "crate":       (["container"],                          1.0, 0.15),
    "metal_misc":  (["dirty_metal", "scratchy_metal"],      2.0, 0.5),
}

# Wall variety: car index % 3 picks one of these roles
WALL_VARIANTS = ["wall_a", "wall_b", "wall_c"]


def build_role_materials(tex_sets):
    """Create one textured material per role. Returns role -> material."""
    unreal.log("=" * 64)
    unreal.log("2. TEXTURED MATERIAL CREATION")
    unreal.log("=" * 64)

    role_mats = {}
    for role, (candidates, tiling, metallic) in ROLE_DEFS.items():
        chosen = None
        for cand in candidates:
            entry = tex_sets.get(cand)
            if entry and entry["complete"]:
                chosen = (cand, entry)
                break
            if entry and not entry["complete"]:
                unreal.log(f"  [{role}] candidate '{cand}' incomplete, "
                           f"trying next")
        if not chosen:
            unreal.log_warning(f"  [{role}] NO complete texture set among "
                               f"{candidates} -- role left untextured")
            _missing_roles.append(role)
            continue

        set_name, entry = chosen
        mat = build_textured_material(
            f"M_Tex_{role}_{set_name}", entry, tiling=tiling, metallic=metallic)
        if mat:
            role_mats[role] = (mat, set_name)
        else:
            _missing_roles.append(role)

    unreal.log(f"  Role materials ready: {len(role_mats)}/{len(ROLE_DEFS)}")
    return role_mats


# ===========================================================================
# 4. ACTOR CLASSIFICATION & RETEXTURE
# ===========================================================================

# Actors we must never touch (special materials, placeholders, hidden blockers)
SKIP_SUBSTRINGS = (
    "NPCVis_",       # fix_world_polish NPC placeholder bodies
    "_Block",        # GW_ hidden collision blockers
    "EndCap",        # GW_ end-cap blockers
    "Ice_", "Ice ",  # freezer ice (M_Ice_Blue is intentional)
    "Stalactite", "BreachFrame",
    "Curtain",       # kronole purple curtains
    "Memorial", "Shrine", "Cairn", "Plaque", "WallMap",
    "Brazier",       # reads as fire object
)

# Only actors with these label prefixes were placed by our build scripts
OWNED_PREFIXES = ("Z1_", "Conn_", "GW_", "Pipe_", "Sign_")

# Ordered (first match wins) keyword tuple -> role
LABEL_ROLE_RULES = [
    (("Conn_", "GW_"),                                          "gangway"),
    (("Pipe_",),                                                "pipe"),
    (("Door", "Header"),                                        "door"),
    (("Sign_",),                                                "trim"),
    (("Catwalk", "Platform", "HiddenFloor"),                    "floor_plate"),
    (("Floor", "Corridor", "RingFloor", "BossArena"),           "floor"),
    (("Ceiling", "Ceil", "Roof"),                               "ceiling"),
    (("Barricade", "Sandbag"),                                  "barricade"),
    (("Bunk", "Bed", "Mattress", "Crib", "Table", "Bench",
      "Seat", "Workbench", "Shelf", "TestDummy", "Debris"),     "furniture"),
    # NOTE: no "Cache" keyword here -- every Car-13 label contains "Cache"
    # via the "SmugglerCache"/"Smugglers_Cache" prefix and would misclassify.
    (("Crate", "LockedStorage", "Equipment"),                   "crate"),
    (("Wall", "Partition", "FrontL", "FrontR", "BackL",
      "BackR", "FalseWall"),                                    "wall"),
    (("Anvil", "ToolRack", "Post", "Pillar", "Antenna",
      "Booth", "Console", "ElecPanel", "SteamValve",
      "WarmPipe"),                                              "trim"),
]

_car_re = re.compile(r"Car(\d+)")
_num_re = re.compile(r"^(?:Conn_|GW_)(\d+)")


def _car_index_for(label, actor):
    """Best-effort car index: parse the label, else derive from X position."""
    m = _car_re.search(label)
    if m:
        return int(m.group(1)) % NUM_CARS
    m = _num_re.match(label)
    if m:
        return int(m.group(1)) % NUM_CARS
    try:
        x = actor.get_actor_location().x
        return max(0, min(NUM_CARS - 1, int((x + CAR_STRIDE / 2.0) // CAR_STRIDE)))
    except Exception:
        return 0


def _classify(label, actor):
    """Return role name, or None to skip this actor."""
    for s in SKIP_SUBSTRINGS:
        if s in label:
            return None
    if not any(label.startswith(p) for p in OWNED_PREFIXES):
        return None  # not placed by our build scripts -- leave alone
    for keywords, role in LABEL_ROLE_RULES:
        for kw in keywords:
            if kw in label:
                if role == "wall":
                    return WALL_VARIANTS[_car_index_for(label, actor) % 3]
                return role
    return "metal_misc"  # any remaining Z1_ prop box


def retexture_level(role_mats):
    unreal.log("=" * 64)
    unreal.log("3. RETEXTURE /Game/Maps/Zone1_Tail")
    unreal.log("=" * 64)

    try:
        actors = level_lib.get_all_level_actors()
    except Exception as e:
        unreal.log_error(f"  Could not enumerate actors: {e}")
        return

    for actor in actors:
        if not isinstance(actor, unreal.StaticMeshActor):
            continue
        try:
            label = actor.get_actor_label()
        except Exception:
            continue
        _stats["actors_checked"] += 1

        role = _classify(label, actor)
        if role is None:
            _stats["actors_skipped"] += 1
            continue

        entry = role_mats.get(role)
        if not entry:
            _stats["actors_skipped"] += 1
            continue
        mat, _set_name = entry

        try:
            sm = actor.get_component_by_class(unreal.StaticMeshComponent)
            if not sm or not sm.static_mesh:
                _stats["actors_skipped"] += 1
                continue
            # Pipes placed with real ModularPipes meshes already carry the
            # baked M_RustyMetal -- don't stomp their fitted UVs.
            if role == "pipe":
                try:
                    current = sm.get_material(0)
                    if current and "/Materials/ModularPipes/" in current.get_path_name():
                        _stats["actors_skipped"] += 1
                        continue
                except Exception:
                    pass
            # Mark the actor modified so the level package is dirtied —
            # without this, headless save_all_dirty_levels() is a no-op.
            try:
                actor.modify()
                sm.modify()
            except Exception:
                pass
            sm.set_material(0, mat)
            _retextured_by_role[role] = _retextured_by_role.get(role, 0) + 1
        except Exception as e:
            unreal.log_warning(f"  set_material failed on {label}: {e}")

    total = sum(_retextured_by_role.values())
    unreal.log(f"  Checked {_stats['actors_checked']} StaticMeshActors, "
               f"retextured {total}, skipped {_stats['actors_skipped']}")


# ===========================================================================
# Level load / save (patterns from fix_world_polish.py)
# ===========================================================================

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


def save_everything():
    unreal.log("=" * 64)
    unreal.log("4. SAVE")
    unreal.log("=" * 64)
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        les.save_all_dirty_levels()
        # Belt and suspenders: force-save the current level even if the
        # dirty flag was lost (observed with headless set_material-only runs).
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
            unreal.log(f"  Saved {MAT_DIR}")
    except Exception as e:
        unreal.log_warning(f"  save_directory failed for {MAT_DIR}: {e}")


# ===========================================================================
# Entry point
# ===========================================================================

def run():
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  Zone 1 Texture Upgrade (flat color -> textured PBR)")
    unreal.log("=" * 64)
    unreal.log("")

    if not load_zone1():
        return

    # 1. Inventory
    tex_sets = inventory_texture_sets()
    if not tex_sets:
        unreal.log_error("No texture sets found under /Game/Textures -- "
                         "run import_assets.py first")
        return

    # 2. Build role materials
    role_mats = build_role_materials(tex_sets)
    if not role_mats:
        unreal.log_error("No role materials could be built -- aborting "
                         "before touching the level")
        return

    # 3. Retexture the level
    retexture_level(role_mats)

    # 4. Save
    save_everything()

    # 5. Summary
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  TEXTURE UPGRADE COMPLETE")
    unreal.log("=" * 64)
    unreal.log(f"  Texture sets found:      {_stats['sets_found']} "
               f"({_stats['sets_complete']} complete, "
               f"{_stats['sets_incomplete']} incomplete)")
    unreal.log(f"  Materials created:       {_stats['materials_created']}")
    unreal.log(f"  Materials rewired:       {_stats['materials_rewired']}")
    unreal.log(f"  Actors checked:          {_stats['actors_checked']}")
    unreal.log(f"  Actors skipped:          {_stats['actors_skipped']} "
               f"(NPCVis/blockers/special/foreign)")
    unreal.log("  Retextured by role:")
    for role in sorted(_retextured_by_role):
        set_name = role_mats[role][1] if role in role_mats else "?"
        unreal.log(f"    {role:12s} -> {set_name:16s} "
                   f"{_retextured_by_role[role]:4d} actors")
    if _missing_roles:
        unreal.log(f"  Roles with no texture:   {', '.join(_missing_roles)}")
    unreal.log("")


run()
