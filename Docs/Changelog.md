# Changelog

## 2026-09-23 (1)

El cuerpo del icono de la app (`LGA_FolderSwitch.ico`, `.png`, `_1024.png`) era `#231a16`, un
marron oscuro que no coincidia con el negro neutro `#262626` que usa el resto de las apps LGA
(PipeSync, ShotPlayer, MediaTools). Se recoloreo el cuerpo a `#262626` en los tres archivos,
conservando el desregistro CMY de las planchas y el alfa; el `.ico` mantiene todos sus tamanos
(16 a 256 px). `LGA_FolderSwitch_menubar.png` no se toco: es la plantilla monocroma del tray.

[commit sugerido: "El icono de la app usa el mismo negro que el resto del pack"]

## 2026-09-18 (15)

El repo no tenia roadmap ni registro de decisiones, que la Base pide a toda app LGA: lo pendiente
(observaciones de revisiones, arreglos diferidos) y las decisiones de Lega quedaban solo en la
conversacion. Ahora estan `Docs/Doc_Roadmap.md` (registro compartido LGA, checklist de app nueva,
carpeta de un manager cerrado con la X, respuesta de "Check now", marco nativo en otros entornos) y
`Docs/Doc_Decisiones.md` (D-01 abierta: como responde "Check now"; y las decididas de la ventana,
los updates, el foco, las tarjetas, las recientes y la configuracion en AppData).

[commit sugerido: "docs: roadmap y registro de decisiones"]

## 2026-09-18 (14)

El menu de recientes era un `QMenu` con la sombra dura de Windows y un margen de icono enorme.
Ahora es `RecentFoldersPopup` (opcion A1 del canvas): tarjeta redondeada con sombra propia,
numeros en cajas violetas como el Shot Player, nombre y carpeta contenedora; se elige con el
mouse, con 1-5 o con flechas + Enter. La configuracion pasa del registro a
`%APPDATA%\LGA\LGA_FolderSwitch\settings.ini` (`AppSettings`), con migracion automatica, y el
desinstalador borra esa carpeta, las descargas del update, la clave vieja y el inicio con
Windows cuando apunta a esa instalacion.

[commit sugerido: "feat: popup propio de carpetas recientes y configuracion en AppData"]

## 2026-09-18 (13)

Nuevo atajo `Ctrl+Alt+Shift+O`: dentro de un file dialog abre, donde esta el mouse, un menu con
las ultimas 5 carpetas; elegir una la aplica al dialogo (fuente "Recent"). El historial guarda
la carpeta de Explorer/XYplorer al salir de esa ventana y cada carpeta aplicada, sin repetidas,
en la clave `recentFolders`. `HotkeyFilter` registra los dos atajos y la ventana muestra el
nuevo con su propio aviso de "In use". `--ui-shot` suma `recent-menu` y `recent-menu-empty`.
La ayuda dice solo "Developed by Lega Pugliese". Hay un `README.md` en ingles.

[commit sugerido: "feat: menu de carpetas recientes con Ctrl+Alt+Shift+O y README"]

## 2026-09-18 (12)

La ventana tenia una barra de pestanas con una sola pestana (SETTINGS), que no aportaba nada, y
los updates estaban escondidos en la ayuda. Ahora la ventana va sin el marco de Windows y con
una barra de titulo propia (`TitleBar`, reemplaza a `TabHeader`): icono, nombre, `?` solido (el
de File Manager S3), minimizar y cerrar; `MainWindow` le devuelve sombra, esquinas y minimizado
con estilos nativos y `WM_NCCALCSIZE`. Tres tarjetas: estado + automatico + atajo, ultima
carpeta, e inicio con Windows + updates ("Check for updates at startup", clave nueva
`checkUpdatesAtStartup`, y "Check now"). Nada toma foco de teclado (filtro global en `Theme`).
La ayuda usa el encabezado de las otras apps LGA, con el link a GitHub con hover.

