; Inno Setup Script for Cron Job Manager
; Requires Inno Setup 6.x - Download from https://jrsoftware.org/isinfo.php

#define MyAppName "Cron Job Manager"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "CronGUI"
#define MyAppURL "https://github.com/cron-gui"
#define MyAppExeName "cron-gui.exe"

[Setup]
; Application information
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}

; Installation settings
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=
InfoBeforeFile=installer_welcome.txt
OutputDir=.
OutputBaseFilename=CronJobManager_Setup_{#MyAppVersion}
SetupIconFile=icons\app_icon.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern

; Windows version requirements
MinVersion=10.0

; Privileges
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog

; Visual settings
WizardImageFile=
WizardSmallImageFile=
DisableWelcomePage=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "autostart"; Description: "Start automatically with Windows"; GroupDescription: "Startup Options:"

[Files]
; Main application
Source: "build\cron-gui.exe"; DestDir: "{app}"; Flags: ignoreversion

; Qt DLLs and plugins
Source: "build\*.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "build\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "build\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "build\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "build\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "build\generic\*"; DestDir: "{app}\generic"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "build\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "build\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
; Auto-start registry entry (if selected during install)
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "CronGUI"; ValueData: """{app}\{#MyAppExeName}"" --hidden"; Flags: uninsdeletevalue; Tasks: autostart

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallRun]
; Kill the application before uninstalling
Filename: "taskkill"; Parameters: "/F /IM cron-gui.exe"; Flags: runhidden; RunOnceId: "KillApp"

[UninstallDelete]
; Clean up app data
Type: filesandordirs; Name: "{localappdata}\CronGUI"

[Code]
// Custom welcome page content
procedure InitializeWizard;
begin
  // The welcome page is automatically shown due to DisableWelcomePage=no
  // Additional customization can be done here
end;

function InitializeSetup(): Boolean;
begin
  Result := True;
end;
