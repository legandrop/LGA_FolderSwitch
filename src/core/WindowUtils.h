#ifndef FOLDERSWITCH_WINDOWUTILS_H
#define FOLDERSWITCH_WINDOWUTILS_H

#include <windows.h>
#include <QString>

// Helpers de clasificacion de ventanas Win32: Explorer, XYplorer y file dialogs
// comunes (#32770). Usados por el controller para decidir cuando ofrecer/inyectar
// un path.
namespace WindowUtils {

// Explorer: clase "CabinetWClass".
bool isExplorerWindow(HWND hwnd);

// XYplorer: clase "ThunderRT6FormDC" O el exe del proceso es xyplorer.exe
// (case-insensitive). El chequeo por exe es el definitivo.
bool isXYplorerWindow(HWND hwnd);

// True si hwnd es un manager de archivos que sabemos resolver (Explorer o XYplorer).
bool isFileManagerWindow(HWND hwnd);

// File dialog comun de Windows: clase "#32770" y tiene un hijo/descendiente de
// clase "DUIViewWndClassName" o "SHELLDLL_DefView", o un hijo directo "ComboBoxEx32".
bool isFileDialogWindow(HWND hwnd);

// Nombre de archivo (sin path) del ejecutable dueno de hwnd, en minusculas.
// QString vacio si no se pudo obtener.
QString processExeName(HWND hwnd);

} // namespace WindowUtils

#endif // FOLDERSWITCH_WINDOWUTILS_H
