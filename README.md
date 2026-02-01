# Cron Job Manager

A Windows application that runs your scripts automatically on a schedule, even when you're not looking. Set it and forget it.

## Installation

### Option 1: Installer (Recommended)
1. Download `CronJobManager_Setup_1.0.0.exe`
2. Run the installer
3. Choose "Start with Windows" if you want jobs to run automatically after restart
4. Done! The app will start and appear in your system tray

### Option 2: Portable
1. Download `CronJobManager_Portable_1.0.0.zip`
2. Extract to any folder
3. Run `cron-gui.exe`

## How to Use

### Adding a Scheduled Job

1. **Open the app** - Double-click the clock icon in your system tray
2. **Click "Add Job"**
3. **Fill in the details:**
   - **Name**: Give your job a descriptive name (e.g., "Backup Database")
   - **Script**: Click "Browse" and select your script file
   - **Arguments**: Add any command-line arguments your script needs (optional)
   - **Interval**: Set how often to run (e.g., every 1 hour, every 30 minutes, every 2 days)
   - **Enabled**: Check this to activate the job
4. **Click "Save"**

Your job will now run automatically at the scheduled interval.

### Managing Jobs

| Action | How To |
|--------|--------|
| **Edit a job** | Select the job in the list and click "Edit", or double-click it |
| **Delete a job** | Select the job and click "Remove" |
| **Run immediately** | Select the job and click "Run Now" |
| **Enable/Disable** | Edit the job and toggle the "Enabled" checkbox |

### Monitoring

- **Activity Log**: Shows when jobs run and whether they succeeded or failed
- **Next Run column**: Shows when each job will run next
- **Last Run column**: Shows when each job last executed
- **Tray notifications**: Pop-up alerts when jobs complete

## Supported Script Types

### Auto-Detected (No Configuration Needed)

| File Type | Extension | How It Runs |
|-----------|-----------|-------------|
| Python | `.py` | Runs with `python` |
| PowerShell | `.ps1` | Runs with `powershell -ExecutionPolicy Bypass` |
| Batch | `.bat`, `.cmd` | Runs with `cmd /c` |
| Node.js | `.js` | Runs with `node` |
| Ruby | `.rb` | Runs with `ruby` |
| Perl | `.pl` | Runs with `perl` |
| PHP | `.php` | Runs with `php` |
| Bash | `.sh` | Runs with `bash` |
| Executable | `.exe` | Runs directly |

### Custom Commands (Any Script Type)

For scripts not listed above, or when you need a specific runtime:

1. Check **"Use custom command/runtime"**
2. Enter the command or path to the interpreter:
   - Just the name if it's in your PATH: `ruby`, `perl`, `php`, `lua`
   - Full path for portable runtimes: `C:\Tools\lua\lua.exe`

**Examples:**

| Script Type | Custom Command | Script Path |
|-------------|----------------|-------------|
| Lua | `lua` | `C:\scripts\myscript.lua` |
| R | `Rscript` | `C:\scripts\analysis.R` |
| Go (run) | `go run` | `C:\scripts\main.go` |
| Java | `java -jar` | `C:\scripts\app.jar` |
| Custom Python | `C:\Python39\python.exe` | `C:\scripts\legacy.py` |

This allows you to run **any script type** as long as you have the runtime installed on your system.

## Background Operation

This app is designed to run continuously in the background:

- **Minimize to tray**: Closing the window doesn't stop the app - it minimizes to your system tray
- **Jobs keep running**: Your scheduled jobs continue to execute even when the window is closed
- **System tray icon**: Look for the blue clock icon near your system clock
- **To open**: Double-click the tray icon, or right-click and select "Show Window"
- **To exit completely**: Use Task Manager (Ctrl+Shift+Esc) to end the process

## Auto-Start with Windows

To have your jobs run automatically when your computer starts:

1. Check the **"Start with Windows"** checkbox in the main window, OR
2. Enable it during installation

The app will start hidden in your system tray and begin running jobs immediately.

## Where Data is Stored

| Data | Location |
|------|----------|
| Job configurations | `%AppData%\CronGUI\cronjobs.json` |
| Auto-start setting | Windows Registry |

## Tips

- **Test your scripts first**: Make sure your script works when run manually before scheduling it
- **Use absolute paths**: In your scripts, use full paths to files (e.g., `C:\Data\file.txt` not just `file.txt`)
- **Check the log**: If a job isn't working, check the Activity Log for error messages
- **Shorter intervals = more checks**: The app checks for due jobs every 30 seconds

## Requirements

- Windows 10 or later
- For Python scripts: Python must be installed and in your PATH
- For Node.js scripts: Node.js must be installed and in your PATH

## License

MIT License
