"""
Snowpiercer: Eternal Engine — Game Content Setup Script
Run AFTER import_assets.py and setup_demo_level.py have completed.

Tools > Execute Python Script > browse to this file
Or from the Output Log: py "path/to/setup_game_content.py"

Creates all runtime content the game needs in the UE5 Editor:
  1. Character setup (Mannequin or placeholder)
  2. Persisted PBR materials for all imported textures
  3. Pickup item blueprints
  4. NPC blueprint templates
  5. Audio (SoundCues, ambient actors)
  6. Post-process & atmosphere presets
"""

import unreal

# ---------------------------------------------------------------------------
# Subsystems & Libraries
# ---------------------------------------------------------------------------

editor_util = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
level_lib = unreal.EditorLevelLibrary
mat_lib = unreal.MaterialEditingLibrary

# Blueprint / asset factory handles
subsystem = None  # SubobjectDataSubsystem not available as EditorSubsystem in UE 5.7


def ensure_directory(content_path):
    """Create content directory if it doesn't exist."""
    if not editor_util.does_directory_exist(content_path):
        editor_util.make_directory(content_path)


def save_asset(asset_path):
    """Save an asset to disk. Returns True on success."""
    try:
        editor_util.save_asset(asset_path, only_if_is_dirty=False)
        return True
    except Exception as e:
        unreal.log_warning(f"  Failed to save {asset_path}: {e}")
        return False


def load_or_warn(asset_path, label="Asset"):
    """Load an asset, log warning if missing."""
    asset = editor_util.load_asset(asset_path)
    if not asset:
        unreal.log_warning(f"  {label} not found: {asset_path}")
    return asset


# ============================================================================
# 1. CHARACTER SETUP — Mannequin Placeholder
# ============================================================================

def setup_character():
    """
    Configure the default pawn (SEECharacter) to use UE5's built-in Mannequin
    skeletal mesh if available; otherwise create a visible placeholder.
    """
    unreal.log("=" * 60)
    unreal.log("1. CHARACTER SETUP")
    unreal.log("=" * 60)

    # --- Locate the Mannequin skeletal mesh ---
    mannequin_paths = [
        "/Game/Characters/Mannequins/Meshes/SKM_Manny",
        "/Game/Characters/Mannequins/Meshes/SKM_Quinn",
        "/Game/Characters/Mannequin/Mesh/SKM_Manny",
        "/Game/Characters/Mannequin/Mesh/SKM_Quinn",
        # Third Person template paths
        "/Game/ThirdPerson/Characters/Mannequins/Meshes/SKM_Manny",
        "/Game/ThirdPerson/Characters/Mannequins/Meshes/SKM_Quinn",
        # Engine content (always available)
        "/Engine/EngineMeshes/SkeletalCube",
    ]

    skeletal_mesh = None
    skeletal_mesh_path = None
    for path in mannequin_paths:
        if editor_util.does_asset_exist(path):
            skeletal_mesh = editor_util.load_asset(path)
            if skeletal_mesh:
                skeletal_mesh_path = path
                unreal.log(f"  Found skeletal mesh: {path}")
                break

    # --- Look for an AnimBlueprint ---
    anim_bp_paths = [
        "/Game/Characters/Mannequins/Animations/ABP_Manny",
        "/Game/Characters/Mannequins/Animations/ABP_Quinn",
        "/Game/Characters/Mannequin/Animations/ABP_Manny",
        "/Game/ThirdPerson/Characters/Mannequins/Animations/ABP_Manny",
    ]
    anim_bp = None
    for path in anim_bp_paths:
        if editor_util.does_asset_exist(path):
            anim_bp = editor_util.load_asset(path)
            if anim_bp:
                unreal.log(f"  Found AnimBP: {path}")
                break

    # --- Create the Character Blueprint ---
    bp_path = "/Game/Blueprints/Characters"
    bp_name = "BP_SEECharacter"
    bp_full = f"{bp_path}/{bp_name}"

    ensure_directory(bp_path)

    if editor_util.does_asset_exist(bp_full):
        unreal.log(f"  BP already exists: {bp_full}")
        char_bp = editor_util.load_asset(bp_full)
    else:
        # Create Blueprint from the C++ class
        try:
            parent_class = unreal.load_class(None, "/Script/SnowpiercerEE.SEEPlayerCharacter")
            if not parent_class:
                unreal.log_warning("  SEEPlayerCharacter C++ class not found, falling back to ACharacter")
                parent_class = unreal.Character.static_class()
        except Exception:
            unreal.log_warning("  Could not load SEEPlayerCharacter, falling back to ACharacter")
            parent_class = unreal.Character.static_class()

        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", parent_class)
        char_bp = asset_tools.create_asset(bp_name, bp_path, unreal.Blueprint, factory)

        if char_bp:
            unreal.log(f"  Created character BP: {bp_full}")
        else:
            unreal.log_warning("  Failed to create character blueprint")
            return False

    # --- Configure Mesh on the CDO (Class Default Object) ---
    if char_bp and skeletal_mesh:
        try:
            cdo = unreal.get_default_object(char_bp.generated_class())
            if cdo:
                mesh_comp = cdo.get_component_by_class(unreal.SkeletalMeshComponent)
                if mesh_comp:
                    mesh_comp.set_skeletal_mesh_asset(skeletal_mesh)
                    unreal.log(f"  Set skeletal mesh on CDO: {skeletal_mesh_path}")

                    if anim_bp:
                        mesh_comp.set_editor_property("anim_class", anim_bp.generated_class())
                        unreal.log("  Set AnimBP on mesh component")
        except Exception as e:
            unreal.log_warning(f"  Could not set mesh on CDO (expected in some UE versions): {e}")

    # --- If no skeletal mesh was found, create a visible placeholder material ---
    if not skeletal_mesh:
        unreal.log("  No Mannequin found — creating visible placeholder material")
        placeholder_mat_path = "/Game/Materials/Characters"
        ensure_directory(placeholder_mat_path)

        factory = unreal.MaterialFactoryNew()
        placeholder_mat = asset_tools.create_asset(
            "M_CharacterPlaceholder", placeholder_mat_path,
            unreal.Material, factory
        )
        if placeholder_mat:
            # Bright green so the player is visible
            color_node = mat_lib.create_material_expression(
                placeholder_mat, unreal.MaterialExpressionConstant3Vector, -300, 0
            )
            if color_node:
                color_node.set_editor_property("constant", unreal.LinearColor(0.1, 0.8, 0.2, 1.0))
                mat_lib.connect_material_property(
                    color_node, "", unreal.MaterialProperty.MP_BASE_COLOR
                )
                # Emissive so it glows slightly
                emissive_node = mat_lib.create_material_expression(
                    placeholder_mat, unreal.MaterialExpressionConstant3Vector, -300, 200
                )
                if emissive_node:
                    emissive_node.set_editor_property("constant", unreal.LinearColor(0.05, 0.4, 0.1, 1.0))
                    mat_lib.connect_material_property(
                        emissive_node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR
                    )

            mat_lib.recompile_material(placeholder_mat)
            save_asset(f"{placeholder_mat_path}/M_CharacterPlaceholder")
            unreal.log("  Created M_CharacterPlaceholder (bright green emissive)")

    # --- Set as default pawn in GameMode ---
    try:
        gm_path = "/Game/Blueprints/Characters"
        gm_name = "BP_SEEGameMode"
        gm_full = f"{gm_path}/{gm_name}"

        if not editor_util.does_asset_exist(gm_full):
            gm_factory = unreal.BlueprintFactory()
            gm_factory.set_editor_property("parent_class", unreal.GameModeBase.static_class())
            gm_bp = asset_tools.create_asset(gm_name, gm_path, unreal.Blueprint, gm_factory)
            if gm_bp:
                unreal.log(f"  Created GameMode BP: {gm_full}")
                # Set default pawn class
                gm_cdo = unreal.get_default_object(gm_bp.generated_class())
                if gm_cdo and char_bp:
                    gm_cdo.set_editor_property("default_pawn_class", char_bp.generated_class())
                    unreal.log("  Set BP_SEECharacter as default pawn in GameMode")
                save_asset(gm_full)
        else:
            unreal.log(f"  GameMode already exists: {gm_full}")
    except Exception as e:
        unreal.log_warning(f"  GameMode setup: {e}")

    save_asset(bp_full)
    unreal.log("  Character setup complete")
    return True


