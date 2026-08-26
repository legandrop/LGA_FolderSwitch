#define MyAppName "LGA FolderSwitch"
#define MyAppVersion "0.1"
#define MyAppPublisher "LGA"
#define MyAppExeName "LGA_FolderSwitch.exe"
#define MyAppOutputDir "installer"

[Setup]
AppId={{EE6C0266-A700-4239-9208-C1FAC99CBC11}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName=C:\Portable\LGA\FolderSwitch
DefaultGroupName={#MyAppName}
OutputDir={#MyAppOutputDir}
OutputBaseFilename=LGA_FolderSwitch_Setup_v{#MyAppVersion}
SetupIconFile=resources\icons\LGA_FolderSwitch.ico
PrivilegesRequired=lowest
UsePreviousAppDir=no
DirExistsWarning=no
Compression=lzma2
LZMANumBlockThreads=4
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "autostart"; Description: "Iniciar {#MyAppName} con Windows"; GroupDescription: "Opciones adicionales"; Flags: checkedonce

[Files]
Source: "deploy\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "LGA_FolderSwitch"; ValueData: """{app}\{#MyAppExeName}"""; Flags: uninsdeletevalue; Tasks: autostart

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
// El script de referencia (LinkRedirector) no resuelve el cierre de la instancia
// corriendo dentro del .iss (lo hace afuera, en instalador.bat, solo antes de
// generar el deploy). Como FolderSwitch vive en la bandeja y el .exe instalado
// queda bloqueado si hay una instancia activa, ese cierre se agrega aca, antes
// de instalar y antes de desinstalar.
procedure CloseRunningApp;
var
  ResultCode: Integer;
begin
  Exec('taskkill.exe', '/F /IM {#MyAppExeName}', '', SW_HIDE,
    ewWaitUntilTerminated, ResultCode);
end;

function InitializeSetup(): Boolean;
begin
  CloseRunningApp;
  Result := True;
end;

function InitializeUninstall(): Boolean;
begin
  CloseRunningApp;
  Result := True;
end;
