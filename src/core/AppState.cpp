#include "core/AppState.h"
#include "core/AppSettings.h"

#include <QDebug>
#include <QSettings>

AppState::AppState(Persistence persistence, QObject *parent)
    : QObject(parent)
    , m_persistence(persistence)
{
    if (m_persistence == Persistence::Settings) {
        const auto settings = AppSettings::open();
        m_enabled = settings->value(QStringLiteral("enabled"), true).toBool();
        m_autoSwitch = settings->value(QStringLiteral("autoSwitch"), true).toBool();
        m_checkUpdatesAtStartup = settings->value(QStringLiteral("checkUpdatesAtStartup"), true).toBool();
        m_recentFolders = settings->value(QStringLiteral("recentFolders")).toStringList().mid(0, kMaxRecentFolders);
    }
}

void AppState::write(const char *key, bool value)
{
    if (m_persistence != Persistence::Settings) {
        return;
    }
    const auto settings = AppSettings::open();
    settings->setValue(QLatin1String(key), value);
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

void AppState::setCheckUpdatesAtStartup(bool check)
{
    if (check == m_checkUpdatesAtStartup) {
        return;
    }
    m_checkUpdatesAtStartup = check;
    write("checkUpdatesAtStartup", check);
    qInfo() << "[AppState] Check updates at startup:" << check;
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

void AppState::setRecentHotkeyRegistered(bool registered)
{
    if (registered == m_recentHotkeyRegistered) {
        return;
    }
    m_recentHotkeyRegistered = registered;
    emit changed();
}

void AppState::addRecentFolder(const QString &path)
{
    const QString folder = path.trimmed();
    if (folder.isEmpty()) {
        return;
    }
    // Rutas de Windows: la misma carpeta con otra capitalizacion o sin la barra final es la misma.
    const auto sameFolder = [&folder](const QString &other) {
        const auto strip = [](QString p) {
            while (p.size() > 3 && (p.endsWith(QLatin1Char('\\')) || p.endsWith(QLatin1Char('/')))) {
                p.chop(1);
            }
            return p;
        };
        return strip(other).compare(strip(folder), Qt::CaseInsensitive) == 0;
    };
    QStringList updated{folder};
    for (const QString &existing : m_recentFolders) {
        if (!sameFolder(existing) && updated.size() < kMaxRecentFolders) {
            updated.append(existing);
        }
    }
    if (updated == m_recentFolders) {
        return;
    }
    m_recentFolders = updated;
    if (m_persistence == Persistence::Settings) {
        const auto settings = AppSettings::open();
        settings->setValue(QStringLiteral("recentFolders"), m_recentFolders);
    }
}

void AppState::setLastSwitch(const LastSwitch &lastSwitch)
{
    m_lastSwitch = lastSwitch;
    emit changed();
}
