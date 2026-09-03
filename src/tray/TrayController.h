#ifndef FOLDERSWITCH_TRAYCONTROLLER_H
#define FOLDERSWITCH_TRAYCONTROLLER_H

#include <QObject>
#include <QString>

#include <windows.h>

class QSystemTrayIcon;
class QMenu;
class MainWindow;
class ForegroundWatcher;
class HotkeyFilter;
class UpdateService;

// Maneja el icono de la bandeja del sistema, la ventana de Settings, y la logica
// central de deteccion (Explorer/XYplorer -> file dialog) + inyeccion de path.
class TrayController : public QObject
{
    Q_OBJECT

public:
    explicit TrayController(QObject *parent = nullptr);
    ~TrayController() override;

    bool isAvailable() const;
    void showWarning(const QString &title, const QString &message);

public slots:
    void showSettings();

private slots:
    void onTrayActivated(int reason);
    void quit();
    void onForegroundChanged(quintptr hwnd);
    void onHotkeyPressed();
    void checkForUpdatesManual();

private:
    enum class ManagerType { None, Explorer, XYplorer };

    void applyTrayIcon();
    // Primer arranque de una copia INSTALADA: activa el inicio con Windows una sola
    // vez y abre Settings para que se vea. Desde build/ o deploy/ no hace nada.
    void runFirstLaunchSetupIfNeeded();
    // Resuelve el path fresco del ultimo manager guardado (si sigue vivo).
    QString resolveLastManagerPath() const;
    // Programa (con delay) la inyeccion de path en el dialogo dado.
    void scheduleSwitch(HWND dialogHwnd, int delayMs);
    void performSwitch(HWND dialogHwnd);

    QSystemTrayIcon *m_tray = nullptr;
    QMenu *m_menu = nullptr;
    MainWindow *m_window = nullptr;
    ForegroundWatcher *m_foregroundWatcher = nullptr;
    HotkeyFilter *m_hotkeyFilter = nullptr;
    UpdateService *m_updateService = nullptr;

    // Estado del ultimo manager (Explorer/XYplorer) visto en foreground.
    HWND m_lastManagerHwnd = nullptr;
    ManagerType m_lastManagerType = ManagerType::None;
    qint64 m_lastManagerSeenMs = 0;

    // Ultimo file dialog visto en foreground (aunque despues pierda el foco).
    HWND m_lastDialogHwnd = nullptr;

    // Dialogo al que hay que volver a inyectar: se marca cuando el usuario pasa
    // del dialogo al manager; sobrevive a ventanas intermedias (Alt+Tab, etc.).
    HWND m_pendingReturnDialog = nullptr;

    // Ultimo dialog al que ya se le inyecto path, para no re-inyectar en el mismo
    // HWND dos veces seguidas (se resetea cuando volvemos a ver el manager).
    HWND m_lastSwitchedDialogHwnd = nullptr;
};

#endif // FOLDERSWITCH_TRAYCONTROLLER_H
