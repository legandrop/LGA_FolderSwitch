#ifndef FOLDERSWITCH_AUTOSTART_H
#define FOLDERSWITCH_AUTOSTART_H

#include <QString>

// Inicio automatico con Windows via HKCU\Software\Microsoft\Windows\CurrentVersion\Run,
// con la Win32 Registry API directa (no QSettings): mismo patron que LinkRedirector y
// FrameRev. Doc del patron: ../LGA_Base_QT_C_Py/docs/Doc_Autostart_Windows.md.
//
// La entrada de Run la maneja SOLO la app: el instalador no la escribe ni la borra.
// Cuando la escribia el instalador con el mismo nombre de valor, pisaba la ruta que
// habia puesto la app (la copia de build/ dejaba de arrancar) y al desinstalar
// borraba el valor aunque ya apuntara a otra copia. Asi se perdio el inicio con
// Windows en esta app sin que ningun log lo mostrara.
namespace AutoStart {

// True si Run apunta al ejecutable ACTUAL y Windows no lo tiene deshabilitado desde
// Task Manager > Startup. Siempre pregunta al registro en vivo, no cachea nada.
bool isEnabled();

// Activa o desactiva el inicio automatico con la ruta del ejecutable actual. Al
// activar, borra ademas la marca de "deshabilitado" de Task Manager, si la hubiera:
// sin eso el valor queda escrito pero Windows no lo lanza.
bool setEnabled(bool enabled);

// Valor crudo guardado en Run (vacio si no existe). Solo para diagnostico/log.
QString storedCommand();

// True si Task Manager > Startup tiene este valor marcado como deshabilitado
// (clave StartupApproved). El valor de Run existe igual, pero Windows lo ignora.
bool disabledByTaskManager();

// True si este binario corre desde una salida de desarrollo: carpeta contenedora
// `build*`/`deploy*`, o adentro del arbol de build o del repo que inyecta CMake
// (LGA_BUILD_TREE_DIR / LGA_SOURCE_TREE_DIR). Misma regla que LgaRegistry en
// FrameRev. Se usa para que el primer arranque no registre un build de desarrollo
// por su cuenta; el checkbox de Settings sigue disponible desde cualquier copia.
bool runsFromDevelopmentTree();

} // namespace AutoStart

#endif // FOLDERSWITCH_AUTOSTART_H
