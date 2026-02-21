"""
Snowpiercer: Eternal Engine -- Zone 1 Population Script
Run in editor: Tools > Execute Python Script

Reads TargetPoint spawn markers placed by build_zone1.py and spawns actual
Blueprint actors (NPCs, pickups, boss encounter, ambient sound) at those
locations.

Prerequisites:
  1. Run setup_game_content.py first (creates all BP assets)
  2. Run build_zone1.py first (creates car geometry + spawn markers)
  3. Then run this script to populate the world
"""

import unreal

# ---------------------------------------------------------------------------
# API shortcuts
# ---------------------------------------------------------------------------

level_lib = unreal.EditorLevelLibrary
editor_util = unreal.EditorAssetLibrary

# ---------------------------------------------------------------------------
# Blueprint asset paths (created by setup_game_content.py)
# ---------------------------------------------------------------------------

BP_PICKUPS = {
    "ProteinBlock": "/Game/Blueprints/Pickups/BP_Pickup_ProteinBlock",
    "Bandage":      "/Game/Blueprints/Pickups/BP_Pickup_Bandage",
    "ScrapMetal":   "/Game/Blueprints/Pickups/BP_Pickup_ScrapMetal",
    "Pipe":         "/Game/Blueprints/Pickups/BP_Pickup_Pipe",
    "Shiv":         "/Game/Blueprints/Pickups/BP_Pickup_Shiv",
    "WaterRation":  "/Game/Blueprints/Pickups/BP_Pickup_WaterRation",
    "Cloth":        "/Game/Blueprints/Pickups/BP_Pickup_Cloth",
    "Medicine":     "/Game/Blueprints/Pickups/BP_Pickup_Medicine",
    "Lockpick":     "/Game/Blueprints/Pickups/BP_Pickup_Lockpick",
    "Coal":         "/Game/Blueprints/Pickups/BP_Pickup_Coal",
    "Wrench":       "/Game/Blueprints/Pickups/BP_Pickup_Wrench",
    "Wire":         "/Game/Blueprints/Pickups/BP_Pickup_Wire",
    "FoodRation":   "/Game/Blueprints/Pickups/BP_Pickup_Ration",
    "KeyCard":      "/Game/Blueprints/Pickups/BP_Pickup_KeyCard",
}

BP_NPCS = {
    "Civilian":   "/Game/Blueprints/NPCs/BP_NPC_Civilian",
    "Jackboot":   "/Game/Blueprints/NPCs/BP_NPC_Jackboot",
    "Merchant":   "/Game/Blueprints/NPCs/BP_NPC_Merchant",
    "Breachman":  "/Game/Blueprints/NPCs/BP_NPC_Breachman",
    "FirstClass": "/Game/Blueprints/NPCs/BP_NPC_FirstClass",
}

# ---------------------------------------------------------------------------
# Spawn point → actor mapping
#
# Each entry: (spawn label substring, BP path, actor label)
# Matched against TargetPoint actor labels placed by build_zone1.py
# ---------------------------------------------------------------------------

# Named NPC spawns (unique characters at specific markers)
NAMED_NPC_SPAWNS = {
    "Spawn_Z1_Car05_Elders_Gilliam":        ("Civilian",  "NPC_Gilliam"),
    "Spawn_Z1_Car06_Sickbay_DrAsha":        ("Civilian",  "NPC_DrAsha"),
    "Spawn_Z1_Car12_KronoleDen_KronoleKim": ("Civilian",  "NPC_KronoleKim"),
    "Spawn_Z1_Car13_SmugglerCache_Merchant":("Merchant",  "NPC_Merchant_Smuggler"),
    "Spawn_Z1_Car14_MartyrsGate_Mourner":   ("Civilian",  "NPC_Mourner"),
    "Spawn_Z1_Car08_ListenPost_Whisper":    ("Civilian",  "NPC_Whisper"),
}

# Generic NPC spawns (pattern-matched by car prefix)
GENERIC_NPC_PATTERNS = [
    ("_NPC_",    "Civilian"),   # Generic NPC markers → Civilian BP
    ("_Enemy_",  "Jackboot"),   # Enemy markers → Jackboot guard BP
]