[commit sugerido: "feat: ventana sin pestanas, barra de titulo propia y updates a la vista"]

## 2026-09-18 (11)

FolderSwitch es de instancia unica (con otra copia abierta, la nueva sale en silencio por el
`QLockFile`), pero `compilar.bat` y el instalador cerraban solo la copia que iban a pisar: con otra
copia abierta, la compilada o la recien instalada no arrancaba. Ahora `compilar.bat` sin
`--no-run` y `PrepareToInstall` cierran TODAS las copias de la app con `tools\close_by_path.ps1`
rev 4 de la Base (`-AllInstances`; la app no lanza auxiliares). `compilar.bat --no-run` sigue
cerrando solo el exe del arbol que compila, y el desinstalador solo lo que corre desde `{app}`.
Probado con procesos senuelo y con un `.iss` minimo de prueba.

[commit sugerido: "fix: compilar e instalador cierran todas las instancias"]

## 2026-09-18 (10)

La version pasa a 0.10: el minor lleva dos digitos. El auto-update no la hubiera entendido:
`VersionCompare` completaba el minor con ceros a la derecha hasta tres digitos, o sea que lo
leia como decimal, y 0.10 valia lo mismo que 0.1 y menos que 0.9. Ahora compara cada segmento
como entero (0.10 > 0.9 > 0.1) y una version ilegible no ofrece update. El numero sigue saliendo
solo de `CMakeLists.txt`; cambian tambien el valor por defecto del `.iss` y la version de
ejemplo del estado `update-dialog` de `--ui-shot`.

[commit sugerido: "fix: version 0.10 y comparacion de versiones por segmento"]

## 2026-09-18 (9)

