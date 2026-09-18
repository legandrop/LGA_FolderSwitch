#ifndef FOLDERSWITCH_TRAYMENU_H
#define FOLDERSWITCH_TRAYMENU_H

#include <QColor>
#include <QPixmap>

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

// Icono de la bandeja tenido del color de la barra (blanco o negro). En pausa va atenuado, para que
// se vea desde la bandeja sin abrir nada.
QPixmap trayIconPixmap(const QColor &color, bool paused);

#endif // FOLDERSWITCH_TRAYMENU_H
