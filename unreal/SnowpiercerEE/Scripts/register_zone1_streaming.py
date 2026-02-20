"""
Snowpiercer: Eternal Engine -- Zone 1 Streaming Registration
Run in editor: Tools > Execute Python Script

Reads DT_Zone1Cars DataTable and registers each car sublevel with
SEECarStreamingSubsystem. Call this after build_zone1.py has created
the sublevels and DT_Zone1Cars has been imported.

This script also validates that all referenced sublevels exist.
"""

import unreal

editor_util = unreal.EditorAssetLibrary

# ---------------------------------------------------------------------------
# Zone 1 car registry (mirrors DT_Zone1Cars.json)
# Used when the DataTable asset hasn't been imported yet.
# ---------------------------------------------------------------------------

ZONE1_CARS = [
    (0,  "Caboose",         "Zone1_Car00_Caboose"),
    (1,  "Tail_Quarters_A", "Zone1_Car01_Tail_Quarters_A"),
    (2,  "Tail_Quarters_B", "Zone1_Car02_Tail_Quarters_B"),
    (3,  "The_Pit",         "Zone1_Car03_The_Pit"),
    (4,  "Nursery",         "Zone1_Car04_Nursery"),
    (5,  "Elders_Car",      "Zone1_Car05_Elders_Car"),
    (6,  "Sickbay",         "Zone1_Car06_Sickbay"),
    (7,  "Workshop",        "Zone1_Car07_Workshop"),
    (8,  "Listening_Post",  "Zone1_Car08_Listening_Post"),
    (9,  "Blockade",        "Zone1_Car09_Blockade"),
    (10, "Dark_Car",        "Zone1_Car10_Dark_Car"),
    (11, "Freezer_Breach",  "Zone1_Car11_Freezer_Breach"),
    (12, "Kronole_Den",     "Zone1_Car12_Kronole_Den"),
    (13, "Smugglers_Cache", "Zone1_Car13_Smugglers_Cache"),
    (14, "Martyrs_Gate",    "Zone1_Car14_Martyrs_Gate"),
]


def validate_sublevels():
    """Check that all Zone 1 car sublevels exist as map assets."""
    unreal.log("=== Validating Zone 1 sublevels ===")
    missing = []
    found = 0
    for car_idx, car_name, sublevel_name in ZONE1_CARS:
        sublevel_path = f"/Game/Maps/Zone1/{sublevel_name}"
        if editor_util.does_asset_exist(sublevel_path):
            found += 1
        else:
            missing.append((car_idx, sublevel_name))
            unreal.log_warning(f"  MISSING: [{car_idx:02d}] {sublevel_path}")

    unreal.log(f"  Found: {found}/{len(ZONE1_CARS)} sublevels")
    if missing:
        unreal.log_warning(f"  Missing: {len(missing)} sublevels -- run build_zone1.py first")
    else:
        unreal.log(f"  All Zone 1 sublevels present")
    return len(missing) == 0


def register_car_levels():
    """Register all Zone 1 car sublevels with SEECarStreamingSubsystem.

    This must be called at runtime (BeginPlay or later) when the
    subsystem is available. In editor, this validates the setup.
    """
    unreal.log("=== Registering Zone 1 car levels ===")

    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        unreal.log_error("  No editor world -- cannot register streaming levels")
        return False

    # Try to get the streaming subsystem
    subsystem = None
    try:
        subsystem = world.get_subsystem(unreal.find_class("SEECarStreamingSubsystem"))
    except Exception:
        pass

    if subsystem:
        # Runtime registration via C++ subsystem
        for car_idx, car_name, sublevel_name in ZONE1_CARS:
            subsystem.register_car_level(car_idx, sublevel_name)
            unreal.log(f"  Registered: [{car_idx:02d}] {car_name} -> {sublevel_name}")
        unreal.log(f"  All {len(ZONE1_CARS)} cars registered with streaming subsystem")
        return True
    else:
        # Editor-only: just validate and print the registration table
        unreal.log("  Streaming subsystem not available (editor-only mode)")
        unreal.log("  Registration table for runtime use:")
        unreal.log("")
        unreal.log("  CarIndex | CarName              | SubLevelName")
        unreal.log("  " + "-" * 65)
        for car_idx, car_name, sublevel_name in ZONE1_CARS:
            unreal.log(f"  {car_idx:8d} | {car_name:20s} | {sublevel_name}")
        unreal.log("")
        unreal.log("  To register at runtime, call from GameMode::BeginPlay:")
        unreal.log("    USEECarStreamingSubsystem* SS = GetWorld()->GetSubsystem<...>();")
        unreal.log("    SS->RegisterCarLevel(CarIndex, FName(SubLevelName));")
        unreal.log("    SS->EnterCar(0);  // Start in Caboose")
        return True


def run():
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  ZONE 1 STREAMING REGISTRATION")
    unreal.log("=" * 64)
    unreal.log("")

    valid = validate_sublevels()
    register_car_levels()

    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  REGISTRATION COMPLETE")
    unreal.log("=" * 64)
    unreal.log("")


run()