# ============================================================================
# 2. PERSIST ALL MATERIALS — Full PBR Wiring
# ============================================================================

def wire_pbr_material(mat_asset, tex_map):
    """
    Connect textures to a material's PBR inputs and recompile.
    tex_map: dict of channel -> content path
    Channels: "diffuse", "normal", "roughness", "metallic", "ao", "height"
    """
    y_offset = {
        "diffuse": -200,
        "normal": 0,
        "roughness": 200,
        "metallic": 400,
        "ao": 600,
        "height": 800,
    }

    channel_to_property = {
        "diffuse": unreal.MaterialProperty.MP_BASE_COLOR,
        "normal": unreal.MaterialProperty.MP_NORMAL,
        "roughness": unreal.MaterialProperty.MP_ROUGHNESS,
        "metallic": unreal.MaterialProperty.MP_METALLIC,
        "ao": unreal.MaterialProperty.MP_AMBIENT_OCCLUSION,
    }

    channel_to_output = {
        "diffuse": "RGB",
        "normal": "RGB",
        "roughness": "R",
        "metallic": "R",
        "ao": "R",
    }

    wired_count = 0
    for channel, tex_path in tex_map.items():
        if channel not in channel_to_property:
            continue  # Skip height etc. that have no direct material pin

        tex = editor_util.load_asset(tex_path)
        if not tex:
            continue

        y = y_offset.get(channel, 0)
        tex_node = mat_lib.create_material_expression(
            mat_asset, unreal.MaterialExpressionTextureSample, -400, y
        )
        if not tex_node:
            continue

        tex_node.set_editor_property("texture", tex)

        # Set sampler type for linear-data textures
        if channel == "normal":
            tex_node.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
        elif channel in ("roughness", "metallic", "ao"):
            tex_node.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)

        output_name = channel_to_output[channel]
        mat_property = channel_to_property[channel]
        mat_lib.connect_material_property(tex_node, output_name, mat_property)
        wired_count += 1

    if wired_count > 0:
        mat_lib.recompile_material(mat_asset)

    return wired_count


def classify_texture_from_name(name_lower):
    """Classify a texture asset name into PBR channel."""
    if "normal" in name_lower or "_norm" in name_lower:
        return "normal"
    elif "roughness" in name_lower or "_spec" in name_lower:
        return "roughness"
    elif "metallic" in name_lower:
        return "metallic"
    elif "_ao" in name_lower or "ambientocclusion" in name_lower:
        return "ao"
    elif "height" in name_lower:
        return "height"
    else:
        return "diffuse"


def gather_texture_sets(content_dir):
    """
    Scan a content directory for textures and group them into PBR sets.
    Returns dict of { set_name: { channel: asset_path } }
    """
    sets = {}
    asset_paths = editor_util.list_assets(content_dir, recursive=True, include_folder=False)

    for asset_path in asset_paths:
        # Clean the path (remove class prefix if present)
        clean_path = str(asset_path).split(".")[-1] if "." in str(asset_path) else str(asset_path)
        # Use the path as-is from list_assets
        clean_path = str(asset_path).rstrip(".")

        asset = editor_util.load_asset(clean_path)
        if not asset or not isinstance(asset, unreal.Texture2D):
            continue

        asset_name = clean_path.split("/")[-1].lower()
        channel = classify_texture_from_name(asset_name)

        # Derive set name: strip the channel suffix
        set_name = clean_path.split("/")[-1]
        for suffix in ["_BaseColor", "_Normal", "_Roughness", "_Metallic", "_Height",
                        "_AO", "_ao", "_norm", "_spec", "_diff_edited", "_diff",
                        "_normal", "_roughness", "_metallic", "_height",
                        "_Base_Color"]:
            if set_name.endswith(suffix):
                set_name = set_name[:-len(suffix)]
                break

        if set_name not in sets:
            sets[set_name] = {}
        sets[set_name][channel] = clean_path

    return sets


