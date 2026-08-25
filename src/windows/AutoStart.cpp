#include "windows/AutoStart.h"
#include "windows/RegistryHelper.h"

#include <QCoreApplication>
#include <QDir>

namespace {

const QString kRunKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
const QString kValueName = "LGA_FolderSwitch";

QString currentExeQuoted()
{
    const QString exe = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    return "\"" + exe + "\"";
}

} // namespace

namespace AutoStart {

bool isEnabled()
{
    const QString stored = RegistryHelper::readString(HKEY_CURRENT_USER, kRunKey, kValueName);
    if (stored.isEmpty()) {
        return false;
    }
    // Considerar habilitado solo si apunta al exe actual.
    return stored.contains(QDir::toNativeSeparators(QCoreApplication::applicationFilePath()),
                           Qt::CaseInsensitive);
}

bool setEnabled(bool enabled)
{
    if (enabled) {
        return RegistryHelper::writeString(HKEY_CURRENT_USER, kRunKey, kValueName, currentExeQuoted());
    }
    const std::wstring sub = kRunKey.toStdWString();
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, sub.c_str(), 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        return true; // no existe la clave: nada que borrar
    }
    const std::wstring val = kValueName.toStdWString();
    LONG rc = RegDeleteValueW(hKey, val.c_str());
    RegCloseKey(hKey);
    return rc == ERROR_SUCCESS || rc == ERROR_FILE_NOT_FOUND;
}

} // namespace AutoStart
