"""
Snowpiercer: Eternal Engine -- Zone 1 DENSIFICATION script
Run in editor: Tools > Execute Python Script (after populate_zone1.py)

The Tail should feel CRAMMED. This script layers ~47 additional NPCs on top
of the ~19 placed by populate_zone1.py, bringing Zone1_Tail to ~60-66 souls.

Idempotent: every actor spawned here is labeled with the NPC_D_ prefix and
all NPC_D_* actors are destroyed at the start of each run, so re-running
never duplicates population. populate_zone1.py's own actors (NPC_*, Boss_*)
are NOT touched.

Label routing (SEENPCBrainSubsystem.cpp PickDialogueEntryNode, label match
is case-insensitive Contains):
  Gilliam / Elder / Whisper / Pike   -> Pike_01     (old-man-Pike tree)
  Hothead / Samuel / Firebrand       -> Samuel_01   (young firebrand)
  Kronole / Smuggler / Dealer        -> Dealer_01   (kronole dealer)
  Mourner / Tanya / Widow            -> Tanya_01    (grief tree)
  DrAsha / Asha / Sickbay / Injured  -> Injured_01  (Mara's tree)
  Workshop / Mechanic / Car07        -> Mechanic_01 (workshop tree)
  Jackboot class / 'Jackboot' label  -> hostile, Guard_01 (set by class path)
  everything else (generic civilian) -> Injured_01 fallback (still talkable)

So: generic civilian labels deliberately avoid all fragments above, and the
named archetypes deliberately include them.
"""

import math
import random

import unreal

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

LEVEL_PATH = "/Game/Maps/Zone1_Tail"
LABEL_PREFIX = "NPC_D_"     # densification prefix -- cleanup key
SEED = 20260612             # fixed seed => deterministic re-runs

CAR_STRIDE = 13000.0        # car i center x = i * 13000
SPAWN_Z = 100.0
X_SCATTER = 5000.0          # |x offset from car center| < 5000
Y_MAX = 1500.0              # |y| <= 1500 (interior wall is at 2000)
Y_CORRIDOR = 500.0          # keep |y| > 500 in non-combat cars
# Members stand 120-195cm off the huddle center: pairwise distance tops out
# at ~2*195=390cm (pairs) / ~1.73*195=338cm (trios), i.e. always <400cm.
MEMBER_RADIUS_MIN = 120.0
MEMBER_RADIUS_MAX = 195.0

BP_NPCS = {
    "Civilian":   "/Game/Blueprints/NPCs/BP_NPC_Civilian",
    "Jackboot":   "/Game/Blueprints/NPCs/BP_NPC_Jackboot",
    "Merchant":   "/Game/Blueprints/NPCs/BP_NPC_Merchant",
    "Breachman":  "/Game/Blueprints/NPCs/BP_NPC_Breachman",
}

# ---------------------------------------------------------------------------
# Per-car roster
#   (bp_key, label_template, count, placement_mode)
#   label_template uses {i} for 1-based index within that line.
#   placement modes:
#     side  -- random scatter, off the corridor (500 < |y| < 1500)
#     bunk  -- bunk alcoves along the walls (800 < |y| < 1600)
#     open  -- combat car, corridor allowed (|y| < 1500)
#     pit   -- ring of spectators around the fight pit at car center
#     gate  -- guard line near the forward (+X) gate, facing tailward
# ---------------------------------------------------------------------------

