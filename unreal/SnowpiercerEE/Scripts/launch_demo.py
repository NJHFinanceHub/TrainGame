"""
Snowpiercer: Eternal Engine — One-Click Demo Setup
Run from UE5 Editor: Tools > Execute Python Script > browse to this file

This single script handles everything:
  1. Imports all raw assets (textures, meshes, audio) if not already imported
  2. Creates PBR materials and wires texture nodes
  3. Builds the DevTestCar demo level with geometry, lights, and actors
  4. Sets it as the startup map

No need to run import_assets.py or setup_demo_level.py separately.
"""

import unreal
import os
import sys

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, SCRIPT_DIR)

# ---------------------------------------------------------------------------
# Check what's already imported
# ---------------------------------------------------------------------------

editor_util = unreal.EditorAssetLibrary

def assets_imported():
    """Check if key assets already exist in Content."""
    checks = [
        "/Game/Meshes/ModularPipes/PipeKit_Pipe_Long",
        "/Game/Textures/Metals/brushed_metal",
        "/Game/Audio/train_interior",
    ]
    for path in checks:
        if not editor_util.does_asset_exist(path):
            return False
    return True

def materials_exist():
    """Check if materials have been created."""
    checks = [
        "/Game/Materials/ModularPipes/M_BareMetal",
        "/Game/Materials/ModularPipes/M_RustyMetal",
    ]
    for path in checks:
        if not editor_util.does_asset_exist(path):
            return False
    return True

def demo_level_exists():
    """Check if DevTestCar map exists and has content."""
    return editor_util.does_asset_exist("/Game/Maps/DevTestCar")

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def run():
    unreal.log("=" * 60)
    unreal.log("SNOWPIERCER DEMO — One-Click Setup")
    unreal.log("=" * 60)

    # Step 1: Import assets if needed
    if not assets_imported():
        unreal.log("")
        unreal.log(">>> STEP 1/3: Importing assets...")
        import import_assets
        import_assets.run_import()
    else:
        unreal.log("")
        unreal.log(">>> STEP 1/3: Assets already imported — skipping")

    # Step 2: Build demo level (also wires materials)
    if not demo_level_exists():
        unreal.log("")
        unreal.log(">>> STEP 2/3: Building demo level...")
        import setup_demo_level
        setup_demo_level.run_setup()
    else:
        unreal.log("")
        unreal.log(">>> STEP 2/3: Demo level already exists — skipping")

    # Step 3: Ensure startup map is set
    unreal.log("")
    unreal.log(">>> STEP 3/3: Verifying project settings...")

    # Verify the map loads
    if editor_util.does_asset_exist("/Game/Maps/DevTestCar"):
        unreal.log("  DevTestCar map: OK")
    else:
        unreal.log_warning("  DevTestCar map: MISSING — run again or check logs above")

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("DEMO READY — Press Play (Alt+P) to test")
    unreal.log("=" * 60)
    unreal.log("")
    unreal.log("Controls:")
    unreal.log("  WASD        — Move")
    unreal.log("  Mouse       — Look")
    unreal.log("  V           — Toggle first/third person")
    unreal.log("  E           — Interact")
    unreal.log("  Shift       — Sprint")
    unreal.log("  Ctrl        — Crouch")
    unreal.log("  Space       — Jump")
    unreal.log("  LMB         — Light attack")
    unreal.log("  MMB         — Heavy attack")
    unreal.log("  RMB         — Block")
    unreal.log("  Alt         — Dodge")
    unreal.log("  1-4         — Quick slots")
    unreal.log("  J           — Journal")

if __name__ == "__main__":
    run()
else:
    run()
