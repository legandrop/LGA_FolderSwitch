#ifndef FOLDERSWITCH_TRAYMENU_H
#define FOLDERSWITCH_TRAYMENU_H

#include <QIcon>

class QAction;
class QMenu;

// Menu e icono de la bandeja, separados de TrayController para que --ui-shot los pueda dibujar
// sin crear un QSystemTrayIcon (que pondria un icono en la bandeja real).
struct TrayMenuActions
{
    QAction *header = nullptr;  // "FolderSwitch · On|Paused", deshabilitado
    QAction *toggle = nullptr;  // "Pause switching" / "Resume switching"
    QAction *settings = nullptr;
    QAction *updates = nullptr;
    QAction *quit = nullptr;
};

TrayMenuActions buildTrayMenu(QMenu *menu);
// Textos segun On/Paused. Lo llama quien escucha AppState::changed.
void refreshTrayMenu(const TrayMenuActions &actions, bool enabled);

// Icono de la bandeja. Activo: la marca en color con desregistro CMY, cuerpo blanco sobre barra
// oscura y #262626 sobre barra clara (PNG por tamano en resources/icons/tray/, los genera
// tools/logo/tray_color.py). En pausa: la silueta monocroma del color de la barra, atenuada al 40 %;
// asi el color quiere decir que la app esta andando y se ve desde la bandeja sin abrir nada.
QIcon trayIcon(bool lightBar, bool paused);

#endif // FOLDERSWITCH_TRAYMENU_H