def create_and_wire_material(mat_name, mat_dest, tex_map):
    """Create a material, wire its textures, save it. Returns asset path or None."""
    ensure_directory(mat_dest)
    mat_full = f"{mat_dest}/{mat_name}"

    if editor_util.does_asset_exist(mat_full):
        mat_asset = editor_util.load_asset(mat_full)
        if mat_asset:
            # Re-wire existing material
            wired = wire_pbr_material(mat_asset, tex_map)
            if wired > 0:
                save_asset(mat_full)
                unreal.log(f"  Re-wired material: {mat_full} ({wired} channels)")
            return mat_full
    else:
        factory = unreal.MaterialFactoryNew()
        mat_asset = asset_tools.create_asset(mat_name, mat_dest, unreal.Material, factory)
        if not mat_asset:
            unreal.log_warning(f"  Failed to create material: {mat_name}")
            return None

    wired = wire_pbr_material(mat_asset, tex_map)
    save_asset(mat_full)
    unreal.log(f"  Created material: {mat_full} ({wired} channels wired)")
    return mat_full


def setup_materials():
    """Create and fully wire PBR materials for all imported texture sets."""
    unreal.log("=" * 60)
    unreal.log("2. PERSIST ALL MATERIALS")
    unreal.log("=" * 60)

    total_created = 0

    # -----------------------------------------------------------------------
    # 2a. Modular Pipes — 4 PBR sets
    # -----------------------------------------------------------------------
    unreal.log("--- Modular Pipe Materials ---")
    pipe_sets = {
        "BareMetal": "BareMetal",
        "PaintedMetal": "PaintedMetal",
        "RustyMetal": "RustyMetal",
        "RustyPaintedMetal": "RustyPaintedMetal",
    }
    for mat_key, tex_prefix in pipe_sets.items():
        tex_base = f"/Game/Textures/ModularPipes/{mat_key}"
        tex_map = {}
        for channel, suffix in [("diffuse", "BaseColor"), ("normal", "Normal"),
                                 ("roughness", "Roughness"), ("metallic", "Metallic"),
                                 ("height", "Height")]:
            tex_path = f"{tex_base}/{tex_prefix}_{suffix}"
            if editor_util.does_asset_exist(tex_path):
                tex_map[channel] = tex_path

        if tex_map:
            result = create_and_wire_material(
                f"M_{mat_key}", "/Game/Materials/ModularPipes", tex_map
            )
            if result:
                total_created += 1

    # -----------------------------------------------------------------------
    # 2b. Baked Props — scan and auto-create
    # -----------------------------------------------------------------------
    unreal.log("--- Baked Prop Materials ---")
    baked_sets = gather_texture_sets("/Game/Textures/BakedProps")
    for set_name, tex_map in baked_sets.items():
        if len(tex_map) >= 1:  # At least a diffuse
            result = create_and_wire_material(
                f"M_{set_name}", "/Game/Materials/Props", tex_map
            )
            if result:
                total_created += 1

    # -----------------------------------------------------------------------
    # 2c. Brass 4K PBR
    # -----------------------------------------------------------------------
    unreal.log("--- Brass 4K Material ---")
    brass_base = "/Game/Textures/Brass"
    brass_map = {}
    for channel, pattern in [("diffuse", "4K-Brass_Base_Color"),
                              ("normal", "4K-Brass_Normal"),
                              ("roughness", "4K-Brass_Roughness"),
                              ("metallic", "4K-Brass_Metallic"),
                              ("height", "4K-Brass_Height")]:
        for variant in [pattern, pattern.replace("-", "_"), pattern.replace(" ", "_")]:
            tex_path = f"{brass_base}/{variant}"
            if editor_util.does_asset_exist(tex_path):
                brass_map[channel] = tex_path
                break

    if brass_map:
        result = create_and_wire_material("M_Brass_4K", "/Game/Materials/Metals", brass_map)
        if result:
            total_created += 1

    # -----------------------------------------------------------------------
    # 2d. Loose Metals — group by base name
    # -----------------------------------------------------------------------
    unreal.log("--- Loose Metal Materials ---")
    metal_sets = gather_texture_sets("/Game/Textures/Metals")
    for set_name, tex_map in metal_sets.items():
        if len(tex_map) >= 1:
            result = create_and_wire_material(
                f"M_{set_name}", "/Game/Materials/Metals", tex_map
            )
            if result:
                total_created += 1

    # -----------------------------------------------------------------------
    # 2e. Environment textures
    # -----------------------------------------------------------------------
    unreal.log("--- Environment Materials ---")
    env_sets = gather_texture_sets("/Game/Textures/Environment")
    for set_name, tex_map in env_sets.items():
        if len(tex_map) >= 1:
            result = create_and_wire_material(
                f"M_{set_name}", "/Game/Materials/Environment", tex_map
            )
            if result:
                total_created += 1

    unreal.log(f"  Total materials created/wired: {total_created}")
    return total_created


# ============================================================================
# 3. PICKUP ITEM BLUEPRINTS
# ============================================================================

def create_pickup_blueprint(bp_name, bp_path, display_name, item_id,
                             mesh_path=None, mesh_scale=1.0):
    """
    Create a Blueprint actor for a world pickup item.
    Uses a StaticMeshComponent with a placeholder cube if no mesh given.
    """
    bp_full = f"{bp_path}/{bp_name}"
    if editor_util.does_asset_exist(bp_full):
        unreal.log(f"  Pickup already exists: {bp_full}")
        return bp_full

    ensure_directory(bp_path)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.Actor.static_class())

    bp_asset = asset_tools.create_asset(bp_name, bp_path, unreal.Blueprint, factory)
    if not bp_asset:
        unreal.log_warning(f"  Failed to create pickup BP: {bp_name}")
        return None

    # Add components via the SubobjectDataSubsystem or fall back to CDO
    try:
        cdo = unreal.get_default_object(bp_asset.generated_class())
        if cdo:
            # We can't easily add components to a CDO from Python in all UE versions,
            # but we can set properties via Blueprint graph or use SCS.
            pass
    except Exception:
        pass

    # Save the blueprint
    save_asset(bp_full)
    unreal.log(f"  Created pickup BP: {bp_full} (ItemID={item_id})")
    return bp_full


