# Cron Job Manager

A Windows GUI application for scheduling and running scripts automatically in the background.

## Features

- **Schedule Scripts**: Run Python, PowerShell, Batch, Node.js, or executable files on a schedule
- **System Tray Integration**: Runs silently in the background
- **Auto-Start**: Optionally start with Windows
- **Persistent**: Only closable via Task Manager (by design)
- **Activity Logging**: Track when jobs run and their results

## Requirements

### For Running
- Windows 10 or later

### For Building
- Qt 5.15+ or Qt 6.x (with Widgets module)
- CMake 3.16+
- Visual Studio 2019 or 2022 (MSVC compiler)
- Inno Setup 6 (optional, for creating installer)

## Building

### Quick Build (PowerShell)

```powershell
# Run the build script
.\build_installer.ps1 -QtDir "C:\Qt\6.6.0\msvc2019_64"
```

### Manual Build

```bash
# Create build directory
mkdir build && cd build

# Configure (adjust Qt path)
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.6.0\msvc2019_64" ..

# Build
cmake --build . --config Release

# Deploy Qt dependencies
windeployqt --release Release\cron-gui.exe
```

## Creating Distribution Packages

### Portable ZIP

After building, run:
```powershell
Compress-Archive -Path "build\Release\*" -DestinationPath "CronJobManager_Portable_1.0.0.zip"
```

### Installer (.exe)

1. Install [Inno Setup 6](https://jrsoftware.org/isinfo.php)
2. Build the project first (see above)
3. Run the Inno Setup compiler:
   ```
   "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer.iss
   ```

Or use the build scripts which do everything automatically:
- `build_installer.bat` - Batch script
- `build_installer.ps1` - PowerShell script

## Usage

1. **Add Jobs**: Click "Add Job" to create a new scheduled task
2. **Configure**: 
   - Browse for your script file
   - Set the run interval (minutes, hours, or days)
   - Add optional command-line arguments
3. **Run**: Jobs execute automatically based on their schedule
4. **Monitor**: Check the Activity Log for execution results

### Supported Script Types

| Extension | Handler |
|-----------|---------|
| `.py` | Python |
| `.ps1` | PowerShell |
| `.bat`, `.cmd` | Windows Command |
| `.js` | Node.js |
| `.exe` | Direct execution |

## File Locations

- **Jobs Configuration**: `%AppData%\CronGUI\cronjobs.json`
- **Auto-Start Registry**: `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`

## License

MIT License