# Boss spawn
BOSS_SPAWN_LABEL = "Spawn_Z1_Car14_MartyrsGate_Boss_CmdrGrey"

# Item spawn definitions per car type
# Maps spawn label substrings to pickup types
ITEM_SPAWNS = {
    "QuestItem":          "KeyCard",
    "CraftStation":       "Wrench",
    "Forge":              "ScrapMetal",
    "Intel_":             "Lockpick",
    "SteamValve":         "Pipe",
    "HiddenCompartment":  "ScrapMetal",
    "WarmSpot_":          "Coal",
    "HidingSpot_":        "Bandage",
    "Nest_Juvenile":      "ProteinBlock",
    "Nest_Alpha":         "Shiv",
}

# Per-car scattered loot tables (car prefix → list of pickup types)
CAR_LOOT = {
    "Z1_Car00": ["ProteinBlock", "Cloth"],
    "Z1_Car01": ["ProteinBlock", "Bandage", "WaterRation"],
    "Z1_Car02": ["ProteinBlock", "Bandage", "Cloth"],
    "Z1_Car03": ["Shiv", "Bandage", "ScrapMetal"],
    "Z1_Car04": ["Bandage", "Cloth", "WaterRation"],
    "Z1_Car05": ["FoodRation", "WaterRation"],
    "Z1_Car06": ["Medicine", "Bandage"],
    "Z1_Car07": ["Wrench", "ScrapMetal", "Wire", "Pipe"],
    "Z1_Car08": ["Lockpick", "Wire"],
    "Z1_Car09": ["Shiv", "Pipe", "Bandage", "ScrapMetal"],
    "Z1_Car10": ["Bandage", "Coal"],
    "Z1_Car11": ["Coal", "Cloth", "WaterRation"],
    "Z1_Car12": ["Medicine", "Bandage"],
    "Z1_Car13": ["ScrapMetal", "Lockpick", "KeyCard"],
    "Z1_Car14": ["Bandage", "Shiv", "FoodRation"],
}

# Ambient sound cue path (created by setup_game_content.py)
AMBIENT_SOUND_CUE = "/Game/Audio/SoundCues/SC_Ambient_TrainHum"

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

_spawned_count = {"npcs": 0, "pickups": 0, "sounds": 0, "boss": 0, "triggers": 0}


def _load_bp_class(bp_path):
    """Load a Blueprint class for spawning."""
    full_path = bp_path + "_C"
    try:
        cls = unreal.load_class(None, full_path)
        if cls:
            return cls
    except Exception:
        pass
    # Try loading the asset and getting generated class
    try:
        bp = editor_util.load_asset(bp_path)
        if bp:
            return bp.generated_class()
    except Exception:
        pass
    return None


def _get_all_target_points():
    """Collect all TargetPoint actors in the level, keyed by label."""
    points = {}
    actors = level_lib.get_all_level_actors()
    for actor in actors:
        if isinstance(actor, unreal.TargetPoint):
            label = actor.get_actor_label()
            if label.startswith("Spawn_"):
                points[label] = actor
    return points


def _spawn_bp_at(bp_path, location, label, rotation=None):
    """Spawn a Blueprint actor at a location with a label."""
    if rotation is None:
        rotation = unreal.Rotator(0.0, 0.0, 0.0)

    bp_class = _load_bp_class(bp_path)
    if not bp_class:
        unreal.log_warning(f"  Could not load BP: {bp_path}")
        return None

    actor = level_lib.spawn_actor_from_class(bp_class, location, rotation)
    if actor:
        actor.set_actor_label(label)
    return actor


def _spawn_ambient_sound(label, location):
    """Spawn an AmbientSound actor with the train hum cue."""
    if not editor_util.does_asset_exist(AMBIENT_SOUND_CUE):
        unreal.log_warning(f"  Ambient sound cue not found: {AMBIENT_SOUND_CUE}")
        return None

    sound_cue = editor_util.load_asset(AMBIENT_SOUND_CUE)
    actor = level_lib.spawn_actor_from_class(
        unreal.AmbientSound, location, unreal.Rotator(0.0, 0.0, 0.0))
    if actor:
        actor.set_actor_label(label)
        audio_comp = actor.get_component_by_class(unreal.AudioComponent)
        if audio_comp and sound_cue:
            audio_comp.set_sound(sound_cue)
    return actor


