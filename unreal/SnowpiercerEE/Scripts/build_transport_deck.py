"""
Snowpiercer: Eternal Engine -- Lower Transport Deck Level Builder
Run in editor: Tools > Execute Python Script

Builds the Lower Transport Deck beneath cars 15-82 (Third Class through
First Class). A sub-level cargo system with mini-rail tracks, access points,
security devices, and environmental details.

The transport deck runs as a continuous lower level beneath the main
passenger deck. Ceiling height is ~1200cm (12m) — cramped, industrial,
functional. Mini-rail tracks run down the center with passing sidings.

Car ranges and zones (narrative numbering):
  Cars 15-35: Third Class Lower     — Food distribution, water pipes, raw
  Cars 36-55: Second Class Lower    — Cargo sorting, laundry runs, Night Car cellars
  Cars 56-70: Working Spine Lower   — Machinery access, heavy industrial, loud
  Cars 71-82: First Class Lower     — Luxury goods transit, smuggling routes, security

Each lower deck segment: 12000cm x 4000cm x 1200cm (120m x 40m x 12m)
Positioned directly beneath upper deck floor (Z offset = -1350cm from upper floor)

Map: /Game/Maps/TransportDeck_Lower
"""

import unreal

# ---------------------------------------------------------------------------
# API shortcuts
# ---------------------------------------------------------------------------

level_lib = unreal.EditorLevelLibrary
editor_util = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mat_lib = unreal.MaterialEditingLibrary

# ---------------------------------------------------------------------------
# Dimensions (cm) — 10x scale, lower deck is shorter than upper
# ---------------------------------------------------------------------------

CAR_LENGTH  = 12000.0
CAR_WIDTH   = 4000.0
DECK_HEIGHT = 1200.0    # Lower deck ceiling: 12m (cramped industrial)
WALL_THICK  = 150.0
DOOR_WIDTH  = 1200.0    # Narrower doors than upper level
DOOR_HEIGHT = 1000.0    # Low clearance doors
CAR_GAP     = 1000.0    # Same connector spacing as upper

# Vertical offset: lower deck sits beneath upper floor
# Upper floor is at Z=0, lower deck ceiling at Z=-150 (wall thickness)
# Lower deck floor at Z=-1350
DECK_Z_OFFSET = -1350.0  # Floor of lower deck

# Car range: narrative cars 15-82
DECK_START = 15
DECK_END   = 82
NUM_SEGMENTS = DECK_END - DECK_START + 1  # 68 segments

# ---------------------------------------------------------------------------
# Zone definitions for lower deck sections
# ---------------------------------------------------------------------------

DECK_ZONES = [
    # (start, end, name, floor_rgb, wall_rgb, ceil_rgb, light_rgb, light_int, description)
    (15, 35, "ThirdClass_Lower",
     (0.04, 0.04, 0.04), (0.06, 0.05, 0.04), (0.05, 0.04, 0.03),
     (200, 160, 100), 1200.0,
     "Food distribution, water pipes, raw industrial"),
    (36, 55, "SecondClass_Lower",
     (0.06, 0.05, 0.04), (0.08, 0.07, 0.05), (0.06, 0.05, 0.04),
     (220, 180, 120), 1500.0,
     "Cargo sorting, laundry, Night Car cellars"),
    (56, 70, "WorkingSpine_Lower",
     (0.05, 0.05, 0.06), (0.07, 0.07, 0.08), (0.05, 0.05, 0.06),
     (180, 200, 220), 1800.0,
     "Machinery access, heavy industrial, loud"),
    (71, 82, "FirstClass_Lower",
     (0.07, 0.06, 0.05), (0.09, 0.08, 0.06), (0.07, 0.06, 0.05),
     (240, 200, 150), 1600.0,
     "Luxury goods transit, smuggling, security"),
]

# ---------------------------------------------------------------------------
# Segment definitions — notable lower deck segments
# ---------------------------------------------------------------------------

