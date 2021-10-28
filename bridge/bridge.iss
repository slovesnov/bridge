; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!
#define GTKPATH "C:\soft\msys64\mingw64\"
#define SITE "http://slovesnov.users.sf.net/"
#define APPNAME "bridge"
#define VERSION "5.2"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{E7506159-838C-471E-A941-ECAFF1EB6EC0}
AppName={#APPNAME}
AppVersion={#VERSION}
AppPublisher=Alexey Slovesnov
AppPublisherURL={#SITE}?{#APPNAME}
AppSupportURL={#SITE}?{#APPNAME}
AppUpdatesURL={#SITE}?{#APPNAME}
DefaultDirName={commonpf}\{#APPNAME}
DefaultGroupName={#APPNAME}
OutputBaseFilename={#APPNAME}{#VERSION}Setup
SetupIconFile={#APPNAME}\icon\app.ico
Compression=lzma
SolidCompression=yes
ChangesAssociations=yes

[REGISTRY]
Root: HKCR; Subkey: ".bts"; ValueType: string; ValueName: ""; ValueData: "BridgeStudio"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".pts"; ValueType: string; ValueName: ""; ValueData: "BridgeStudio"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".pbn"; ValueType: string; ValueName: ""; ValueData: "BridgeStudio"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "BridgeStudio"; ValueType: string; ValueName: ""; ValueData: "BridgeStudio"; Flags: uninsdeletekey
Root: HKCR; Subkey: "BridgeStudio\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\bridge.exe,0"
Root: HKCR; Subkey: "BridgeStudio\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\bridge.exe"" ""%1"""


[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"

[Messages]
english.BeveledLabel=English
russian.BeveledLabel=Russian
italian.BeveledLabel=Italian

[InstallDelete]

[Files]
;pan-up-symbolic.symbolic.png, pan-down-symbolic.symbolic.png Arrow up/down. It's combobox arrow
;pan-end-symbolic.symbolic.png right arrow for menu
;list-add-symbolic.symbolic.png list-remove-symbolic.symbolic.png +/- for "font selection"
;edit-find-symbolic.symbolic.png "search image" for "font selection"
;for select font option {list-add-symbolic.symbolic.png list-remove-symbolic.symbolic.png edit-find-symbolic.symbolic.png
;spinner process-working-symbolic.svg
;BEGIN file open
;glib-2.0\schemas\gschemas.compiled - otherwise hangs
;Adwaita\16x16\ different folder
;Adwaita\index.theme
;END file open 
;Source: "{#GTKPATH}share\icons\Adwaita\16x16\ui\pan-up-symbolic.symbolic.png"; DestDir: "{app}\share\icons\Adwaita\16x16\actions"; Flags: onlyifdoesntexist
;Source: "{#GTKPATH}share\icons\Adwaita\16x16\ui\pan-down-symbolic.symbolic.png"; DestDir: "{app}\share\icons\Adwaita\16x16\actions"; Flags: onlyifdoesntexist
;Source: "{#GTKPATH}share\icons\Adwaita\16x16\ui\pan-end-symbolic.symbolic.png"; DestDir: "{app}\share\icons\Adwaita\16x16\actions"; Flags: onlyifdoesntexist
;Source: "{#GTKPATH}share\icons\Adwaita\16x16\actions\list-add-symbolic.symbolic.png"; DestDir: "{app}\share\icons\Adwaita\16x16\actions"; Flags: onlyifdoesntexist
;Source: "{#GTKPATH}share\icons\Adwaita\16x16\actions\list-remove-symbolic.symbolic.png"; DestDir: "{app}\share\icons\Adwaita\16x16\actions"; Flags: onlyifdoesntexist
;Source: "{#GTKPATH}share\icons\Adwaita\16x16\actions\edit-find-symbolic.symbolic.png"; DestDir: "{app}\share\icons\Adwaita\16x16\actions"; Flags: onlyifdoesntexist
Source: "{#GTKPATH}share\icons\Adwaita\16x16\*"; DestDir: "{app}\share\icons\Adwaita\16x16"; Flags: onlyifdoesntexist recursesubdirs
Source: "{#GTKPATH}share\icons\Adwaita\index.theme"; DestDir: "{app}\share\icons\Adwaita"; Flags: ignoreversion
Source: "{#GTKPATH}share\icons\Adwaita\scalable-up-to-32\status\process-working-symbolic.svg"; DestDir: "{app}\share\icons\Adwaita\scalable-up-to-32\status"; Flags: onlyifdoesntexist
Source: "{#GTKPATH}share\glib-2.0\schemas\gschemas.compiled"; DestDir: "{app}\share\glib-2.0\schemas"; Flags: ignoreversion
;c:\soft\msys64\mingw64\share\glib-2.0\schemas\gschemas.compiled

;new gtk version so ignoreversion for dlls
Source: "{#GTKPATH}bin\*.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
;gspawn-win64-helper.exe needs for gtk_show_uri_on_window() function
Source: "{#GTKPATH}bin\gspawn-win64-helper.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#GTKPATH}lib\gdk-pixbuf-2.0\2.10.0\loaders.cache"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0"; Flags: ignoreversion
Source: "{#GTKPATH}lib\gdk-pixbuf-2.0\2.10.0\loaders\*.dll"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion

Source: "Release\{#APPNAME}.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
//TODO app.css
Source: "*.css"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#APPNAME}\images\*"; Excludes: "*.txt"; DestDir: "{app}\bin\{#APPNAME}\images"; Flags: onlyifdoesntexist
Source: "{#APPNAME}\lng\*"; DestDir: "{app}\bin\{#APPNAME}\lng"; Flags: ignoreversion
Source: "{#APPNAME}\problems\*";  DestDir: "{app}\bin\problems"; Flags: onlyifdoesntexist
Source: "{#APPNAME}\problems\bts\*";  DestDir: "{app}\bin\problems\bts"; Flags: onlyifdoesntexist
; NOTE: Don't use "Flags: ignoreversion" on any shared system files


[Icons]
Name: "{group}\{#APPNAME}"; Filename: "{app}\bin\{#APPNAME}.exe"
Name: "{commondesktop}\{#APPNAME}"; Filename: "{app}\bin\{#APPNAME}.exe"
Name: "{group}\{cm:UninstallProgram,{#APPNAME}}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\bin\{#APPNAME}.exe"; Description: "{cm:LaunchProgram,{#APPNAME}}"; Flags: nowait postinstall skipifsilent

[Code]
//procedure CurStepChanged(CurStep: TSetupStep);
//begin
//  Log('CurStepChanged(' + IntToStr(Ord(CurStep)) + ') called');
//  if CurStep = ssInstall then
//    DeleteFile(ExpandConstant('{app}\*.*'));
//end;
// DelTree('C:\Test\*.tmp', False, True, False);
