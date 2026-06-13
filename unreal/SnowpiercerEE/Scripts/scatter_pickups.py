"""Scatter additional pickups across Zone1_Tail so there's real loot to find —
weapons, consumables, crafting, and the new equippable armor (for the paper-doll).
Spawns ASEEPickupActor (C++) with ItemID/Quantity set. Idempotent (prefix LOOT_)."""
import unreal

editor_util = unreal.EditorAssetLibrary
CAR_STRIDE = 13000.0
NUM_CARS = 15

# (ItemID, Quantity) loot table, weighted toward consumables/crafting with
# armor + weapons sprinkled as rarer finds.
LOOT = [
    ("Item_ProteinBlock", 2), ("Item_ProteinBlock", 1), ("Item_WaterRation", 1),
    ("Item_Bandage", 1), ("Item_Bandage", 2), ("Item_Painkillers", 1),
    ("Item_ScrapMetal", 3), ("Item_ScrapMetal", 2), ("Item_ClothStrips", 2),
    ("Item_Wire", 2), ("Item_Coal", 2),
    ("Item_Shiv", 1), ("Item_RustyPipe", 1), ("Item_Wrench", 1),
    ("Item_Armor_TailRags", 1), ("Item_Armor_WorkerCoat", 1),
    ("Item_Armor_JackbootHelmet", 1), ("Item_Armor_ScrapPlate", 1),
    ("Item_Armor_EngineMonkHood", 1), ("Item_Armor_KronoleGuard", 1),
    ("Item_Armor_FreezerSuit", 1),
]

# Per-car loot count (rear/tail richer in scraps, mid cars have gear)
PER_CAR = [3, 4, 4, 2, 3, 3, 3, 4, 2, 2, 2, 3, 3, 3, 2]

def load_level():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world or "Zone1_Tail" not in world.get_path_name():
        les.load_level("/Game/Maps/Zone1_Tail")
    return les

les = load_level()

# Cleanup prior LOOT_ actors
removed = 0
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    try:
        if actor.get_actor_label().startswith("LOOT_"):
            actor.modify()
            unreal.EditorLevelLibrary.destroy_actor(actor)
            removed += 1
    except Exception:
        continue

pickup_cls = unreal.load_class(None, "/Script/SnowpiercerEE.SEEPickupActor")
if not pickup_cls:
    unreal.log_error("ASEEPickupActor class not found — aborting")
else:
    # deterministic spread
    li = 0
    placed = 0
    for car in range(NUM_CARS):
        car_x = car * CAR_STRIDE
        count = PER_CAR[car] if car < len(PER_CAR) else 2
        for i in range(count):
            item_id, qty = LOOT[li % len(LOOT)]
            li += 1
            # Spread along the car in the CLEAR WALK AISLE (|y|<650 is kept free
            # of props by the dressing passes), so spawns never collide with bunks
            # and the player walks right over the pickups.
            x = car_x - 4500.0 + (i + 1) * (9000.0 / (count + 1))
            y = 400.0 if (i % 2 == 0) else -400.0
            loc = unreal.Vector(x, y, 110.0)
            try:
                actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
                    pickup_cls, loc, unreal.Rotator(0.0, float((i * 47) % 360), 0.0))
                if not actor:
                    unreal.log_warning(f"  spawn returned None car{car} {item_id}")
                    continue
                actor.set_actor_label(f"LOOT_Car{car:02d}_{item_id}_{i}")
                try:
                    actor.modify()
                    actor.set_editor_property("ItemID", item_id)  # str -> FName
                    actor.set_editor_property("Quantity", qty)
                except Exception as e:
                    unreal.log_warning(f"  set props failed {item_id}: {e}")
                placed += 1
            except Exception as e:
                unreal.log_warning(f"  spawn failed car{car} {item_id}: {e}")
        unreal.log(f"  Car {car:02d}: +{count} loot")

    les.save_all_dirty_levels()
    try:
        les.save_current_level()
    except Exception:
        pass
    unreal.log(f"SCATTER PICKUPS DONE -- removed {removed}, placed {placed}")