`--ui-shot` no andaba en el build normal: `compilar.bat` solo copiaba el plugin `qwindows`, y
una corrida con `QT_QPA_PLATFORM=offscreen` no arrancaba y Qt mostraba un cartel fatal en el
escritorio. Ahora `compilar.bat` verifica y copia tambien `platforms\qoffscreen.dll` junto al
exe, en `build\` y en `build-release\`. `deploy.bat` no cambia: arma su propia lista.

[commit sugerido: "fix: compilar.bat deja qoffscreen.dll junto al exe para --ui-shot"]

## 2026-09-18 (8)

El repo no tenia `.gitattributes`: Git guardaba los bytes como llegaban, con nueve archivos en
CRLF entre otros en LF y los `.bat` en LF, que es lo que hacia fallar un `goto` a una etiqueta
nueva en `cmd.exe`. Ahora hay `.gitattributes` y `.editorconfig` con la politica LGA (LF por
defecto, CRLF en `.bat`/`.cmd`/`.ps1`, binarios marcados), la renormalizacion va en su propio
commit, sin cambios de contenido, y `.git-blame-ignore-revs` la saca de `git blame`.

[commit sugerido: "docs: changelog de la unificacion de line endings"]

## 2026-09-18 (7)

Con el atajo ocupado por otra app, el aviso decia "Automatic switching still works.", y no era
cierto si la app estaba en pausa o con el auto-switch apagado: en esos casos no hay cambio
automatico que siga andando. Ahora dice "Automatic switching isn't affected.", que vale en
cualquier estado: el atajo ocupado no cambia nada del cambio automatico.

[commit sugerido: "fix: el aviso del atajo ocupado no promete el cambio automatico"]

## 2026-09-18 (6)

Tres detalles de la ventana nueva. "Manual shortcut" iba sangrado bajo "Switch automatically" y
parecia una sub-opcion, pero el atajo funciona aparte (ni el auto-switch ni la pausa lo apagan):
ahora tiene su propia fila, separada. Con el atajo ocupado, el chip `In use` quedaba contra el
borde derecho y el aviso se leia como un caption mas: el chip va pegado a la `O` y el aviso, como
el de un cambio fallido, en el color de error. Y ante un cambio fallido se aconsejaba reintentar
con Ctrl+Alt+O aunque el atajo no estuviera registrado: en ese caso dice que se elija la carpeta
a mano. En pausa, la tarjeta aclara que el atajo sigue andando. `--ui-shot` suma
`failed-hotkey-busy`.

[commit sugerido: "fix: el atajo manual en su propia fila y avisos segun la causa"]

## 2026-09-18 (5)

`instalador.bat` preguntaba con `choice` aunque no hubiera consola: con la entrada redirigida
podia ejecutar el instalador o publicar, no tenia `--no-run`, salia en 0 aunque fallara la
publicacion y no generaba `SHA256SUMS`. Ahora detecta la consola al principio y, sin ella, solo
genera en local. Suma `--no-run`, sale con 1 si falla un paso de publicacion, borra el
`SHA256SUMS` viejo antes de ISCC, genera el nuevo (sale con 1 si no puede) y lo exige antes del
tag. El `.iss` dice en su cabecera que es fuente a mano y espera 1,5 s tras cerrar la app al
instalar y al desinstalar. Sin etiquetas nuevas: con los `.bat` en LF, un `goto` a una etiqueta
nueva fallo en la prueba.

[commit sugerido: "fix: instalador sin consola no ejecuta ni publica, --no-run y SHA256SUMS"]

## 2026-09-18 (4)

No habia ayuda ni forma de ver la version, y pausar la app era un checkbox mas. El `?` de la
barra abre un Help como el de LGA_VideoDownloader: version, `Check now`, autor y como se usa. El
menu del tray suma un encabezado `FolderSwitch · On|Paused` y `Pause`/`Resume switching`, con la
paleta de `Doc_MenuContextual.md` (es un QMenu y no LgaContextMenu, que vive en
filemanagers3_core). En pausa el icono del tray va atenuado. Menu, icono, tooltip y la tarjeta de
estado leen el mismo `AppState`. El dialogo "Update Available" toma el tema y se arma aparte, asi
`--ui-shot` lo dibuja sin el updater. Estados nuevos: `help`, `tray-menu` y `update-dialog`.

[commit sugerido: "feat: Help y menu del tray con el estilo LGA"]

## 2026-09-18 (3)

La ventana de Settings eran tres checkboxes en castellano: no decia si la app estaba andando, el
path se cortaba contra el borde derecho y un fallo del cambio de carpeta o del hotkey solo quedaba
en el log. Ahora, en ingles y con el estilo de LGA_VideoDownloader: tarjeta de estado con
`Pause`/`Resume`, ultima carpeta con origen, resultado (`Applied`/`Not applied`) y hora, y
opciones con el atajo y su aviso `In use` si `RegisterHotKey` fallo. El estado vive en
`AppState`, unica fuente de verdad de On/Paused con las mismas claves de siempre en el registro.
La ventana tiene ancho fijo de 440 y el alto de su contenido (396 a 420 px).

[commit sugerido: "feat: ventana de Settings redisenada, en ingles"]

## 2026-09-18 (2)

La hoja de estilo arrancaba con `QWidget { background-color }` global: pinta cada contenedor con
el fondo de la ventana y deja rectangulos de otro color adentro de cualquier tarjeta. Tambien el
checkbox tildado era un cuadrado violeta sin tilde. Se reemplaza `dark_theme.qss` por `Theme`,
copia recortada del de LGA_VideoDownloader: mismos tokens, paleta oscura como red para lo que
ninguna regla nombra (tooltips, QMessageBox, progreso), reglas por objectName, y la misma Inter
embebida (`Inter-*.ttf` en vez de `Inter_18pt-*`). Entran `ElidedLabel`, `Chip` e iconos
vectoriales de VD. La tilde es un PNG: el exe no carga Qt6Svg y el deploy no lleva `qsvg`.

[commit sugerido: "feat: tema LGA compartido con Video Downloader"]

## 2026-09-18

No habia forma de ver la ventana sin abrirla en el escritorio. Se suma `--ui-shot <estado>
<out.png> [--dpr N]`, que arma la ventana real con datos de prueba y la dibuja a un PNG con su
`.json` (geometria y fuente resuelta), como el de LGA_VideoDownloader. Sale antes de la instancia
unica, sin bandeja, hotkey, updater ni debug.log, y solo con `QT_QPA_PLATFORM=offscreen` (si no,
sale con 2). `MainWindow::Mode::Capture` no conecta ninguna escritura. De paso, en el modo normal
el estado inicial se carga antes de conectar los checkboxes: antes cada arranque reescribia
`autoSwitch` y `enabled` en el registro. El plugin `qoffscreen.dll` se copia a mano al build de
prueba: tocar `compilar.bat` (en LF) le rompio la busqueda de etiquetas a cmd.exe.

[commit sugerido: "feat: modo --ui-shot para capturar la ventana sin escritorio"]

## 2026-09-17 (4)

El `.iss` cerraba la app con `taskkill /F /IM` en `InitializeSetup`, o sea por nombre y
antes de elegir carpeta: se llevaba una copia de build o de otra instalacion, y tambien la
de la bandeja aunque despues se cancelara el setup. Ahora el cierre va en
`PrepareToInstall`, con `{app}` ya elegido, y en `InitializeUninstall`, y cierra solo lo que
corre desde `{app}` con `tools\close_by_path.ps1`. El script viaja dentro del setup (se
extrae a `{tmp}`) y ademas se instala en `{app}\tools`, porque el desinstalador no tiene el
`{tmp}` del setup. Probado con un `.iss` minimo con el mismo `[Code]` y otro `AppId`:
cierra la copia de `{app}` y deja vivas la de una carpeta hermana y la de un repo.

[commit sugerido: "fix: el instalador cierra la app por ruta y despues de elegir carpeta"]

## 2026-09-17 (3)

`compilar.bat`, `deploy.bat` e `instalador.bat` cerraban por nombre todos los
`LGA_FolderSwitch.exe` abiertos, incluida la copia de la bandeja con la que se estaba
trabajando. Ahora cada uno cierra solo la copia que corre desde su propio arbol (`build\`
o `build-release\`, y `deploy\`), por la ruta real del proceso con
`tools\close_by_path.ps1`, copia de `LGA_Base_QT_C_Py`. Si la instalada esta abierta, el
build nuevo sale en silencio por el lock de instancia unica: hay que cerrarla a mano.

`compilar.bat` y `deploy.bat` toman ademas su carpeta antes de leer los argumentos: `shift`
corria tambien el parametro cero y, llamados con argumentos desde otra carpeta, resolvian
rutas (y el `start` del final) contra la carpeta actual.

[commit sugerido: "fix: los scripts cierran la app por ruta, no por nombre"]

## 2026-09-17 (2)

El comentario que acompana la seccion `[UninstallDelete]` del `.iss` decia que la
config chica de FolderSwitch vive en AppData. No es cierto: la app la guarda con
`QSettings(QStringLiteral("LGA"), QStringLiteral("FolderSwitch"))`, o sea en formato
nativo, que en Windows es el registro — `HKCU\Software\LGA\FolderSwitch`. No hay
ningun archivo de config en disco.

El comportamiento no cambia (el desinstalador tampoco tocaba el registro, y sigue sin
tocarlo a proposito para que la config sobreviva a una reinstalacion): lo que se
corrige es la explicacion, que mandaba a buscar la config al lugar equivocado.

[commit sugerido: "docs: la config vive en el registro, no en AppData"]

## 2026-09-17

Desinstalar dejaba la carpeta de la app sin borrar. Inno Setup solo saca los archivos
que copio el instalador, y `debug.log` —el unico archivo que FolderSwitch escribe en
`{app}`, cuando `config\debug_flags.txt` tiene `log=true`— quedaba huerfano: con un
archivo adentro, la carpeta de instalacion tampoco se borra.

`LGA_FolderSwitch_installer.iss` suma una seccion `[UninstallDelete]` con
`Type: files` para `{app}\debug.log`. No se agrega nada mas porque no hay nada mas:
el resto de lo que hay en la carpeta lo instalo el propio setup, y la config chica
vive en AppData y sobrevive a una reinstalacion a proposito. Criterio general en
`../LGA_Base_QT_C_Py/docs/Doc_Rutas_Instalacion.md`. Verificado compilando el `.iss`
con ISCC.

[commit sugerido: "fix: el desinstalador se lleva el debug.log que deja la app"]

## 2026-09-04 (2)

La entrada de `Run` nunca existio en el registro real, y por eso ningun reinicio la
ejecuto. Las escrituras "a mano" se habian hecho desde una terminal empaquetada en
MSIX (la terminal de una app de la Microsoft Store), que tiene registro virtualizado por
paquete: la escritura va a un hive privado, releerla desde ahi la muestra puesta, y
Windows no la ve. El sintoma que lo delato: la app abierta por el usuario loguea
`valor en Run: "(ninguno)"` y la misma app lanzada desde esa terminal loguea el valor.

El checkbox "Iniciar con Windows" vuelve a estar HABILITADO desde un build: el click
del usuario es la unica via que escribe donde Windows lee, y el cambio anterior la
habia tapado. Lo que sigue prohibido desde un arbol de desarrollo es la escritura
AUTOMATICA (primer arranque), que es la que puede pisar la entrada sola. El toggle
ahora loguea el valor que quedo en el registro, para poder verificarlo desde afuera.

[commit sugerido: "fix: el checkbox de inicio con Windows vuelve a ser clickeable desde un build"]

## 2026-09-04

Seguia sin arrancar con Windows despues del arreglo anterior. El log de arranque del
sistema (`Microsoft-Windows-Shell-Core/Operational`, eventos 9707) mostro la clave
`Run` ejecutandose entera —Seer, GoogleDrive, StreamDeck, LinkRedirector y FrameRev—
sin un solo intento de FolderSwitch. La entrada de FrameRev sobrevive porque FrameRev
nunca la toca desde `build/`; la de FolderSwitch no sobrevivia porque la app la
reescribia en cada arranque.

`AutoStart` pasa a ser copia del modulo de FrameRev: `availability()` bloquea escribir
al registro desde `build*`/`deploy*` o el arbol del repo, el checkbox queda
deshabilitado ahi con el motivo en el tooltip, y el `connect` va despues del
`setChecked` inicial para que reflejar el estado no dispare una escritura. Verificado:
la app arranca y la entrada queda byte a byte igual.

[commit sugerido: "fix: AutoStart copiado de FrameRev, sin escribir en el registro desde un build"]

## 2026-09-03

La app no arrancaba con Windows: en la clave `Run` del registro no quedaba ninguna
entrada `LGA_FolderSwitch`. El instalador escribia esa MISMA entrada con la ruta
instalada (pisando la que la app habia puesto para la copia de `build/`) y la
borraba al desinstalar, apuntara a donde apuntara. En LinkRedirector y FrameRev el
instalador no toca `Run`: la maneja solo la app. Ahora el `.iss` no la escribe; una
copia instalada activa el inicio sola en su primer arranque (patron de FrameRev;
desde `build/` o `deploy/` no). `AutoStart` ademas limpia el "deshabilitado" de
Task Manager > Startup, y el checkbox relee el estado real al abrirse.

El log tampoco ayudaba: el primer mensaje sale antes de la QApplication, `config/`
se resolvia contra el directorio de trabajo, y lanzada por `Run` (cwd = System32)
no escribia nada. La ruta ahora sale de `GetModuleFileName` y al arrancar se
loguean el exe y el estado del inicio automatico. Patron documentado en
`LGA_Base_QT_C_Py/docs/Doc_Autostart_Windows.md`.

[commit sugerido: "fix: la entrada Run la maneja solo la app, y el log sirve al arrancar con Windows"]

## 2026-08-30

Faltaba el circuito de publicacion y actualizacion que ya tienen las otras apps.
Como este repo es publico, las releases van al mismo repo (no hay repo _Release
aparte) y el slug se dio de alta en `repos.json` de LGA_Updates.

`instalador.bat` nuevo, calcado del de MediaTools_v2: preflight de git/gh, deploy,
guard de que el binario lleve la version del CMakeLists (unica fuente; el .iss la
recibe por `/DMyAppVersion`), Inno Setup, y tag + release en GitHub con refresco
del manifiesto.

Auto-update dentro de la app: `UpdateService` (version reducida del de
FileManagerS3) lee el manifiesto de LGA_Updates, compara con `VersionCompare`
(copia fiel de FM), descarga con progreso, verifica SHA-256 fail-closed y lanza
el instalador. Chequeo automatico al arrancar y "Check for Updates..." en el tray.
`compilar.bat`/`deploy.bat` suman Qt6Network y el plugin TLS.

[commit sugerido: "feat: instalador con release en GitHub y auto-update desde el manifiesto"]

## 2026-08-26 (2)

La app seguia sin arrancar con Windows: moria con "Qt6Widgets.dll was not found"
antes de escribir una linea de log. El .exe de `build/` no tenia ninguna DLL al
lado, asi que dependia del PATH para encontrar Qt, y la Run key lo lanza sin el
PATH que arma el script de compilacion. Peor: cuando el PATH si tenia algo,
cargaba Qt 6.8.2 y el runtime de MinGW que trae Git -- ni la version ni el
compilador con los que se compila.

`compilar.bat` ahora copia el runtime al lado del .exe con el mismo esquema que
LinkRedirector (verificar -> copiar -> windeployqt -> re-verificar). Verificado
lanzando el .exe con PATH de solo system32: carga Qt 6.5.3 y el MinGW correctos
desde su propia carpeta.

[commit sugerido: "fix: copiar el runtime de Qt al lado del exe, como LinkRedirector"]

## 2026-08-26

Tres cosas del arranque y el empaquetado.

La app no arrancaba con Windows aunque la Run key estaba bien puesta: al lanzarse
desde el arranque el shell todavia no tiene lista la bandeja, `isSystemTrayAvailable()`
daba false y `main` salia antes de loguear nada. Por eso andaba al abrirla a mano y no
al reiniciar. Ahora espera hasta 90 s a que la bandeja aparezca, y no usa un
QMessageBox modal, que nadie ve en el arranque.

El .exe seguia con el icono viejo: el .rc solo NOMBRA al .ico, asi que cambiar el
icono no cambiaba el .rc y windres nunca se recompilaba. Se ato con OBJECT_DEPENDS.

Se agrego `deploy.bat` (arma `deploy/` con el runtime de Qt y MinGW) y
`LGA_FolderSwitch_installer.iss` (Inno Setup), con tarea opcional de iniciar con
Windows que escribe la MISMA entrada de registro que usa la UI de la app.

[commit sugerido: "fix: esperar la bandeja al arrancar, icono del exe, e instalador"]

## 2026-08-25 (11)

El path se escribia en el campo pero el browser de Nuke no aplicaba nada:
`ValuePattern::SetValue` cambia el texto sin disparar la senal de edicion del
widget, asi que nadie se entera del cambio (a mano se veia igual: escribir un
espacio y borrarlo lo destrababa). Ahora se escribe el path menos su ultimo
caracter con SetValue y ese ultimo se tipea como tecla real via
`SendInput`/`KEYEVENTF_UNICODE` -- unicode y no virtual-key porque la barra
invertida cambia de lugar segun la distribucion de teclado. Queda el path exacto,
con evento de edicion, y sin Enter ni boton: aceptar commitearia la carpeta como
si fuera el archivo.

[commit sugerido: "fix: tipear el ultimo caracter para que el dialogo Qt aplique el path"]

## 2026-08-25 (10)

En el browser de Nuke, el camino de UI Automation escribia el path y ademas
"aceptaba": el dialogo del Read se cerraba eligiendo la carpeta como si fuera el
archivo. Se comprobo que pasa igual invocando el boton "Open" que mandando Enter con
el campo enfocado por UIA (el foco de teclado era correcto, no era ese el problema):
en ese browser cualquier aceptar commitea el texto. Ahora se escribe el path y nada
mas. El camino Win32 no cambia: ahi el IDOK es lo que hace navegar y funciona bien.

[commit sugerido: "fix: en dialogos Qt escribir el path sin aceptar"]

## 2026-08-25 (9)

El camino de UI Automation escribia el path con barras normales (`N:/x/y/`) porque
asi lo mostraba Nuke, pero los dialogos lo quieren como lo escribe Windows. Ahora se
normaliza a barras invertidas con barra final (`C:\x\y\`). La barra final importa:
sin ella algunos dialogos toman el texto como nombre de archivo en vez de navegar a
la carpeta. La normalizacion se movio al punto unico de despacho en
`DialogSwitcher::switchDialog`, asi vale igual para el camino Win32 y el de UIA en
vez de estar duplicada y divergiendo.

[commit sugerido: "fix: normalizar el path a barras invertidas con barra final"]

## 2026-08-25 (8)

Los dialogos de Nuke no son dialogos de Windows: son clase `Qt653QWindowIcon` y
`EnumChildWindows` devuelve CERO hijos, porque Qt dibuja sus widgets internamente.
No hay ningun `Edit` que tocar, asi que la inyeccion Win32 era imposible ahi y la
deteccion (que exigia `#32770`) ni los veia. Se agrego una segunda via por UI
Automation: `isQtFileDialog` clasifica por clase `Qt*` + owner no nulo (la ventana
principal de Nuke comparte clase pero no tiene owner) + presencia de un campo y un
boton de aceptar; y `UiaSwitcher` escribe el path con `ValuePattern::SetValue` e
invoca el boton. Medido sobre el dialogo real: UIA cuesta 9 ms, barato para
clasificar en cada cambio de foco, y se cachea por ventana.

