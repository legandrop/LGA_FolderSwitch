#include "qa/UiShot.h"

#include "core/AppState.h"
#include "ui/MainWindow.h"
#include "ui/TitleBar.h"
#include "ui/UiWidgets.h"
#include "ui/HelpDialog.h"
#include "ui/RecentFoldersPopup.h"
#include "tray/TrayMenu.h"
#include "updates/UpdateDialog.h"

#include <QAbstractButton>
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFontInfo>
#include <QGuiApplication>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QStringList>
#include <QAbstractButton>
#include <QPixmap>
#include <QSaveFile>
#include <QDateTime>
#include <QDialog>
#include <QHBoxLayout>
#include <QMenu>
#include <QScopedPointer>
#include <QVBoxLayout>

#include <cstdio>

// Modelo: src/qa/uishot.cpp de LGA_VideoDownloader. Nunca se llama show() sobre una ventana de
// nivel superior (WA_DontShowOnScreen + render), asi que no aparece nada ni se toma el foco.
// Lo que NO se construye aca, a proposito: QSystemTrayIcon, TrayController, HotkeyFilter,
// ForegroundWatcher y UpdateService. Uno solo de esos pondria un icono en la bandeja real,
// registraria Ctrl+Alt+O o saldria a la red.

namespace {

const QStringList kStates = {
    QStringLiteral("on"),
    QStringLiteral("empty"),
    QStringLiteral("paused"),
    QStringLiteral("failed"),
    QStringLiteral("hotkey-busy"),
    QStringLiteral("failed-hotkey-busy"),
    QStringLiteral("help"),
    // Hover sin mouse: se marca el widget como "debajo del mouse" (el mismo estado que deja Qt al
    // entrar el cursor) y se dibuja. Prueba la pintura del hover, no que el evento llegue.
    QStringLiteral("hover-help"),
    QStringLiteral("hover-close"),
    QStringLiteral("help-link-hover"),
    QStringLiteral("tray-menu"),
    QStringLiteral("recent-menu"),
    QStringLiteral("recent-menu-empty"),
    QStringLiteral("update-dialog"),
};

const QString kFixturePath =
    QStringLiteral("N:\\Proyectos\\2026_Serie_Ficticia\\Shots\\EP104_SH0230\\Comp\\Renders\\v012\\");

QJsonObject geometryOf(const QWidget *widget, const QWidget *root)
{
    const QPoint topLeft = widget->mapTo(root, QPoint(0, 0));
    return QJsonObject{{QStringLiteral("x"), topLeft.x()},
                       {QStringLiteral("y"), topLeft.y()},
                       {QStringLiteral("w"), widget->width()},
                       {QStringLiteral("h"), widget->height()}};
}

QJsonObject fontOf(const QWidget *widget)
{
    const QFontInfo info(widget->font());
    return QJsonObject{{QStringLiteral("family"), info.family()},
                       {QStringLiteral("weight"), info.weight()},
                       {QStringLiteral("pixelSize"), info.pixelSize()}};
}

void settle(QWidget &root)
{
    // Los layouts de un widget nunca mostrado se resuelven en el primer render: se hacen varias
    // pasadas con los eventos pendientes procesados para no capturar un estado intermedio.
    for (int pass = 0; pass < 3; ++pass) {
        QCoreApplication::sendPostedEvents();
        QPixmap warmup(1, 1);
        root.render(&warmup);
    }
    QCoreApplication::sendPostedEvents();
}

} // namespace