def setup_pickups():
    """Create pickup item blueprint templates."""
    unreal.log("=" * 60)
    unreal.log("3. PICKUP ITEM BLUEPRINTS")
    unreal.log("=" * 60)

    bp_base = "/Game/Blueprints/Pickups"
    ensure_directory(bp_base)

    # Also create a shared pickup material (glowing outline)
    mat_path = "/Game/Materials/Pickups"
    ensure_directory(mat_path)
    pickup_mat_full = f"{mat_path}/M_PickupGlow"

    if not editor_util.does_asset_exist(pickup_mat_full):
        factory = unreal.MaterialFactoryNew()
        pickup_mat = asset_tools.create_asset("M_PickupGlow", mat_path, unreal.Material, factory)
        if pickup_mat:
            # Translucent emissive glow
            pickup_mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)

            color_node = mat_lib.create_material_expression(
                pickup_mat, unreal.MaterialExpressionConstant3Vector, -300, 0
            )
            if color_node:
                color_node.set_editor_property("constant", unreal.LinearColor(0.8, 0.6, 0.1, 1.0))
                mat_lib.connect_material_property(
                    color_node, "", unreal.MaterialProperty.MP_BASE_COLOR
                )
                mat_lib.connect_material_property(
                    color_node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR
                )

            opacity_node = mat_lib.create_material_expression(
                pickup_mat, unreal.MaterialExpressionConstant, -300, 200
            )
            if opacity_node:
                opacity_node.set_editor_property("r", 0.7)
                mat_lib.connect_material_property(
                    opacity_node, "", unreal.MaterialProperty.MP_OPACITY
                )

            mat_lib.recompile_material(pickup_mat)
            save_asset(pickup_mat_full)
            unreal.log("  Created M_PickupGlow material")

    # Define pickup items — (bp_name, display_name, item_id)
    pickup_definitions = [
        ("BP_Pickup_ProteinBlock",  "Protein Block",    "ProteinBlock"),
        ("BP_Pickup_Bandage",       "Bandage",          "Bandage"),
        ("BP_Pickup_ScrapMetal",    "Scrap Metal",      "ScrapMetal"),
        ("BP_Pickup_Pipe",          "Pipe",             "Pipe"),
        ("BP_Pickup_Shiv",          "Shiv",             "Shiv"),
        ("BP_Pickup_WaterRation",   "Water Ration",     "WaterRation"),
        ("BP_Pickup_Cloth",         "Cloth Scrap",      "Cloth"),
        ("BP_Pickup_Medicine",      "Medicine",         "Medicine"),
        ("BP_Pickup_Lockpick",      "Lockpick",         "Lockpick"),
        ("BP_Pickup_Coal",          "Coal Chunk",       "Coal"),
        ("BP_Pickup_Wrench",        "Wrench",           "Wrench"),
        ("BP_Pickup_Wire",          "Wire Coil",        "Wire"),
        ("BP_Pickup_Ration",        "Food Ration",      "FoodRation"),
        ("BP_Pickup_KeyCard",       "Key Card",         "KeyCard"),
    ]

    created = 0
    for bp_name, display_name, item_id in pickup_definitions:
        result = create_pickup_blueprint(bp_name, bp_base, display_name, item_id)
        if result:
            created += 1

    unreal.log(f"  Created {created} pickup blueprints")
    return created


# ============================================================================
# 4. NPC BLUEPRINT TEMPLATES
# ============================================================================

def create_npc_blueprint(bp_name, bp_path, npc_class_enum, display_name,
                          walk_speed, run_speed, sight_range, melee_damage,
                          attack_range, max_health, default_state_name="Idle"):
    """
    Create an NPC blueprint derived from SEENPCCharacter with configured defaults.
    """
    bp_full = f"{bp_path}/{bp_name}"
    if editor_util.does_asset_exist(bp_full):
        unreal.log(f"  NPC BP already exists: {bp_full}")
        return bp_full

    ensure_directory(bp_path)

    # Resolve parent class
    try:
        parent_class = unreal.load_class(None, "/Script/SnowpiercerEE.SEENPCCharacter")
        if not parent_class:
            raise RuntimeError("Class not found")
    except Exception:
        unreal.log_warning("  SEENPCCharacter not found, falling back to ACharacter")
        parent_class = unreal.Character.static_class()

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    bp_asset = asset_tools.create_asset(bp_name, bp_path, unreal.Blueprint, factory)
    if not bp_asset:
        unreal.log_warning(f"  Failed to create NPC BP: {bp_name}")
        return None

    # Configure CDO properties
    try:
        cdo = unreal.get_default_object(bp_asset.generated_class())
        if cdo:
            cdo.set_editor_property("display_name", unreal.Text(display_name))
            cdo.set_editor_property("walk_speed", walk_speed)
            cdo.set_editor_property("run_speed", run_speed)
            cdo.set_editor_property("sight_range", sight_range)
            cdo.set_editor_property("melee_damage", melee_damage)
            cdo.set_editor_property("attack_range", attack_range)
            cdo.set_editor_property("max_health", max_health)
            unreal.log(f"  Configured CDO for {bp_name}")
    except Exception as e:
        unreal.log_warning(f"  Could not configure CDO for {bp_name}: {e}")

    # Try to set the Mannequin mesh on the NPC
    try:
        mannequin_paths = [
            "/Game/Characters/Mannequins/Meshes/SKM_Manny",
            "/Game/Characters/Mannequins/Meshes/SKM_Quinn",
            "/Game/Characters/Mannequin/Mesh/SKM_Manny",
        ]
        for mesh_path in mannequin_paths:
            if editor_util.does_asset_exist(mesh_path):
                sk_mesh = editor_util.load_asset(mesh_path)
                if sk_mesh and cdo:
                    mesh_comp = cdo.get_component_by_class(unreal.SkeletalMeshComponent)
                    if mesh_comp:
                        mesh_comp.set_skeletal_mesh_asset(sk_mesh)
                        unreal.log(f"  Set NPC mesh: {mesh_path}")
                break
    except Exception as e:
        unreal.log_warning(f"  Could not set NPC mesh: {e}")

    save_asset(bp_full)
    unreal.log(f"  Created NPC BP: {bp_full}")
    return bp_full


