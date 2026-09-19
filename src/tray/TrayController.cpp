#include "tray/TrayController.h"
#include "ui/MainWindow.h"
#include "core/AppState.h"
#include "core/AppSettings.h"
#include "tray/TrayMenu.h"
#include "ui/HelpDialog.h"
#include "ui/RecentFoldersPopup.h"
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
#include <QCursor>
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
    m_state = new AppState(AppState::Persistence::Settings, this);
    m_window = new MainWindow(m_state, MainWindow::Mode::Normal);

    m_menu = new QMenu();
    m_menuActions = buildTrayMenu(m_menu);
    connect(m_menuActions.toggle, &QAction::triggered, this, [this]() { m_state->setEnabled(!m_state->enabled()); });
    connect(m_menuActions.settings, &QAction::triggered, this, &TrayController::showSettings);
    connect(m_menuActions.updates, &QAction::triggered, this, &TrayController::checkForUpdatesManual);
    connect(m_menuActions.quit, &QAction::triggered, this, &TrayController::quit);
    connect(m_window, &MainWindow::helpRequested, this, &TrayController::showHelp);

    m_tray = new QSystemTrayIcon(this);
    refreshFromState();
    connect(m_state, &AppState::changed, this, &TrayController::refreshFromState);
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
    m_state->setHotkeyRegistered(m_hotkeyFilter->isRegistered());
    m_state->setRecentHotkeyRegistered(m_hotkeyFilter->isRecentRegistered());
    connect(m_hotkeyFilter, &HotkeyFilter::hotkeyPressed, this, &TrayController::onHotkeyPressed);
    connect(m_hotkeyFilter, &HotkeyFilter::recentHotkeyPressed, this, &TrayController::onRecentHotkeyPressed);
    QCoreApplication::instance()->installNativeEventFilter(m_hotkeyFilter);

    // parentWindow es m_window (normalmente oculto): sus dialogos de resultado
    // igual se centran en pantalla al no tener un padre visible.
    m_updateService = new UpdateService(m_window, this);
    if (m_state->checkUpdatesAtStartup()) {
        m_updateService->scheduleAutomaticCheck();
    } else {
        qInfo() << "[TrayController] Chequeo de updates al arrancar: desactivado por el usuario";
    }
    connect(m_window, &MainWindow::checkUpdatesRequested, this, &TrayController::checkForUpdatesManual);

    runFirstLaunchSetupIfNeeded();
}

// Copia de MainWindow::runFirstLaunchSetupIfNeeded() de FrameRev: el "se abre con Windows
// por defecto" se activa DE VERDAD en el primer arranque de una copia instalada, y no lo
// escribe el instalador. Cuando lo escribia el instalador (con el MISMO nombre de valor que
// usa la app) pisaba la entrada de la copia de build/ y la borraba al desinstalar; asi se
// perdio el inicio con Windows de esta app. La marca se guarda SOLO cuando corre una copia
// instalada: un arranque desde build/ no consume el primer arranque de la instalacion futura.
void TrayController::runFirstLaunchSetupIfNeeded()
{
    const auto settings = AppSettings::open();
    if (settings->value(QStringLiteral("firstRunCompleted"), false).toBool()) {
        return;
    }

    const AutoStart::Availability availability = AutoStart::availability();
    if (!availability.available) {
        // Desde una salida de desarrollo no se toca el registro NI se consume el primer
        // arranque: la instalacion futura tiene que conservarlo.
        qInfo() << "[TrayController] Primer arranque: no se activa el inicio automatico ("
                << availability.text << ")";
        return;
    }

    settings->setValue(QStringLiteral("firstRunCompleted"), true);
    const bool ok = AutoStart::setEnabled(true);
    qInfo() << "[TrayController] Primer arranque: inicio con Windows activado ok:" << ok;
    showSettings();
}

void TrayController::applyTrayIcon()
{
    const QColor barColor = systemBarIsLight() ? QColor(Qt::black) : QColor(Qt::white);
    m_tray->setIcon(QIcon(trayIconPixmap(barColor, !m_state->enabled())));
}

void TrayController::refreshFromState()
{
    // Menu, icono y tooltip leen el mismo AppState que la tarjeta de estado de Settings.
    const bool enabled = m_state->enabled();
    refreshTrayMenu(m_menuActions, enabled);
    applyTrayIcon();
    m_tray->setToolTip(enabled ? QStringLiteral("LGA FolderSwitch") : QStringLiteral("LGA FolderSwitch — paused"));
}

void TrayController::showHelp()
{
    HelpDialog dialog(m_window);
    dialog.execOver(m_window);
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
    applyFolder(dialogHwnd, path,
                m_lastManagerType == ManagerType::XYplorer ? QStringLiteral("XYplorer") : QStringLiteral("Explorer"));
}

