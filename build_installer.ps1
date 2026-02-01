# Cron Job Manager - Build and Package Script (PowerShell)
# Run this script to build the application and create installer/ZIP

param(
    [string]$QtDir = "",
    [string]$BuildType = "Release"
)

Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  Cron Job Manager - Build and Package" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# Try to find Qt installation
$QtSearchPaths = @(
    "C:\Qt\6.7.0\msvc2019_64",
    "C:\Qt\6.6.0\msvc2019_64",
    "C:\Qt\6.5.0\msvc2019_64",
    "C:\Qt\5.15.2\msvc2019_64",
    "$env:USERPROFILE\Qt\6.6.0\msvc2019_64"
)

if (-not $QtDir) {
    foreach ($path in $QtSearchPaths) {
        if (Test-Path "$path\bin\qmake.exe") {
            $QtDir = $path
            Write-Host "[INFO] Found Qt at: $QtDir" -ForegroundColor Green
            break
        }
    }
}

if (-not $QtDir -or -not (Test-Path "$QtDir\bin\qmake.exe")) {
    Write-Host "[WARNING] Qt installation not found!" -ForegroundColor Yellow
    Write-Host "Please specify Qt path using: .\build_installer.ps1 -QtDir 'C:\Qt\6.x.x\msvc2019_64'" -ForegroundColor Yellow
    Write-Host ""
}

$ProjectRoot = $PSScriptRoot
Set-Location $ProjectRoot

# Step 1: Clean and create build directory
Write-Host "[1/6] Preparing build directory..." -ForegroundColor Yellow
if (Test-Path "build") {
    Remove-Item -Recurse -Force "build"
}
New-Item -ItemType Directory -Path "build" | Out-Null
Set-Location "build"

# Step 2: Configure with CMake
Write-Host ""
Write-Host "[2/6] Configuring with CMake..." -ForegroundColor Yellow
$cmakeArgs = @("-G", "Visual Studio 17 2022", "-A", "x64")
if ($QtDir) {
    $cmakeArgs += "-DCMAKE_PREFIX_PATH=$QtDir"
}
$cmakeArgs += ".."

& cmake $cmakeArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] CMake configuration failed!" -ForegroundColor Red
    exit 1
}

# Step 3: Build
Write-Host ""
Write-Host "[3/6] Building $BuildType configuration..." -ForegroundColor Yellow
& cmake --build . --config $BuildType
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Build failed!" -ForegroundColor Red
    exit 1
}

# Step 4: Deploy Qt dependencies
Write-Host ""
Write-Host "[4/6] Deploying Qt dependencies..." -ForegroundColor Yellow
$windeployqt = "$QtDir\bin\windeployqt.exe"
if (Test-Path $windeployqt) {
    & $windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw "$BuildType\cron-gui.exe"
} else {
    Write-Host "[WARNING] windeployqt not found. Manual DLL copying may be required." -ForegroundColor Yellow
}

Set-Location $ProjectRoot

# Step 5: Create ZIP file
Write-Host ""
Write-Host "[5/6] Creating portable ZIP..." -ForegroundColor Yellow
$zipPath = "CronJobManager_Portable_1.0.0.zip"
if (Test-Path $zipPath) {
    Remove-Item $zipPath
}
Compress-Archive -Path "build\$BuildType\*" -DestinationPath $zipPath -Force
if (Test-Path $zipPath) {
    Write-Host "[SUCCESS] Created: $zipPath" -ForegroundColor Green
}

# Step 6: Create installer with Inno Setup
Write-Host ""
Write-Host "[6/6] Creating installer..." -ForegroundColor Yellow
$innoSetup = "C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
if (Test-Path $innoSetup) {
    & $innoSetup "installer.iss"
    if (Test-Path "CronJobManager_Setup_1.0.0.exe") {
        Write-Host "[SUCCESS] Created: CronJobManager_Setup_1.0.0.exe" -ForegroundColor Green
    }
} else {
    Write-Host "[INFO] Inno Setup not installed." -ForegroundColor Yellow
    Write-Host "Download from: https://jrsoftware.org/isinfo.php" -ForegroundColor Yellow
    Write-Host "Then run: & '$innoSetup' installer.iss" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  Build Complete!" -ForegroundColor Cyan  
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Output files in root folder:" -ForegroundColor White
Write-Host "  - CronJobManager_Portable_1.0.0.zip" -ForegroundColor Gray
Write-Host "  - CronJobManager_Setup_1.0.0.exe (if Inno Setup installed)" -ForegroundColor Gray
Write-Host ""