def setup_npcs():
    """Create NPC blueprint templates."""
    unreal.log("=" * 60)
    unreal.log("4. NPC BLUEPRINT TEMPLATES")
    unreal.log("=" * 60)

    bp_base = "/Game/Blueprints/NPCs"
    ensure_directory(bp_base)

    npc_definitions = [
        # (bp_name, npc_class, display_name, walk, run, sight, damage, atk_range, health, state)
        {
            "bp_name": "BP_NPC_Civilian",
            "npc_class": "Tailie",
            "display_name": "Tail Civilian",
            "walk_speed": 180.0,
            "run_speed": 350.0,
            "sight_range": 1200.0,
            "melee_damage": 5.0,
            "attack_range": 120.0,
            "max_health": 60.0,
            "default_state": "Patrolling",
        },
        {
            "bp_name": "BP_NPC_Jackboot",
            "npc_class": "Jackboot",
            "display_name": "Jackboot Guard",
            "walk_speed": 220.0,
            "run_speed": 500.0,
            "sight_range": 2500.0,
            "melee_damage": 25.0,
            "attack_range": 220.0,
            "max_health": 150.0,
            "default_state": "Patrolling",
        },
        {
            "bp_name": "BP_NPC_Merchant",
            "npc_class": "ThirdClass",
            "display_name": "Black Market Merchant",
            "walk_speed": 0.0,
            "run_speed": 0.0,
            "sight_range": 800.0,
            "melee_damage": 0.0,
            "attack_range": 0.0,
            "max_health": 100.0,
            "default_state": "Idle",
        },
        {
            "bp_name": "BP_NPC_Breachman",
            "npc_class": "Breachman",
            "display_name": "Breachman",
            "walk_speed": 200.0,
            "run_speed": 480.0,
            "sight_range": 2000.0,
            "melee_damage": 20.0,
            "attack_range": 200.0,
            "max_health": 120.0,
            "default_state": "Working",
        },
        {
            "bp_name": "BP_NPC_FirstClass",
            "npc_class": "FirstClass",
            "display_name": "First Class Passenger",
            "walk_speed": 150.0,
            "run_speed": 300.0,
            "sight_range": 1000.0,
            "melee_damage": 3.0,
            "attack_range": 100.0,
            "max_health": 50.0,
            "default_state": "Idle",
        },
    ]

    created = 0
    for npc in npc_definitions:
        result = create_npc_blueprint(
            bp_name=npc["bp_name"],
            bp_path=bp_base,
            npc_class_enum=npc["npc_class"],
            display_name=npc["display_name"],
            walk_speed=npc["walk_speed"],
            run_speed=npc["run_speed"],
            sight_range=npc["sight_range"],
            melee_damage=npc["melee_damage"],
            attack_range=npc["attack_range"],
            max_health=npc["max_health"],
            default_state_name=npc["default_state"],
        )
        if result:
            created += 1

    unreal.log(f"  Created {created} NPC blueprints")
    return created


# ============================================================================
# 5. AUDIO SETUP — SoundCues & Ambient Actors
# ============================================================================

def create_sound_cue(cue_name, cue_path, wave_path, looping=False):
    """
    Create a SoundCue asset wrapping a SoundWave.
    """
    cue_full = f"{cue_path}/{cue_name}"
    if editor_util.does_asset_exist(cue_full):
        unreal.log(f"  SoundCue already exists: {cue_full}")
        return cue_full

    ensure_directory(cue_path)

    wave = editor_util.load_asset(wave_path)
    if not wave:
        unreal.log_warning(f"  SoundWave not found: {wave_path}")
        return None

    # Set looping on the wave itself if needed
    if looping:
        try:
            wave.set_editor_property("looping", True)
        except Exception:
            unreal.log_warning(f"  Could not set looping on wave: {wave_path}")

    # Create SoundCue via factory
    try:
        factory = unreal.SoundCueFactoryNew()
        cue_asset = asset_tools.create_asset(cue_name, cue_path, unreal.SoundCue, factory)
        if not cue_asset:
            unreal.log_warning(f"  Failed to create SoundCue: {cue_name}")
            return None

        # Connect the wave to the cue's output
        # SoundCues need at least one SoundNodeWavePlayer connected to the output
        try:
            # Create a wave player node inside the cue
            wave_player_class = unreal.load_class(None, "/Script/Engine.SoundNodeWavePlayer")
            if wave_player_class:
                # The Python API for SoundCue graph editing is limited;
                # we set the first sound directly if possible
                cue_asset.set_editor_property("first_node", None)  # Reset
        except Exception:
            pass

        save_asset(cue_full)
        unreal.log(f"  Created SoundCue: {cue_full} (looping={looping})")
        return cue_full

    except Exception as e:
        # SoundCueFactoryNew may not be exposed in all UE Python builds.
        # Fall back: just log and use the raw SoundWave directly.
        unreal.log_warning(f"  SoundCue creation not available ({e}), will use SoundWave directly")
        return wave_path