SEGMENT_DEFS = {
    # car_idx: (name, features, has_access_hatch, has_elevator, has_stairs,
    #           has_junction, is_smuggle_route, security_level)
    # security_level: 0=none, 1=basic, 2=moderate, 3=heavy
    15: ("Food_Staging",        "Crate stacks, loading platform", True, False, True, False, False, 0),
    16: ("Greenhouse_Access",   "Root systems visible, damp", True, False, False, False, False, 0),
    18: ("Water_Main",          "Pipe junction, valve controls", True, False, True, False, False, 0),
    20: ("Distribution_Hub_A",  "Multi-track junction, sorting area", True, True, True, True, False, 1),
    22: ("Spice_Storage",       "Aromatic, sealed containers", False, False, True, False, True, 0),
    25: ("Cold_Storage_Access", "Refrigeration units, frost on walls", True, False, True, False, False, 1),
    28: ("Water_Treatment_Sub", "Filtration machinery, loud pumps", True, True, True, True, False, 1),
    30: ("Night_Car_Cellar",    "Kronole lab, hidden room", False, False, True, False, True, 0),
    31: ("Drawer_Machinery",    "Suspension pod systems, eerie", True, False, False, False, True, 2),
    35: ("Border_Checkpoint_Sub", "Security gate, scanner", True, False, True, True, False, 2),
    38: ("Second_Clinic_Sub",   "Medical supply depot", True, True, True, False, False, 1),
    40: ("Market_Warehouse",    "Goods storage, merchant drops", True, False, True, False, True, 0),
    42: ("Workshop_Foundry",    "Forge access, molten glow", True, False, True, False, False, 0),
    45: ("Tailor_Stockroom",    "Fabric rolls, dye vats", False, False, True, False, True, 0),
    48: ("Bath_House_Pipes",    "Steam pipes, condensation", True, False, False, False, False, 0),
    50: ("First_Dining_Cellar", "Wine storage, cheese aging", True, True, True, True, False, 2),
    52: ("First_Quarters_Sub",  "Secure personal storage", False, False, True, False, False, 3),
    55: ("Aquarium_Machinery",  "Pump systems, water tanks", True, False, True, False, False, 1),
    57: ("Bowling_Mechanics",   "Pin systems, maintenance", False, False, True, False, False, 0),
    59: ("Cigar_Lounge_Vault",  "Humidor system, contraband drop", False, False, True, False, True, 2),
    60: ("Wine_Cellar_Deep",    "Temperature-controlled racks", True, True, True, True, False, 2),
    62: ("First_Quarter_Sub_E", "Hidden passage, rebel cache", False, False, True, False, True, 1),
    64: ("Penthouse_Vault",     "Safes, valuables, heavy security", True, True, True, False, False, 3),
    66: ("Gallery_Archive",     "Art storage, fragile cargo", True, True, True, False, False, 2),
    69: ("First_Clinic_Lab",    "Research lab, experimental drugs", True, False, True, False, True, 3),
    70: ("Theatre_Basement",    "Costume storage, trap doors", True, False, True, True, False, 1),
    72: ("Salon_Supplies",      "Chemical storage, ventilation", False, False, True, False, False, 0),
    74: ("Bay_Door_Mechanism",  "Massive gears, exterior access", True, True, True, True, False, 3),
    76: ("Spa_Plumbing",        "Hot/cold pipe systems", True, False, True, False, False, 1),
    78: ("Cocktail_Storage",    "Liquor reserves, smuggle drop", False, False, True, False, True, 0),
    80: ("Sky_Lounge_Mech",     "Window mechanisms, motor systems", True, False, True, False, False, 1),
    82: ("Hospitality_Bridge_Sub", "Junction terminus, maintenance bay", True, True, True, True, False, 2),
}

# ---------------------------------------------------------------------------
# Material cache & creation (same pattern as build_zone1.py)
# ---------------------------------------------------------------------------

SURFACE_COLORS = {
    "Deck_Floor_Industrial":  (0.03, 0.03, 0.03),
    "Deck_Wall_Pipe":         (0.05, 0.04, 0.04),
    "Deck_Ceiling_Low":       (0.04, 0.03, 0.03),
    "Deck_Door_Heavy":        (0.06, 0.05, 0.04),
    "Deck_Connector":         (0.04, 0.04, 0.05),
    "Rail_Track":             (0.08, 0.07, 0.06),
    "Rail_Cart":              (0.06, 0.06, 0.07),
    "Cargo_Crate_Wood":       (0.07, 0.05, 0.03),
    "Cargo_Crate_Metal":      (0.06, 0.06, 0.07),
    "Cargo_Barrel":           (0.05, 0.04, 0.03),
    "Security_Red":           (0.12, 0.02, 0.02),
    "Access_Hatch":           (0.08, 0.07, 0.05),
    "Elevator_Frame":         (0.07, 0.07, 0.08),
    "Stairs_Grating":         (0.06, 0.06, 0.06),
    "Pipe_Rust":              (0.08, 0.04, 0.03),
    "Pipe_Clean":             (0.06, 0.06, 0.08),
    "Kronole_Glow":           (0.10, 0.04, 0.12),
}

_mat_cache = {}


def get_material(name, r, g, b, roughness=0.7, metallic=0.8):
    if name in _mat_cache:
        return _mat_cache[name]

    mat_path = f"/Game/Materials/TransportDeck/M_{name}"
    if editor_util.does_asset_exist(mat_path):
        mat = editor_util.load_asset(mat_path)
        _mat_cache[name] = mat
        return mat

    dir_path = "/Game/Materials/TransportDeck"
    if not editor_util.does_directory_exist(dir_path):
        editor_util.make_directory(dir_path)

    mat = asset_tools.create_asset(f"M_{name}", dir_path,
                                   unreal.Material, unreal.MaterialFactoryNew())
    if not mat:
        unreal.log_warning(f"  Failed to create material: {name}")
        return None

    color_node = mat_lib.create_material_expression(
        mat, unreal.MaterialExpressionConstant3Vector, -300, -200)
    if color_node:
        color_node.set_editor_property("constant", unreal.LinearColor(r, g, b, 1.0))
        mat_lib.connect_material_property(color_node, "", unreal.MaterialProperty.MP_BASE_COLOR)

    rough_node = mat_lib.create_material_expression(
        mat, unreal.MaterialExpressionConstant, -300, 0)
    if rough_node:
        rough_node.set_editor_property("r", roughness)
        mat_lib.connect_material_property(rough_node, "", unreal.MaterialProperty.MP_ROUGHNESS)

    metal_node = mat_lib.create_material_expression(
        mat, unreal.MaterialExpressionConstant, -300, 200)
    if metal_node:
        metal_node.set_editor_property("r", metallic)
        mat_lib.connect_material_property(metal_node, "", unreal.MaterialProperty.MP_METALLIC)

    mat_lib.recompile_material(mat)
    _mat_cache[name] = mat
    return mat


def _load_surface_mats():
    mats = {}
    for name, (r, g, b) in SURFACE_COLORS.items():
        rough = 0.85 if "Floor" in name else 0.75
        metal = 0.9 if "Metal" in name or "Rail" in name else 0.7
        mats[name] = get_material(name, r, g, b, roughness=rough, metallic=metal)
    return mats


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


