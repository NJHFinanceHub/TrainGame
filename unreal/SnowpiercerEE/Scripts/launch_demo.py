"""
Snowpiercer: Eternal Engine — One-Click Investor Demo
Run from UE5 Editor: Tools > Execute Python Script > browse to this file

This single script handles EVERYTHING needed for the full Zone 1 demo:
  1. Imports all raw assets (textures, meshes, audio) if not already imported
  2. Creates DataTables (items, dialogue, quests, collectibles)
  3. Sets up game content (character, materials, pickups, NPCs, audio, atmosphere)
  4. Builds Zone 1: The Tail (15 unique cars with full interiors)
  5. Sets Zone1_Tail as the startup map

No need to run any other scripts. One click, full demo.
"""

import unreal
import os
import sys

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, SCRIPT_DIR)

editor_util = unreal.EditorAssetLibrary

# ---------------------------------------------------------------------------
# Checks — skip steps that are already done
# ---------------------------------------------------------------------------

def assets_imported():
    """Check if key raw assets already exist in Content."""
    checks = [
        "/Game/Meshes/ModularPipes/PipeKit_Pipe_Long",
        "/Game/Textures/Metals/brushed_metal",
        "/Game/Audio/train_interior",
    ]
    return all(editor_util.does_asset_exist(p) for p in checks)

def datatables_exist():
    """Check if DataTables have been created."""
    checks = [
        "/Game/DataTables/DT_Items",
        "/Game/DataTables/DT_Dialogue_Zone1",
        "/Game/DataTables/DT_Quests",
        "/Game/DataTables/DT_Collectibles",
    ]
    return all(editor_util.does_asset_exist(p) for p in checks)

def game_content_exists():
    """Check if game content (character BP, materials, pickups) exists."""
    checks = [
        "/Game/Blueprints/Characters/BP_SEECharacter",
        "/Game/Materials/ModularPipes/M_BareMetal",
        "/Game/Blueprints/Pickups/BP_Pickup_ProteinBlock",
    ]
    return all(editor_util.does_asset_exist(p) for p in checks)

def zone1_exists():
    """Check if Zone 1 map exists."""
    return editor_util.does_asset_exist("/Game/Maps/Zone1_Tail")

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

TOTAL_STEPS = 5