CAR_ROSTERS = [
    # car, theme, lines
    (0, "Memorial", [
        ("Civilian", "NPC_D_Mourner_Car00_{i}", 2, "side"),       # -> Tanya_01
    ]),
    (1, "Bunk quarters", [
        ("Civilian", "NPC_D_Car01_Bunkmate_{i}", 6, "bunk"),      # fallback tree
    ]),
    (2, "Bunk quarters", [
        ("Civilian", "NPC_D_Car02_Bunkmate_{i}", 5, "bunk"),      # fallback tree
    ]),
    (3, "The Pit", [
        ("Civilian", "NPC_D_Hothead_Car03_{i}", 1, "pit"),        # -> Samuel_01
        ("Civilian", "NPC_D_Car03_Spectator_{i}", 3, "pit"),      # fallback tree
    ]),
    (4, "Nursery", [
        ("Civilian", "NPC_D_Tanya_Caretaker_Car04_{i}", 3, "side"),  # -> Tanya_01
    ]),
    (5, "Elders", [
        ("Civilian", "NPC_D_Elder_Car05_{i}", 1, "side"),         # -> Pike_01
        ("Civilian", "NPC_D_Car05_Tailie_{i}", 3, "side"),        # fallback tree
    ]),
    (6, "Sickbay", [
        ("Civilian", "NPC_D_Injured_Car06_{i}", 1, "side"),       # -> Injured_01
        ("Civilian", "NPC_D_Car06_Patient_{i}", 2, "side"),       # fallback tree
    ]),
    (7, "Workshop", [
        ("Civilian", "NPC_D_Mechanic_Car07_{i}", 1, "side"),      # -> Mechanic_01
        ("Civilian", "NPC_D_Car07_Apprentice_{i}", 2, "side"),    # 'Car07' -> Mechanic_01
    ]),
    (8, "Listening post", [
        ("Civilian", "NPC_D_Car08_Listener_{i}", 2, "side"),      # fallback tree
    ]),
    (9, "Blockade", [
        ("Jackboot", "NPC_D_Car09_Jackboot_{i}", 4, "open"),      # hostile, Guard_01
    ]),
    (10, "Dark car", [
        ("Civilian", "NPC_D_Car10_Drifter_{i}", 1, "side"),       # lone soul
    ]),
    (11, "Freezer breach", [
        ("Breachman", "NPC_D_Car11_Breachman_{i}", 1, "side"),    # -> Mechanic_01 (class)
    ]),
    (12, "Kronole den", [
        ("Civilian", "NPC_D_KronoleKim2", 1, "side"),             # -> Dealer_01
        ("Civilian", "NPC_D_Car12_Addict_{i}", 3, "side"),        # fallback tree
    ]),
    (13, "Smugglers cache", [
        ("Merchant", "NPC_D_Smuggler_Car13_{i}", 1, "side"),      # -> Dealer_01
        ("Civilian", "NPC_D_Car13_Lookout_{i}", 1, "side"),       # fallback tree
    ]),
    (14, "Martyrs gate", [
        ("Jackboot", "NPC_D_Car14_Gate_Jackboot_{i}", 3, "gate"), # hostile, Guard_01
    ]),
]

# ---------------------------------------------------------------------------
# Editor API helpers
# ---------------------------------------------------------------------------

def _actor_subsystem():
    try:
        eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        if eas:
            return eas
    except Exception:
        pass
    return None


def _all_actors():
    eas = _actor_subsystem()
    if eas:
        return eas.get_all_level_actors()
    return unreal.EditorLevelLibrary.get_all_level_actors()


def _spawn(cls, location, rotation):
    eas = _actor_subsystem()
    if eas:
        return eas.spawn_actor_from_class(cls, location, rotation)
    return unreal.EditorLevelLibrary.spawn_actor_from_class(cls, location, rotation)


def _destroy(actor):
    eas = _actor_subsystem()
    if eas:
        eas.destroy_actor(actor)
    else:
        unreal.EditorLevelLibrary.destroy_actor(actor)


def _load_bp_class(bp_path):
    """Load a Blueprint generated class for spawning (same as populate_zone1)."""
    try:
        cls = unreal.load_class(None, bp_path + "_C")
        if cls:
            return cls
    except Exception:
        pass
    try:
        bp = unreal.EditorAssetLibrary.load_asset(bp_path)
        if bp:
            return bp.generated_class()
    except Exception:
        pass
    return None


# ---------------------------------------------------------------------------
# Placement
# ---------------------------------------------------------------------------