def place_light(label, location, color_rgb, intensity, radius=5000.0):
    pl = level_lib.spawn_actor_from_class(
        unreal.PointLight,
        location,
        unreal.Rotator(0.0, 0.0, 0.0)
    )
    if not pl:
        return None
    pl.set_actor_label(label)
    plc = pl.get_component_by_class(unreal.PointLightComponent)
    if plc:
        plc.set_editor_property("intensity", intensity)
        plc.set_editor_property("attenuation_radius", radius)
        plc.set_editor_property("light_color",
            unreal.Color(int(color_rgb[0]), int(color_rgb[1]), int(color_rgb[2]), 255))
    return pl


def place_target_point(label, location):
    tp = level_lib.spawn_actor_from_class(
        unreal.TargetPoint,
        location,
        unreal.Rotator(0.0, 0.0, 0.0)
    )
    if tp:
        tp.set_actor_label(label)
    return tp


# ---------------------------------------------------------------------------
# Cleanup
# ---------------------------------------------------------------------------

_actor_count = 0


def _inc(n=1):
    global _actor_count
    _actor_count += n


def cleanup():
    unreal.log("=== Cleaning previous Transport Deck actors ===")
    actors = level_lib.get_all_level_actors()
    removed = 0
    prefixes = (
        "TD_", "Rail_", "Cart_", "Cargo_TD_", "Access_", "Security_TD_",
        "Light_TD_", "Pipe_TD_", "Spawn_TD_", "PlayerStart_TD",
        "Sky", "Sun", "Fog_TD", "PostProcess_TD", "Ambient_TD",
    )
    for actor in actors:
        if isinstance(actor, unreal.WorldSettings):
            continue
        label = actor.get_actor_label()
        if any(label.startswith(p) for p in prefixes):
            level_lib.destroy_actor(actor)
            removed += 1
    unreal.log(f"  Removed {removed} actors")


# ---------------------------------------------------------------------------
# Zone lookup
# ---------------------------------------------------------------------------

def get_zone(car_idx):
    for start, end, name, floor_c, wall_c, ceil_c, light_c, light_i, desc in DECK_ZONES:
        if start <= car_idx <= end:
            return name, floor_c, wall_c, ceil_c, light_c, light_i, desc
    return "Unknown", (0.05, 0.05, 0.05), (0.05, 0.05, 0.05), (0.05, 0.05, 0.05), (200, 200, 200), 1500.0, ""


# ---------------------------------------------------------------------------
# Segment shell builder
# ---------------------------------------------------------------------------

def get_segment_x(car_idx):
    """Get the X position for a car index (same spacing as upper deck)."""
    total_span = CAR_LENGTH + CAR_GAP
    return car_idx * total_span


