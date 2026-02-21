"""
Fix NPC Visibility — Assign placeholder mesh to all NPC Blueprints
Run in editor: Tools > Execute Python Script

Adds the UE5 Mannequin mesh if available, otherwise creates a visible
capsule-shaped static mesh placeholder so NPCs are visible in-game.
"""

import unreal

editor_util = unreal.EditorAssetLibrary
level_lib = unreal.EditorLevelLibrary

# ---------------------------------------------------------------------------
# Find a usable skeletal mesh
# ---------------------------------------------------------------------------

MANNEQUIN_PATHS = [
    # Project-local
    "/Game/Characters/Mannequins/Meshes/SKM_Manny",
    "/Game/Characters/Mannequins/Meshes/SKM_Quinn",
    "/Game/Characters/Mannequin/Mesh/SKM_Manny",
    "/Game/ThirdPerson/Characters/Mannequins/Meshes/SKM_Manny",
    # Engine content (requires Show Engine Content in content browser)
    "/Engine/EngineMeshes/SkeletalCube",
    "/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP",
]


def find_skeletal_mesh():
    """Try to find any usable skeletal mesh."""
    for path in MANNEQUIN_PATHS:
        if editor_util.does_asset_exist(path):
            mesh = editor_util.load_asset(path)
            if mesh and isinstance(mesh, unreal.SkeletalMesh):
                unreal.log(f"  Found skeletal mesh: {path}")
                return mesh, path
    return None, None


# ---------------------------------------------------------------------------
# Patch NPC Blueprints
# ---------------------------------------------------------------------------

NPC_BPS = [
    "/Game/Blueprints/NPCs/BP_NPC_Civilian",
    "/Game/Blueprints/NPCs/BP_NPC_Jackboot",
    "/Game/Blueprints/NPCs/BP_NPC_Merchant",
    "/Game/Blueprints/NPCs/BP_NPC_Breachman",
    "/Game/Blueprints/NPCs/BP_NPC_FirstClass",
]

# Also patch the player character
CHAR_BP = "/Game/Blueprints/Characters/BP_SEECharacter"


def patch_bp_mesh(bp_path, sk_mesh):
    """Set the skeletal mesh on a Blueprint's CDO."""
    if not editor_util.does_asset_exist(bp_path):
        unreal.log_warning(f"  BP not found: {bp_path}")
        return False

    bp = editor_util.load_asset(bp_path)
    if not bp:
        return False

    try:
        cdo = unreal.get_default_object(bp.generated_class())
        if not cdo:
            unreal.log_warning(f"  Could not get CDO for {bp_path}")
            return False

        mesh_comp = cdo.get_component_by_class(unreal.SkeletalMeshComponent)
        if mesh_comp:
            mesh_comp.set_skeletal_mesh_asset(sk_mesh)
            # Make sure the mesh is visible
            mesh_comp.set_editor_property("visible", True)
            mesh_comp.set_editor_property("hidden_in_game", False)
            editor_util.save_asset(bp_path)
            unreal.log(f"  Patched: {bp_path}")
            return True
        else:
            unreal.log_warning(f"  No SkeletalMeshComponent on CDO: {bp_path}")
            return False
    except Exception as e:
        unreal.log_warning(f"  Error patching {bp_path}: {e}")
        return False


def add_mannequin_content():
    """Try to add the Third Person Mannequin content pack."""
    # Check if we can find it in engine templates
    template_paths = [
        "/Engine/EngineSky/SM_SkySphere",  # Just to test engine content access
    ]
    for p in template_paths:
        if editor_util.does_asset_exist(p):
            unreal.log("  Engine content is accessible")
            break