def _split_clusters(rng, count):
    """Split a headcount into conversational clusters of 2-3 (1 only if count==1)."""
    sizes = []
    rem = count
    while rem > 0:
        if rem <= 3:            # 1, 2 or 3: emit as-is (1 only when count==1)
            sizes.append(rem)
            rem = 0
        elif rem == 4:          # avoid 3+1
            sizes.append(2)
            rem -= 2
        else:
            s = rng.choice([2, 3])
            sizes.append(s)
            rem -= s
    return sizes


def _clamp_pos(car_x, px, py, y_lo, y_hi):
    """Keep a point inside the car footprint and its allowed Y band."""
    px = max(car_x - X_SCATTER, min(car_x + X_SCATTER, px))
    if y_lo is not None:
        sign = 1.0 if py >= 0.0 else -1.0
        py = sign * min(max(abs(py), y_lo), y_hi)
    else:
        py = max(-y_hi, min(y_hi, py))
    return px, py


def _scatter_positions(rng, car_idx, count, mode):
    """Cluster-scatter positions for 'side' / 'bunk' / 'open' modes.

    Returns a list of (unreal.Vector, yaw). Members of a cluster stand
    within CLUSTER_RADIUS of the cluster center and face inward so groups
    read as conversations; singletons get a random yaw.
    """
    car_x = car_idx * CAR_STRIDE
    out = []
    for size in _split_clusters(rng, count):
        cx = car_x + rng.uniform(-(X_SCATTER - 500.0), X_SCATTER - 500.0)
        if mode == "bunk":
            cy = rng.choice([-1.0, 1.0]) * rng.uniform(950.0, 1450.0)
            y_lo, y_hi = 800.0, 1600.0
        elif mode == "open":
            cy = rng.uniform(-1100.0, 1100.0)
            y_lo, y_hi = None, Y_MAX
        else:  # side: stay off the walking corridor
            cy = rng.choice([-1.0, 1.0]) * rng.uniform(
                Y_CORRIDOR + 150.0, Y_MAX - 150.0)
            y_lo, y_hi = Y_CORRIDOR, Y_MAX
        for m in range(size):
            if size == 1:
                px, py = cx, cy
                yaw = rng.uniform(-180.0, 180.0)
            else:
                ang = (360.0 / size) * m + rng.uniform(-25.0, 25.0)
                rad = rng.uniform(MEMBER_RADIUS_MIN, MEMBER_RADIUS_MAX)
                px = cx + rad * math.cos(math.radians(ang))
                py = cy + rad * math.sin(math.radians(ang))
                yaw = math.degrees(math.atan2(cy - py, cx - px))  # face the huddle
            px, py = _clamp_pos(car_x, px, py, y_lo, y_hi)
            out.append((unreal.Vector(px, py, SPAWN_Z), yaw))
    return out


def _pit_positions(rng, car_idx, count):
    """Spectators ringing the fight pit at the car center, facing inward."""
    car_x = car_idx * CAR_STRIDE
    base_angles = [45.0, 135.0, 225.0, 315.0]
    out = []
    for i in range(count):
        ang = base_angles[i % len(base_angles)] + rng.uniform(-12.0, 12.0)
        rad = rng.uniform(780.0, 980.0)
        px = car_x + rad * math.cos(math.radians(ang))
        py = rad * math.sin(math.radians(ang))
        if abs(py) < Y_CORRIDOR + 20.0:  # never stand in the walking corridor
            py = math.copysign(Y_CORRIDOR + 20.0, py)
        yaw = math.degrees(math.atan2(-py, car_x - px))  # face the ring
        out.append((unreal.Vector(px, py, SPAWN_Z), yaw))
    return out


def _gate_positions(rng, car_idx, count):
    """Guard line near the forward (+X) gate, facing back down the train."""
    car_x = car_idx * CAR_STRIDE
    lane_ys = [0.0, -750.0, 750.0]
    out = []
    for i in range(count):
        px = car_x + rng.uniform(3800.0, 4800.0)
        py = lane_ys[i % len(lane_ys)] + rng.uniform(-150.0, 150.0)
        out.append((unreal.Vector(px, py, SPAWN_Z), 180.0))
    return out


