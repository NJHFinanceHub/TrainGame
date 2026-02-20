"""
Snowpiercer: Eternal Engine — Batch Asset Import Script
Run from UE5 Editor: Tools > Execute Python Script > Browse to this file
Or from the Output Log: py "path/to/import_assets.py"

Imports all raw assets (FBX, textures, audio) from the repo root into
the UE Content folder with proper organization and material setup.
"""

import unreal
import os
import sys

# ---------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------

# Repo root is 3 levels up from Scripts/ inside the UE project
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)  # SnowpiercerEE/
RIG_DIR = os.path.dirname(os.path.dirname(PROJECT_DIR))  # mayor/rig/

# Asset destinations (UE Content paths)
DEST = {
    "meshes_pipes":     "/Game/Meshes/ModularPipes",
    "tex_pipes":        "/Game/Textures/ModularPipes",
    "tex_baked":        "/Game/Textures/BakedProps",
    "tex_metals":       "/Game/Textures/Metals",
    "tex_environment":  "/Game/Textures/Environment",
    "tex_brass":        "/Game/Textures/Brass",
    "audio":            "/Game/Audio",
    "concept_art":      "/Game/Reference/ConceptArt",
}

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_util = unreal.EditorAssetLibrary

def ensure_directory(content_path):
    """Create content directory if it doesn't exist."""
    if not editor_util.does_directory_exist(content_path):
        editor_util.make_directory(content_path)

def import_fbx(source_path, dest_path, asset_name=None):
    """Import a single FBX file as a static mesh."""
    if not os.path.exists(source_path):
        unreal.log_warning(f"FBX not found: {source_path}")
        return None

    ensure_directory(dest_path)

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source_path)
    task.set_editor_property("destination_path", dest_path)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)

    # FBX import options
    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", False)
    options.set_editor_property("import_materials", False)  # We'll create materials manually
    options.set_editor_property("import_textures", False)
    options.set_editor_property("import_animations", False)
    options.static_mesh_import_data.set_editor_property("combine_meshes", True)
    options.static_mesh_import_data.set_editor_property("generate_lightmap_u_vs", True)
    options.static_mesh_import_data.set_editor_property("auto_generate_collision", True)
    task.set_editor_property("options", options)

    asset_tools.import_asset_tasks([task])

    imported = task.get_editor_property("imported_object_paths")
    if imported:
        unreal.log(f"  Imported FBX: {imported[0]}")
        return imported[0]
    else:
        unreal.log_warning(f"  FBX import returned no paths: {source_path}")
        return None

def import_texture(source_path, dest_path):
    """Import a single texture file."""
    if not os.path.exists(source_path):
        unreal.log_warning(f"Texture not found: {source_path}")
        return None

    ensure_directory(dest_path)

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source_path)
    task.set_editor_property("destination_path", dest_path)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)

    asset_tools.import_asset_tasks([task])

    imported = task.get_editor_property("imported_object_paths")
    if imported:
        unreal.log(f"  Imported Texture: {imported[0]}")
        return imported[0]
    else:
        unreal.log_warning(f"  Texture import returned no paths: {source_path}")
        return None

def import_audio(source_path, dest_path):
    """Import a single audio file as SoundWave."""
    if not os.path.exists(source_path):
        unreal.log_warning(f"Audio not found: {source_path}")
        return None

    ensure_directory(dest_path)

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source_path)
    task.set_editor_property("destination_path", dest_path)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)

    asset_tools.import_asset_tasks([task])

    imported = task.get_editor_property("imported_object_paths")
    if imported:
        unreal.log(f"  Imported Audio: {imported[0]}")
        return imported[0]
    else:
        unreal.log_warning(f"  Audio import returned no paths: {source_path}")
        return None

def set_texture_type(asset_path, tex_type):
    """Set compression settings based on texture role (normal, spec, etc)."""
    tex = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not tex:
        return

    if tex_type == "normal":
        tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        tex.set_editor_property("srgb", False)
    elif tex_type in ("spec", "roughness", "metallic", "ao", "height"):
        tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
        tex.set_editor_property("srgb", False)
    # BaseColor/Diffuse stays default (TC_Default, sRGB=True)

def classify_texture(filename):
    """Determine texture type from filename conventions."""
    name_lower = filename.lower()
    if "_norm" in name_lower or "_normal" in name_lower or "Normal" in filename:
        return "normal"
    elif "_spec" in name_lower or "Roughness" in filename:
        return "roughness"
    elif "_metallic" in name_lower or "Metallic" in filename:
        return "metallic"
    elif "_ao" in name_lower:
        return "ao"
    elif "_height" in name_lower or "Height" in filename:
        return "height"
    else:
        return "diffuse"