def build_segment_shell(car_idx, mats):
    """Build floor, ceiling, walls for a single lower deck segment."""
    car_x = get_segment_x(car_idx)
    zone_name, floor_c, wall_c, ceil_c, light_c, light_i, _ = get_zone(car_idx)

    seg_name = SEGMENT_DEFS.get(car_idx, (f"{zone_name}_{car_idx}", "", False, False, False, False, False, 0))[0]
    prefix = f"TD_Seg{car_idx:02d}_{seg_name}"

    # Zone-specific materials
    mat_floor = get_material(f"{zone_name}_Floor", *floor_c, roughness=0.9, metallic=0.85)
    mat_wall = get_material(f"{zone_name}_Wall", *wall_c, roughness=0.8, metallic=0.75)
    mat_ceil = get_material(f"{zone_name}_Ceil", *ceil_c, roughness=0.85, metallic=0.7)
    mat_door = mats["Deck_Door_Heavy"]
    mat_conn = mats["Deck_Connector"]

    half_l = CAR_LENGTH / 2.0
    half_w = CAR_WIDTH / 2.0
    half_h = DECK_HEIGHT / 2.0
    floor_z = DECK_Z_OFFSET
    ceil_z = DECK_Z_OFFSET + DECK_HEIGHT

    # Floor
    place_box(f"{prefix}_Floor",
              unreal.Vector(car_x, 0.0, floor_z - WALL_THICK / 2.0),
              (CAR_LENGTH, CAR_WIDTH, WALL_THICK), mat_floor)
    # Ceiling (= underside of upper deck floor)
    place_box(f"{prefix}_Ceiling",
              unreal.Vector(car_x, 0.0, ceil_z + WALL_THICK / 2.0),
              (CAR_LENGTH, CAR_WIDTH, WALL_THICK), mat_ceil)
    # Left wall
    place_box(f"{prefix}_WallL",
              unreal.Vector(car_x, half_w + WALL_THICK / 2.0, floor_z + half_h),
              (CAR_LENGTH, WALL_THICK, DECK_HEIGHT), mat_wall)
    # Right wall
    place_box(f"{prefix}_WallR",
              unreal.Vector(car_x, -(half_w + WALL_THICK / 2.0), floor_z + half_h),
              (CAR_LENGTH, WALL_THICK, DECK_HEIGHT), mat_wall)
    _inc(4)

    # Front connector (to next segment)
    front_x = car_x + half_l + WALL_THICK / 2.0
    if car_idx < DECK_END:
        side_w = (CAR_WIDTH - DOOR_WIDTH) / 2.0
        place_box(f"{prefix}_FrontL",
                  unreal.Vector(front_x, DOOR_WIDTH / 2.0 + side_w / 2.0, floor_z + half_h),
                  (WALL_THICK, side_w, DECK_HEIGHT), mat_wall)
        place_box(f"{prefix}_FrontR",
                  unreal.Vector(front_x, -(DOOR_WIDTH / 2.0 + side_w / 2.0), floor_z + half_h),
                  (WALL_THICK, side_w, DECK_HEIGHT), mat_wall)
        header_h = DECK_HEIGHT - DOOR_HEIGHT
        place_box(f"{prefix}_FrontHeader",
                  unreal.Vector(front_x, 0.0, floor_z + DOOR_HEIGHT + header_h / 2.0),
                  (WALL_THICK, DOOR_WIDTH, header_h), mat_door)

        # Connector gangway
        conn_x = car_x + half_l + WALL_THICK + CAR_GAP / 2.0
        place_box(f"TD_Conn_{car_idx:02d}_Floor",
                  unreal.Vector(conn_x, 0.0, floor_z - WALL_THICK / 2.0),
                  (CAR_GAP, DOOR_WIDTH, WALL_THICK), mat_conn)
        place_box(f"TD_Conn_{car_idx:02d}_WL",
                  unreal.Vector(conn_x, DOOR_WIDTH / 2.0 + WALL_THICK / 2.0, floor_z + DOOR_HEIGHT / 2.0),
                  (CAR_GAP, WALL_THICK, DOOR_HEIGHT), mat_conn)
        place_box(f"TD_Conn_{car_idx:02d}_WR",
                  unreal.Vector(conn_x, -(DOOR_WIDTH / 2.0 + WALL_THICK / 2.0), floor_z + DOOR_HEIGHT / 2.0),
                  (CAR_GAP, WALL_THICK, DOOR_HEIGHT), mat_conn)
        place_box(f"TD_Conn_{car_idx:02d}_Ceil",
                  unreal.Vector(conn_x, 0.0, floor_z + DOOR_HEIGHT + WALL_THICK / 2.0),
                  (CAR_GAP, DOOR_WIDTH + WALL_THICK * 2.0, WALL_THICK), mat_conn)
        _inc(7)
    else:
        # Last segment — solid end wall
        place_box(f"{prefix}_FrontWall",
                  unreal.Vector(front_x, 0.0, floor_z + half_h),
                  (WALL_THICK, CAR_WIDTH, DECK_HEIGHT), mat_wall)
        _inc(1)

    # Back wall (first segment gets sealed wall)
    back_x = car_x - half_l - WALL_THICK / 2.0
    if car_idx > DECK_START:
        side_w = (CAR_WIDTH - DOOR_WIDTH) / 2.0
        place_box(f"{prefix}_BackL",
                  unreal.Vector(back_x, DOOR_WIDTH / 2.0 + side_w / 2.0, floor_z + half_h),
                  (WALL_THICK, side_w, DECK_HEIGHT), mat_wall)
        place_box(f"{prefix}_BackR",
                  unreal.Vector(back_x, -(DOOR_WIDTH / 2.0 + side_w / 2.0), floor_z + half_h),
                  (WALL_THICK, side_w, DECK_HEIGHT), mat_wall)
        header_h = DECK_HEIGHT - DOOR_HEIGHT
        place_box(f"{prefix}_BackHeader",
                  unreal.Vector(back_x, 0.0, floor_z + DOOR_HEIGHT + header_h / 2.0),
                  (WALL_THICK, DOOR_WIDTH, header_h), mat_door)
        _inc(3)
    else:
        place_box(f"{prefix}_BackWall",
                  unreal.Vector(back_x, 0.0, floor_z + half_h),
                  (WALL_THICK, CAR_WIDTH, DECK_HEIGHT), mat_wall)
        _inc(1)


# ---------------------------------------------------------------------------
# Mini-rail track builder
# ---------------------------------------------------------------------------

def build_rail_track(car_idx, mats):
    """Build mini-rail tracks running through each segment."""
    car_x = get_segment_x(car_idx)
    prefix = f"Rail_Seg{car_idx:02d}"
    mat_track = mats["Rail_Track"]
    half_l = CAR_LENGTH / 2.0
    floor_z = DECK_Z_OFFSET

    # Two parallel rails (left and right) running the length of the segment
    rail_gauge = 300.0  # 3m gauge
    rail_h = 40.0       # Rail height
    rail_w = 30.0       # Rail width

    place_box(f"{prefix}_RailL",
              unreal.Vector(car_x, rail_gauge / 2.0, floor_z + rail_h / 2.0),
              (CAR_LENGTH, rail_w, rail_h), mat_track)
    place_box(f"{prefix}_RailR",
              unreal.Vector(car_x, -rail_gauge / 2.0, floor_z + rail_h / 2.0),
              (CAR_LENGTH, rail_w, rail_h), mat_track)
    _inc(2)

    # Cross-ties every 400cm
    tie_spacing = 400.0
    num_ties = int(CAR_LENGTH / tie_spacing)
    mat_tie = get_material("Rail_Tie", 0.04, 0.03, 0.03, roughness=0.9, metallic=0.5)
    for i in range(num_ties):
        tie_x = car_x - half_l + (i + 0.5) * tie_spacing
        place_box(f"{prefix}_Tie_{i}",
                  unreal.Vector(tie_x, 0.0, floor_z + 10.0),
                  (60.0, rail_gauge + 200.0, 20.0), mat_tie)
        _inc()

    # Junction switch (if this segment has one)
    seg_def = SEGMENT_DEFS.get(car_idx)
    if seg_def and seg_def[5]:  # has_junction
        # Siding track branching left
        siding_start = car_x + half_l * 0.3
        siding_end = car_x + half_l * 0.7
        siding_y = CAR_WIDTH / 2.0 - 600.0

        place_box(f"{prefix}_Siding_L",
                  unreal.Vector((siding_start + siding_end) / 2.0, siding_y, floor_z + rail_h / 2.0),
                  (siding_end - siding_start, rail_w, rail_h), mat_track)
        place_box(f"{prefix}_Siding_R",
                  unreal.Vector((siding_start + siding_end) / 2.0, siding_y - rail_gauge, floor_z + rail_h / 2.0),
                  (siding_end - siding_start, rail_w, rail_h), mat_track)
        _inc(2)

        # Switch mechanism marker
        place_target_point(f"Spawn_TD_{car_idx:02d}_Junction",
                           unreal.Vector(siding_start, rail_gauge, floor_z + 200.0))
        _inc()