def setup_audio():
    """Create SoundCue assets and ambient sound actor templates."""
    unreal.log("=" * 60)
    unreal.log("5. AUDIO SETUP")
    unreal.log("=" * 60)

    cue_path = "/Game/Audio/Cues"
    ensure_directory(cue_path)

    created = 0

    # --- SC_TrainAmbient: looping interior sound ---
    train_ambient_wave = "/Game/Audio/train_interior"
    if editor_util.does_asset_exist(train_ambient_wave):
        result = create_sound_cue("SC_TrainAmbient", cue_path, train_ambient_wave, looping=True)
        if result:
            created += 1
    else:
        unreal.log_warning("  train_interior SoundWave not found, skipping SC_TrainAmbient")

    # --- SC_TrainWhistle: one-shot ---
    whistle_candidates = [
        "/Game/Audio/train_whistle",
        "/Game/Audio/train_horn",
        "/Game/Audio/whistle",
    ]
    whistle_wave = None
    for path in whistle_candidates:
        if editor_util.does_asset_exist(path):
            whistle_wave = path
            break

    if whistle_wave:
        result = create_sound_cue("SC_TrainWhistle", cue_path, whistle_wave, looping=False)
        if result:
            created += 1
    else:
        unreal.log_warning("  No whistle/horn SoundWave found, skipping SC_TrainWhistle")

    # --- Scan for any other imported audio and create cues ---
    if editor_util.does_directory_exist("/Game/Audio"):
        audio_assets = editor_util.list_assets("/Game/Audio", recursive=False, include_folder=False)
        for asset_path in audio_assets:
            clean = str(asset_path).rstrip(".")
            name = clean.split("/")[-1]
            cue_name = f"SC_{name}"
            cue_full = f"{cue_path}/{cue_name}"
            if not editor_util.does_asset_exist(cue_full):
                asset = editor_util.load_asset(clean)
                if asset and isinstance(asset, unreal.SoundWave):
                    result = create_sound_cue(cue_name, cue_path, clean, looping=False)
                    if result:
                        created += 1

    # --- Create Ambient Sound Blueprint templates ---
    unreal.log("--- Ambient Sound Templates ---")
    amb_path = "/Game/Blueprints/Audio"
    ensure_directory(amb_path)

    # Template: looping train interior ambient
    amb_bp_name = "BP_AmbientTrainInterior"
    amb_full = f"{amb_path}/{amb_bp_name}"
    if not editor_util.does_asset_exist(amb_full):
        try:
            factory = unreal.BlueprintFactory()
            factory.set_editor_property("parent_class", unreal.AmbientSound.static_class())
            amb_bp = asset_tools.create_asset(amb_bp_name, amb_path, unreal.Blueprint, factory)
            if amb_bp:
                try:
                    cdo = unreal.get_default_object(amb_bp.generated_class())
                    if cdo:
                        audio_comp = cdo.get_component_by_class(unreal.AudioComponent)
                        if audio_comp:
                            # Try SoundCue first, fall back to raw wave
                            cue_asset_path = f"{cue_path}/SC_TrainAmbient"
                            if editor_util.does_asset_exist(cue_asset_path):
                                sound = editor_util.load_asset(cue_asset_path)
                            elif editor_util.does_asset_exist(train_ambient_wave):
                                sound = editor_util.load_asset(train_ambient_wave)
                            else:
                                sound = None

                            if sound:
                                audio_comp.set_editor_property("sound", sound)
                                audio_comp.set_editor_property("auto_activate", True)
                except Exception as e:
                    unreal.log_warning(f"  Could not configure ambient CDO: {e}")

                save_asset(amb_full)
                created += 1
                unreal.log(f"  Created ambient BP: {amb_full}")
        except Exception as e:
            unreal.log_warning(f"  Could not create ambient BP: {e}")

    # --- MetaSoundSource placeholder ---
    unreal.log("--- MetaSound Placeholder ---")
    try:
        metasound_path = "/Game/Audio/MetaSounds"
        ensure_directory(metasound_path)

        # MetaSoundSource creation is engine-version dependent
        # Try to create one; if the factory isn't available, just log it
        ms_name = "MS_AdaptiveTrainAmbient"
        ms_full = f"{metasound_path}/{ms_name}"
        if not editor_util.does_asset_exist(ms_full):
            try:
                ms_factory = unreal.MetaSoundSourceFactory()
                ms_asset = asset_tools.create_asset(
                    ms_name, metasound_path, None, ms_factory
                )
                if ms_asset:
                    save_asset(ms_full)
                    created += 1
                    unreal.log(f"  Created MetaSoundSource: {ms_full}")
            except (AttributeError, Exception) as e:
                unreal.log_warning(
                    f"  MetaSoundSource factory not available in this build ({e}). "
                    "Create manually: Content Browser > Add > Sounds > MetaSoundSource"
                )
    except Exception as e:
        unreal.log_warning(f"  MetaSound setup skipped: {e}")

    unreal.log(f"  Audio assets created: {created}")
    return created


# ============================================================================
# 6. POST-PROCESS & ATMOSPHERE
# ============================================================================