[commit sugerido: "feat: soporte de dialogos Qt (Nuke) via UI Automation"]

## 2026-08-25 (7)

La deteccion andaba (XYplorer, path resuelto, logica de retorno) pero la inyeccion
fallaba con "No se encontro el edit de nombre de archivo": la busqueda del combo
miraba solo hijos DIRECTOS del dialogo, y segun la app ese combo cuelga de un
contenedor intermedio. El dialogo de Notepad, con el que se habia probado, lo tiene
directo; el de otras apps no. Ahora se recorre todo el arbol de descendientes con
tres pasadas (ComboBoxEx32 > ComboBox > Edit, ComboBox > Edit pelado, y cualquier
Edit visible y habilitado). Si aun asi no aparece, se loguean las clases del dialogo
para poder identificarlo.

[commit sugerido: "fix: buscar el edit del dialogo en todo el arbol, no solo hijos directos"]

## 2026-08-25 (6)

`compilar.bat` compilaba bien pero nunca abria la app: las dos ramas del final
(`--no-run` y el caso normal) hacian `exit /b 0`, asi que el `.exe` no se lanzaba
nunca. Ademas le faltaba el `taskkill` previo que si tiene LinkRedirector; como la
app vive en la bandeja, una instancia viva bloquea el `.exe` y el link falla con
"Permission denied", y el QLockFile hace salir en silencio a la instancia nueva.
Se agrego el taskkill, se lanza el exe al terminar, y Debug/Release usan arboles
separados (`build/` y `build-release/`) porque antes `--release` reusaba la cache
de Debug y se ignoraba.