# ---------------------------------------------------------------------------
# Cart placement
# ---------------------------------------------------------------------------

def build_cart(car_idx, cart_offset_x, mats):
    """Place a mini-rail cart at a position within a segment."""
    car_x = get_segment_x(car_idx) + cart_offset_x
    prefix = f"Cart_Seg{car_idx:02d}"
    mat_cart = mats["Rail_Cart"]
    floor_z = DECK_Z_OFFSET

    # Cart body — low flat platform on rails
    cart_w = 250.0  # Slightly narrower than rail gauge
    cart_l = 600.0
    cart_h = 150.0

    place_box(f"{prefix}_Body",
              unreal.Vector(car_x, 0.0, floor_z + 40.0 + cart_h / 2.0),
              (cart_l, cart_w, cart_h), mat_cart)
    _inc()

    # Cargo box on cart
    mat_crate = mats["Cargo_Crate_Wood"]
    place_box(f"{prefix}_Cargo",
              unreal.Vector(car_x, 0.0, floor_z + 40.0 + cart_h + 150.0),
              (400.0, 200.0, 300.0), mat_crate)
    _inc()

    # Boarding point marker
    place_target_point(f"Spawn_TD_{car_idx:02d}_CartBoard",
                       unreal.Vector(car_x, 300.0, floor_z + 200.0))
    _inc()


# ---------------------------------------------------------------------------
# Access point builders
# ---------------------------------------------------------------------------

def build_access_hatch(car_idx, x_offset, mats):
    """Place a floor hatch connecting upper and lower decks."""
    car_x = get_segment_x(car_idx) + x_offset
    prefix = f"Access_Hatch_{car_idx:02d}"
    mat_hatch = mats["Access_Hatch"]
    ceil_z = DECK_Z_OFFSET + DECK_HEIGHT

    # Hatch frame in ceiling (player drops down / climbs up)
    hatch_size = 400.0
    place_box(f"{prefix}_Frame",
              unreal.Vector(car_x, 0.0, ceil_z + WALL_THICK / 2.0),
              (hatch_size + 100.0, hatch_size + 100.0, WALL_THICK + 20.0), mat_hatch)
    _inc()

    # Ladder (thin tall box)
    place_box(f"{prefix}_Ladder",
              unreal.Vector(car_x, hatch_size / 2.0 + 30.0, DECK_Z_OFFSET + DECK_HEIGHT / 2.0),
              (40.0, 40.0, DECK_HEIGHT), mats["Stairs_Grating"])
    _inc()

    # Interaction marker
    place_target_point(f"Spawn_TD_{car_idx:02d}_HatchEntry",
                       unreal.Vector(car_x, 0.0, DECK_Z_OFFSET + 200.0))
    _inc()


def build_freight_elevator(car_idx, x_offset, mats):
    """Place a freight elevator shaft between decks."""
    car_x = get_segment_x(car_idx) + x_offset
    prefix = f"Access_Elevator_{car_idx:02d}"
    mat_frame = mats["Elevator_Frame"]
    floor_z = DECK_Z_OFFSET

    # Elevator shaft walls
    shaft_w = 600.0
    shaft_d = 600.0
    shaft_h = DECK_HEIGHT + WALL_THICK * 2  # Full height including ceilings

    # Shaft housing (4 walls around the elevator)
    for i, (dy, sx, sy) in enumerate([
        (shaft_d / 2.0 + 15.0, shaft_w + 30.0, 30.0),
        (-(shaft_d / 2.0 + 15.0), shaft_w + 30.0, 30.0),
    ]):
        place_box(f"{prefix}_ShaftWall_{i}",
                  unreal.Vector(car_x, dy, floor_z + DECK_HEIGHT / 2.0),
                  (sx, sy, shaft_h), mat_frame)
        _inc()

    # Elevator platform
    place_box(f"{prefix}_Platform",
              unreal.Vector(car_x, 0.0, floor_z + 50.0),
              (shaft_w - 50.0, shaft_d - 50.0, 30.0), mat_frame)
    _inc()

    # Interaction markers (upper and lower)
    place_target_point(f"Spawn_TD_{car_idx:02d}_ElevUpper",
                       unreal.Vector(car_x, shaft_d, 200.0))
    place_target_point(f"Spawn_TD_{car_idx:02d}_ElevLower",
                       unreal.Vector(car_x, shaft_d, floor_z + 200.0))
    _inc(2)


