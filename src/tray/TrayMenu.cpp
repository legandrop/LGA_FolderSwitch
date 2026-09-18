#include "tray/TrayMenu.h"

#include <QAction>
#include <QMenu>
#include <QPainter>

TrayMenuActions buildTrayMenu(QMenu *menu)
{
    // Estilo: bloque QMenu de Theme (paleta de LGA_Base_QT_C_Py/docs/Doc_MenuContextual.md).
    // No es LgaContextMenu porque ese componente vive en filemanagers3_core y esta app no lo enlaza.
    TrayMenuActions a;
    a.header = menu->addAction(QString());
    a.header->setEnabled(false);
    a.toggle = menu->addAction(QString());
    menu->addSeparator();
    a.settings = menu->addAction(QStringLiteral("Settings..."));
    a.updates = menu->addAction(QStringLiteral("Check for Updates..."));
    menu->addSeparator();
    a.quit = menu->addAction(QStringLiteral("Quit"));
    return a;
}

void refreshTrayMenu(const TrayMenuActions &actions, bool enabled)
{
    actions.header->setText(enabled ? QStringLiteral("FolderSwitch · On") : QStringLiteral("FolderSwitch · Paused"));
    actions.toggle->setText(enabled ? QStringLiteral("Pause switching") : QStringLiteral("Resume switching"));
}

QPixmap trayIconPixmap(const QColor &color, bool paused)
{
    QPixmap px(QStringLiteral(":/icons/LGA_FolderSwitch_menubar.png"));
    if (px.isNull()) {
        px = QPixmap(QStringLiteral(":/icons/LGA_FolderSwitch.png"));
    }
    QColor tint = color;
    if (paused) {
        tint.setAlphaF(0.4);
    }
    QPainter p(&px);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(px.rect(), tint);
    p.end();
    return px;
}
