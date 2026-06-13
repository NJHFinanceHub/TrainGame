"""
Snowpiercer: Eternal Engine -- Character Model Import & Assignment
Run headless: UnrealEditor-Cmd <uproject> -ExecutePythonScript=<this file>

Imports the Quaternius Zombie Apocalypse Kit (CC0, rigged FBX, March 2024)
from Assets/Characters/QuaterniusZombieApocalypse and assigns skeletal meshes
to the player and NPC blueprints so characters are real gritty post-apoc
humans instead of placeholder geometry or mis-cast fantasy adventurers.

Pack: "Zombie Apocalypse Kit - March 2024" by Quaternius
License: CC0 (https://creativecommons.org/publicdomain/zero/1.0/)
Source: https://archive.org/download/zombie-apocalypse-kit/
        Zombie%20Apocalypse%20Kit%20-%20March%202024-20240909T025008Z-001.zip
        (originally from https://quaternius.com/)

Characters included (4 survivors x 20 anims each + 4 zombie/infected enemies):
  Characters_Sam    -- ragged survivor, casual jacket/jeans/boots
  Characters_Lis    -- female survivor, practical layered clothing
  Characters_Matt   -- stocky male survivor, heavy coat
  Characters_Shaun  -- wiry male survivor, hooded vest
  Zombie_Basic      -- infected humanoid, torn clothes (enemy)
  Zombie_Chubby     -- large/heavy infected, hulking frame (boss material)
  Zombie_Ribcage    -- skeletal infected (alternate enemy)
  Zombie_Arm        -- partial infected (environmental/scripted use)

Why this fits Snowpiercer: all models are gritty, realistic-proportion,
modern-clothed humans or infected humanoids -- appropriate for the tail end
of an apocalyptic train. Fully replaces the fantasy Knight/Mage/Barbarian set.

Idempotent: re-running re-imports only missing assets and re-assigns meshes.
"""

import os
import unreal

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)
REPO_ROOT = os.path.dirname(os.path.dirname(PROJECT_DIR))
# Quaternius Zombie Apocalypse Kit (CC0): rigged FBX humanoids with 20 anims
FBX_DIR = os.path.join(
    REPO_ROOT, "Assets", "Characters", "QuaterniusZombieApocalypse",
    "Zombie Apocalypse Kit - March 2024", "Characters", "FBX")
MODEL_EXT = ".fbx"

DEST = "/Game/Characters/QuaterniusZombieApocalypse"
editor_util = unreal.EditorAssetLibrary

# Snowpiercer role casting (all are modern/gritty post-apoc humans):
#   Characters_Sam    = ragged survivor, informal wear   -> player + merchants
#   Characters_Lis    = female survivor, practical gear  -> Tailie civilians
#   Characters_Matt   = stocky survivor, heavy coat      -> Jackboot guard
#   Characters_Shaun  = wiry survivor, hooded vest       -> First Class attendant
#   Zombie_Chubby     = large/hulking infected frame     -> Breachman (heavyset)
#   Zombie_Basic      = standard infected humanoid       -> Boss (scaled up x3.4)
ROLE_MODELS = {
    "player":     "Characters_Sam",
    "civilian":   "Characters_Lis",
    "jackboot":   "Characters_Matt",
    "merchant":   "Characters_Sam",
    "breachman":  "Zombie_Chubby",
    "firstclass": "Characters_Shaun",
    "boss":       "Zombie_Basic",
}

# Stand height for a scaled character (capsule is 176cm tall)
TARGET_HEIGHT = 170.0

BP_FOR_ROLE = {
    "player":     "/Game/Blueprints/Characters/BP_SEECharacter",
    "civilian":   "/Game/Blueprints/NPCs/BP_NPC_Civilian",
    "jackboot":   "/Game/Blueprints/NPCs/BP_NPC_Jackboot",
    "merchant":   "/Game/Blueprints/NPCs/BP_NPC_Merchant",
    "breachman":  "/Game/Blueprints/NPCs/BP_NPC_Breachman",
    "firstclass": "/Game/Blueprints/NPCs/BP_NPC_FirstClass",
}