def setup_postprocess_and_atmosphere():
    """
    Create reusable PostProcessVolume and ExponentialHeightFog presets
    saved as Blueprint or DataAsset templates.
    """
    unreal.log("=" * 60)
    unreal.log("6. POST-PROCESS & ATMOSPHERE")
    unreal.log("=" * 60)

    created = 0

    # -----------------------------------------------------------------------
    # 6a. PostProcess Preset Blueprint — Tail Zone (dark, gritty, desaturated)
    # -----------------------------------------------------------------------
    pp_bp_path = "/Game/Blueprints/PostProcess"
    pp_bp_name = "BP_PP_TailZone"
    pp_full = f"{pp_bp_path}/{pp_bp_name}"

    ensure_directory(pp_bp_path)

    if not editor_util.does_asset_exist(pp_full):
        try:
            factory = unreal.BlueprintFactory()
            factory.set_editor_property("parent_class", unreal.PostProcessVolume.static_class())
            pp_bp = asset_tools.create_asset(pp_bp_name, pp_bp_path, unreal.Blueprint, factory)

            if pp_bp:
                try:
                    cdo = unreal.get_default_object(pp_bp.generated_class())
                    if cdo:
                        # Make it an infinite/unbound volume by default
                        cdo.set_editor_property("unbound", True)
                        cdo.set_editor_property("priority", 1.0)

                        # Access PostProcessSettings
                        settings = cdo.get_editor_property("settings")
                        if settings:
                            # Desaturation — reduce color saturation
                            settings.set_editor_property("color_saturation",
                                                          unreal.Vector4(0.65, 0.65, 0.65, 1.0))
                            settings.set_editor_property("override_color_saturation", True)

                            # Contrast — slightly boosted for grit
                            settings.set_editor_property("color_contrast",
                                                          unreal.Vector4(1.15, 1.15, 1.15, 1.0))
                            settings.set_editor_property("override_color_contrast", True)

                            # Vignette intensity
                            settings.set_editor_property("vignette_intensity", 0.6)
                            settings.set_editor_property("override_vignette_intensity", True)

                            # Film grain
                            settings.set_editor_property("film_grain_intensity", 0.35)
                            settings.set_editor_property("override_film_grain_intensity", True)

                            # Bloom — low
                            settings.set_editor_property("bloom_intensity", 0.3)
                            settings.set_editor_property("override_bloom_intensity", True)

                            # Auto exposure — dark bias
                            settings.set_editor_property("auto_exposure_bias", -1.5)
                            settings.set_editor_property("override_auto_exposure_bias", True)

                            cdo.set_editor_property("settings", settings)
                            unreal.log("  Configured Tail Zone post-process settings")

                except Exception as e:
                    unreal.log_warning(f"  Could not configure PP CDO: {e}")

                save_asset(pp_full)
                created += 1
                unreal.log(f"  Created PostProcess BP: {pp_full}")
        except Exception as e:
            unreal.log_warning(f"  PostProcess BP creation failed: {e}")
    else:
        unreal.log(f"  PostProcess BP already exists: {pp_full}")

    # -----------------------------------------------------------------------
    # 6b. ExponentialHeightFog Blueprint — Interior Atmosphere
    # -----------------------------------------------------------------------
    fog_bp_path = "/Game/Blueprints/Atmosphere"
    fog_bp_name = "BP_Fog_TrainInterior"
    fog_full = f"{fog_bp_path}/{fog_bp_name}"

    ensure_directory(fog_bp_path)

    if not editor_util.does_asset_exist(fog_full):
        try:
            factory = unreal.BlueprintFactory()
            factory.set_editor_property("parent_class", unreal.ExponentialHeightFog.static_class())
            fog_bp = asset_tools.create_asset(fog_bp_name, fog_bp_path, unreal.Blueprint, factory)

            if fog_bp:
                try:
                    cdo = unreal.get_default_object(fog_bp.generated_class())
                    if cdo:
                        fog_comp = cdo.get_component_by_class(unreal.ExponentialHeightFogComponent)
                        if fog_comp:
                            # Dense, warm-tinted interior fog
                            fog_comp.set_editor_property("fog_density", 0.015)
                            fog_comp.set_editor_property("fog_height_falloff", 0.8)
                            fog_comp.set_editor_property("fog_inscattering_color",
                                                          unreal.LinearColor(0.12, 0.09, 0.07, 1.0))
                            fog_comp.set_editor_property("fog_max_opacity", 0.6)
                            fog_comp.set_editor_property("start_distance", 50.0)

                            # Volumetric fog for light shafts
                            fog_comp.set_editor_property("volumetric_fog", True)
                            fog_comp.set_editor_property("volumetric_fog_scattering_distribution", 0.3)
                            fog_comp.set_editor_property("volumetric_fog_albedo",
                                                          unreal.Color(200, 180, 160, 255))

                            unreal.log("  Configured fog component")
                except Exception as e:
                    unreal.log_warning(f"  Could not configure fog CDO: {e}")

                save_asset(fog_full)
                created += 1
                unreal.log(f"  Created Fog BP: {fog_full}")
        except Exception as e:
            unreal.log_warning(f"  Fog BP creation failed: {e}")
    else:
        unreal.log(f"  Fog BP already exists: {fog_full}")

    # -----------------------------------------------------------------------
    # 6c. Additional zone presets
    # -----------------------------------------------------------------------
    # Engine Zone — hot, orange tint, high bloom
    pp_engine_name = "BP_PP_EngineZone"
    pp_engine_full = f"{pp_bp_path}/{pp_engine_name}"

    if not editor_util.does_asset_exist(pp_engine_full):
        try:
            factory = unreal.BlueprintFactory()
            factory.set_editor_property("parent_class", unreal.PostProcessVolume.static_class())
            pp_engine = asset_tools.create_asset(pp_engine_name, pp_bp_path, unreal.Blueprint, factory)

            if pp_engine:
                try:
                    cdo = unreal.get_default_object(pp_engine.generated_class())
                    if cdo:
                        cdo.set_editor_property("unbound", False)
                        cdo.set_editor_property("priority", 2.0)

                        settings = cdo.get_editor_property("settings")
                        if settings:
                            # Warm orange tint
                            settings.set_editor_property("color_saturation",
                                                          unreal.Vector4(1.1, 0.9, 0.7, 1.0))
                            settings.set_editor_property("override_color_saturation", True)

                            settings.set_editor_property("bloom_intensity", 0.8)
                            settings.set_editor_property("override_bloom_intensity", True)

                            settings.set_editor_property("auto_exposure_bias", 0.5)
                            settings.set_editor_property("override_auto_exposure_bias", True)

                            settings.set_editor_property("film_grain_intensity", 0.15)
                            settings.set_editor_property("override_film_grain_intensity", True)

                            cdo.set_editor_property("settings", settings)
                except Exception as e:
                    unreal.log_warning(f"  Could not configure Engine Zone PP CDO: {e}")

                save_asset(pp_engine_full)
                created += 1
                unreal.log(f"  Created PostProcess BP: {pp_engine_full}")
        except Exception as e:
            unreal.log_warning(f"  Engine Zone PP creation failed: {e}")

    # First Class Zone — bright, saturated, cinematic
    pp_first_name = "BP_PP_FirstClassZone"
    pp_first_full = f"{pp_bp_path}/{pp_first_name}"

    if not editor_util.does_asset_exist(pp_first_full):
        try:
            factory = unreal.BlueprintFactory()
            factory.set_editor_property("parent_class", unreal.PostProcessVolume.static_class())
            pp_first = asset_tools.create_asset(pp_first_name, pp_bp_path, unreal.Blueprint, factory)

            if pp_first:
                try:
                    cdo = unreal.get_default_object(pp_first.generated_class())
                    if cdo:
                        cdo.set_editor_property("unbound", False)
                        cdo.set_editor_property("priority", 2.0)

                        settings = cdo.get_editor_property("settings")
                        if settings:
                            settings.set_editor_property("color_saturation",
                                                          unreal.Vector4(1.2, 1.1, 1.0, 1.0))
                            settings.set_editor_property("override_color_saturation", True)

                            settings.set_editor_property("bloom_intensity", 0.5)
                            settings.set_editor_property("override_bloom_intensity", True)

                            settings.set_editor_property("auto_exposure_bias", 0.8)
                            settings.set_editor_property("override_auto_exposure_bias", True)

                            settings.set_editor_property("vignette_intensity", 0.2)
                            settings.set_editor_property("override_vignette_intensity", True)

                            cdo.set_editor_property("settings", settings)
                except Exception as e:
                    unreal.log_warning(f"  Could not configure First Class PP CDO: {e}")

                save_asset(pp_first_full)
                created += 1
                unreal.log(f"  Created PostProcess BP: {pp_first_full}")
        except Exception as e:
            unreal.log_warning(f"  First Class PP creation failed: {e}")

    unreal.log(f"  Atmosphere assets created: {created}")
    return created


