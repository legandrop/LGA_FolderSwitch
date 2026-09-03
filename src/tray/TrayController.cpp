#include "tray/TrayController.h"
#include "ui/MainWindow.h"
#include "core/ForegroundWatcher.h"
#include "core/HotkeyFilter.h"
#include "core/WindowUtils.h"
#include "core/FolderResolver.h"
#include "core/DialogSwitcher.h"
#include "updates/UpdateService.h"
#include "windows/AutoStart.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QDateTime>
#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QSettings>
#include <QStyleHints>
#include <QSystemTrayIcon>
#include <QTimer>

namespace {

constexpr qint64 kManagerFreshnessMs = 60000; // 60 s
constexpr int kSwitchDelayMs = 200;

QIcon tintedTrayIcon(const QColor &color)
{
    QPixmap px(QStringLiteral(":/icons/LGA_FolderSwitch_menubar.png"));
    if (px.isNull()) {
        px = QPixmap(QStringLiteral(":/icons/LGA_FolderSwitch.png"));
    }
    QPainter p(&px);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(px.rect(), color);
    p.end();
    return QIcon(px);
}

// True si la barra de tareas / area de notificacion es CLARA.
bool systemBarIsLight()
{
    const QSettings personalize(
        QStringLiteral(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)"),
        QSettings::NativeFormat);
    return personalize.value(QStringLiteral("SystemUsesLightTheme"), 0).toInt() != 0;
}

} // namespace

TrayController::TrayController(QObject *parent)
    : QObject(parent)
{
    m_window = new MainWindow();
    connect(m_window, &MainWindow::autoSwitchToggled, this, [](bool checked) {
        qDebug() << "[TrayController] Auto-switch:" << checked;
    });
    connect(m_window, &MainWindow::masterEnabledToggled, this, [](bool checked) {
        qDebug() << "[TrayController] Activado:" << checked;
    });

    m_menu = new QMenu();
    QAction *openAction = m_menu->addAction(QStringLiteral("Open Settings"));
    m_menu->addSeparator();
    QAction *checkForUpdatesAction = m_menu->addAction(QStringLiteral("Check for Updates..."));
    m_menu->addSeparator();
    QAction *quitAction = m_menu->addAction(QStringLiteral("Quit"));
    connect(openAction, &QAction::triggered, this, &TrayController::showSettings);
    connect(checkForUpdatesAction, &QAction::triggered, this, &TrayController::checkForUpdatesManual);
    connect(quitAction, &QAction::triggered, this, &TrayController::quit);

    m_tray = new QSystemTrayIcon(this);
    applyTrayIcon();
    m_tray->setToolTip(QStringLiteral("LGA FolderSwitch"));
    m_tray->setContextMenu(m_menu);
    connect(m_tray, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
                onTrayActivated(static_cast<int>(reason));
            });
    m_tray->show();

    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
            [this](Qt::ColorScheme) { applyTrayIcon(); });

    m_foregroundWatcher = new ForegroundWatcher(this);
    connect(m_foregroundWatcher, &ForegroundWatcher::foregroundChanged,
            this, &TrayController::onForegroundChanged);

    m_hotkeyFilter = new HotkeyFilter(this);
    connect(m_hotkeyFilter, &HotkeyFilter::hotkeyPressed, this, &TrayController::onHotkeyPressed);
    QCoreApplication::instance()->installNativeEventFilter(m_hotkeyFilter);

    // parentWindow es m_window (normalmente oculto): sus dialogos de resultado
    // igual se centran en pantalla al no tener un padre visible.
    m_updateService = new UpdateService(m_window, this);
    m_updateService->scheduleAutomaticCheck();

    runFirstLaunchSetupIfNeeded();
}

// Antes esto lo hacia el instalador escribiendo la clave Run con la ruta instalada.
// Con el MISMO nombre de valor que usa la app, pisaba la entrada de la copia de
// build/ y la borraba al desinstalar: asi se perdio el inicio con Windows. Ahora la
// entrada la maneja solo la app (LinkRedirector nunca dejo que el instalador la
// toque) y el "activado por defecto al instalar" pasa a este primer arranque, como
// en FrameRev. La marca se guarda SOLO cuando corre una copia instalada: un
// arranque desde build/ no consume el primer arranque de la instalacion futura.
void TrayController::runFirstLaunchSetupIfNeeded()
{
    if (AutoStart::runsFromDevelopmentTree()) {
        return;
    }
    QSettings settings(QStringLiteral("LGA"), QStringLiteral("FolderSwitch"));
    if (settings.value(QStringLiteral("firstRunCompleted"), false).toBool()) {
        return;
    }
    settings.setValue(QStringLiteral("firstRunCompleted"), true);

    const bool ok = AutoStart::setEnabled(true);
    qInfo() << "[TrayController] Primer arranque: inicio con Windows activado ok:" << ok;
    showSettings();
}

void TrayController::applyTrayIcon()
{
    const QIcon trayIcon = tintedTrayIcon(systemBarIsLight() ? Qt::black : Qt::white);
    m_tray->setIcon(trayIcon);
}

TrayController::~TrayController()
{
    if (m_hotkeyFilter) {
        QCoreApplication::instance()->removeNativeEventFilter(m_hotkeyFilter);
    }
    delete m_menu;
    delete m_window;
}