def build_maintenance_stairs(car_idx, x_offset, mats):
    """Place a maintenance staircase between decks."""
    car_x = get_segment_x(car_idx) + x_offset
    prefix = f"Access_Stairs_{car_idx:02d}"
    mat_stairs = mats["Stairs_Grating"]
    floor_z = DECK_Z_OFFSET

    # Staircase: series of stepped platforms
    num_steps = 8
    stair_width = 400.0
    step_h = (DECK_HEIGHT + WALL_THICK) / num_steps
    step_d = 250.0

    for i in range(num_steps):
        step_x = car_x + (i - num_steps / 2.0) * step_d
        step_z = floor_z + (i + 0.5) * step_h
        place_box(f"{prefix}_Step_{i}",
                  unreal.Vector(step_x, -CAR_WIDTH / 2.0 + stair_width / 2.0 + 300.0, step_z),
                  (step_d, stair_width, 30.0), mat_stairs)
        _inc()

    # Railing
    railing_len = num_steps * step_d
    railing_center_x = car_x
    railing_z = floor_z + DECK_HEIGHT / 2.0 + 300.0
    place_box(f"{prefix}_Railing",
              unreal.Vector(railing_center_x, -CAR_WIDTH / 2.0 + 300.0, railing_z),
              (railing_len, 20.0, 400.0), mat_stairs)
    _inc()

    # Entry marker
    place_target_point(f"Spawn_TD_{car_idx:02d}_StairsEntry",
                       unreal.Vector(car_x, -CAR_WIDTH / 2.0 + 500.0, floor_z + 200.0))
    _inc()


# ---------------------------------------------------------------------------
# Security device placement
# ---------------------------------------------------------------------------

def build_security(car_idx, security_level, mats):
    """Place security devices based on segment security level."""
    if security_level == 0:
        return

    car_x = get_segment_x(car_idx)
    prefix = f"Security_TD_{car_idx:02d}"
    mat_security = mats["Security_Red"]
    floor_z = DECK_Z_OFFSET
    half_l = CAR_LENGTH / 2.0

    if security_level >= 1:
        # Pressure plates at entry/exit
        for i, dx in enumerate([-half_l + 500.0, half_l - 500.0]):
            place_box(f"{prefix}_PressurePlate_{i}",
                      unreal.Vector(car_x + dx, 0.0, floor_z + 10.0),
                      (300.0, CAR_WIDTH * 0.6, 15.0), mat_security)
            _inc()

    if security_level >= 2:
        # Laser tripwires (thin red lines)
        for i in range(2):
            tw_x = car_x - half_l * 0.3 + i * half_l * 0.6
            place_box(f"{prefix}_Laser_{i}",
                      unreal.Vector(tw_x, 0.0, floor_z + 400.0),
                      (10.0, CAR_WIDTH * 0.7, 5.0), mat_security)
            place_light(f"Light_TD_{car_idx:02d}_Laser_{i}",
                        unreal.Vector(tw_x, 0.0, floor_z + 400.0),
                        (255, 20, 20), 200.0, radius=1000.0)
            _inc(2)

        # Security camera
        place_box(f"{prefix}_Camera",
                  unreal.Vector(car_x, CAR_WIDTH / 2.0 - 200.0, DECK_Z_OFFSET + DECK_HEIGHT - 100.0),
                  (80.0, 80.0, 60.0), mat_security)
        place_target_point(f"Spawn_TD_{car_idx:02d}_Camera",
                           unreal.Vector(car_x, CAR_WIDTH / 2.0 - 200.0, DECK_Z_OFFSET + DECK_HEIGHT - 100.0))
        _inc(2)

    if security_level >= 3:
        # Motion sensor
        place_box(f"{prefix}_MotionSensor",
                  unreal.Vector(car_x + half_l * 0.5, 0.0, DECK_Z_OFFSET + DECK_HEIGHT - 100.0),
                  (60.0, 60.0, 40.0), mat_security)
        _inc()

        # Patrol drone spawn
        place_target_point(f"Spawn_TD_{car_idx:02d}_Drone",
                           unreal.Vector(car_x, 0.0, floor_z + DECK_HEIGHT * 0.7))
        _inc()


# ---------------------------------------------------------------------------
# Pipe and environment details
# ---------------------------------------------------------------------------

def build_pipes(car_idx, mats):
    """Place environmental pipes along ceiling and walls."""
    car_x = get_segment_x(car_idx)
    prefix = f"Pipe_TD_{car_idx:02d}"
    half_l = CAR_LENGTH / 2.0
    half_w = CAR_WIDTH / 2.0
    floor_z = DECK_Z_OFFSET

    mat_rust = mats["Pipe_Rust"]
    mat_clean = mats["Pipe_Clean"]

    # Two pipe runs along left ceiling corner
    place_box(f"{prefix}_CeilPipeL",
              unreal.Vector(car_x, half_w - 200.0, floor_z + DECK_HEIGHT - 100.0),
              (CAR_LENGTH, 80.0, 80.0), mat_rust)
    place_box(f"{prefix}_CeilPipeR",
              unreal.Vector(car_x, -(half_w - 200.0), floor_z + DECK_HEIGHT - 100.0),
              (CAR_LENGTH, 80.0, 80.0), mat_clean)
    _inc(2)

    # Vertical pipe run on one wall
    place_box(f"{prefix}_VertPipe",
              unreal.Vector(car_x - half_l + 500.0, half_w - 100.0, floor_z + DECK_HEIGHT / 2.0),
              (60.0, 60.0, DECK_HEIGHT), mat_rust)
    _inc()

    # Valve wheels (small boxes as markers)
    if car_idx % 4 == 0:  # Every 4th segment
        place_box(f"{prefix}_Valve",
                  unreal.Vector(car_x, half_w - 100.0, floor_z + 600.0),
                  (60.0, 20.0, 60.0), mat_clean)
        place_target_point(f"Spawn_TD_{car_idx:02d}_Valve",
                           unreal.Vector(car_x, half_w - 200.0, floor_z + 600.0))
        _inc(2)