int runUiShot(const QStringList &args)
{
    // Segundo cinturon ademas de run_headless.ps1: este exe viaja en el instalador, y con la
    // plataforma de Windows un error de este camino podria mostrar algo en el escritorio.
    if (QGuiApplication::platformName() != QLatin1String("offscreen")) {
        fprintf(stderr, "ui-shot: requires QT_QPA_PLATFORM=offscreen (platform is '%s')\n",
                qPrintable(QGuiApplication::platformName()));
        return 2;
    }
    const int index = args.indexOf(QStringLiteral("--ui-shot"));
    if (index < 0 || index + 2 >= args.size()) {
        fprintf(stderr, "usage: --ui-shot <%s> <out.png> [--dpr <1..3>]\n", qPrintable(kStates.join('|')));
        return 2;
    }
    const QString state = args.at(index + 1);
    const QString outPath = QFileInfo(args.at(index + 2)).absoluteFilePath();
    qreal dpr = 1.0;
    const int dprIndex = args.indexOf(QStringLiteral("--dpr"));
    if (dprIndex >= 0) {
        bool ok = false;
        dpr = args.value(dprIndex + 1).toDouble(&ok);
        if (!ok || dpr < 1.0 || dpr > 3.0) {
            fprintf(stderr, "ui-shot: invalid --dpr\n");
            return 2;
        }
    }
    if (!kStates.contains(state)) {
        fprintf(stderr, "ui-shot: unknown state '%s'\n", qPrintable(state));
        return 2;
    }
    if (!outPath.endsWith(QLatin1String(".png"), Qt::CaseInsensitive) || QFileInfo::exists(outPath)
        || !QFileInfo(outPath).dir().exists()) {
        fprintf(stderr, "ui-shot: output must be a new .png in an existing folder\n");
        return 2;
    }

    // AppState sin persistencia: nunca lee ni escribe QSettings. Todo el estado es del fixture.
    AppState appState(AppState::Persistence::None);
    AppState::LastSwitch last;
    last.path = kFixturePath;
    last.source = QStringLiteral("Explorer");
    last.applied = true;
    last.when = QDateTime(QDate(2026, 9, 18), QTime(12, 41));
    if (state == QLatin1String("paused")) {
        appState.setEnabled(false);
    } else if (state == QLatin1String("failed")) {
        last.source = QStringLiteral("XYplorer");
        last.applied = false;
    } else if (state == QLatin1String("hotkey-busy")) {
        appState.setHotkeyRegistered(false);
        appState.setRecentHotkeyRegistered(false);
    } else if (state == QLatin1String("failed-hotkey-busy")) {
        // Fallo con el atajo sin registrar: el consejo no puede ser "reintentar con el atajo".
        last.source = QStringLiteral("XYplorer");
        last.applied = false;
        appState.setHotkeyRegistered(false);
    }
    if (state != QLatin1String("empty")) {
        appState.setLastSwitch(last);
    }

    MainWindow mainWindow(&appState, MainWindow::Mode::Capture);
    mainWindow.setAttribute(Qt::WA_DontShowOnScreen, true);
    mainWindow.applyAutoStartFixture(false, true);
    mainWindow.refresh();
    settle(mainWindow);
    mainWindow.refresh();
    settle(mainWindow);

    // Lo que se dibuja: la ventana de Settings, o un lienzo aparte para el menu del tray y el
    // dialogo de update (que en la app son ventanas propias).
    QWidget *root = &mainWindow;
    QScopedPointer<QWidget> canvas;
    if (state == QLatin1String("hover-help") || state == QLatin1String("hover-close")) {
        const QString name = state == QLatin1String("hover-help") ? QStringLiteral("Help") : QStringLiteral("Close");
        for (QAbstractButton *button : mainWindow.titleBar()->findChildren<QAbstractButton *>()) {
            if (button->accessibleName() == name) {
                button->setAttribute(Qt::WA_UnderMouse, true);
                button->update();
            }
        }
        settle(mainWindow);
    } else if (state == QLatin1String("help") || state == QLatin1String("help-link-hover")) {
        // Velo y dialogo como hijos comunes de la ventana, no ventanas propias: se dibujan con el
        // mismo render y no hay nada que mostrar.
        auto *scrim = new Scrim(mainWindow.centralWidget());
        scrim->setVisible(true);
        auto *help = new HelpDialog(mainWindow.centralWidget());
        help->setWindowFlags(Qt::Widget);
        help->fitHeight();
        help->move((mainWindow.width() - help->width()) / 2, (mainWindow.height() - help->height()) / 2);
        help->setVisible(true);
        if (state == QLatin1String("help-link-hover")) {
            if (auto *link = help->findChild<QLabel *>(QStringLiteral("helpLink"))) {
                Ui::setStyleProperty(link, "hover", true);
            }
        }
        settle(mainWindow);
    } else if (state == QLatin1String("tray-menu")) {
        canvas.reset(new QWidget);
        canvas->setObjectName(QStringLiteral("central"));
        canvas->setAttribute(Qt::WA_DontShowOnScreen, true);
        canvas->setStyleSheet(QStringLiteral("QWidget#central { background-color: #101010; }"));
        auto *layout = new QVBoxLayout(canvas.data());
        layout->setContentsMargins(20, 16, 20, 20);
        layout->setSpacing(14);
        // Iconos de la bandeja On y Paused, como los pinta TrayController (sin QSystemTrayIcon).
        auto *icons = new QHBoxLayout();
        icons->setSpacing(18);
        for (const bool paused : {false, true}) {
            auto *icon = new QLabel(canvas.data());
            icon->setObjectName(paused ? QStringLiteral("trayIconPaused") : QStringLiteral("trayIconOn"));
            // pixmap(size, dpr) toma el PNG del tamano fisico que pide la bandeja real (16 * dpr).
            QPixmap px = trayIcon(false, paused).pixmap(QSize(16, 16), dpr);
            icon->setPixmap(px);
            icons->addWidget(icon);
        }
        icons->addStretch(1);
        layout->addLayout(icons);
        auto *menu = new QMenu(canvas.data());
        menu->setWindowFlags(Qt::Widget);
        const TrayMenuActions actions = buildTrayMenu(menu);
        refreshTrayMenu(actions, true);
        menu->setActiveAction(actions.toggle);
        layout->addWidget(menu);
        root = canvas.data();
        settle(*root);
        root->adjustSize();
        settle(*root);
    } else if (state == QLatin1String("recent-menu") || state == QLatin1String("recent-menu-empty")) {
        // El menu de Ctrl+Alt+Shift+O sobre un fondo oscuro, como se ve encima de un dialogo.
        QStringList folders;
        if (state == QLatin1String("recent-menu")) {
            folders = {kFixturePath,
                       QStringLiteral("N:\\Proyectos\\2026_Serie_Ficticia\\Shots\\EP104_SH0230\\Comp\\Nuke\\"),
                       QStringLiteral("D:\\Descargas\\Referencias & moodboard\\"),
                       QStringLiteral("C:\\Users\\Public\\Documents\\"),
                       QStringLiteral("N:\\Proyectos\\2026_Serie_Ficticia\\Entregas\\Semana 38\\")};
        }
        canvas.reset(new QWidget);
        canvas->setObjectName(QStringLiteral("central"));
        canvas->setAttribute(Qt::WA_DontShowOnScreen, true);
        canvas->setStyleSheet(QStringLiteral("QWidget#central { background-color: #101010; }"));
        auto *layout = new QVBoxLayout(canvas.data());
        layout->setContentsMargins(0, 0, 0, 0);
        auto *popup = new RecentFoldersPopup(folders, canvas.data());
        popup->setWindowFlags(Qt::Widget);
        // Mouse sobre la segunda fila, como el mockup.
        popup->setCurrentIndex(folders.size() > 1 ? 1 : -1);
        layout->addWidget(popup);
        root = canvas.data();
        settle(*root);
        root->adjustSize();
        settle(*root);
    } else if (state == QLatin1String("update-dialog")) {
        canvas.reset(createUpdateAvailableDialog(nullptr, QStringLiteral("LGA FolderSwitch"), QStringLiteral("0.11"),
                                                 QStringLiteral(FOLDERSWITCH_VERSION)));
        canvas->setAttribute(Qt::WA_DontShowOnScreen, true);
        root = canvas.data();
        settle(*root);
        root->adjustSize();
        settle(*root);
    }
    QWidget &window = *root;

    const QSize logical = window.size();
    QPixmap pixmap(logical * dpr);
    pixmap.setDevicePixelRatio(dpr);
    // Magenta: si algo queda sin pintar, salta a la vista en vez de confundirse con el fondo.
    pixmap.fill(Qt::magenta);
    window.render(&pixmap, QPoint(), QRegion(), QWidget::DrawWindowBackground | QWidget::DrawChildren);

    const QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB32);
    if (!image.save(outPath, "PNG")) {
        fprintf(stderr, "ui-shot: could not save %s\n", qPrintable(outPath));
        return 1;
    }
    const QImage check(outPath);
    if (check.size() != logical * dpr) {
        fprintf(stderr, "ui-shot: saved image has unexpected size\n");
        return 1;
    }

    // Guarda de fuente: sin Inter la captura no sirve de evidencia (todo sale "un poco distinto").
    const QFontInfo windowFont(window.font());
    const bool fontOk = windowFont.family() == QLatin1String("Inter");

    QJsonObject descriptor;
    descriptor.insert(QStringLiteral("state"), state);
    descriptor.insert(QStringLiteral("dpr"), dpr);
    descriptor.insert(QStringLiteral("logical"), QJsonArray{logical.width(), logical.height()});
    descriptor.insert(QStringLiteral("physical"), QJsonArray{check.width(), check.height()});
    descriptor.insert(QStringLiteral("pid"), qint64(QCoreApplication::applicationPid()));
    descriptor.insert(QStringLiteral("version"), QStringLiteral(FOLDERSWITCH_VERSION));
    descriptor.insert(QStringLiteral("platform"), QGuiApplication::platformName());
    descriptor.insert(QStringLiteral("fixture"), true);
    descriptor.insert(QStringLiteral("windowFont"), fontOf(&window));
    QJsonArray tree;
    for (QWidget *widget : window.findChildren<QWidget *>()) {
        if (!widget->isVisibleTo(&window)) {
            continue;
        }
        QJsonObject entry = geometryOf(widget, &window);
        entry.insert(QStringLiteral("class"), QString::fromLatin1(widget->metaObject()->className()));
        entry.insert(QStringLiteral("name"), widget->objectName());
        if (auto *label = qobject_cast<QLabel *>(widget)) {
            entry.insert(QStringLiteral("text"), label->text().left(80));
            entry.insert(QStringLiteral("font"), fontOf(widget));
        } else if (auto *elided = qobject_cast<ElidedLabel *>(widget)) {
            entry.insert(QStringLiteral("text"), elided->text());
            entry.insert(QStringLiteral("shown"), elided->shownText());
            entry.insert(QStringLiteral("font"), fontOf(widget));
        } else if (auto *button = qobject_cast<QAbstractButton *>(widget)) {
            entry.insert(QStringLiteral("text"), button->text().left(80));
            entry.insert(QStringLiteral("checked"), button->isChecked());
            entry.insert(QStringLiteral("font"), fontOf(widget));
        }
        tree.append(entry);
    }
    descriptor.insert(QStringLiteral("widgets"), tree);

    QSaveFile json(outPath + QStringLiteral(".json"));
    if (!json.open(QIODevice::WriteOnly) || json.write(QJsonDocument(descriptor).toJson()) < 0 || !json.commit()) {
        fprintf(stderr, "ui-shot: could not write the .json descriptor\n");
        return 1;
    }
    if (!fontOk) {
        fprintf(stderr, "ui-shot: font resolved to '%s', expected Inter\n", qPrintable(windowFont.family()));
        return 1;
    }
    fprintf(stdout, "ui-shot ok state=%s size=%dx%d dpr=%.2f font=%s\n", qPrintable(state), logical.width(),
            logical.height(), dpr, qPrintable(windowFont.family()));
    return 0;
}
