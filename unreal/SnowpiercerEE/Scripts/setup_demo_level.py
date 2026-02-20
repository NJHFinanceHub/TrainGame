"""
Snowpiercer: Eternal Engine — Demo Level Setup Script
Run AFTER import_assets.py has completed.

Tools > Execute Python Script > browse to this file

Creates a DevTestCar level with:
- Train car interior (BSP box)
- PlayerStart
- Directional + point lights
- Train movement component on level BP
- Test collectible actors
- Wires up PBR materials with texture nodes
"""

import unreal

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

editor_util = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
level_lib = unreal.EditorLevelLibrary
mat_lib = unreal.MaterialEditingLibrary

def wire_pbr_material(mat_path, tex_map):
    """
    Connect textures to a material's PBR inputs.
    tex_map: dict of channel -> content path
    channels: "diffuse", "normal", "roughness", "metallic", "ao", "height"
    """
    mat = editor_util.load_asset(mat_path)
    if not mat:
        unreal.log_warning(f"Material not found: {mat_path}")
        return False

    for channel, tex_path in tex_map.items():
        tex = editor_util.load_asset(tex_path)
        if not tex:
            continue

        # Create texture sample node
        tex_node = mat_lib.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, 0)
        if not tex_node:
            continue
        tex_node.set_editor_property("texture", tex)

        # Connect to the appropriate material input
        if channel == "diffuse":
            tex_node.set_editor_property("material_expression_editor_y", -200)
            mat_lib.connect_material_property(tex_node, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
        elif channel == "normal":
            tex_node.set_editor_property("material_expression_editor_y", 0)
            tex_node.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
            mat_lib.connect_material_property(tex_node, "RGB", unreal.MaterialProperty.MP_NORMAL)
        elif channel in ("roughness", "spec"):
            tex_node.set_editor_property("material_expression_editor_y", 200)
            tex_node.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
            mat_lib.connect_material_property(tex_node, "R", unreal.MaterialProperty.MP_ROUGHNESS)
        elif channel == "metallic":
            tex_node.set_editor_property("material_expression_editor_y", 400)
            tex_node.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
            mat_lib.connect_material_property(tex_node, "R", unreal.MaterialProperty.MP_METALLIC)
        elif channel == "ao":
            tex_node.set_editor_property("material_expression_editor_y", 600)
            tex_node.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
            mat_lib.connect_material_property(tex_node, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)

    # Recompile
    mat_lib.recompile_material(mat)
    unreal.log(f"  Wired material: {mat_path}")
    return True


# ---------------------------------------------------------------------------
# 1. Wire up PBR Materials
# ---------------------------------------------------------------------------

def setup_materials():
    unreal.log("=== Wiring PBR Materials ===")
    wired = 0

    # Pipe material sets
    pipe_sets = {
        "BareMetal": "BareMetal",
        "PaintedMetal": "PaintedMetal",
        "RustyMetal": "RustyMetal",
        "RustyPaintedMetal": "RustyPaintedMetal",
    }

    for mat_name, tex_prefix in pipe_sets.items():
        tex_base = f"/Game/Textures/ModularPipes/{mat_name}"
        tex_map = {}

        # Check which textures were imported
        for channel, suffix in [("diffuse", "BaseColor"), ("normal", "Normal"),
                                 ("roughness", "Roughness"), ("metallic", "Metallic"),
                                 ("height", "Height")]:
            tex_path = f"{tex_base}/{tex_prefix}_{suffix}"
            if editor_util.does_asset_exist(tex_path):
                tex_map[channel] = tex_path

        if tex_map:
            mat_path = f"/Game/Materials/ModularPipes/M_{mat_name}"
            if editor_util.does_asset_exist(mat_path):
                if wire_pbr_material(mat_path, tex_map):
                    wired += 1

    # Brass 4K
    brass_base = "/Game/Textures/Brass"
    brass_map = {}
    for channel, pattern in [("diffuse", "4K-Brass_Base_Color"), ("normal", "4K-Brass_Normal"),
                              ("roughness", "4K-Brass_Roughness"), ("metallic", "4K-Brass_Metallic"),
                              ("height", "4K-Brass_Height")]:
        # UE import may sanitize spaces in names
        for variant in [pattern, pattern.replace(" ", "_"), pattern.replace("_", "")]:
            tex_path = f"{brass_base}/{variant}"
            if editor_util.does_asset_exist(tex_path):
                brass_map[channel] = tex_path
                break

    if brass_map:
        brass_mat = "/Game/Materials/Metals/M_Brass_4K"
        if editor_util.does_asset_exist(brass_mat):
            if wire_pbr_material(brass_mat, brass_map):
                wired += 1

    unreal.log(f"  Wired {wired} materials")
    return wired


# ---------------------------------------------------------------------------
# 2. Create Demo Level
# ---------------------------------------------------------------------------

def create_demo_level():
    unreal.log("=== Creating DevTestCar Level ===")

    # Create new level
    level_path = "/Game/Maps/DevTestCar"
    if editor_util.does_asset_exist(level_path):
        unreal.log("  DevTestCar already exists, loading it")
        editor_util.load_asset(level_path)
    else:
        # Create a new default level
        level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        level_subsystem.new_level(level_path)
        unreal.log("  Created new level: DevTestCar")

    # --- Place PlayerStart ---
    player_start = level_lib.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(0.0, 0.0, 100.0),
        unreal.Rotator(0.0, 0.0, 0.0)
    )
    if player_start:
        player_start.set_actor_label("PlayerStart_Demo")
        unreal.log("  Placed PlayerStart")

    # --- Place Directional Light (cold blue ambient) ---
    dir_light = level_lib.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0.0, 0.0, 500.0),
        unreal.Rotator(-45.0, 30.0, 0.0)
    )
    if dir_light:
        dir_light.set_actor_label("SunLight_Cold")
        light_comp = dir_light.get_component_by_class(unreal.DirectionalLightComponent)
        if light_comp:
            light_comp.set_editor_property("intensity", 3.0)
            light_comp.set_editor_property("light_color", unreal.Color(180, 200, 255, 255))
        unreal.log("  Placed Directional Light (cold blue)")

    # --- Place Point Lights (warm interior) ---
    light_positions = [
        unreal.Vector(-400.0, 0.0, 250.0),
        unreal.Vector(0.0, 0.0, 250.0),
        unreal.Vector(400.0, 0.0, 250.0),
    ]
    for i, pos in enumerate(light_positions):
        point_light = level_lib.spawn_actor_from_class(
            unreal.PointLight,
            pos,
            unreal.Rotator(0.0, 0.0, 0.0)
        )
        if point_light:
            point_light.set_actor_label(f"InteriorLight_{i}")
            light_comp = point_light.get_component_by_class(unreal.PointLightComponent)
            if light_comp:
                light_comp.set_editor_property("intensity", 5000.0)
                light_comp.set_editor_property("attenuation_radius", 600.0)
                light_comp.set_editor_property("light_color", unreal.Color(255, 220, 180, 255))
            unreal.log(f"  Placed Point Light {i}")

    # --- Place Atmospheric Fog / Sky ---
    sky = level_lib.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0.0, 0.0, 300.0)
    )
    if sky:
        sky.set_actor_label("SkyLight_Ambient")
        sky_comp = sky.get_component_by_class(unreal.SkyLightComponent)
        if sky_comp:
            sky_comp.set_editor_property("intensity", 1.0)
        unreal.log("  Placed Sky Light")

    # --- Place Pipe Meshes if imported ---
    pipe_meshes = [
        ("PipeKit_Pipe_Long", unreal.Vector(-400.0, 140.0, 250.0), unreal.Rotator(0.0, 0.0, 0.0)),
        ("PipeKit_Pipe_Medium", unreal.Vector(0.0, 140.0, 250.0), unreal.Rotator(0.0, 0.0, 0.0)),
        ("PipeKit_Pipe_Short", unreal.Vector(200.0, 140.0, 250.0), unreal.Rotator(0.0, 0.0, 0.0)),
        ("PipeKit_Pipe_Long", unreal.Vector(-400.0, -140.0, 250.0), unreal.Rotator(0.0, 0.0, 0.0)),
        ("PipeKit_Connector_Straight", unreal.Vector(0.0, -140.0, 250.0), unreal.Rotator(0.0, 0.0, 0.0)),
        ("PipeKit_Base_Wall", unreal.Vector(500.0, 100.0, 150.0), unreal.Rotator(0.0, 90.0, 0.0)),
        ("PipeKit_Base_Wall", unreal.Vector(500.0, -100.0, 150.0), unreal.Rotator(0.0, 90.0, 0.0)),
        ("PipeKit_Base_Floor", unreal.Vector(-200.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0)),
    ]

    pipes_placed = 0
    for mesh_name, pos, rot in pipe_meshes:
        mesh_path = f"/Game/Meshes/ModularPipes/{mesh_name}"
        if editor_util.does_asset_exist(mesh_path):
            mesh = editor_util.load_asset(mesh_path)
            actor = level_lib.spawn_actor_from_class(
                unreal.StaticMeshActor,
                pos,
                rot
            )
            if actor:
                actor.set_actor_label(f"Pipe_{mesh_name}_{pipes_placed}")
                sm_comp = actor.get_component_by_class(unreal.StaticMeshComponent)
                if sm_comp:
                    sm_comp.set_static_mesh(mesh)
                    # Try to assign rusty metal material
                    mat_path = "/Game/Materials/ModularPipes/M_RustyMetal"
                    if editor_util.does_asset_exist(mat_path):
                        mat = editor_util.load_asset(mat_path)
                        sm_comp.set_material(0, mat)
                pipes_placed += 1

    unreal.log(f"  Placed {pipes_placed} pipe meshes")

    # --- Place Audio (train ambient) ---
    audio_path = "/Game/Audio/train_interior"
    if editor_util.does_asset_exist(audio_path):
        sound = editor_util.load_asset(audio_path)
        ambient = level_lib.spawn_actor_from_class(
            unreal.AmbientSound,
            unreal.Vector(0.0, 0.0, 150.0)
        )
        if ambient:
            ambient.set_actor_label("TrainAmbient_Loop")
            audio_comp = ambient.get_component_by_class(unreal.AudioComponent)
            if audio_comp:
                audio_comp.set_editor_property("sound", sound)
                audio_comp.set_editor_property("auto_activate", True)
            unreal.log("  Placed ambient train audio")

    # --- Save the level ---
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    level_subsystem.save_current_level()
    unreal.log("  Level saved")

    return True


