"""
Snowpiercer: Eternal Engine — Fix Demo Visibility
Run in the editor with DevTestCar open.

Adds floor geometry, sky atmosphere, fog, and post-process volume
so the level isn't a black void.
"""

import unreal

level_lib = unreal.EditorLevelLibrary
editor_util = unreal.EditorAssetLibrary

def run():
    unreal.log("=== Fixing Demo Visibility ===")

    # --- Add a large floor plane using a scaled cube ---
    # Use engine's built-in cube mesh as floor
    floor = level_lib.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0.0, 0.0, -10.0),
        unreal.Rotator(0.0, 0.0, 0.0)
    )
    if floor:
        floor.set_actor_label("Floor_TrainCar")
        sm = floor.get_component_by_class(unreal.StaticMeshComponent)
        if sm:
            # Use engine basic cube mesh
            cube_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
            if cube_mesh:
                sm.set_static_mesh(cube_mesh)
                # Scale to train car floor: 12m x 3m x 0.1m (UE units = cm)
                floor.set_actor_scale3d(unreal.Vector(12.0, 3.0, 0.1))
                # Try to apply a metal material
                mat_path = "/Game/Materials/ModularPipes/M_BareMetal"
                if not editor_util.does_asset_exist(mat_path):
                    mat_path = "/Game/Materials/ModularPipes/M_PaintedMetal"
                if editor_util.does_asset_exist(mat_path):
                    mat = editor_util.load_asset(mat_path)
                    sm.set_material(0, mat)
                unreal.log("  Placed floor (12m x 3m)")

    # --- Walls (left and right) ---
    for side, y_pos, label in [("Left", 150.0, "Wall_Left"), ("Right", -150.0, "Wall_Right")]:
        wall = level_lib.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(0.0, y_pos, 140.0),
            unreal.Rotator(0.0, 0.0, 0.0)
        )
        if wall:
            wall.set_actor_label(label)
            sm = wall.get_component_by_class(unreal.StaticMeshComponent)
            if sm:
                cube_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
                if cube_mesh:
                    sm.set_static_mesh(cube_mesh)
                    # 12m long, 0.1m thick, 2.8m tall
                    wall.set_actor_scale3d(unreal.Vector(12.0, 0.1, 2.8))
                    mat_path = "/Game/Materials/ModularPipes/M_PaintedMetal"
                    if editor_util.does_asset_exist(mat_path):
                        mat = editor_util.load_asset(mat_path)
                        sm.set_material(0, mat)
                    unreal.log(f"  Placed {label}")

    # --- Ceiling ---
    ceiling = level_lib.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0.0, 0.0, 285.0),
        unreal.Rotator(0.0, 0.0, 0.0)
    )
    if ceiling:
        ceiling.set_actor_label("Ceiling_TrainCar")
        sm = ceiling.get_component_by_class(unreal.StaticMeshComponent)
        if sm:
            cube_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
            if cube_mesh:
                sm.set_static_mesh(cube_mesh)
                ceiling.set_actor_scale3d(unreal.Vector(12.0, 3.0, 0.1))
                mat_path = "/Game/Materials/ModularPipes/M_RustyMetal"
                if editor_util.does_asset_exist(mat_path):
                    mat = editor_util.load_asset(mat_path)
                    sm.set_material(0, mat)
                unreal.log("  Placed ceiling")

    # --- End walls (front and back) ---
    for side, x_pos, label in [("Front", 600.0, "Wall_Front"), ("Back", -600.0, "Wall_Back")]:
        endwall = level_lib.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(x_pos, 0.0, 140.0),
            unreal.Rotator(0.0, 0.0, 0.0)
        )
        if endwall:
            endwall.set_actor_label(label)
            sm = endwall.get_component_by_class(unreal.StaticMeshComponent)
            if sm:
                cube_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
                if cube_mesh:
                    sm.set_static_mesh(cube_mesh)
                    # 3m wide, 0.1m thick, 2.8m tall
                    endwall.set_actor_scale3d(unreal.Vector(0.1, 3.0, 2.8))
                    mat_path = "/Game/Materials/ModularPipes/M_RustyPaintedMetal"
                    if editor_util.does_asset_exist(mat_path):
                        mat = editor_util.load_asset(mat_path)
                        sm.set_material(0, mat)
                    unreal.log(f"  Placed {label}")

    # --- Sky Atmosphere (visible through windows / for ambient) ---
    sky_atmo = level_lib.spawn_actor_from_class(
        unreal.SkyAtmosphere,
        unreal.Vector(0.0, 0.0, 0.0)
    )
    if sky_atmo:
        sky_atmo.set_actor_label("SkyAtmosphere_Frozen")
        unreal.log("  Placed Sky Atmosphere")

    # --- Exponential Height Fog (cold atmosphere) ---
    fog = level_lib.spawn_actor_from_class(
        unreal.ExponentialHeightFog,
        unreal.Vector(0.0, 0.0, 0.0)
    )
    if fog:
        fog.set_actor_label("InteriorHaze")
        fog_comp = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
        if fog_comp:
            fog_comp.set_editor_property("fog_density", 0.002)
            fog_comp.set_editor_property("fog_inscattering_color", unreal.LinearColor(0.4, 0.45, 0.55, 1.0))
        unreal.log("  Placed Exponential Height Fog")

    # --- Post Process Volume (cold color grade) ---
    ppv = level_lib.spawn_actor_from_class(
        unreal.PostProcessVolume,
        unreal.Vector(0.0, 0.0, 140.0)
    )
    if ppv:
        ppv.set_actor_label("PostProcess_TrainInterior")
        ppv.set_editor_property("unbound", True)
        settings = ppv.get_editor_property("settings")
        # Slight blue tint, desaturated for cold industrial look
        settings.set_editor_property("override_auto_exposure_bias", True)
        settings.set_editor_property("auto_exposure_bias", 0.5)
        ppv.set_editor_property("settings", settings)
        unreal.log("  Placed Post Process Volume (unbound)")

    # --- Make sure PlayerStart is at a good height ---
    actors = level_lib.get_all_level_actors()
    for actor in actors:
        if isinstance(actor, unreal.PlayerStart):
            actor.set_actor_location(unreal.Vector(0.0, 0.0, 50.0), False, False)
            unreal.log("  Repositioned PlayerStart to (0, 0, 50)")
            break

    # --- Save ---
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    level_subsystem.save_current_level()

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("VISIBILITY FIX COMPLETE")
    unreal.log("=" * 60)
    unreal.log("  - Train car box: 12m x 3m x 2.85m (floor, walls, ceiling)")
    unreal.log("  - Sky atmosphere + fog + post-process")
    unreal.log("  - PlayerStart repositioned above floor")
    unreal.log("")
    unreal.log("Press Play to test. Build Lighting if shadows look flat.")

run()