def make_placeholder_visible_in_level():
    """Find all NPC actors in the level and give them a visible static mesh
    if they don't have a skeletal mesh."""
    actors = level_lib.get_all_level_actors()
    patched = 0

    # Engine primitive shapes
    cube_path = "/Engine/BasicShapes/Cube"
    cylinder_path = "/Engine/BasicShapes/Cylinder"

    shape_mesh = None
    for path in [cylinder_path, cube_path]:
        if editor_util.does_asset_exist(path):
            shape_mesh = editor_util.load_asset(path)
            if shape_mesh:
                break

    if not shape_mesh:
        unreal.log_warning("  No engine shapes found for placeholder")
        return 0

    for actor in actors:
        label = actor.get_actor_label()
        if not label.startswith("NPC_") and not label.startswith("Boss_"):
            continue

        # Check if actor already has a visible mesh
        sk_comp = actor.get_component_by_class(unreal.SkeletalMeshComponent)
        has_mesh = False
        if sk_comp:
            try:
                current_mesh = sk_comp.get_editor_property("skeletal_mesh_asset")
                if current_mesh:
                    has_mesh = True
            except:
                pass

        if has_mesh:
            continue

        # Add a static mesh component as visual placeholder
        # Scale to roughly human size: cylinder is 100cm, we want ~180cm tall
        sm_comp = actor.get_component_by_class(unreal.StaticMeshComponent)
        if not sm_comp:
            # Can't add components to spawned actors easily via Python,
            # so instead we'll spawn a static mesh actor at the same location
            # and label it so we know it's an NPC placeholder
            loc = actor.get_actor_location()
            rot = actor.get_actor_rotation()

            placeholder = level_lib.spawn_actor_from_class(
                unreal.StaticMeshActor, loc, rot)
            if placeholder:
                placeholder.set_actor_label(f"Vis_{label}")
                sm = placeholder.get_component_by_class(unreal.StaticMeshComponent)
                if sm:
                    sm.set_static_mesh(shape_mesh)
                    # Scale to human proportions (cylinder default is 100x100x100)
                    placeholder.set_actor_scale3d(unreal.Vector(0.8, 0.8, 1.8))

                    # Color by type
                    if "Jackboot" in label or "Boss" in label:
                        # Red for enemies
                        mat_path = "/Engine/EngineMaterials/DefaultLitMaterial"
                    else:
                        # Default for friendlies
                        mat_path = "/Engine/EngineMaterials/DefaultLitMaterial"

                patched += 1

    return patched


# ---------------------------------------------------------------------------
# Entry
# ---------------------------------------------------------------------------

def run():
    unreal.log("=" * 60)
    unreal.log("FIX NPC VISIBILITY")
    unreal.log("=" * 60)

    # Step 1: Try to find a skeletal mesh
    sk_mesh, sk_path = find_skeletal_mesh()

    if sk_mesh:
        # Patch all NPC BPs with the skeletal mesh
        unreal.log(f"\nUsing skeletal mesh: {sk_path}")
        patched = 0
        for bp_path in NPC_BPS:
            if patch_bp_mesh(bp_path, sk_mesh):
                patched += 1
        # Also patch player character
        if editor_util.does_asset_exist(CHAR_BP):
            patch_bp_mesh(CHAR_BP, sk_mesh)

        unreal.log(f"\nPatched {patched} NPC Blueprints with skeletal mesh")
        unreal.log("Re-run populate_zone1.py to respawn NPCs with meshes")
    else:
        unreal.log("\nNo skeletal mesh found in project or engine content")
        unreal.log("Adding visible placeholder shapes to NPC actors in level...")

        # Spawn visible placeholder shapes next to NPC actors
        patched = make_placeholder_visible_in_level()
        unreal.log(f"Added {patched} visible NPC placeholders")

    unreal.log("")
    unreal.log("If NPCs still invisible, add Mannequin content to project:")
    unreal.log("  Content Browser > Add > Add Feature or Content Pack")
    unreal.log("  > Third Person > Add to Project")
    unreal.log("  Then re-run this script")
    unreal.log("=" * 60)


if __name__ == "__main__" or True:
    run()
