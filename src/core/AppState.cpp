#include "core/AppState.h"

#include <QDebug>
#include <QSettings>

AppState::AppState(Persistence persistence, QObject *parent)
    : QObject(parent)
    , m_persistence(persistence)
{
    if (m_persistence == Persistence::Settings) {
        const QSettings settings(QStringLiteral("LGA"), QStringLiteral("FolderSwitch"));
        m_enabled = settings.value(QStringLiteral("enabled"), true).toBool();
        m_autoSwitch = settings.value(QStringLiteral("autoSwitch"), true).toBool();
    }
}

void AppState::write(const char *key, bool value)
{
    if (m_persistence != Persistence::Settings) {
        return;
    }
    QSettings settings(QStringLiteral("LGA"), QStringLiteral("FolderSwitch"));
    settings.setValue(QLatin1String(key), value);
}

void AppState::setEnabled(bool enabled)
{
    if (enabled == m_enabled) {
        return;
    }
    m_enabled = enabled;
    write("enabled", enabled);
    qInfo() << "[AppState] Switching" << (enabled ? "ON" : "PAUSED");
    emit changed();
}

void AppState::setAutoSwitch(bool autoSwitch)
{
    if (autoSwitch == m_autoSwitch) {
        return;
    }
    m_autoSwitch = autoSwitch;
    write("autoSwitch", autoSwitch);
    qInfo() << "[AppState] Auto-switch:" << autoSwitch;
    emit changed();
}

void AppState::setHotkeyRegistered(bool registered)
{
    if (registered == m_hotkeyRegistered) {
        return;
    }
    m_hotkeyRegistered = registered;
    emit changed();
}

void AppState::setLastSwitch(const LastSwitch &lastSwitch)
{
    m_lastSwitch = lastSwitch;
    emit changed();
}
