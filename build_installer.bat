@echo off
setlocal EnableDelayedExpansion

echo ============================================
echo   Cron Job Manager - Build and Package
echo ============================================
echo.

:: Configuration - Update these paths for your system
set QT_DIR=C:\Qt\6.6.0\msvc2019_64
set CMAKE_GENERATOR=Visual Studio 17 2022
set INNO_SETUP=C:\Program Files (x86)\Inno Setup 6\ISCC.exe

:: Check for Qt
if not exist "%QT_DIR%\bin\qmake.exe" (
    echo [WARNING] Qt not found at %QT_DIR%
    echo Please update QT_DIR in this script to match your Qt installation.
    echo.
    echo Common locations:
    echo   C:\Qt\6.x.x\msvc2019_64
    echo   C:\Qt\5.15.x\msvc2019_64
    echo.
)

:: Step 1: Create build directory
echo [1/5] Creating build directory...
if exist build rmdir /s /q build
mkdir build
cd build

:: Step 2: Configure with CMake
echo.
echo [2/5] Configuring with CMake...
cmake -G "%CMAKE_GENERATOR%" -A x64 -DCMAKE_PREFIX_PATH="%QT_DIR%" ..
if errorlevel 1 (
    echo [ERROR] CMake configuration failed!
    pause
    exit /b 1
)

:: Step 3: Build Release
echo.
echo [3/5] Building Release configuration...
cmake --build . --config Release
if errorlevel 1 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

:: Step 4: Deploy Qt dependencies
echo.
echo [4/5] Deploying Qt dependencies...
if exist "%QT_DIR%\bin\windeployqt.exe" (
    "%QT_DIR%\bin\windeployqt.exe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw Release\cron-gui.exe
) else (
    echo [WARNING] windeployqt not found. You may need to manually copy Qt DLLs.
)

cd ..

:: Step 5: Create ZIP file
echo.
echo [5/5] Creating distribution packages...

:: Create portable ZIP
echo Creating portable ZIP...
if exist "CronJobManager_Portable_1.0.0.zip" del "CronJobManager_Portable_1.0.0.zip"

:: Use PowerShell to create ZIP
powershell -Command "Compress-Archive -Path 'build\Release\*' -DestinationPath 'CronJobManager_Portable_1.0.0.zip' -Force"

if exist "CronJobManager_Portable_1.0.0.zip" (
    echo [SUCCESS] Created: CronJobManager_Portable_1.0.0.zip
) else (
    echo [WARNING] Failed to create ZIP file
)

:: Create installer with Inno Setup
if exist "%INNO_SETUP%" (
    echo.
    echo Creating installer with Inno Setup...
    "%INNO_SETUP%" installer.iss
    if exist "CronJobManager_Setup_1.0.0.exe" (
        echo [SUCCESS] Created: CronJobManager_Setup_1.0.0.exe
    )
) else (
    echo.
    echo [INFO] Inno Setup not found at: %INNO_SETUP%
    echo To create the installer:
    echo   1. Download Inno Setup from https://jrsoftware.org/isinfo.php
    echo   2. Install it to the default location
    echo   3. Run this script again, or compile installer.iss manually
)

echo.
echo ============================================
echo   Build Complete!
echo ============================================
echo.
echo Output files (in root folder):
echo   - CronJobManager_Portable_1.0.0.zip (portable version)
echo   - CronJobManager_Setup_1.0.0.exe (installer, if Inno Setup available)
echo.
echo Build folder:
echo   - build\Release\cron-gui.exe (main executable)
echo.
pause