bool TrayController::isAvailable() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

void TrayController::showWarning(const QString &title, const QString &message)
{
    if (m_tray) {
        m_tray->showMessage(title, message, QSystemTrayIcon::Warning, 10000);
    }
}

void TrayController::showSettings()
{
    if (!m_window) {
        return;
    }
    m_window->show();
    m_window->raise();
    m_window->activateWindow();
}

void TrayController::onTrayActivated(int reason)
{
    if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger) {
        showSettings();
    }
}

void TrayController::quit()
{
    qApp->quit();
}

void TrayController::checkForUpdatesManual()
{
    if (m_updateService) {
        m_updateService->checkForUpdates(true);
    }
}

QString TrayController::resolveLastManagerPath() const
{
    if (!m_lastManagerHwnd || !IsWindow(m_lastManagerHwnd)) {
        return QString();
    }
    if (m_lastManagerType == ManagerType::Explorer) {
        return FolderResolver::resolveExplorerPath(m_lastManagerHwnd);
    }
    if (m_lastManagerType == ManagerType::XYplorer) {
        return FolderResolver::resolveXYplorerPath(m_lastManagerHwnd);
    }
    return QString();
}

void TrayController::performSwitch(HWND dialogHwnd)
{
    if (!dialogHwnd || !IsWindow(dialogHwnd)) {
        qDebug() << "[TrayController] Dialog ya no existe, se cancela el switch.";
        return;
    }
    const QString path = resolveLastManagerPath();
    if (path.isEmpty()) {
        qDebug() << "[TrayController] No se pudo resolver el path del manager guardado.";
        return;
    }
    if (m_window) {
        m_window->setLastDetectedFolder(path);
    }
    const bool ok = DialogSwitcher::switchDialog(dialogHwnd, path);
    qDebug() << "[TrayController] switchDialog" << (ok ? "OK" : "FALLO") << "path=" << path;
}

void TrayController::scheduleSwitch(HWND dialogHwnd, int delayMs)
{
    QTimer::singleShot(delayMs, this, [this, dialogHwnd]() {
        performSwitch(dialogHwnd);
    });
}

void TrayController::onForegroundChanged(quintptr hwndValue)
{
    HWND hwnd = reinterpret_cast<HWND>(hwndValue);
    if (!hwnd) {
        return;
    }

    if (WindowUtils::isFileManagerWindow(hwnd)) {
        const bool isExplorer = WindowUtils::isExplorerWindow(hwnd);
        qDebug() << "[TrayController] Foreground:" << (isExplorer ? "Explorer" : "XYplorer") << hwnd;
        m_lastManagerHwnd = hwnd;
        m_lastManagerType = isExplorer ? ManagerType::Explorer : ManagerType::XYplorer;
        m_lastManagerSeenMs = QDateTime::currentMSecsSinceEpoch();
        m_lastSwitchedDialogHwnd = nullptr;
        // Si el usuario venia de un dialogo que sigue vivo, al volver a ESE
        // dialogo hay que inyectar. Inmune a ventanas intermedias (Alt+Tab).
        if (m_lastDialogHwnd && IsWindow(m_lastDialogHwnd)) {
            m_pendingReturnDialog = m_lastDialogHwnd;
            qDebug() << "[TrayController] Marcado dialogo pendiente de retorno:" << m_pendingReturnDialog;
        }
    } else if (WindowUtils::isFileDialogWindow(hwnd) || WindowUtils::isQtFileDialog(hwnd)) {
        qDebug() << "[TrayController] Foreground: file dialog" << hwnd;
        m_lastDialogHwnd = hwnd;

        const bool autoSwitchOn = m_window && m_window->autoSwitchEnabled();
        const bool masterOn = m_window && m_window->masterEnabled();
        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        const bool managerFresh = m_lastManagerHwnd &&
                                  (nowMs - m_lastManagerSeenMs) < kManagerFreshnessMs;
        const bool isPendingReturn = (hwnd == m_pendingReturnDialog);
        const bool alreadySwitchedThisDialog = (hwnd == m_lastSwitchedDialogHwnd);

        if (autoSwitchOn && masterOn && managerFresh && isPendingReturn &&
            !alreadySwitchedThisDialog) {
            m_pendingReturnDialog = nullptr;
            m_lastSwitchedDialogHwnd = hwnd;
            scheduleSwitch(hwnd, kSwitchDelayMs);
        } else if (!alreadySwitchedThisDialog) {
            qDebug() << "[TrayController] Sin auto-switch: auto=" << autoSwitchOn
                     << "master=" << masterOn << "fresh=" << managerFresh
                     << "pendingReturn=" << isPendingReturn;
        }
    }
}

void TrayController::onHotkeyPressed()
{
    HWND fg = GetForegroundWindow();
    if (!fg || !(WindowUtils::isFileDialogWindow(fg) || WindowUtils::isQtFileDialog(fg))) {
        qDebug() << "[TrayController] Hotkey: la ventana en foreground no es un file dialog.";
        return;
    }
    if (!m_lastManagerHwnd || !IsWindow(m_lastManagerHwnd)) {
        qDebug() << "[TrayController] Hotkey: no hay manager guardado valido.";
        return;
    }
    performSwitch(fg);
}
