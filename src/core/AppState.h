#ifndef FOLDERSWITCH_APPSTATE_H
#define FOLDERSWITCH_APPSTATE_H

#include <QDateTime>
#include <QObject>
#include <QString>

// Estado de la app que ve el usuario: la UNICA fuente de verdad para On/Paused (clave `enabled`),
// el auto-switch, si el hotkey quedo registrado y el ultimo cambio de carpeta. La tarjeta de
// estado, el menu del tray, su encabezado y el icono atenuado leen de aca y escriben aca: asi no
// pueden quedar dos controles diciendo cosas distintas.
//
// Persistence::Settings lee y escribe HKCU\Software\LGA\FolderSwitch (mismas claves de siempre).
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
    bool hotkeyRegistered() const { return m_hotkeyRegistered; }
    LastSwitch lastSwitch() const { return m_lastSwitch; }

    // Cada setter escribe (si corresponde) y avisa SOLO si el valor cambio.
    void setEnabled(bool enabled);
    void setAutoSwitch(bool autoSwitch);
    void setHotkeyRegistered(bool registered);
    void setLastSwitch(const LastSwitch &lastSwitch);

signals:
    void changed();

private:
    void write(const char *key, bool value);

    Persistence m_persistence;
    bool m_enabled = true;
    bool m_autoSwitch = true;
    bool m_hotkeyRegistered = true;
    LastSwitch m_lastSwitch;
};

#endif // FOLDERSWITCH_APPSTATE_H
