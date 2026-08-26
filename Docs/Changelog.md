# Changelog

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
