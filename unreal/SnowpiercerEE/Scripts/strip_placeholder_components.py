"""Remove the VisBody/VisHead placeholder shape components that
fix_world_polish.py added to the character BPs — they now render inside the
real KayKit skeletal meshes ('sloppily overlapped' models)."""
import unreal

editor_util = unreal.EditorAssetLibrary
BPS = [
    "/Game/Blueprints/Characters/BP_SEECharacter",
    "/Game/Blueprints/NPCs/BP_NPC_Civilian",
    "/Game/Blueprints/NPCs/BP_NPC_Jackboot",
    "/Game/Blueprints/NPCs/BP_NPC_Merchant",
    "/Game/Blueprints/NPCs/BP_NPC_Breachman",
    "/Game/Blueprints/NPCs/BP_NPC_FirstClass",
]
KILL_NAMES = ("VisBody", "VisHead")

sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
total = 0
for path in BPS:
    if not editor_util.does_asset_exist(path):
        unreal.log_warning(f"  missing BP: {path}")
        continue
    bp = unreal.load_asset(path)
    try:
        handles = sds.k2_gather_subobject_data_for_blueprint(bp)
    except Exception as e:
        unreal.log_warning(f"  gather failed on {path}: {e}")
        continue
    removed_here = 0
    for handle in handles:
        try:
            data = sds.k2_find_subobject_data_from_handle(handle)
            obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
            if not obj:
                continue
            name = obj.get_name()
            if any(name.startswith(k) for k in KILL_NAMES):
                deleted = sds.delete_subobject(handles[0], handle, bp)
                removed_here += 1
        except Exception as e:
            unreal.log_warning(f"  removal issue on {path}: {e}")
    if removed_here:
        editor_util.save_asset(path)
    unreal.log(f"  {path}: removed {removed_here}")
    total += removed_here

# Also sweep any leftover NPCVis_* placeholder actors in the level
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
world = unreal.EditorLevelLibrary.get_editor_world()
if not world or "Zone1_Tail" not in world.get_path_name():
    les.load_level("/Game/Maps/Zone1_Tail")
lvl_removed = 0
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    try:
        if actor.get_actor_label().startswith("NPCVis_"):
            actor.modify()
            unreal.EditorLevelLibrary.destroy_actor(actor)
            lvl_removed += 1
    except Exception:
        continue
if lvl_removed:
    les.save_all_dirty_levels()
    try:
        les.save_current_level()
    except Exception:
        pass

unreal.log(f"STRIP DONE -- {total} BP components, {lvl_removed} level actors removed")