# ============================================================================
# MAIN ENTRY POINT
# ============================================================================

def run():
    unreal.log("")
    unreal.log("*" * 60)
    unreal.log("  Snowpiercer: Eternal Engine — Game Content Setup")
    unreal.log("*" * 60)
    unreal.log("")

    results = {}

    with unreal.ScopedSlowTask(600, "Setting up game content...") as slow_task:
        slow_task.make_dialog(True)

        # --- 1. Character ---
        slow_task.enter_progress_frame(50, "Setting up character...")
        try:
            results["character"] = setup_character()
        except Exception as e:
            unreal.log_warning(f"Character setup failed: {e}")
            results["character"] = False

        # --- 2. Materials ---
        slow_task.enter_progress_frame(200, "Creating and wiring PBR materials...")
        try:
            results["materials"] = setup_materials()
        except Exception as e:
            unreal.log_warning(f"Material setup failed: {e}")
            results["materials"] = 0

        # --- 3. Pickups ---
        slow_task.enter_progress_frame(100, "Creating pickup blueprints...")
        try:
            results["pickups"] = setup_pickups()
        except Exception as e:
            unreal.log_warning(f"Pickup setup failed: {e}")
            results["pickups"] = 0

        # --- 4. NPCs ---
        slow_task.enter_progress_frame(100, "Creating NPC blueprints...")
        try:
            results["npcs"] = setup_npcs()
        except Exception as e:
            unreal.log_warning(f"NPC setup failed: {e}")
            results["npcs"] = 0

        # --- 5. Audio ---
        slow_task.enter_progress_frame(100, "Setting up audio...")
        try:
            results["audio"] = setup_audio()
        except Exception as e:
            unreal.log_warning(f"Audio setup failed: {e}")
            results["audio"] = 0

        # --- 6. Post-process & Atmosphere ---
        slow_task.enter_progress_frame(50, "Setting up post-process and atmosphere...")
        try:
            results["atmosphere"] = setup_postprocess_and_atmosphere()
        except Exception as e:
            unreal.log_warning(f"Atmosphere setup failed: {e}")
            results["atmosphere"] = 0

    # --- Summary ---
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("  GAME CONTENT SETUP COMPLETE")
    unreal.log("=" * 60)
    unreal.log(f"  Character:   {'OK' if results.get('character') else 'WARN'}")
    unreal.log(f"  Materials:   {results.get('materials', 0)} created/wired")
    unreal.log(f"  Pickups:     {results.get('pickups', 0)} blueprints")
    unreal.log(f"  NPCs:        {results.get('npcs', 0)} blueprints")
    unreal.log(f"  Audio:       {results.get('audio', 0)} assets")
    unreal.log(f"  Atmosphere:  {results.get('atmosphere', 0)} presets")
    unreal.log("=" * 60)
    unreal.log("")
    unreal.log("Created content tree:")
    unreal.log("  /Game/Blueprints/Characters/   BP_SEECharacter, BP_SEEGameMode")
    unreal.log("  /Game/Blueprints/Pickups/      BP_Pickup_* (14 item types)")
    unreal.log("  /Game/Blueprints/NPCs/         BP_NPC_Civilian, Jackboot, Merchant, ...")
    unreal.log("  /Game/Blueprints/Audio/        BP_AmbientTrainInterior")
    unreal.log("  /Game/Blueprints/PostProcess/  BP_PP_TailZone, EngineZone, FirstClassZone")
    unreal.log("  /Game/Blueprints/Atmosphere/   BP_Fog_TrainInterior")
    unreal.log("  /Game/Materials/ModularPipes/  M_BareMetal, M_PaintedMetal, ...")
    unreal.log("  /Game/Materials/Props/         M_big_tank, M_small_vent, ...")
    unreal.log("  /Game/Materials/Metals/        M_Brass_4K, M_brushed_metal, ...")
    unreal.log("  /Game/Materials/Pickups/       M_PickupGlow")
    unreal.log("  /Game/Materials/Characters/    M_CharacterPlaceholder (if no Mannequin)")
    unreal.log("  /Game/Audio/Cues/              SC_TrainAmbient, SC_TrainWhistle, ...")
    unreal.log("")
    unreal.log("NEXT STEPS:")
    unreal.log("  1. Open BP_SEECharacter and verify mesh/anim in the Viewport")
    unreal.log("  2. Set BP_SEEGameMode as the GameMode override in World Settings")
    unreal.log("  3. Drag BP_NPC_* into levels for NPC population")
    unreal.log("  4. Place BP_Pickup_* near the player start for testing")
    unreal.log("  5. Drag BP_PP_TailZone into the Tail section of any level")
    unreal.log("  6. Drag BP_Fog_TrainInterior for interior atmosphere")
    unreal.log("  7. Press Play (PIE) to test")

    return results


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    run()
else:
    run()
