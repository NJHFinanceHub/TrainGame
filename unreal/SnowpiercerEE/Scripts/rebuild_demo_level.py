"""
Snowpiercer: Eternal Engine — Full Train Rebuild
Run in editor: Tools > Execute Python Script

Builds 100 connected train cars from Caboose (rear) to Engine (front).
Each car: 100m long x 15m wide x 8m tall with themed materials and lighting.

Layout (back to front):
  Cars 0-4:    TAIL — Dark, cramped, makeshift, no windows
  Cars 5-14:   THIRD CLASS — Industrial, agriculture, clinics, mess halls
  Cars 15-29:  UTILITY — Greenhouses, aquaculture, seed banks, cattle
  Cars 30-49:  SECOND CLASS — Night car, carnival, quarters, drawers
  Cars 50-74:  FIRST CLASS — Dining, aquarium, sauna, library, bowling
  Cars 75-89:  HOSPITALITY — Gardens, spa, lounge, art gallery
  Cars 90-94:  CONDUCTOR — Command, comms, armory, brig
  Cars 95-98:  ENGINE SUPPORT — Battery, cooling, fuel
  Car 99:      THE ETERNAL ENGINE
"""

import unreal

level_lib = unreal.EditorLevelLibrary
editor_util = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

# ---------------------------------------------------------------------------
# Config — all units in cm (UE default)
# ---------------------------------------------------------------------------

CAR_LENGTH = 100000.0   # 1000m long (10x scale)
CAR_WIDTH  = 15000.0    # 150m wide (10x scale)
CAR_HEIGHT = 8000.0     # 80m tall (10x scale)
WALL_THICK = 200.0      # 2m walls (10x scale)
DOOR_WIDTH = 2500.0     # 25m door opening (10x scale)
DOOR_HEIGHT = 3500.0    # 35m door height (10x scale)
CAR_GAP = 1500.0        # 15m connector between cars (10x scale)

NUM_CARS = 100

# ---------------------------------------------------------------------------
# Car section definitions
# ---------------------------------------------------------------------------

SECTIONS = [
    # (start, end, name, floor_color, wall_color, ceil_color, light_color, light_intensity, fog_density)
    # Tail — pitch dark, rusty, desperate
    (0, 4, "Tail",
     (0.03, 0.03, 0.03), (0.05, 0.04, 0.03), (0.04, 0.03, 0.02),
     (200, 120, 60), 2000.0, 0.008),
    # Third Class — industrial, dim fluorescent
    (5, 14, "ThirdClass",
     (0.06, 0.06, 0.07), (0.08, 0.08, 0.09), (0.07, 0.06, 0.05),
     (220, 200, 170), 4000.0, 0.005),
    # Utility — greenhouses, agriculture, slightly green-tinted
    (15, 29, "Utility",
     (0.05, 0.07, 0.05), (0.07, 0.09, 0.07), (0.06, 0.08, 0.06),
     (200, 240, 180), 5000.0, 0.004),
    # Second Class — warmer, lived-in, amber lighting
    (30, 49, "SecondClass",
     (0.10, 0.08, 0.06), (0.12, 0.10, 0.08), (0.09, 0.07, 0.05),
     (255, 210, 150), 6000.0, 0.003),
    # First Class — luxurious, bright warm gold
    (50, 74, "FirstClass",
     (0.15, 0.12, 0.08), (0.18, 0.15, 0.10), (0.14, 0.11, 0.07),
     (255, 230, 190), 8000.0, 0.002),
    # Hospitality — spa/lounge, cool elegant
    (75, 89, "Hospitality",
     (0.12, 0.13, 0.15), (0.15, 0.16, 0.18), (0.10, 0.11, 0.14),
     (220, 230, 255), 7000.0, 0.002),
    # Conductor — military, cold steel blue
    (90, 94, "Conductor",
     (0.08, 0.09, 0.12), (0.10, 0.11, 0.14), (0.07, 0.08, 0.11),
     (180, 200, 255), 5000.0, 0.004),
    # Engine Support — utilitarian, red emergency lighting
    (95, 98, "EngineSupport",
     (0.06, 0.04, 0.04), (0.08, 0.05, 0.05), (0.05, 0.03, 0.03),
     (255, 100, 80), 4000.0, 0.006),
    # The Eternal Engine — bright white-blue, humming power
    (99, 99, "Engine",
     (0.10, 0.10, 0.12), (0.14, 0.14, 0.18), (0.12, 0.12, 0.16),
     (200, 220, 255), 10000.0, 0.001),
]

