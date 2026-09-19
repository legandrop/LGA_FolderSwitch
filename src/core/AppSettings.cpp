#include "core/AppSettings.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include <windows.h>

namespace {
const wchar_t *kLegacyKey = L"Software\\LGA\\FolderSwitch";
const wchar_t *kLegacyParent = L"Software\\LGA";
} // namespace

namespace AppSettings {

QString filePath()
{
    // AppDataLocation ya incluye organizacion y app: %APPDATA%/LGA/LGA_FolderSwitch.
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath(QStringLiteral("settings.ini"));
}

std::unique_ptr<QSettings> open()
{
    QDir().mkpath(QFileInfo(filePath()).absolutePath());
    return std::make_unique<QSettings>(filePath(), QSettings::IniFormat);
}

void migrateFromRegistry()
{
    if (QFileInfo::exists(filePath())) {
        return;
    }
    const QSettings legacy(QStringLiteral("HKEY_CURRENT_USER\\Software\\LGA\\FolderSwitch"), QSettings::NativeFormat);
    const QStringList keys = legacy.allKeys();
    if (keys.isEmpty()) {
        return;
    }
    auto settings = open();
    for (const QString &key : keys) {
        settings->setValue(key, legacy.value(key));
    }
    settings->sync();
    if (settings->status() != QSettings::NoError) {
        // Sin .ini escrito no se borra nada: la proxima vez se reintenta con el registro intacto.
        qWarning() << "[AppSettings] No se pudo escribir" << filePath() << "; el registro queda como estaba";
        return;
    }
    RegDeleteTreeW(HKEY_CURRENT_USER, kLegacyKey);
    RegDeleteKeyW(HKEY_CURRENT_USER, kLegacyKey);
    // Software\LGA solo se borra si quedo vacio del todo: sin subclaves NI valores (RegDeleteKeyW
    // se llevaria los valores de otra app sin avisar).
    HKEY parent = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kLegacyParent, 0, KEY_READ, &parent) == ERROR_SUCCESS) {
        DWORD subKeys = 0;
        DWORD values = 0;
        const bool empty = RegQueryInfoKeyW(parent, nullptr, nullptr, nullptr, &subKeys, nullptr, nullptr, &values,
                                            nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS
                           && subKeys == 0 && values == 0;
        RegCloseKey(parent);
        if (empty) {
            RegDeleteKeyW(HKEY_CURRENT_USER, kLegacyParent);
        }
    }
    qInfo() << "[AppSettings] Configuracion migrada del registro a" << filePath() << "claves:" << keys.size();
}

} // namespace AppSettings