[commit sugerido: "fix: compilar.bat no lanzaba la app ni mataba la instancia previa"]

## 2026-08-25 (5)

Se fijo la marca "carpeta que apunta" (T3) y se regeneraron los assets. El generador
escribia los PNG y el ICO con la ruta que compone sobre blanco, asi que los archivos
quedaban RGB sin alfa: sobre la barra de tareas oscura se habrian visto como un
cuadrado blanco. Ahora `emit.py` usa la ruta con alfa real para todo el raster de
color, y cada tamano del ICO (16..256) se renderiza a su tamano en vez de escalarse
desde uno grande. El menubar sigue siendo silueta negra con alfa, que la app tintea
en runtime.

[commit sugerido: "fix: assets del icono en RGBA con alfa real"]

## 2026-08-25 (4)

La primera marca fallaba en tres cosas medibles contra el resto del set: 1.37 de
proporcion W/H (los otros van 0.88-1.00), 7.9% de verde (los otros 3.7-4.8%) y solo
46% de core. La flecha gris encima se comia la masa oscura, y una forma ancha alarga
el borde inferior, que es justo donde cyan y amarillo se solapan en verde. Se
reemplazo por silueta maciza sin glifo, mas alta y con el lado derecho en punta:
0.93 / 5.0% / 58%, los tres en rango. El generador ahora mide esas tres cifras, asi
que cualquier variante se compara con datos y no a ojo.