# Car names for specific landmark cars
CAR_NAMES = {
    0: "Caboose",
    1: "Tail_Quarters_A", 2: "Tail_Quarters_B", 3: "Tail_Storage", 4: "BioSecurity",
    5: "ThirdClass_Quarters_A", 6: "ThirdClass_Quarters_B", 7: "ThirdClass_Quarters_C",
    8: "Mess_Hall", 9: "Brakemen_Garrison", 10: "Sanitation",
    11: "ThirdClass_Clinic", 12: "ThirdClass_Market", 13: "Cattle_Car", 14: "Border_Checkpoint",
    15: "Greenhouse_A", 16: "Greenhouse_B", 17: "Greenhouse_C", 18: "Greenhouse_D",
    19: "Aquaculture", 20: "Seed_Bank", 21: "Apiary", 22: "Spice_House",
    23: "Poultry_A", 24: "Poultry_B", 25: "Refrigeration_A", 26: "Refrigeration_B",
    27: "Battery_Station_A", 28: "Water_Treatment", 29: "Utility_Junction",
    30: "Night_Car", 31: "Drawers_A", 32: "Drawers_B", 33: "Drawers_C",
    34: "SecondClass_Quarters_A", 35: "SecondClass_Quarters_B", 36: "SecondClass_Quarters_C",
    37: "SecondClass_Quarters_D", 38: "SecondClass_Clinic", 39: "Carnival_Car",
    40: "SecondClass_Market", 41: "School_Car", 42: "Workshop", 43: "Laundry",
    44: "Bakery", 45: "Tailor", 46: "Chapel", 47: "SecondClass_Lounge",
    48: "Bath_House", 49: "Battery_Station_B",
    50: "FirstClass_Dining", 51: "FirstClass_Quarters_A", 52: "FirstClass_Quarters_B",
    53: "FirstClass_Quarters_C", 54: "FirstClass_Quarters_D",
    55: "Aquarium_Car", 56: "Library_Car", 57: "Bowling_Alley",
    58: "Sauna_Car", 59: "Cigar_Lounge", 60: "Wine_Cellar",
    61: "Music_Hall", 62: "FirstClass_Quarters_E", 63: "FirstClass_Quarters_F",
    64: "Penthouse_A", 65: "Penthouse_B", 66: "Art_Gallery",
    67: "Jeweler", 68: "Perfumery", 69: "FirstClass_Clinic",
    70: "Theatre", 71: "Observatory", 72: "Salon",
    73: "Meat_Locker", 74: "Bay_Door",
    75: "Spa_Reception", 76: "Spa_Pools", 77: "Zen_Garden",
    78: "Cocktail_Lounge", 79: "Dance_Hall", 80: "Sky_Lounge",
    81: "Solarium", 82: "Promenade_A", 83: "Promenade_B",
    84: "VIP_Dining", 85: "VIP_Quarters_A", 86: "VIP_Quarters_B",
    87: "Sculpture_Garden", 88: "Grand_Ballroom", 89: "Hospitality_Bridge",
    90: "Command_Center", 91: "Communications", 92: "Armory",
    93: "Brig", 94: "Officers_Quarters",
    95: "Battery_Array", 96: "Cooling_Systems", 97: "Fuel_Processing", 98: "Engine_Access",
    99: "Eternal_Engine",
}


# ---------------------------------------------------------------------------
# Material cache
# ---------------------------------------------------------------------------

_mat_cache = {}

