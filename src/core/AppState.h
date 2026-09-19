#ifndef FOLDERSWITCH_APPSTATE_H
#define FOLDERSWITCH_APPSTATE_H

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QStringList>

// Estado de la app que ve el usuario: la UNICA fuente de verdad para On/Paused (clave `enabled`),
// el auto-switch, el chequeo de updates al arrancar, si los atajos quedaron registrados, el ultimo
// cambio de carpeta y las carpetas recientes. La tarjeta de estado, el menu del tray, su encabezado
// y el icono atenuado leen de aca y escriben aca: asi no pueden quedar dos controles diciendo cosas
// distintas.
//
// Persistence::Settings lee y escribe %APPDATA%\LGA\LGA_FolderSwitch\settings.ini (AppSettings).
// Persistence::None no toca QSettings nunca: es el de la captura --ui-shot.
class AppState : public QObject
{
    Q_OBJECT

public:
    enum class Persistence { Settings, None };

    struct LastSwitch
    {
        QString path;
        QString source;  // "Explorer" o "XYplorer"
        bool applied = false;
        QDateTime when;
        bool isValid() const { return !path.isEmpty(); }
    };

    explicit AppState(Persistence persistence, QObject *parent = nullptr);

    bool enabled() const { return m_enabled; }
    bool autoSwitch() const { return m_autoSwitch; }
    // Si el chequeo de updates corre solo al arrancar. Se lee una vez, en el arranque: cambiarlo
    // vale desde el proximo inicio (el boton "Check now" busca en el momento).
    bool checkUpdatesAtStartup() const { return m_checkUpdatesAtStartup; }
    bool hotkeyRegistered() const { return m_hotkeyRegistered; }
    // Ctrl+Alt+Shift+O, el menu de carpetas recientes.
    bool recentHotkeyRegistered() const { return m_recentHotkeyRegistered; }
    // Ultimas carpetas de Explorer/XYplorer, la mas reciente primero, sin repetidas.
    QStringList recentFolders() const { return m_recentFolders; }
    static constexpr int kMaxRecentFolders = 5;
    LastSwitch lastSwitch() const { return m_lastSwitch; }

    // Cada setter escribe (si corresponde) y avisa SOLO si el valor cambio.
    void setEnabled(bool enabled);
    void setAutoSwitch(bool autoSwitch);
    void setCheckUpdatesAtStartup(bool check);
    void setHotkeyRegistered(bool registered);
    void setRecentHotkeyRegistered(bool registered);
    // La pone primera (si ya estaba, la mueve) y recorta a kMaxRecentFolders. No emite changed():
    // la ventana no muestra la lista, solo la lee el menu cuando se abre.
    void addRecentFolder(const QString &path);
    void setLastSwitch(const LastSwitch &lastSwitch);

signals:
    void changed();

private:
    void write(const char *key, bool value);

    Persistence m_persistence;
    bool m_enabled = true;
    bool m_autoSwitch = true;
    bool m_checkUpdatesAtStartup = true;
    bool m_hotkeyRegistered = true;
    bool m_recentHotkeyRegistered = true;
    QStringList m_recentFolders;
    LastSwitch m_lastSwitch;
};

#endif // FOLDERSWITCH_APPSTATE_H