[commit sugerido: "fix: marca mas alta, sin glifo claro y con el verde en rango"]

## 2026-08-25 (3)

La app usaba un icono prestado de LinkRedirector. Se diseñó marca propia dentro del
sistema de las apps LGA: silueta maciza de carpeta, tres planchas CMY desregistradas
a 120°, y flecha gris encima (no calada: calarla deja flecos, porque el hueco de cada
plancha cae desplazado). Paleta y offsets medidos de los PNG existentes, no estimados.
El glifo se ajusta al core — la intersección de las tres planchas — y no a la silueta.
Se agregó `tools/logo/` (geometría única en Python que emite PNG, ICO y SVG) y se
verificó la lectura a 16 px componiendo sobre capturas reales de la barra y el tray.

[commit sugerido: "feat: marca propia de LGA_FolderSwitch y generador de iconos"]

## 2026-08-25 (2)

El auto-switch fallaba al volver de XYplorer: la condición exigía que la ventana
inmediatamente anterior al diálogo fuera el file manager, y cualquier ventana
intermedia (task switcher de Alt+Tab) la rompía en silencio. Se reemplazó por
tracking de retorno: al pasar de un diálogo vivo al manager se marca ese diálogo
como pendiente, y al volver a ese mismo HWND se inyecta, sin importar ventanas
intermedias. Además: debug log opcional (`log=true` en `config/debug_flags.txt`
→ `debug.log` en la raíz) porque la app WIN32 no tiene consola, y `.gitignore`.

[commit sugerido: "fix: auto-switch robusto ante ventanas intermedias + debug log"]

## 2026-08-25

Implementación inicial de LGA_FolderSwitch: app de tray que detecta la carpeta activa
en Explorer o XYplorer y la inyecta en el diálogo Open/Save de Windows (`#32770`) al
volver a él, o a demanda con Ctrl+Alt+O. Usa `SetWinEventHook` para seguir la ventana
en foreground, COM (`IShellWindows`/`IWebBrowser2`) para leer el path de Explorer,
parseo de título para XYplorer, y la técnica QuickSwitch (inyectar en el combo de
nombre de archivo + `WM_COMMAND`/`IDOK`) para el switch. Settings con QSettings,
autostart por registro, y estética dark de LGA.

[commit sugerido: "feat: implementación inicial de LGA_FolderSwitch"]
