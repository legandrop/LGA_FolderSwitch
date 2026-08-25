#ifndef FOLDERSWITCH_FOLDERRESOLVER_H
#define FOLDERSWITCH_FOLDERRESOLVER_H

#include <QString>

#include <windows.h>

// Resuelve la carpeta activa de una ventana de Explorer o XYplorer.
// CoInitializeEx ya se llama en main.cpp antes de app.exec().
namespace FolderResolver {

// Explorer via COM (IShellWindows/IWebBrowser2). QString() si no se pudo
// resolver (carpeta virtual, LocationURL vacio, etc.).
QString resolveExplorerPath(HWND hwnd);

// XYplorer via titulo de ventana (formato "<path> - XYplorer ...").
QString resolveXYplorerPath(HWND hwnd);

} // namespace FolderResolver

#endif // FOLDERSWITCH_FOLDERRESOLVER_H