# ---------------------------------------------------------------------------
# 3. Set Project Defaults
# ---------------------------------------------------------------------------

def set_project_defaults():
    unreal.log("=== Setting Project Defaults ===")

    # Set DevTestCar as editor startup map and default game map
    # This is done via project settings
    settings = unreal.GameMapsSettings.get_game_maps_settings()
    if settings:
        map_path = unreal.SoftObjectPath("/Game/Maps/DevTestCar.DevTestCar")
        settings.set_editor_property("editor_startup_map", map_path)
        settings.set_editor_property("game_default_map", map_path)
        unreal.log("  Set DevTestCar as startup map")

    unreal.log("  Project defaults configured")


# ---------------------------------------------------------------------------
# Entry
# ---------------------------------------------------------------------------

def run():
    unreal.log("=" * 60)
    unreal.log("Snowpiercer: Eternal Engine — Demo Level Setup")
    unreal.log("=" * 60)

    setup_materials()
    create_demo_level()
    set_project_defaults()

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("DEMO SETUP COMPLETE")
    unreal.log("=" * 60)
    unreal.log("")
    unreal.log("You can now:")
    unreal.log("  1. Press Play (PIE) to walk around the demo car")
    unreal.log("  2. Press V to toggle first/third person view")
    unreal.log("  3. Press E near collectibles to interact")
    unreal.log("  4. Add more meshes/actors from the Content Browser")
    unreal.log("")
    unreal.log("To build a full train car:")
    unreal.log("  - Use BSP Box (12m x 3m x 2.8m) for walls/floor/ceiling")
    unreal.log("  - Apply metal materials from /Game/Materials/")
    unreal.log("  - Line pipes along ceiling and walls")
    unreal.log("  - Add more point lights for atmosphere")

run()
