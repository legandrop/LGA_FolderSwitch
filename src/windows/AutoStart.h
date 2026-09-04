#ifndef FOLDERSWITCH_AUTOSTART_H
#define FOLDERSWITCH_AUTOSTART_H

// Maneja el inicio automatico de LGA_FolderSwitch con el sistema:
// HKCU\Software\Microsoft\Windows\CurrentVersion\Run, con la Win32 Registry API directa
// (no QSettings). Copia del modulo de LGA_FrameRev (src/platform/AutoStart.cpp), que es la
// implementacion de referencia: misma estructura, misma politica, mismos nombres.
// Doc del patron: ../LGA_Base_QT_C_Py/docs/Doc_Autostart_Windows.md.
//
// La entrada de Run la maneja SOLO la app: el instalador no la escribe ni la borra.

#include <QString>

namespace AutoStart {

// True si LGA_FolderSwitch esta registrado para iniciar con el sistema Y el valor apunta al
// ejecutable ACTUAL -- siempre pregunta al registro en vivo, no cachea nada en ningun lado.
bool isEnabled();

// Activa o desactiva el inicio automatico. Usa la ruta del ejecutable actual.
bool setEnabled(bool enabled);

// Por que NO se puede activar desde este binario (ver availability()).
enum class Unavailability {
    None,            ///< se puede
    DevelopmentTree, ///< corre desde build/, deploy/ o el repo (runsFromDevelopmentTree())
};

struct Availability
{
    bool available = false;
    Unavailability reason = Unavailability::None;
    /// Texto corto EN INGLES para la UI (tooltip del checkbox); vacio si `available`.
    QString text;
};

// Si el inicio automatico se puede activar desde ESTE binario. setEnabled() NO chequea esto a
// proposito: es la escritura pelada al registro. La politica la aplica quien decide activar
// (el checkbox de Settings y el primer arranque).
//
// Por que importa, medido en esta maquina el 2026-09-04: un valor de Run que la app reescribe
// en cada arranque desde build/ es un valor que TAMBIEN se puede pisar o borrar sola, y asi se
// perdio el inicio con Windows de FolderSwitch. FrameRev nunca toca su entrada desde una salida
// de desarrollo, y por eso la suya sobrevive intacta a cualquier recompilada.
Availability availability();

// True si este binario corre desde una salida de desarrollo: carpeta contenedora `build*` /
// `deploy*`, o adentro del arbol de build o del repo que inyecta CMake (LGA_BUILD_TREE_DIR /
// LGA_SOURCE_TREE_DIR). Misma regla que LgaRegistry, que es de donde la toma FrameRev.
bool runsFromDevelopmentTree();

// Valor crudo guardado en Run (vacio si no existe). Solo para diagnostico/log.
QString storedCommand();

// True si Task Manager > Startup tiene este valor marcado como deshabilitado (clave
// StartupApproved). El valor de Run existe igual, pero Windows lo ignora. Solo para
// diagnostico/log: isEnabled() se mantiene identico al de FrameRev a proposito.
bool disabledByTaskManager();

// Ruta del ejecutable actual entre comillas dobles (para que sobreviva a rutas con espacios),
// tal cual se escribe en el valor del registro.
QString currentExeQuoted();

} // namespace AutoStart

#endif // FOLDERSWITCH_AUTOSTART_H