# ---------------------------------------------------------------------------
# Cleanup
# ---------------------------------------------------------------------------

def cleanup():
    """Remove previously spawned population actors."""
    unreal.log("=== Cleaning previous Zone 1 population actors ===")
    actors = level_lib.get_all_level_actors()
    removed = 0
    prefixes = ("NPC_", "Pickup_Z1_", "Boss_", "Sound_Z1_", "Trigger_Boss_")
    for actor in actors:
        if isinstance(actor, unreal.WorldSettings):
            continue
        label = actor.get_actor_label()
        if any(label.startswith(p) for p in prefixes):
            level_lib.destroy_actor(actor)
            removed += 1
    unreal.log(f"  Removed {removed} population actors")


# ---------------------------------------------------------------------------
# Main population logic
# ---------------------------------------------------------------------------

def populate():
    """Populate Zone 1 with NPCs, pickups, boss, and ambient audio."""

    unreal.log("=" * 60)
    unreal.log("ZONE 1 POPULATION SCRIPT -- Snowpiercer: Eternal Engine")
    unreal.log("=" * 60)

    # Clean previous population
    cleanup()

    # Gather all spawn markers
    spawn_points = _get_all_target_points()
    unreal.log(f"Found {len(spawn_points)} spawn markers")

    if len(spawn_points) == 0:
        unreal.log_error("No spawn markers found! Run build_zone1.py first.")
        return

    # ------ Named NPCs ------
    unreal.log("\n--- Spawning Named NPCs ---")
    for spawn_label, (npc_type, actor_label) in NAMED_NPC_SPAWNS.items():
        tp = spawn_points.get(spawn_label)
        if not tp:
            unreal.log_warning(f"  Marker not found: {spawn_label}")
            continue
        bp_path = BP_NPCS.get(npc_type)
        if not bp_path:
            unreal.log_warning(f"  Unknown NPC type: {npc_type}")
            continue
        loc = tp.get_actor_location()
        actor = _spawn_bp_at(bp_path, loc, actor_label)
        if actor:
            _spawned_count["npcs"] += 1
            unreal.log(f"  Spawned {actor_label} at {spawn_label}")

    # ------ Generic NPCs (pattern match) ------
    unreal.log("\n--- Spawning Generic NPCs ---")
    for label, tp in spawn_points.items():
        for pattern, npc_type in GENERIC_NPC_PATTERNS:
            if pattern in label:
                # Skip if it's a named NPC spawn we already handled
                if label in NAMED_NPC_SPAWNS:
                    break
                bp_path = BP_NPCS.get(npc_type)
                if not bp_path:
                    break
                loc = tp.get_actor_location()
                actor_label = f"NPC_{npc_type}_{label.split('_')[-1]}"
                # Make label unique with car info
                car_part = ""
                for part in label.split("_"):
                    if part.startswith("Car"):
                        car_part = part
                        break
                if car_part:
                    actor_label = f"NPC_{car_part}_{npc_type}_{label.split('_')[-1]}"
                actor = _spawn_bp_at(bp_path, loc, actor_label)
                if actor:
                    _spawned_count["npcs"] += 1
                    unreal.log(f"  Spawned {actor_label}")
                break

    # ------ Boss: Commander Grey ------
    unreal.log("\n--- Spawning Boss ---")
    boss_tp = spawn_points.get(BOSS_SPAWN_LABEL)
    if boss_tp:
        boss_loc = boss_tp.get_actor_location()
        boss_actor = _spawn_bp_at(BP_NPCS["Jackboot"], boss_loc, "Boss_CmdrGrey")
        if boss_actor:
            _spawned_count["boss"] += 1
            unreal.log(f"  Spawned Commander Grey at Car 14 boss arena")

        # Boss encounter trigger volume (large box around arena)
        trigger = level_lib.spawn_actor_from_class(
            unreal.TriggerBox,
            unreal.Vector(boss_loc.x - 500.0, boss_loc.y, boss_loc.z),
            unreal.Rotator(0.0, 0.0, 0.0))
        if trigger:
            trigger.set_actor_label("Trigger_Boss_CmdrGrey")
            trigger.set_actor_scale3d(unreal.Vector(50.0, 40.0, 30.0))
            _spawned_count["triggers"] += 1
            unreal.log(f"  Placed boss encounter trigger volume")
    else:
        unreal.log_warning(f"  Boss spawn marker not found: {BOSS_SPAWN_LABEL}")

    # ------ Pickups at specific markers ------
    unreal.log("\n--- Spawning Pickups at Markers ---")
    for label, tp in spawn_points.items():
        for item_pattern, pickup_type in ITEM_SPAWNS.items():
            if item_pattern in label:
                bp_path = BP_PICKUPS.get(pickup_type)
                if not bp_path:
                    break
                loc = tp.get_actor_location()
                # Build a unique label
                car_part = ""
                for part in label.split("_"):
                    if part.startswith("Car"):
                        car_part = part
                        break
                pickup_label = f"Pickup_Z1_{car_part}_{pickup_type}_{label.split('_')[-1]}"
                actor = _spawn_bp_at(bp_path, loc, pickup_label)
                if actor:
                    _spawned_count["pickups"] += 1
                    unreal.log(f"  Spawned {pickup_label}")
                break

    # ------ Scattered loot per car ------
    unreal.log("\n--- Spawning Scattered Loot ---")
    # Car X positions: car_idx * (CAR_LENGTH + CAR_GAP) = idx * 13000
    CAR_LENGTH = 12000.0
    CAR_GAP = 1000.0
    CAR_STRIDE = CAR_LENGTH + CAR_GAP

    for car_prefix, loot_list in CAR_LOOT.items():
        car_idx = int(car_prefix.split("Car")[1])
        car_x = car_idx * CAR_STRIDE
        half_l = CAR_LENGTH / 2.0

        for i, pickup_type in enumerate(loot_list):
            bp_path = BP_PICKUPS.get(pickup_type)
            if not bp_path:
                continue
            # Distribute items along car length, offset Y to avoid center path
            item_x = car_x - half_l + 2000.0 + i * (CAR_LENGTH / (len(loot_list) + 1))
            item_y = 800.0 if (i % 2 == 0) else -800.0
            loc = unreal.Vector(item_x, item_y, 200.0)
            pickup_label = f"Pickup_Z1_{car_prefix}_{pickup_type}_{i}"
            actor = _spawn_bp_at(bp_path, loc, pickup_label)
            if actor:
                _spawned_count["pickups"] += 1

        unreal.log(f"  {car_prefix}: {len(loot_list)} items scattered")

    # ------ Ambient Sound per car ------
    unreal.log("\n--- Spawning Ambient Sounds ---")
    for car_idx in range(15):
        car_x = car_idx * CAR_STRIDE
        sound_label = f"Sound_Z1_Car{car_idx:02d}_Ambient"
        actor = _spawn_ambient_sound(sound_label, unreal.Vector(car_x, 0.0, 1500.0))
        if actor:
            _spawned_count["sounds"] += 1

    # ------ Summary ------
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("ZONE 1 POPULATION COMPLETE")
    unreal.log("=" * 60)
    unreal.log(f"  NPCs spawned:     {_spawned_count['npcs']}")
    unreal.log(f"  Boss spawned:     {_spawned_count['boss']}")
    unreal.log(f"  Pickups spawned:  {_spawned_count['pickups']}")
    unreal.log(f"  Sounds spawned:   {_spawned_count['sounds']}")
    unreal.log(f"  Triggers placed:  {_spawned_count['triggers']}")
    total = sum(_spawned_count.values())
    unreal.log(f"  TOTAL ACTORS:     {total}")
    unreal.log("")
    unreal.log("Next steps:")
    unreal.log("  1. Rebuild C++ (GameMode change)")
    unreal.log("  2. Open Zone1_Tail map")
    unreal.log("  3. Set GameMode Override to SnowpiercerEEGameMode")
    unreal.log("  4. Press Play")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__" or True:
    populate()