def get_material(name, r, g, b, roughness=0.7, metallic=0.8):
    key = name
    if key in _mat_cache:
        return _mat_cache[key]

    mat_path = f"/Game/Materials/Train/M_{name}"
    if editor_util.does_asset_exist(mat_path):
        mat = editor_util.load_asset(mat_path)
        _mat_cache[key] = mat
        return mat

    dir_path = "/Game/Materials/Train"
    if not editor_util.does_directory_exist(dir_path):
        editor_util.make_directory(dir_path)

    mat = asset_tools.create_asset(f"M_{name}", "/Game/Materials/Train",
                                     unreal.Material, unreal.MaterialFactoryNew())
    if not mat:
        return None

    mat_lib = unreal.MaterialEditingLibrary

    color_node = mat_lib.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -300, -200)
    if color_node:
        color_node.set_editor_property("constant", unreal.LinearColor(r, g, b, 1.0))
        mat_lib.connect_material_property(color_node, "", unreal.MaterialProperty.MP_BASE_COLOR)

    rough_node = mat_lib.create_material_expression(mat, unreal.MaterialExpressionConstant, -300, 0)
    if rough_node:
        rough_node.set_editor_property("r", roughness)
        mat_lib.connect_material_property(rough_node, "", unreal.MaterialProperty.MP_ROUGHNESS)

    metal_node = mat_lib.create_material_expression(mat, unreal.MaterialExpressionConstant, -300, 200)
    if metal_node:
        metal_node.set_editor_property("r", metallic)
        mat_lib.connect_material_property(metal_node, "", unreal.MaterialProperty.MP_METALLIC)

    mat_lib.recompile_material(mat)
    editor_util.save_asset(mat_path, only_if_is_dirty=False)
    _mat_cache[key] = mat
    return mat


def get_section(car_idx):
    for start, end, name, floor_c, wall_c, ceil_c, light_c, light_i, fog_d in SECTIONS:
        if start <= car_idx <= end:
            return name, floor_c, wall_c, ceil_c, light_c, light_i, fog_d
    return "Unknown", (0.1,0.1,0.1), (0.1,0.1,0.1), (0.1,0.1,0.1), (255,255,255), 5000.0, 0.003


# ---------------------------------------------------------------------------
# Geometry helpers
# ---------------------------------------------------------------------------

_cube_mesh = None

def get_cube():
    global _cube_mesh
    if _cube_mesh is None:
        _cube_mesh = editor_util.load_asset("/Engine/BasicShapes/Cube")
    return _cube_mesh


def place_box(label, location, size, material=None):
    actor = level_lib.spawn_actor_from_class(
        unreal.StaticMeshActor,
        location,
        unreal.Rotator(0.0, 0.0, 0.0)
    )
    if not actor:
        return None
    actor.set_actor_label(label)
    sm = actor.get_component_by_class(unreal.StaticMeshComponent)
    if sm:
        cube = get_cube()
        if cube:
            sm.set_static_mesh(cube)
            actor.set_actor_scale3d(unreal.Vector(
                size[0] / 100.0,
                size[1] / 100.0,
                size[2] / 100.0
            ))
            if material:
                sm.set_material(0, material)
    return actor


# ---------------------------------------------------------------------------
# Cleanup
# ---------------------------------------------------------------------------

def cleanup():
    unreal.log("=== Cleaning up all existing level actors ===")
    actors = level_lib.get_all_level_actors()
    removed = 0
    keep_types = (unreal.WorldSettings, unreal.GameModeBase)
    for actor in actors:
        label = actor.get_actor_label()
        # Keep world settings, delete everything else we placed
        if isinstance(actor, unreal.WorldSettings):
            continue
        if (label.startswith("Car") or label.startswith("Connector") or
            label.startswith("Light_") or label.startswith("Pipe_") or
            label.startswith("Floor") or label.startswith("Wall") or
            label.startswith("Ceiling") or label.startswith("Sky") or
            label.startswith("Interior") or label.startswith("PostProcess") or
            label.startswith("PlayerStart") or label.startswith("Sun") or
            label.startswith("Fog") or label.startswith("Ambient")):
            level_lib.destroy_actor(actor)
            removed += 1
    unreal.log(f"  Removed {removed} actors")


# ---------------------------------------------------------------------------
# Build train
# ---------------------------------------------------------------------------