void TrayController::applyFolder(HWND dialogHwnd, const QString &path, const QString &source)
{
    const bool ok = DialogSwitcher::switchDialog(dialogHwnd, path);
    AppState::LastSwitch last;
    last.path = path;
    last.source = source;
    last.applied = ok;
    last.when = QDateTime::currentDateTime();
    m_state->setLastSwitch(last);
    m_state->addRecentFolder(path);
    qDebug() << "[TrayController] switchDialog" << (ok ? "OK" : "FALLO") << "path=" << path << "source=" << source;
}

void TrayController::recordManagerFolder(HWND managerHwnd, ManagerType type)
{
    // Diferido: el foreground llega desde el hook de WinEvent, y resolver la carpeta de Explorer
    // llama a COM. Se hace en el loop normal, ya fuera del callback.
    QTimer::singleShot(0, this, [this, managerHwnd, type]() {
        if (!IsWindow(managerHwnd)) {
            return;
        }
        const QString path = type == ManagerType::XYplorer ? FolderResolver::resolveXYplorerPath(managerHwnd)
                                                           : FolderResolver::resolveExplorerPath(managerHwnd);
        if (!path.isEmpty()) {
            m_state->addRecentFolder(path);
            qDebug() << "[TrayController] Carpeta reciente:" << path;
        }
    });
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

    // Historial: la carpeta que queda en un manager cuando el usuario se va de el.
    if (m_prevManagerHwnd && m_prevManagerHwnd != hwnd) {
        recordManagerFolder(m_prevManagerHwnd, m_prevManagerType);
        m_prevManagerHwnd = nullptr;
        m_prevManagerType = ManagerType::None;
    }

    if (WindowUtils::isFileManagerWindow(hwnd)) {
        const bool isExplorer = WindowUtils::isExplorerWindow(hwnd);
        qDebug() << "[TrayController] Foreground:" << (isExplorer ? "Explorer" : "XYplorer") << hwnd;
        m_lastManagerHwnd = hwnd;
        m_lastManagerType = isExplorer ? ManagerType::Explorer : ManagerType::XYplorer;
        m_lastManagerSeenMs = QDateTime::currentMSecsSinceEpoch();
        m_lastSwitchedDialogHwnd = nullptr;
        m_prevManagerHwnd = hwnd;
        m_prevManagerType = m_lastManagerType;
        // Si el usuario venia de un dialogo que sigue vivo, al volver a ESE
        // dialogo hay que inyectar. Inmune a ventanas intermedias (Alt+Tab).
        if (m_lastDialogHwnd && IsWindow(m_lastDialogHwnd)) {
            m_pendingReturnDialog = m_lastDialogHwnd;
            qDebug() << "[TrayController] Marcado dialogo pendiente de retorno:" << m_pendingReturnDialog;
        }
    } else if (WindowUtils::isFileDialogWindow(hwnd) || WindowUtils::isQtFileDialog(hwnd)) {
        qDebug() << "[TrayController] Foreground: file dialog" << hwnd;
        m_lastDialogHwnd = hwnd;

        const bool autoSwitchOn = m_state->autoSwitch();
        const bool masterOn = m_state->enabled();
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
    // Con el menu de recientes abierto, lo que el usuario elija ahi es la carpeta que manda.
    if (m_recentMenuOpen) {
        return;
    }
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

void TrayController::onRecentHotkeyPressed()
{
    // Como Ctrl+Alt+O: solo dentro de un file dialog, y tambien con el cambio automatico en pausa.
    HWND dialog = GetForegroundWindow();
    if (!dialog || !(WindowUtils::isFileDialogWindow(dialog) || WindowUtils::isQtFileDialog(dialog))) {
        qDebug() << "[TrayController] Recientes: la ventana en foreground no es un file dialog.";
        return;
    }
    if (m_recentMenuOpen) {
        return;
    }
    m_recentMenuOpen = true;

    RecentFoldersPopup popup(m_state->recentFolders());
    const QString path = popup.exec(QCursor::pos());
    m_recentMenuOpen = false;

    if (path.isEmpty() || !IsWindow(dialog)) {
        return;
    }
    // Devolverle el foco al dialogo antes de escribirle la ruta, con la misma demora que el cambio
    // automatico.
    SetForegroundWindow(dialog);
    QTimer::singleShot(kSwitchDelayMs, this, [this, dialog, path]() {
        if (IsWindow(dialog)) {
            applyFolder(dialog, path, QStringLiteral("Recent"));
        }
    });
}