def import_fbx(fbx_path, dest_path, asset_name):
    """Import one character model (GLB via Interchange, FBX via legacy)."""
    if editor_util.does_asset_exist(f"{dest_path}/{asset_name}"):
        unreal.log(f"  Already imported: {asset_name}")
        return True

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", fbx_path)
    task.set_editor_property("destination_path", dest_path)
    task.set_editor_property("destination_name", asset_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    # GLB goes through Interchange with default options; force synchronous
    # so headless runs see the results before exiting
    try:
        task.set_editor_property("async_", False)
    except Exception:
        pass
    if fbx_path.lower().endswith(".fbx"):
        options = unreal.FbxImportUI()
        options.set_editor_property("import_mesh", True)
        options.set_editor_property("import_as_skeletal", True)
        options.set_editor_property("import_materials", True)
        options.set_editor_property("import_textures", True)
        options.set_editor_property("import_animations", True)
        task.set_editor_property("options", options)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    # Interchange may nest assets under the destination folder — existence
    # check by skeletal mesh discovery instead of exact path
    ok = (editor_util.does_asset_exist(f"{dest_path}/{asset_name}")
          or find_skeletal_mesh(dest_path, asset_name) is not None)
    unreal.log(f"  {'Imported' if ok else 'FAILED'}: {asset_name}")
    return ok


def find_skeletal_mesh(base_path, name_hint):
    """Locate the skeletal mesh asset produced by an import (name may vary)."""
    direct = f"{base_path}/{name_hint}"
    if editor_util.does_asset_exist(direct):
        a = unreal.load_asset(direct)
        if isinstance(a, unreal.SkeletalMesh):
            return a
    try:
        for p in editor_util.list_assets(base_path, recursive=True,
                                         include_folder=False):
            obj_path = p.split(".")[0]
            if name_hint in obj_path:
                a = unreal.load_asset(obj_path)
                if isinstance(a, unreal.SkeletalMesh):
                    return a
    except Exception:
        pass
    return None


def find_idle_anim(base_path, name_hint):
    """Find an animation sequence from the same import (prefer idle)."""
    best = None
    try:
        for p in editor_util.list_assets(base_path, recursive=True,
                                         include_folder=False):
            obj_path = p.split(".")[0]
            if name_hint not in obj_path:
                continue
            a = unreal.load_asset(obj_path)
            if isinstance(a, unreal.AnimSequence):
                lname = obj_path.lower()
                if "idle" in lname:
                    return a
                if best is None:
                    best = a
    except Exception:
        pass
    return best


def assign_mesh_to_bp(bp_path, skel_mesh, idle_anim):
    """Set the skeletal mesh (and idle anim) on a Character BP's CDO."""
    if not editor_util.does_asset_exist(bp_path):
        unreal.log_warning(f"  BP missing: {bp_path}")
        return False
    bp = unreal.load_asset(bp_path)
    if not bp:
        return False
    try:
        gen_class = bp.generated_class()
        cdo = unreal.get_default_object(gen_class)
        mesh_comp = cdo.get_editor_property("mesh")
        if not mesh_comp:
            unreal.log_warning(f"  No mesh component on {bp_path}")
            return False
        mesh_comp.set_editor_property("skeletal_mesh_asset", skel_mesh)
        # Auto-scale so the character stands TARGET_HEIGHT, feet at capsule base
        scale = 1.0
        try:
            h = skel_mesh.get_bounds().box_extent.z * 2.0
            if h > 1.0:
                scale = TARGET_HEIGHT / h
        except Exception:
            pass
        mesh_comp.set_editor_property(
            "relative_location", unreal.Vector(0.0, 0.0, -88.0))
        mesh_comp.set_editor_property(
            "relative_rotation", unreal.Rotator(0.0, 0.0, -90.0))
        mesh_comp.set_editor_property(
            "relative_scale3d", unreal.Vector(scale, scale, scale))
        if idle_anim:
            try:
                mesh_comp.set_editor_property(
                    "animation_mode",
                    unreal.AnimationMode.ANIMATION_SINGLE_NODE)
                # UE 5.7: single-node anim lives in the AnimationData struct
                anim_data = mesh_comp.get_editor_property("animation_data")
                anim_data.set_editor_property("anim_to_play", idle_anim)
                anim_data.set_editor_property("saved_looping", True)
                anim_data.set_editor_property("saved_playing", True)
                mesh_comp.set_editor_property("animation_data", anim_data)
            except Exception as e:
                unreal.log_warning(f"  Anim assign failed on {bp_path}: {e}")
        editor_util.save_asset(bp_path)
        unreal.log(f"  Assigned {skel_mesh.get_name()} -> {bp_path}")
        return True
    except Exception as e:
        unreal.log_warning(f"  Mesh assign failed on {bp_path}: {e}")
        return False


def remove_placeholder_actors():
    """Remove NPCVis_* box+sphere placeholders now that BPs have real meshes."""
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world or "Zone1_Tail" not in world.get_path_name():
        les.load_level("/Game/Maps/Zone1_Tail")
    removed = 0
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        try:
            if actor.get_actor_label().startswith("NPCVis_"):
                actor.modify()
                unreal.EditorLevelLibrary.destroy_actor(actor)
                removed += 1
        except Exception:
            continue
    unreal.log(f"  Placeholder actors removed: {removed}")
    try:
        les.save_all_dirty_levels()
        les.save_current_level()
    except Exception as e:
        unreal.log_warning(f"  Level save failed: {e}")


def run():
    unreal.log("=" * 64)
    unreal.log("CHARACTER IMPORT & ASSIGNMENT (Quaternius Zombie Apocalypse Kit, CC0)")
    unreal.log("=" * 64)

    if not os.path.isdir(FBX_DIR):
        unreal.log_error(f"FBX dir not found: {FBX_DIR}")
        return

    # 1. Import the role models
    needed = sorted(set(ROLE_MODELS.values()))
    for model in needed:
        src = os.path.join(FBX_DIR, f"{model}{MODEL_EXT}")
        if os.path.isfile(src):
            import_fbx(src, f"{DEST}/{model}", model)
        else:
            unreal.log_warning(f"  Source missing: {src}")

    # 2. Assign to blueprints
    assigned = 0
    boss_mesh = None
    boss_anim = None
    for role, model in ROLE_MODELS.items():
        base = f"{DEST}/{model}"
        sk = find_skeletal_mesh(base, model)
        if not sk:
            unreal.log_warning(f"  No skeletal mesh found for {role} ({model})")
            continue
        anim = find_idle_anim(base, model)
        if role == "boss":
            boss_mesh, boss_anim = sk, anim
            continue
        if assign_mesh_to_bp(BP_FOR_ROLE[role], sk, anim):
            assigned += 1

    # 3. Boss: no dedicated BP -- patch placed Boss_* instances directly
    if boss_mesh:
        world = unreal.EditorLevelLibrary.get_editor_world()
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        if not world or "Zone1_Tail" not in world.get_path_name():
            les.load_level("/Game/Maps/Zone1_Tail")
        for actor in unreal.EditorLevelLibrary.get_all_level_actors():
            try:
                if not actor.get_actor_label().startswith("Boss_"):
                    continue
                comp = actor.get_component_by_class(
                    unreal.SkeletalMeshComponent)
                if comp:
                    actor.modify()
                    comp.modify()
                    comp.set_editor_property("skeletal_mesh_asset", boss_mesh)
                    comp.set_editor_property(
                        "relative_scale3d", unreal.Vector(3.4, 3.4, 3.4))
                    if boss_anim:
                        try:
                            comp.set_editor_property(
                                "animation_mode",
                                unreal.AnimationMode.ANIMATION_SINGLE_NODE)
                            anim_data = comp.get_editor_property(
                                "animation_data")
                            anim_data.set_editor_property(
                                "anim_to_play", boss_anim)
                            anim_data.set_editor_property(
                                "saved_looping", True)
                            anim_data.set_editor_property(
                                "saved_playing", True)
                            comp.set_editor_property("animation_data",
                                                     anim_data)
                        except Exception:
                            pass
                    unreal.log(f"  Boss instance patched: "
                               f"{actor.get_actor_label()}")
            except Exception:
                continue

    # 4. Drop the box+sphere placeholders
    remove_placeholder_actors()

    unreal.log("=" * 64)
    unreal.log(f"DONE -- {assigned} blueprints assigned real character meshes")
    unreal.log("=" * 64)


if __name__ == "__main__" or True:
    run()
