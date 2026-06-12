"""Import the soundtrack WAVs sitting in Content/Audio/Music as SoundWave
assets at /Game/Audio/Music so SEEMusicSubsystem can play them."""
import os
import unreal

editor_util = unreal.EditorAssetLibrary
PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MUSIC_DIR = os.path.join(PROJECT_DIR, "Content", "Audio", "Music")

imported = 0
for fname in ("MUS_Zone1_Explore.wav", "MUS_Zone1_Combat.wav"):
    name = os.path.splitext(fname)[0]
    dest = f"/Game/Audio/Music/{name}"
    if editor_util.does_asset_exist(dest):
        unreal.log(f"  Already imported: {name}")
        continue
    src = os.path.join(MUSIC_DIR, fname)
    if not os.path.isfile(src):
        unreal.log_warning(f"  Missing source wav: {src}")
        continue
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", src)
    task.set_editor_property("destination_path", "/Game/Audio/Music")
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    if editor_util.does_asset_exist(dest):
        # Loop them at the asset level so the subsystem's bLooping force is moot
        try:
            wave = unreal.load_asset(dest)
            wave.set_editor_property("looping", True)
            editor_util.save_loaded_asset(wave)
        except Exception as e:
            unreal.log_warning(f"  Could not set looping on {name}: {e}")
        imported += 1
        unreal.log(f"  Imported: {dest}")
    else:
        unreal.log_warning(f"  FAILED import: {fname}")

unreal.log(f"MUSIC IMPORT DONE -- {imported} new")