def build_train():
    unreal.log("=== Building 100-Car Train ===")

    total_span = CAR_LENGTH + CAR_GAP
    total_actors = 0

    for car_idx in range(NUM_CARS):
        section_name, floor_c, wall_c, ceil_c, light_c, light_i, fog_d = get_section(car_idx)
        car_name = CAR_NAMES.get(car_idx, f"{section_name}_{car_idx}")
        car_x = car_idx * total_span

        # Materials per section (cached)
        mat_floor = get_material(f"{section_name}_Floor", *floor_c, roughness=0.85, metallic=0.9)
        mat_wall = get_material(f"{section_name}_Wall", *wall_c, roughness=0.75, metallic=0.7)
        mat_ceil = get_material(f"{section_name}_Ceiling", *ceil_c, roughness=0.9, metallic=0.6)
        mat_door = get_material(f"{section_name}_Door",
                                wall_c[0]*1.2, wall_c[1]*1.1, wall_c[2]*0.9,
                                roughness=0.6, metallic=0.95)
        mat_conn = get_material("Connector", 0.04, 0.04, 0.05, roughness=0.8, metallic=0.85)

        half_l = CAR_LENGTH / 2.0
        half_w = CAR_WIDTH / 2.0
        half_h = CAR_HEIGHT / 2.0

        # Floor
        place_box(f"Car{car_idx}_{car_name}_Floor",
                  unreal.Vector(car_x, 0.0, -WALL_THICK/2.0),
                  (CAR_LENGTH, CAR_WIDTH, WALL_THICK), mat_floor)

        # Ceiling
        place_box(f"Car{car_idx}_{car_name}_Ceiling",
                  unreal.Vector(car_x, 0.0, CAR_HEIGHT + WALL_THICK/2.0),
                  (CAR_LENGTH, CAR_WIDTH, WALL_THICK), mat_ceil)

        # Left wall
        place_box(f"Car{car_idx}_{car_name}_WallL",
                  unreal.Vector(car_x, half_w + WALL_THICK/2.0, half_h),
                  (CAR_LENGTH, WALL_THICK, CAR_HEIGHT), mat_wall)

        # Right wall
        place_box(f"Car{car_idx}_{car_name}_WallR",
                  unreal.Vector(car_x, -(half_w + WALL_THICK/2.0), half_h),
                  (CAR_LENGTH, WALL_THICK, CAR_HEIGHT), mat_wall)

        total_actors += 4

        # Front wall — door opening if not last car
        front_x = car_x + half_l + WALL_THICK/2.0
        if car_idx < NUM_CARS - 1:
            side_w = (CAR_WIDTH - DOOR_WIDTH) / 2.0
            place_box(f"Car{car_idx}_FrontL",
                      unreal.Vector(front_x, DOOR_WIDTH/2.0 + side_w/2.0, half_h),
                      (WALL_THICK, side_w, CAR_HEIGHT), mat_wall)
            place_box(f"Car{car_idx}_FrontR",
                      unreal.Vector(front_x, -(DOOR_WIDTH/2.0 + side_w/2.0), half_h),
                      (WALL_THICK, side_w, CAR_HEIGHT), mat_wall)
            header_h = CAR_HEIGHT - DOOR_HEIGHT
            place_box(f"Car{car_idx}_FrontHeader",
                      unreal.Vector(front_x, 0.0, DOOR_HEIGHT + header_h/2.0),
                      (WALL_THICK, DOOR_WIDTH, header_h), mat_door)

            # Connector
            conn_x = car_x + half_l + WALL_THICK + CAR_GAP/2.0
            place_box(f"Conn_{car_idx}_Floor",
                      unreal.Vector(conn_x, 0.0, -WALL_THICK/2.0),
                      (CAR_GAP, DOOR_WIDTH, WALL_THICK), mat_conn)
            place_box(f"Conn_{car_idx}_WL",
                      unreal.Vector(conn_x, DOOR_WIDTH/2.0 + WALL_THICK/2.0, DOOR_HEIGHT/2.0),
                      (CAR_GAP, WALL_THICK, DOOR_HEIGHT), mat_conn)
            place_box(f"Conn_{car_idx}_WR",
                      unreal.Vector(conn_x, -(DOOR_WIDTH/2.0 + WALL_THICK/2.0), DOOR_HEIGHT/2.0),
                      (CAR_GAP, WALL_THICK, DOOR_HEIGHT), mat_conn)
            place_box(f"Conn_{car_idx}_Ceil",
                      unreal.Vector(conn_x, 0.0, DOOR_HEIGHT + WALL_THICK/2.0),
                      (CAR_GAP, DOOR_WIDTH + WALL_THICK*2.0, WALL_THICK), mat_conn)
            total_actors += 7
        else:
            place_box(f"Car{car_idx}_FrontWall",
                      unreal.Vector(front_x, 0.0, half_h),
                      (WALL_THICK, CAR_WIDTH, CAR_HEIGHT), mat_wall)
            total_actors += 1

        # Back wall
        back_x = car_x - half_l - WALL_THICK/2.0
        if car_idx > 0:
            side_w = (CAR_WIDTH - DOOR_WIDTH) / 2.0
            place_box(f"Car{car_idx}_BackL",
                      unreal.Vector(back_x, DOOR_WIDTH/2.0 + side_w/2.0, half_h),
                      (WALL_THICK, side_w, CAR_HEIGHT), mat_wall)
            place_box(f"Car{car_idx}_BackR",
                      unreal.Vector(back_x, -(DOOR_WIDTH/2.0 + side_w/2.0), half_h),
                      (WALL_THICK, side_w, CAR_HEIGHT), mat_wall)
            header_h = CAR_HEIGHT - DOOR_HEIGHT
            place_box(f"Car{car_idx}_BackHeader",
                      unreal.Vector(back_x, 0.0, DOOR_HEIGHT + header_h/2.0),
                      (WALL_THICK, DOOR_WIDTH, header_h), mat_door)
            total_actors += 3
        else:
            place_box(f"Car{car_idx}_BackWall",
                      unreal.Vector(back_x, 0.0, half_h),
                      (WALL_THICK, CAR_WIDTH, CAR_HEIGHT), mat_wall)
            total_actors += 1

        # Interior lights — every ~200m along car, alternating sides
        num_lights = int(CAR_LENGTH / 20000.0)
        for li in range(num_lights):
            lx = car_x - half_l + (li + 0.5) * (CAR_LENGTH / num_lights)
            ly = 3000.0 if (li % 2 == 0) else -3000.0
            pl = level_lib.spawn_actor_from_class(
                unreal.PointLight,
                unreal.Vector(lx, ly, CAR_HEIGHT - 800.0),
                unreal.Rotator(0.0, 0.0, 0.0)
            )
            if pl:
                pl.set_actor_label(f"Light_Car{car_idx}_{li}")
                plc = pl.get_component_by_class(unreal.PointLightComponent)
                if plc:
                    plc.set_editor_property("intensity", light_i)
                    plc.set_editor_property("attenuation_radius", 20000.0)
                    plc.set_editor_property("light_color",
                        unreal.Color(int(light_c[0]), int(light_c[1]), int(light_c[2]), 255))
                total_actors += 1

        # Progress log every 10 cars
        if car_idx % 10 == 0:
            unreal.log(f"  Built car {car_idx}: {car_name} [{section_name}] ({total_actors} actors)")

    unreal.log(f"  Total: {NUM_CARS} cars, {total_actors} actors placed")


