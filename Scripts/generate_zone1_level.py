"""
generate_zone1_level.py — UE5 Python editor utility
Reads DT_Zone1Cars DataTable and spawns car actors, PlayerStart, and NPC placeholders.
Run from Edit > Execute Python Script in UE Editor.
"""

import unreal

DATA_TABLE_PATH = "/Game/DataTables/DT_Zone1Cars"
CAR_LENGTH = 5000.0  # 50m in UE units (cm)
CAR_WIDTH = 320.0
CAR_HEIGHT = 260.0
TAG_PREFIX = "Z1_Generated"


def _load_dt():
    dt = unreal.EditorAssetLibrary.load_asset(DATA_TABLE_PATH)
    if not dt:
        unreal.log_error(f"Could not load DataTable at {DATA_TABLE_PATH}")
        return None
    return dt


def _clear_previous():
    """Remove previously generated actors."""
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in actors:
        if actor.actor_has_tag(TAG_PREFIX):
            actor.destroy_actor()
    unreal.log(f"Cleared previous {TAG_PREFIX} actors.")


def _spawn_car(cube_mesh, car_index, display_name, sublevel_name):
    """Spawn a placeholder box for one car."""
    x = car_index * CAR_LENGTH
    location = unreal.Vector(x + CAR_LENGTH / 2.0, 0.0, CAR_HEIGHT / 2.0)
    rotation = unreal.Rotator(0, 0, 0)

    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor, location, rotation
    )
    if not actor:
        return False

    actor.set_actor_label(f"Z1_Car{car_index:02d}_{display_name.replace(' ', '_')}")
    actor.tags.append(TAG_PREFIX)

    smc = actor.static_mesh_component
    if smc and cube_mesh:
        smc.set_static_mesh(cube_mesh)
        smc.set_world_scale3d(
            unreal.Vector(CAR_LENGTH / 100.0, CAR_WIDTH / 100.0, CAR_HEIGHT / 100.0)
        )

    return True


def _spawn_player_start(car_index):
    """Spawn a PlayerStart at car 0 only."""
    if car_index != 0:
        return

    location = unreal.Vector(CAR_LENGTH / 2.0, 0.0, 100.0)
    rotation = unreal.Rotator(0, 0, 0)

    ps = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart, location, rotation
    )
    if ps:
        ps.set_actor_label("Z1_PlayerStart")
        ps.tags.append(TAG_PREFIX)


def _spawn_npc_placeholders(car_index, enemy_count):
    """Spawn target point markers for NPC spawn locations."""
    for i in range(max(enemy_count, 1)):
        x = car_index * CAR_LENGTH + (i + 1) * (CAR_LENGTH / (enemy_count + 2))
        y = 100.0 if (i % 2 == 0) else -100.0
        location = unreal.Vector(x, y, 100.0)

        marker = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.TargetPoint, location, unreal.Rotator(0, 0, 0)
        )
        if marker:
            marker.set_actor_label(f"Z1NPC_Car{car_index:02d}_{i + 1:02d}")
            marker.tags.append(TAG_PREFIX)


def generate_zone1_level():
    dt = _load_dt()
    if not dt:
        return

    row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt)
    if not row_names:
        unreal.log_warning("DT_Zone1Cars contains no rows.")
        return

    _clear_previous()
    cube_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")

    spawned = 0
    for i, row_name in enumerate(row_names):
        # Parse row data via export (simplified — in production, use typed struct access)
        car_index = i
        display_name = str(row_name)
        sublevel_name = f"Z1_Car{i:02d}"

        if _spawn_car(cube_mesh, car_index, display_name, sublevel_name):
            spawned += 1
        _spawn_player_start(car_index)
        _spawn_npc_placeholders(car_index, 2 if i in (8, 9, 14) else 0)

    unreal.EditorLevelLibrary.save_current_level()
    unreal.log(f"Zone1 scaffold complete: spawned {spawned} car actors from {DATA_TABLE_PATH}.")


if __name__ == "__main__":
    generate_zone1_level()
