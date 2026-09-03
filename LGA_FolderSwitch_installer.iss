#define MyAppName "LGA FolderSwitch"
#ifndef MyAppVersion
#define MyAppVersion "0.1"
#endif
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

[Files]
Source: "deploy\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

; SIN seccion [Registry] a proposito. El inicio con Windows (HKCU\...\Run) lo maneja
; SOLO la app: la copia instalada lo activa sola en su primer arranque, y el checkbox
; de Settings lo prende/apaga. Cuando lo escribia el instalador, con el mismo nombre
; de valor que usa la app, pisaba la entrada que apuntaba a otra copia y la borraba al
; desinstalar (uninsdeletevalue): asi se perdio el inicio con Windows. LinkRedirector
; nunca dejo que el instalador la toque. Ver LGA_Base_QT_C_Py/docs/Doc_Autostart_Windows.md.

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
