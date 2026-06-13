"""Robustly kill the VisBody/VisHead placeholder components still on the
character blueprints (they render as boxes+spheres overlapping the real mesh).
The prior SubobjectDataSubsystem deletion didn't persist, so this does THREE
things per component: proper subobject delete, plus CDO hide + zero-scale +
no-collision as a bulletproof visual fallback, then recompiles and saves."""
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
KILL = ("VisBody", "VisHead", "Vis_Body", "Vis_Head", "PlaceholderBody",
        "PlaceholderHead", "BoxBody", "SphereHead")

sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)


def is_placeholder(name):
    return any(name.startswith(k) or k in name for k in KILL)


def delete_via_subobjects(bp):
    removed = 0
    try:
        handles = sds.k2_gather_subobject_data_for_blueprint(bp)
    except Exception as e:
        unreal.log_warning(f"  gather failed: {e}")
        return 0
    root = handles[0] if handles else None
    for h in list(handles):
        try:
            data = sds.k2_find_subobject_data_from_handle(h)
            obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
            if obj and is_placeholder(obj.get_name()):
                try:
                    sds.delete_subobject(root, h, bp)
                    removed += 1
                except Exception:
                    # plural variant fallback
                    try:
                        sds.delete_subobjects(root, [h], bp)
                        removed += 1
                    except Exception as e2:
                        unreal.log_warning(f"  delete failed: {e2}")
        except Exception:
            continue
    return removed


def hide_on_cdo(bp):
    """Bulletproof: hide + zero-scale + no-collision any surviving placeholder
    component on the class default object so it never renders."""
    hidden = 0
    try:
        gen = bp.generated_class()
        cdo = unreal.get_default_object(gen)
    except Exception as e:
        unreal.log_warning(f"  CDO access failed: {e}")
        return 0
    comps = []
    for cls in (unreal.StaticMeshComponent, unreal.PrimitiveComponent,
                unreal.SceneComponent):
        try:
            comps += list(cdo.get_components_by_class(cls))
        except Exception:
            pass
    seen = set()
    for comp in comps:
        try:
            name = comp.get_name()
            if name in seen or not is_placeholder(name):
                continue
            seen.add(name)
            try:
                comp.set_visibility(False, True)
            except Exception:
                pass
            try:
                comp.set_hidden_in_game(True, True)
            except Exception:
                pass
            try:
                comp.set_editor_property("relative_scale3d",
                                         unreal.Vector(0.0001, 0.0001, 0.0001))
            except Exception:
                pass
            try:
                comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
            except Exception:
                pass
            hidden += 1
        except Exception:
            continue
    return hidden


total_del = 0
total_hid = 0
for path in BPS:
    if not editor_util.does_asset_exist(path):
        unreal.log_warning(f"  missing BP: {path}")
        continue
    bp = unreal.load_asset(path)
    if not bp:
        continue
    d = delete_via_subobjects(bp)
    h = hide_on_cdo(bp)
    try:
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    except Exception:
        pass
    editor_util.save_asset(path)
    unreal.log(f"  {path}: deleted {d}, hidden {h}")
    total_del += d
    total_hid += h

# Sweep any stray NPCVis_ placeholder actors in the level too
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
world = unreal.EditorLevelLibrary.get_editor_world()
if not world or "Zone1_Tail" not in world.get_path_name():
    les.load_level("/Game/Maps/Zone1_Tail")
lvl = 0
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    try:
        if actor.get_actor_label().startswith("NPCVis_"):
            actor.modify()
            unreal.EditorLevelLibrary.destroy_actor(actor)
            lvl += 1
    except Exception:
        continue
if lvl:
    les.save_all_dirty_levels()
    try:
        les.save_current_level()
    except Exception:
        pass

unreal.log(f"PLACEHOLDER FIX DONE -- deleted {total_del}, hidden {total_hid} BP comps, {lvl} level actors")