def build_cargo_stacks(car_idx, mats):
    """Place cargo crates/barrels as environmental detail."""
    car_x = get_segment_x(car_idx)
    prefix = f"Cargo_TD_{car_idx:02d}"
    floor_z = DECK_Z_OFFSET
    half_l = CAR_LENGTH / 2.0
    half_w = CAR_WIDTH / 2.0

    mat_wood = mats["Cargo_Crate_Wood"]
    mat_metal = mats["Cargo_Crate_Metal"]
    mat_barrel = mats["Cargo_Barrel"]

    # Crate stacks along walls (varies by zone)
    zone_name = get_zone(car_idx)[0]

    if "ThirdClass" in zone_name:
        # Food crates and water barrels
        for i in range(3):
            cx = car_x - half_l + 2000.0 + i * 3500.0
            place_box(f"{prefix}_Crate_{i}",
                      unreal.Vector(cx, half_w - 500.0, floor_z + 200.0),
                      (400.0, 400.0, 400.0), mat_wood)
            _inc()
        # Barrel
        place_box(f"{prefix}_Barrel",
                  unreal.Vector(car_x + 2000.0, -(half_w - 500.0), floor_z + 200.0),
                  (200.0, 200.0, 350.0), mat_barrel)
        _inc()

    elif "SecondClass" in zone_name:
        # Mixed cargo
        for i in range(2):
            cx = car_x - 2000.0 + i * 4000.0
            place_box(f"{prefix}_MetalCrate_{i}",
                      unreal.Vector(cx, half_w - 500.0, floor_z + 200.0),
                      (500.0, 400.0, 400.0), mat_metal)
            _inc()

    elif "WorkingSpine" in zone_name:
        # Industrial parts, heavy machinery
        place_box(f"{prefix}_MachineBlock",
                  unreal.Vector(car_x - 2000.0, half_w - 600.0, floor_z + 300.0),
                  (800.0, 600.0, 600.0), mat_metal)
        _inc()

    elif "FirstClass" in zone_name:
        # Luxury goods, neatly organized
        for i in range(2):
            cx = car_x - 1500.0 + i * 3000.0
            place_box(f"{prefix}_LuxCrate_{i}",
                      unreal.Vector(cx, half_w - 400.0, floor_z + 200.0),
                      (400.0, 350.0, 350.0), mat_wood)
            _inc()


# ---------------------------------------------------------------------------
# Kronole smuggling route markers
# ---------------------------------------------------------------------------

def build_smuggle_markers(car_idx, mats):
    """Place Kronole Network smuggling indicators."""
    car_x = get_segment_x(car_idx)
    prefix = f"TD_Smuggle_{car_idx:02d}"
    floor_z = DECK_Z_OFFSET
    mat_kronole = mats["Kronole_Glow"]

    # Hidden marking on wall (small glowing indicator)
    place_box(f"{prefix}_Mark",
              unreal.Vector(car_x - 1000.0, CAR_WIDTH / 2.0 - 50.0, floor_z + 400.0),
              (30.0, 10.0, 30.0), mat_kronole)
    _inc()

    # Faint purple light near drop point
    place_light(f"Light_TD_{car_idx:02d}_Kronole",
                unreal.Vector(car_x + 2000.0, 0.0, floor_z + 300.0),
                (160, 80, 180), 300.0, radius=1500.0)
    _inc()

    # Dead drop point
    place_target_point(f"Spawn_TD_{car_idx:02d}_DeadDrop",
                       unreal.Vector(car_x + 2000.0, -500.0, floor_z + 200.0))
    _inc()


# ---------------------------------------------------------------------------
# Segment lighting
# ---------------------------------------------------------------------------

def build_segment_lights(car_idx):
    """Place dim industrial lights for a segment."""
    car_x = get_segment_x(car_idx)
    _, _, _, _, light_c, light_i, _ = get_zone(car_idx)
    prefix = f"Light_TD_{car_idx:02d}"
    floor_z = DECK_Z_OFFSET

    # 2 lights per segment — dim, industrial
    for i in range(2):
        lx = car_x - CAR_LENGTH / 4.0 + i * CAR_LENGTH / 2.0
        ly = 600.0 if (i % 2 == 0) else -600.0
        place_light(f"{prefix}_{i}",
                    unreal.Vector(lx, ly, floor_z + DECK_HEIGHT - 200.0),
                    light_c, light_i * 0.6, radius=4000.0)
        _inc()


# ---------------------------------------------------------------------------
# Main build
# ---------------------------------------------------------------------------

def build_transport_deck():
    """Build all 68 lower transport deck segments."""
    global _actor_count
    _actor_count = 0

    unreal.log("=== Building Lower Transport Deck ===")
    mats = _load_surface_mats()

    for car_idx in range(DECK_START, DECK_END + 1):
        seg_def = SEGMENT_DEFS.get(car_idx)

        # Shell geometry
        build_segment_shell(car_idx, mats)

        # Mini-rail tracks (all segments)
        build_rail_track(car_idx, mats)

        # Pipes and environment
        build_pipes(car_idx, mats)

        # Cargo stacks
        build_cargo_stacks(car_idx, mats)

        # Segment lights
        build_segment_lights(car_idx)

        if seg_def:
            name, features, has_hatch, has_elevator, has_stairs, has_junction, is_smuggle, security = seg_def

            # Access points
            if has_hatch:
                build_access_hatch(car_idx, -2000.0, mats)
            if has_elevator:
                build_freight_elevator(car_idx, 3000.0, mats)
            if has_stairs:
                build_maintenance_stairs(car_idx, 1000.0, mats)

            # Cart at notable segments
            build_cart(car_idx, 0.0, mats)

            # Security
            build_security(car_idx, security, mats)

            # Kronole smuggling markers
            if is_smuggle:
                build_smuggle_markers(car_idx, mats)

        # Progress
        if (car_idx - DECK_START) % 10 == 0:
            zone_name = get_zone(car_idx)[0]
            unreal.log(f"  [{car_idx}/{DECK_END}] {zone_name} ({_actor_count} actors)")

    unreal.log(f"=== Transport Deck build finished: {_actor_count} actors ===")


