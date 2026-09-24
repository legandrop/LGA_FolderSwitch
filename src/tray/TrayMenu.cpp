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

QIcon trayIcon(bool lightBar, bool paused)
{
    if (!paused) {
        // Un PNG por tamano: el desfase de las planchas cae en pixeles enteros en cada uno, y QIcon
        // elige el que corresponde a la escala de la pantalla.
        const QString body = lightBar ? QStringLiteral("dark") : QStringLiteral("white");
        QIcon icon;
        for (int px : {16, 20, 24, 32, 40, 48}) {
            icon.addFile(QStringLiteral(":/icons/tray/tray_%1_%2.png").arg(body).arg(px), QSize(px, px));
        }
        return icon;
    }
    QPixmap px(QStringLiteral(":/icons/LGA_FolderSwitch_menubar.png"));
    if (px.isNull()) {
        px = QPixmap(QStringLiteral(":/icons/LGA_FolderSwitch.png"));
    }
    QColor tint = lightBar ? QColor(Qt::black) : QColor(Qt::white);
    tint.setAlphaF(0.4);
    QPainter p(&px);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(px.rect(), tint);
    p.end();
    return QIcon(px);
}
