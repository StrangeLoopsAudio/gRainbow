#define Version Trim(FileRead(FileOpen("..\VERSION")))
#define PluginName "gRainbow"
#define Publisher "StrangeLoops"
#define Year GetDateTimeString("yyyy","","")

; 'Types': What get displayed during the setup
[Types]
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

; Components are used inside the script and can be composed of a set of 'Types'
[Components]
Name: "standalone"; Description: "Standalone application"; Types: full custom
Name: "vst3"; Description: "VST3 plugin"; Types: full custom

[Setup]
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
AppName={#PluginName}
OutputBaseFilename={#PluginName}-{#Version}-Windows
AppCopyright=Copyright (C) {#Year} {#Publisher}
AppPublisher={#Publisher}
AppVersion={#Version}
DefaultDirName="{commoncf64}\VST3\{#PluginName}.vst3"
DisableDirPage=yes
LicenseFile="..\LICENSE"
UninstallFilesDir="{commonappdata}\{#PluginName}\uninstall"

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#PluginName}Data"

; MSVC adds a .ilk when building the plugin. Let's not include that.
[Files]
Source: "..\Builds\gRainbow_artefacts\Release\VST3\{#PluginName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#PluginName}.vst3\"; Excludes: *.ilk; Flags: ignoreversion recursesubdirs; Components: vst3
Source: "..\Builds\gRainbow_artefacts\Release\Standalone\{#PluginName}.exe"; DestDir: "{commonpf64}\{#Publisher}\{#PluginName}"; Flags: ignoreversion; Components: standalone

[Run]
Filename: "{cmd}"; \
    WorkingDir: "{commoncf64}\VST3"; \
    Parameters: "/C mklink /D ""{commoncf64}\VST3\{#PluginName}Data"" ""{commonappdata}\{#PluginName}"""; \
    Flags: runascurrentuser; Components: vst3