# ---------------------------------------------------------------------------
# Atmosphere
# ---------------------------------------------------------------------------

def setup_atmosphere():
    """Transport deck atmosphere — oppressive, industrial, dim."""
    unreal.log("=== Setting up Transport Deck atmosphere ===")
    total_length = (DECK_END - DECK_START + 1) * (CAR_LENGTH + CAR_GAP)
    start_x = get_segment_x(DECK_START)
    mid_x = start_x + total_length / 2.0

    # Fog — heavy, industrial haze
    fog = level_lib.spawn_actor_from_class(
        unreal.ExponentialHeightFog,
        unreal.Vector(mid_x, 0.0, DECK_Z_OFFSET)
    )
    if fog:
        fog.set_actor_label("Fog_TD_Industrial")
        fc = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
        if fc:
            fc.set_editor_property("fog_density", 0.008)
            fc.set_editor_property("fog_inscattering_color",
                                   unreal.LinearColor(0.04, 0.03, 0.03, 1.0))

    # Post-process — dark, contrasty
    ppv = level_lib.spawn_actor_from_class(
        unreal.PostProcessVolume,
        unreal.Vector(mid_x, 0.0, DECK_Z_OFFSET + DECK_HEIGHT / 2.0)
    )
    if ppv:
        ppv.set_actor_label("PostProcess_TD_Industrial")
        ppv.set_editor_property("unbound", True)
        settings = ppv.get_editor_property("settings")
        settings.set_editor_property("override_auto_exposure_bias", True)
        settings.set_editor_property("auto_exposure_bias", -1.0)
        ppv.set_editor_property("settings", settings)

    # Ambient drone sound placeholder
    place_target_point("Ambient_TD_MachineryDrone",
                       unreal.Vector(mid_x, 0.0, DECK_Z_OFFSET + DECK_HEIGHT / 2.0))

    unreal.log("  Atmosphere configured (dark, industrial, hazy)")


# ---------------------------------------------------------------------------
# Create / load map
# ---------------------------------------------------------------------------

def create_or_load_map():
    map_path = "/Game/Maps/TransportDeck_Lower"
    maps_dir = "/Game/Maps"

    if not editor_util.does_directory_exist(maps_dir):
        editor_util.make_directory(maps_dir)

    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

    if editor_util.does_asset_exist(map_path):
        unreal.log(f"  Loading existing map: {map_path}")
        level_subsystem.load_level(map_path)
    else:
        unreal.log(f"  Creating new map: {map_path}")
        level_subsystem.new_level(map_path)

    return map_path


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def run():
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  Lower Transport Deck (Cars 15-82) [10x SCALE]")
    unreal.log("=" * 64)
    unreal.log("")

    map_path = create_or_load_map()
    cleanup()
    build_transport_deck()
    setup_atmosphere()

    # Player start in transport deck (beneath car 20 — Distribution Hub)
    ps = level_lib.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(get_segment_x(20), 0.0, DECK_Z_OFFSET + 200.0),
        unreal.Rotator(0.0, 0.0, 0.0)
    )
    if ps:
        ps.set_actor_label("PlayerStart_TD_DistributionHub")

    # Save
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    level_subsystem.save_current_level()

    total_length_m = NUM_SEGMENTS * (CAR_LENGTH + CAR_GAP) / 100.0
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  LOWER TRANSPORT DECK BUILD COMPLETE")
    unreal.log("=" * 64)
    unreal.log(f"  Map: {map_path}")
    unreal.log(f"  Segments: {NUM_SEGMENTS} (Cars {DECK_START}-{DECK_END})")
    unreal.log(f"  Segment size: {CAR_LENGTH/100:.0f}m x {CAR_WIDTH/100:.0f}m x {DECK_HEIGHT/100:.0f}m")
    unreal.log(f"  Total length: {total_length_m:.0f}m")
    unreal.log(f"  Deck Z offset: {DECK_Z_OFFSET/100:.1f}m below upper floor")
    unreal.log(f"  Actors placed: {_actor_count}")
    unreal.log("")
    unreal.log("  ZONE LAYOUT:")
    for start, end, name, _, _, _, _, _, desc in DECK_ZONES:
        unreal.log(f"    Cars {start:2d}-{end:2d}: {name:25s} — {desc}")
    unreal.log("")
    unreal.log("  NOTABLE SEGMENTS:")
    for car_idx in sorted(SEGMENT_DEFS.keys()):
        name, features, _, _, _, _, smuggle, security = SEGMENT_DEFS[car_idx]
        flags = []
        if smuggle:
            flags.append("KRONOLE")
        if security > 0:
            flags.append(f"SEC:{security}")
        flag_str = f" [{', '.join(flags)}]" if flags else ""
        unreal.log(f"    [{car_idx:2d}] {name:25s} — {features}{flag_str}")
    unreal.log("")
    unreal.log("  PlayerStart at Distribution Hub (Car 20)")
    unreal.log("  Navigate the lower deck to bypass checkpoints!")
    unreal.log("")


run()