def run():
    unreal.log("")
    unreal.log("*" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  One-Click Investor Demo Setup")
    unreal.log("*" * 64)
    unreal.log("")

    # Step 1: Import raw assets
    if not assets_imported():
        unreal.log(f">>> STEP 1/{TOTAL_STEPS}: Importing raw assets...")
        import import_assets
        import_assets.run_import()
    else:
        unreal.log(f">>> STEP 1/{TOTAL_STEPS}: Raw assets already imported — skipping")

    # Step 2: Create DataTables
    if not datatables_exist():
        unreal.log("")
        unreal.log(f">>> STEP 2/{TOTAL_STEPS}: Creating DataTables (items, dialogue, quests, collectibles)...")
        try:
            import create_datatables
            create_datatables.main()
        except Exception as e:
            unreal.log_warning(f"  DataTable creation had issues: {e}")
            unreal.log_warning("  JSON fallback files written to DataTables/ — import manually if needed")
    else:
        unreal.log(f">>> STEP 2/{TOTAL_STEPS}: DataTables already exist — skipping")

    # Step 3: Set up game content (character, materials, pickups, NPCs, audio, atmosphere)
    if not game_content_exists():
        unreal.log("")
        unreal.log(f">>> STEP 3/{TOTAL_STEPS}: Setting up game content (character, materials, pickups, NPCs, audio)...")
        try:
            import setup_game_content
            setup_game_content.run()
        except Exception as e:
            unreal.log_warning(f"  Game content setup had issues: {e}")
            unreal.log_warning("  Some Blueprints may need manual creation — check logs above")
    else:
        unreal.log(f">>> STEP 3/{TOTAL_STEPS}: Game content already exists — skipping")

    # Step 4: Build Zone 1 (15 cars)
    if not zone1_exists():
        unreal.log("")
        unreal.log(f">>> STEP 4/{TOTAL_STEPS}: Building Zone 1: The Tail (15 unique cars)...")
        try:
            import build_zone1
            # build_zone1 auto-runs on import via run() call at module level
        except Exception as e:
            unreal.log_warning(f"  Zone 1 build had issues: {e}")
            unreal.log_warning("  The level may be partially built — check the editor")
    else:
        unreal.log(f">>> STEP 4/{TOTAL_STEPS}: Zone 1 map already exists — skipping")

    # Step 5: Verify everything
    unreal.log("")
    unreal.log(f">>> STEP 5/{TOTAL_STEPS}: Verifying demo setup...")

    checks = {
        "Zone1_Tail map":       "/Game/Maps/Zone1_Tail",
        "Player character BP":  "/Game/Blueprints/Characters/BP_SEECharacter",
        "Item DataTable":       "/Game/DataTables/DT_Items",
        "Dialogue DataTable":   "/Game/DataTables/DT_Dialogue_Zone1",
        "Quest DataTable":      "/Game/DataTables/DT_Quests",
        "Pickup BP (food)":     "/Game/Blueprints/Pickups/BP_Pickup_ProteinBlock",
        "NPC BP (civilian)":    "/Game/Blueprints/NPCs/BP_NPC_Civilian",
        "NPC BP (jackboot)":    "/Game/Blueprints/NPCs/BP_NPC_Jackboot",
        "Tail post-process":    "/Game/Blueprints/PostProcess/BP_PP_TailZone",
        "Metal material":       "/Game/Materials/ModularPipes/M_BareMetal",
    }

    all_ok = True
    for label, path in checks.items():
        if editor_util.does_asset_exist(path):
            unreal.log(f"  [OK] {label}")
        else:
            unreal.log_warning(f"  [!!] {label} — MISSING")
            all_ok = False

    # Summary
    unreal.log("")
    unreal.log("=" * 64)
    if all_ok:
        unreal.log("  INVESTOR DEMO READY — Press Play (Alt+P)")
    else:
        unreal.log("  DEMO PARTIALLY READY — See warnings above")
    unreal.log("=" * 64)
    unreal.log("")
    unreal.log("Demo Content:")
    unreal.log("  15 unique train cars (Zone 1: The Tail)")
    unreal.log("  25 inventory items (food, weapons, medicine, crafting)")
    unreal.log("  34 dialogue nodes across 5 NPCs")
    unreal.log("  6 quests (1 main + 5 side)")
    unreal.log("  20 collectibles with lore entries")
    unreal.log("  14 pickup blueprints, 5 NPC archetypes")
    unreal.log("  PBR materials, post-process presets, audio")
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
    unreal.log("")
    unreal.log("Zone 1 Car Roster:")
    unreal.log("  [00] Caboose           — Tutorial, memorial, player start")
    unreal.log("  [01] Tail Quarters A   — NPC introductions, bunks")
    unreal.log("  [02] Tail Quarters B   — Communal area, living quarters")
    unreal.log("  [03] The Pit           — Combat tutorial arena")
    unreal.log("  [04] Nursery           — First moral choice")
    unreal.log("  [05] Elders' Car       — Quest hub, companion recruitment")
    unreal.log("  [06] Sickbay           — Medicine introduction")
    unreal.log("  [07] Workshop          — Crafting tutorial")
    unreal.log("  [08] Listening Post    — Intel gathering")
    unreal.log("  [09] Blockade          — First real combat (8 enemies)")
    unreal.log("  [10] Dark Car          — Horror/stealth section")
    unreal.log("  [11] Freezer Breach    — Cold damage timer")
    unreal.log("  [12] Kronole Den       — Drug system introduction")
    unreal.log("  [13] Smuggler's Cache  — Trading system")
    unreal.log("  [14] Martyr's Gate     — BOSS: Commander Grey")
    unreal.log("")

if __name__ == "__main__":
    run()
else:
    run()