# ---------------------------------------------------------------------------
# Global atmosphere
# ---------------------------------------------------------------------------

def setup_atmosphere():
    unreal.log("=== Setting up atmosphere ===")

    total_length = NUM_CARS * (CAR_LENGTH + CAR_GAP)
    mid_x = total_length / 2.0

    # Directional light
    dl = level_lib.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(mid_x, 0.0, 20000.0),
        unreal.Rotator(-35.0, 20.0, 0.0)
    )
    if dl:
        dl.set_actor_label("SunLight_Frozen")
        lc = dl.get_component_by_class(unreal.DirectionalLightComponent)
        if lc:
            lc.set_editor_property("intensity", 1.5)
            lc.set_editor_property("light_color", unreal.Color(150, 170, 210, 255))

    # Sky light
    sl = level_lib.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(mid_x, 0.0, 10000.0)
    )
    if sl:
        sl.set_actor_label("SkyLight_Ambient")
        sc = sl.get_component_by_class(unreal.SkyLightComponent)
        if sc:
            sc.set_editor_property("intensity", 0.3)

    # Sky atmosphere
    sa = level_lib.spawn_actor_from_class(
        unreal.SkyAtmosphere,
        unreal.Vector(mid_x, 0.0, 0.0)
    )
    if sa:
        sa.set_actor_label("SkyAtmosphere_Frozen")

    # Fog
    fog = level_lib.spawn_actor_from_class(
        unreal.ExponentialHeightFog,
        unreal.Vector(mid_x, 0.0, 0.0)
    )
    if fog:
        fog.set_actor_label("Fog_TrainInterior")
        fc = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
        if fc:
            fc.set_editor_property("fog_density", 0.002)
            fc.set_editor_property("fog_inscattering_color", unreal.LinearColor(0.25, 0.28, 0.35, 1.0))

    # Post-process — cold industrial
    ppv = level_lib.spawn_actor_from_class(
        unreal.PostProcessVolume,
        unreal.Vector(mid_x, 0.0, CAR_HEIGHT/2.0)
    )
    if ppv:
        ppv.set_actor_label("PostProcess_Train")
        ppv.set_editor_property("unbound", True)
        settings = ppv.get_editor_property("settings")
        settings.set_editor_property("override_auto_exposure_bias", True)
        settings.set_editor_property("auto_exposure_bias", 0.2)
        ppv.set_editor_property("settings", settings)

    unreal.log("  Atmosphere configured")