def _positions_for(rng, car_idx, count, mode):
    if mode == "pit":
        return _pit_positions(rng, car_idx, count)
    if mode == "gate":
        return _gate_positions(rng, car_idx, count)
    return _scatter_positions(rng, car_idx, count, mode)


# ---------------------------------------------------------------------------
# Cleanup (idempotence)
# ---------------------------------------------------------------------------

def cleanup_previous():
    """Destroy every actor this script spawned on a previous run (NPC_D_*)."""
    unreal.log(f"--- Cleaning previous {LABEL_PREFIX}* densification actors ---")
    removed = 0
    for actor in _all_actors():
        if isinstance(actor, unreal.WorldSettings):
            continue
        try:
            label = actor.get_actor_label()
        except Exception:
            continue
        if label.startswith(LABEL_PREFIX):
            actor.modify()  # mark for undo/dirty before mutating placed actor
            _destroy(actor)
            removed += 1
    unreal.log(f"  Removed {removed} previous densification actors")
    return removed


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def populate_dense():
    unreal.log("=" * 64)
    unreal.log("ZONE 1 DENSIFICATION -- the Tail should be CRAMMED")
    unreal.log("=" * 64)

    # Make sure Zone1_Tail is the loaded level (headless runs start elsewhere)
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world or "Zone1_Tail" not in world.get_path_name():
        unreal.log(f"Loading {LEVEL_PATH} ...")
        les.load_level(LEVEL_PATH)

    cleanup_previous()

    # Preload BP classes once
    bp_classes = {}
    for key, path in BP_NPCS.items():
        cls = _load_bp_class(path)
        if cls is None:
            unreal.log_warning(f"  Could not load {path} -- '{key}' spawns skipped")
        bp_classes[key] = cls

    rng = random.Random(SEED)
    spawned_total = 0
    per_car_added = {}

    unreal.log("--- Spawning densification NPCs ---")
    for car_idx, theme, lines in CAR_ROSTERS:
        # Batch consecutive same-mode lines so named NPCs (elder, dealer,
        # mechanic...) land inside the same conversational clusters / ring
        # formations as their generic neighbors instead of standing alone.
        groups = []
        for line in lines:
            if groups and groups[-1][0] == line[3]:
                groups[-1][1].append(line)
            else:
                groups.append((line[3], [line]))

        added = 0
        for mode, glines in groups:
            positions = _positions_for(
                rng, car_idx, sum(l[2] for l in glines), mode)
            pi = 0
            for bp_key, label_tpl, count, _mode in glines:
                cls = bp_classes.get(bp_key)
                for i in range(1, count + 1):
                    loc, yaw = positions[pi]
                    pi += 1
                    if cls is None:
                        continue
                    rot = unreal.Rotator()
                    rot.yaw = yaw
                    actor = _spawn(cls, loc, rot)
                    if not actor:
                        unreal.log_warning(
                            f"  Car {car_idx:02d}: spawn failed for {bp_key} at {loc}")
                        continue
                    actor.set_actor_label(label_tpl.format(i=i))
                    added += 1
                    spawned_total += 1
        per_car_added[car_idx] = added
        unreal.log(f"  Car {car_idx:02d} ({theme:<16}): +{added}")

    # Census: every brain-adopted actor in the level (NPC_* covers NPC_D_*)
    npc_total = 0
    for actor in _all_actors():
        try:
            label = actor.get_actor_label()
        except Exception:
            continue
        if label.startswith("NPC_") or label.startswith("Boss_"):
            npc_total += 1

    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("DENSIFICATION COMPLETE")
    unreal.log("=" * 64)
    unreal.log(f"  New NPCs spawned this run : {spawned_total}")
    unreal.log(f"  Total NPC/Boss actors now : {npc_total}")
    for car_idx, theme, _lines in CAR_ROSTERS:
        unreal.log(f"    Car {car_idx:02d} {theme:<16} +{per_car_added.get(car_idx, 0)}")

    # Persist (both calls: dirty sublevels AND the current persistent level)
    unreal.log("Saving level ...")
    les.save_all_dirty_levels()
    les.save_current_level()
    unreal.log("Level saved.")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__" or True:
    populate_dense()
