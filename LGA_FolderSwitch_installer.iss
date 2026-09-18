; LGA_FolderSwitch_installer.iss -- FUENTE ESCRITA A MANO, no generada.
;
; A diferencia de las otras apps LGA (LGA_Base_QT_C_Py/docs/Doc_Instaladores_Inno.md: "los .iss
; son GENERADOS" por su instalador.bat), este archivo es el que se edita. instalador.bat solo le
; pasa la version con /DMyAppVersion (la fuente unica es CMakeLists.txt) y no lo reescribe.
; El [Code] de la prueba G3 (Doc_Instaladores_Inno.md 8, .iss minimo con AppId de prueba) se
; copia de aca, tal cual.

#define MyAppName "LGA FolderSwitch"
#ifndef MyAppVersion
#define MyAppVersion "0.10"
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
; Script de cierre por ruta (copia de LGA_Base_QT_C_Py). Viaja dos veces: `dontcopy` para que
; PrepareToInstall lo extraiga a {tmp} (en una actualizacion todavia no hay copia instalada), y
; en {app}\tools para que el desinstalador lo encuentre: al desinstalar no hay {tmp} del setup.
Source: "tools\close_by_path.ps1"; Flags: dontcopy
Source: "tools\close_by_path.ps1"; DestDir: "{app}\tools"; Flags: ignoreversion

; Inno solo desinstala los archivos que copio el. El unico que la app escribe en {app} es
; debug.log (se prende con log=true en config\debug_flags.txt): sin esta seccion queda
; huerfano y la carpeta no se borra. La config chica no vive en disco: FolderSwitch la guarda
; con QSettings en formato nativo, o sea en el registro (HKCU\Software\LGA\FolderSwitch), y el
; desinstalador no la toca a proposito para que sobreviva a una reinstalacion.
[UninstallDelete]
Type: files; Name: "{app}\debug.log"

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
// FolderSwitch vive en la bandeja y una instancia activa bloquea el .exe instalado, asi que se
// cierra antes de instalar y antes de desinstalar. Se cierra POR RUTA, nunca por nombre:
// close_by_path.ps1 -ExeName {#MyAppExeName} -Prefix {app} cierra solo las copias que corren
// desde la carpeta que se va a pisar o a borrar. Antes era un "taskkill /F /IM" en
// InitializeSetup, que ademas corria ANTES de elegir carpeta: se llevaba una copia de build o de
// otra instalacion, y tambien la que el usuario tuviera abierta si despues cancelaba el setup.
// Si PowerShell no esta, o {app} no pasa las guardas del script (menos de dos carpetas debajo de
// la unidad), no se cierra nada e Inno avisa "archivo en uso": es la direccion segura. Las
// comillas van como #34 y powershell.exe con la ruta de {sys}, como en SceneBuilder.
procedure CloseAppByPath(const ScriptPath: String);
var
  ResultCode: Integer;
  Params: String;
begin
  Params := '-NoProfile -NonInteractive -ExecutionPolicy Bypass -File ' + #34 + ScriptPath + #34 +
            ' -ExeName {#MyAppExeName} -Prefix ' + #34 + ExpandConstant('{app}') + #34;
  if Exec(ExpandConstant('{sys}\WindowsPowerShell\v1.0\powershell.exe'), Params, '', SW_HIDE,
          ewWaitUntilTerminated, ResultCode) then
    Log('close_by_path termino con codigo ' + IntToStr(ResultCode))
  else
    Log('close_by_path no se pudo ejecutar: no se cierra nada');
end;

// PrepareToInstall corre con {app} ya elegido y antes de copiar archivos.
function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  Result := '';
  ExtractTemporaryFile('close_by_path.ps1');
  CloseAppByPath(ExpandConstant('{tmp}\close_by_path.ps1'));
  // Stop-Process es asincronico: se le da tiempo al proceso a soltar sus archivos antes de
  // copiar encima. Mismo bloque que SceneBuilder y MediaTools.
  Sleep(1500);
end;

// Al desinstalar {app} ya es la carpeta instalada; el script es la copia de {app}\tools. Si no
// esta (alguien la borro), no se cierra nada e Inno deja lo que este en uso.
function InitializeUninstall(): Boolean;
var
  ScriptPath: String;
begin
  Result := True;
  ScriptPath := ExpandConstant('{app}\tools\close_by_path.ps1');
  if FileExists(ScriptPath) then
  begin
    CloseAppByPath(ScriptPath);
    // El desinstalador borra {app} enseguida: la misma espera que en PrepareToInstall, para
    // que el proceso cerrado suelte el .exe y la carpeta se pueda borrar entera.
    Sleep(1500);
  end
  else
    Log('No esta ' + ScriptPath + ': no se cierra nada');
end;