# ---------------------------------------------------------------------------
# Player start
# ---------------------------------------------------------------------------

def setup_player():
    unreal.log("=== Setting up player ===")

    # Remove existing
    for actor in level_lib.get_all_level_actors():
        if isinstance(actor, unreal.PlayerStart):
            level_lib.destroy_actor(actor)

    # Start in tail section (car 0) — authentic Snowpiercer experience
    ps = level_lib.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(0.0, 0.0, 1000.0),
        unreal.Rotator(0.0, 0.0, 0.0)
    )
    if ps:
        ps.set_actor_label("PlayerStart_Tail")
        unreal.log("  PlayerStart placed in Caboose (Car 0)")
        unreal.log("  Walk forward through 100 cars to reach the Eternal Engine!")


# ---------------------------------------------------------------------------
# Entry
# ---------------------------------------------------------------------------

def run():
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  Full Train Rebuild — 100 Cars")
    unreal.log("=" * 60)
    unreal.log("")

    cleanup()
    build_train()
    setup_atmosphere()
    setup_player()

    # Save level and all dirty assets (materials, etc.)
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    level_subsystem.save_current_level()
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

    total_length_m = (NUM_CARS * (CAR_LENGTH + CAR_GAP)) / 100.0
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("  BUILD COMPLETE")
    unreal.log("=" * 60)
    unreal.log(f"  Total train length: {total_length_m:.0f}m ({total_length_m/1000:.1f}km)")
    unreal.log(f"  Each car: {CAR_LENGTH/100:.0f}m x {CAR_WIDTH/100:.0f}m x {CAR_HEIGHT/100:.0f}m")
    unreal.log(f"  Sections: Tail → 3rd Class → Utility → 2nd Class → 1st Class → Hospitality → Conductor → Engine")
    unreal.log("")
    unreal.log("  LAYOUT (back to front):")
    unreal.log("    Cars 0-4:    TAIL — Dark, desperate, makeshift")
    unreal.log("    Cars 5-14:   THIRD CLASS — Industrial, mess halls, clinics")
    unreal.log("    Cars 15-29:  UTILITY — Greenhouses, aquaculture, agriculture")
    unreal.log("    Cars 30-49:  SECOND CLASS — Night car, carnival, quarters")
    unreal.log("    Cars 50-74:  FIRST CLASS — Dining, aquarium, sauna, library")
    unreal.log("    Cars 75-89:  HOSPITALITY — Spa, gardens, ballroom")
    unreal.log("    Cars 90-94:  CONDUCTOR — Command, armory, brig")
    unreal.log("    Cars 95-98:  ENGINE SUPPORT — Battery, cooling, fuel")
    unreal.log("    Car 99:      THE ETERNAL ENGINE")
    unreal.log("")
    unreal.log("  Press Play — you start in the Tail. Walk to the Engine!")

run()
