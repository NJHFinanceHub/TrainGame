# TrainGame

## GitHub sync (local)

1. Add your GitHub remote (replace `REPLACE_ME` with your user/org):

   ```bash
   git remote add origin https://github.com/REPLACE_ME/TrainGame.git
   ```

2. Verify the remote:

   ```bash
   git remote -v
   ```

3. Pull latest changes for the current branch (`work` in this repo):

   ```bash
   git pull --ff-only origin work
   ```

## Launching the demo locally (UE 5.5)

This repository includes an Unreal project at:

- `unreal/SnowpiercerEE/SnowpiercerEE.uproject`

### Windows

```powershell
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe" "<repo>\unreal\SnowpiercerEE\SnowpiercerEE.uproject"
```

### macOS

```bash
/Users/Shared/Epic\ Games/UE_5.5/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor "<repo>/unreal/SnowpiercerEE/SnowpiercerEE.uproject"
```

### Linux (source or installed build)

```bash
/path/to/UE_5.5/Engine/Binaries/Linux/UnrealEditor "<repo>/unreal/SnowpiercerEE/SnowpiercerEE.uproject"
```

If the project asks to rebuild modules, accept and wait for compilation to finish before pressing Play.
