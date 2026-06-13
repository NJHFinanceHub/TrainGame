"""Import the combat SFX WAVs in Content/Audio/Combat as SoundWave assets at
/Game/Audio/Combat so SEECombatComponent can play hit/swing sounds."""
import os
import unreal

editor_util = unreal.EditorAssetLibrary
PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
COMBAT_DIR = os.path.join(PROJECT_DIR, "Content", "Audio", "Combat")

imported = 0
for fname in ("SFX_MeleeHit_01.wav", "SFX_MeleeSwing_01.wav", "SFX_PlayerDamage.wav"):
    name = os.path.splitext(fname)[0]
    dest = f"/Game/Audio/Combat/{name}"
    if editor_util.does_asset_exist(dest):
        unreal.log(f"  Already imported: {name}")
        continue
    src = os.path.join(COMBAT_DIR, fname)
    if not os.path.isfile(src):
        unreal.log_warning(f"  Missing source wav: {src}")
        continue
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", src)
    task.set_editor_property("destination_path", "/Game/Audio/Combat")
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    if editor_util.does_asset_exist(dest):
        imported += 1
        unreal.log(f"  Imported: {dest}")
    else:
        unreal.log_warning(f"  FAILED import: {fname}")

unreal.log(f"COMBAT SFX IMPORT DONE -- {imported} new")