def create_pbr_material(mat_name, dest_path, textures):
    """
    Create a PBR material instance from a set of classified textures.
    textures = {"diffuse": path, "normal": path, "roughness": path, ...}
    """
    ensure_directory(dest_path)

    factory = unreal.MaterialFactoryNew()
    mat_asset = asset_tools.create_asset(mat_name, dest_path, unreal.Material, factory)

    if not mat_asset:
        unreal.log_warning(f"  Failed to create material: {mat_name}")
        return None

    unreal.log(f"  Created Material: {dest_path}/{mat_name}")
    # Note: Connecting texture nodes to material requires MaterialEditingLibrary
    # which needs more complex node graph setup. The material is created as a
    # placeholder — connect textures manually or via a follow-up script.
    return f"{dest_path}/{mat_name}"

# ---------------------------------------------------------------------------
# Import Pipeline
# ---------------------------------------------------------------------------

def run_import():
    total_imported = 0
    materials_created = 0

    with unreal.ScopedSlowTask(200, "Importing Snowpiercer Assets...") as slow_task:
        slow_task.make_dialog(True)

        # ===================================================================
        # 1. MODULAR PIPES — FBX Meshes
        # ===================================================================
        unreal.log("=== Importing Modular Pipe Meshes ===")
        pipes_dir = os.path.join(RIG_DIR, "ModularPipes")
        if os.path.isdir(pipes_dir):
            for f in sorted(os.listdir(pipes_dir)):
                if f.endswith(".fbx"):
                    slow_task.enter_progress_frame(1, f"Importing {f}")
                    result = import_fbx(
                        os.path.join(pipes_dir, f),
                        DEST["meshes_pipes"]
                    )
                    if result:
                        total_imported += 1

        # ===================================================================
        # 2. MODULAR PIPES — PBR Texture Sets
        # ===================================================================
        unreal.log("=== Importing Pipe Texture Sets ===")
        pipe_tex_root = os.path.join(pipes_dir, "Textures")
        pipe_material_sets = {}
        if os.path.isdir(pipe_tex_root):
            for mat_folder in sorted(os.listdir(pipe_tex_root)):
                mat_path = os.path.join(pipe_tex_root, mat_folder)
                if not os.path.isdir(mat_path):
                    continue

                tex_dest = f"{DEST['tex_pipes']}/{mat_folder}"
                tex_set = {}

                for f in sorted(os.listdir(mat_path)):
                    if f.endswith((".png", ".jpg")):
                        slow_task.enter_progress_frame(1, f"Importing {f}")
                        src = os.path.join(mat_path, f)
                        result = import_texture(src, tex_dest)
                        if result:
                            total_imported += 1
                            tex_type = classify_texture(f)
                            set_texture_type(result, tex_type)
                            tex_set[tex_type] = result

                if tex_set:
                    pipe_material_sets[mat_folder] = tex_set

        # Create materials for pipe texture sets
        for mat_name, tex_set in pipe_material_sets.items():
            mat = create_pbr_material(
                f"M_{mat_name}",
                "/Game/Materials/ModularPipes",
                tex_set
            )
            if mat:
                materials_created += 1

        # ===================================================================
        # 3. BAKED TEXTURES (props: tanks, vents, towers, etc.)
        # ===================================================================
        unreal.log("=== Importing Baked Prop Textures ===")
        baked_dir = os.path.join(RIG_DIR, "baked_textures")
        baked_sets = {}  # Group by prop name
        if os.path.isdir(baked_dir):
            for f in sorted(os.listdir(baked_dir)):
                if not f.endswith((".png", ".jpg")):
                    continue
                slow_task.enter_progress_frame(1, f"Importing {f}")

                result = import_texture(
                    os.path.join(baked_dir, f),
                    DEST["tex_baked"]
                )
                if result:
                    total_imported += 1
                    tex_type = classify_texture(f)
                    set_texture_type(result, tex_type)

                    # Group by prop name (everything before _ao/_diff/_norm/_spec)
                    base = f.rsplit("_", 1)[0] if "_" in f else f.split(".")[0]
                    # Normalize: big_tank_diff_edited -> big_tank
                    for suffix in ("_diff_edited", "_diff", "_norm", "_spec", "_ao"):
                        if base.endswith(suffix.rstrip("_")):
                            base = base[:-(len(suffix.rstrip("_")))]
                            break
                    if base not in baked_sets:
                        baked_sets[base] = {}
                    baked_sets[base][tex_type] = result

        # Create materials for baked prop sets
        for prop_name, tex_set in baked_sets.items():
            if len(tex_set) >= 2:  # At least diffuse + one other
                mat = create_pbr_material(
                    f"M_{prop_name}",
                    "/Game/Materials/Props",
                    tex_set
                )
                if mat:
                    materials_created += 1

        # ===================================================================
        # 4. LOOSE METAL TEXTURES (root-level PBR)
        # ===================================================================
        unreal.log("=== Importing Metal Textures ===")
        metal_files = [
            "brushed_metal.jpg", "brushed_metal_2.jpg",
            "brushed_metal_norm.jpg", "brushed_metal_spec.png",
            "concrete.jpg", "concrete_norm.jpg", "concrete_spec.png",
            "concrete_wall.jpg", "concrete_wall_norm.jpg", "concrete_wall_spec.jpg",
            "dirty_metal.jpg", "dirty_metal_norm.jpg", "dirty_metal_spec.png",
            "marble001_color_2k.png", "metal001_color_2k.png",
            "metal1.jpg", "metal1_norm.jpg", "metal1_spec.png",
            "metal2.jpg", "metal2_spec.jpg",
            "metal_ground.jpg", "metal_ground_norm.jpg", "metal_ground_spec.jpg",
            "metal_orange.jpg", "metal_orange_spec.jpg",
            "metal_pipe.jpg", "metal_pipe_2.jpg",
            "metal_pipe_norm.jpg", "metal_pipe_spec.jpg",
            "metal_planks.png", "metal_plate_diff_4k.png",
            "metal_plates.jpg", "metal_plates_norm.jpg", "metal_plates_spec.jpg",
            "metal_stuff.jpg", "metal_stuff_lighter.jpg",
            "metal_stuff_norm.jpg", "metal_stuff_spec.jpg",
            "metals_norm.jpg",
            "red_metal.jpg", "red_metal_norm.jpg", "red_metal_spec.jpg",
            "rusty_metal_04_diff_4k.png",
            "scratchy_metal.jpg", "scratchy_metal_dark.jpg",
            "scratchy_metal_norm.jpg", "scratchy_metal_spec.jpg",
            "wire.png", "wire_norm.jpg", "wire_spec.jpg",
        ]

        for f in metal_files:
            src = os.path.join(RIG_DIR, f)
            if os.path.exists(src):
                slow_task.enter_progress_frame(1, f"Importing {f}")
                result = import_texture(src, DEST["tex_metals"])
                if result:
                    total_imported += 1
                    tex_type = classify_texture(f)
                    set_texture_type(result, tex_type)

        # Environment textures
        for f in ["textures/crepe_satin_diff_4k.png"]:
            src = os.path.join(RIG_DIR, f)
            if os.path.exists(src):
                slow_task.enter_progress_frame(1, f"Importing {f}")
                result = import_texture(src, DEST["tex_environment"])
                if result:
                    total_imported += 1

        # Brass 4K PBR set
        unreal.log("=== Importing Brass 4K PBR Set ===")
        brass_dir = os.path.join(RIG_DIR, "brass-4K")
        brass_textures = {}
        if os.path.isdir(brass_dir):
            for f in sorted(os.listdir(brass_dir)):
                if f.endswith((".png", ".jpg")):
                    slow_task.enter_progress_frame(1, f"Importing {f}")
                    result = import_texture(
                        os.path.join(brass_dir, f),
                        DEST["tex_brass"]
                    )
                    if result:
                        total_imported += 1
                        tex_type = classify_texture(f)
                        set_texture_type(result, tex_type)
                        brass_textures[tex_type] = result

        if brass_textures:
            mat = create_pbr_material("M_Brass_4K", "/Game/Materials/Metals", brass_textures)
            if mat:
                materials_created += 1

        # ===================================================================
        # 5. AUDIO
        # ===================================================================
        unreal.log("=== Importing Audio ===")
        audio_dir = os.path.join(RIG_DIR, "audio")
        if os.path.isdir(audio_dir):
            for f in sorted(os.listdir(audio_dir)):
                if f.endswith((".mp3", ".wav", ".ogg")):
                    slow_task.enter_progress_frame(1, f"Importing {f}")
                    result = import_audio(
                        os.path.join(audio_dir, f),
                        DEST["audio"]
                    )
                    if result:
                        total_imported += 1

        # ===================================================================
        # 6. CONCEPT ART (reference only)
        # ===================================================================
        unreal.log("=== Importing Concept Art ===")
        concept_dir = os.path.join(RIG_DIR, "concept_art")
        if os.path.isdir(concept_dir):
            for f in sorted(os.listdir(concept_dir)):
                if f.endswith((".png", ".jpg")):
                    slow_task.enter_progress_frame(1, f"Importing {f}")
                    result = import_texture(
                        os.path.join(concept_dir, f),
                        DEST["concept_art"]
                    )
                    if result:
                        total_imported += 1

    # ===================================================================
    # Summary
    # ===================================================================
    unreal.log("=" * 60)
    unreal.log(f"IMPORT COMPLETE: {total_imported} assets imported, {materials_created} materials created")
    unreal.log("=" * 60)
    unreal.log("")
    unreal.log("NEXT STEPS:")
    unreal.log("  1. Open each M_* material and connect texture nodes")
    unreal.log("  2. Create a DevTestCar map in Content/Maps/")
    unreal.log("  3. Place pipe meshes to build a train car interior")
    unreal.log("  4. Assign materials to meshes")
    unreal.log("  5. Add a PlayerStart actor")
    unreal.log("  6. Set as Editor Startup Map in Project Settings")

    return total_imported

# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    run_import()
else:
    # When run via UE Python console, __name__ is the module path
    run_import()
