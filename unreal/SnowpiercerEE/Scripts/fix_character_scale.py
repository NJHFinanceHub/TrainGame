"""Right-size character meshes: scale so each stands capsule height (~170cm),
feet on the floor. Re-runs idempotently."""
import unreal

editor_util = unreal.EditorAssetLibrary
TARGET_HEIGHT = 170.0
CAPSULE_HALF = 88.0

ROLE_BPS = {
    "character-a": "/Game/Blueprints/Characters/BP_SEECharacter",
    "character-b": "/Game/Blueprints/NPCs/BP_NPC_Civilian",
    "character-d": "/Game/Blueprints/NPCs/BP_NPC_Jackboot",
    "character-f": "/Game/Blueprints/NPCs/BP_NPC_Merchant",
    "character-h": "/Game/Blueprints/NPCs/BP_NPC_Breachman",
    "character-j": "/Game/Blueprints/NPCs/BP_NPC_FirstClass",
}

def mesh_height(sk):
    try:
        b = sk.get_bounds()
        return b.box_extent.z * 2.0
    except Exception:
        try:
            return sk.get_editor_property("imported_bounds").box_extent.z * 2.0
        except Exception:
            return 0.0

def find_mesh(model):
    base = f"/Game/Characters/Blocky/{model}"
    for p in editor_util.list_assets(base, recursive=True, include_folder=False):
        a = unreal.load_asset(p.split(".")[0])
        if isinstance(a, unreal.SkeletalMesh):
            return a
    return None

def fix_bp(bp_path, sk, scale):
    bp = unreal.load_asset(bp_path)
    cdo = unreal.get_default_object(bp.generated_class())
    comp = cdo.get_editor_property("mesh")
    comp.set_editor_property("relative_scale3d",
                             unreal.Vector(scale, scale, scale))
    comp.set_editor_property("relative_location",
                             unreal.Vector(0.0, 0.0, -CAPSULE_HALF))
    editor_util.save_asset(bp_path)

for model, bp_path in ROLE_BPS.items():
    sk = find_mesh(model)
    if not sk:
        unreal.log_warning(f"no mesh for {model}")
        continue
    h = mesh_height(sk)
    if h <= 1.0:
        unreal.log_warning(f"degenerate bounds for {model}: {h}")
        continue
    scale = TARGET_HEIGHT / h
    fix_bp(bp_path, sk, scale)
    unreal.log(f"  {model}: raw height {h:.0f} -> scale {scale:.2f} on {bp_path}")

# Boss: same math but 1.25x larger, applied to placed instances
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
world = unreal.EditorLevelLibrary.get_editor_world()
if not world or "Zone1_Tail" not in world.get_path_name():
    les.load_level("/Game/Maps/Zone1_Tail")
boss_sk = find_mesh("character-r")
if boss_sk:
    h = mesh_height(boss_sk)
    if h > 1.0:
        s = TARGET_HEIGHT / h * 1.25
        for actor in unreal.EditorLevelLibrary.get_all_level_actors():
            try:
                if actor.get_actor_label().startswith("Boss_"):
                    comp = actor.get_component_by_class(unreal.SkeletalMeshComponent)
                    if comp:
                        actor.modify(); comp.modify()
                        comp.set_editor_property("relative_scale3d", unreal.Vector(s, s, s))
                        unreal.log(f"  boss scale {s:.2f}")
            except Exception:
                continue
les.save_all_dirty_levels()
try:
    les.save_current_level()
except Exception:
    pass
unreal.log("SCALE FIX DONE")
