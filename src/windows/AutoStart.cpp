#include "windows/AutoStart.h"
#include "windows/RegistryHelper.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

namespace {

const QString kRunKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
const QString kValueName = "LGA_FolderSwitch";

// Donde Task Manager > Startup guarda lo que el usuario deshabilito. Solo se lee, para el
// diagnostico del log: un valor de Run con marca impar existe pero Windows no lo lanza.
const QString kStartupApprovedKey =
    "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run";

// Si `appPath` (limpio, con '/') cuelga de `rootDirLiteral`. Se compara por COMPONENTE de
// ruta: un `startsWith` a secas haria que `C:/x/build2` cuente como adentro de `C:/x/build`.
bool pathIsInside(const QString &appPath, const char *rootDirLiteral)
{
    const QString rootDir = QDir::cleanPath(QDir::fromNativeSeparators(QLatin1String(rootDirLiteral)));
    if (rootDir.isEmpty()) {
        return false;
    }
    // En Windows la misma ruta puede llegar con distinta capitalizacion (`C:` vs `c:`).
    return appPath.startsWith(rootDir + QLatin1Char('/'), Qt::CaseInsensitive);
}

} // namespace

namespace AutoStart {

QString currentExeQuoted()
{
    const QString exe = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    return "\"" + exe + "\"";
}

QString storedCommand()
{
    return RegistryHelper::readString(HKEY_CURRENT_USER, kRunKey, kValueName);
}

bool disabledByTaskManager()
{
    const std::wstring sub = kStartupApprovedKey.toStdWString();
    const std::wstring val = kValueName.toStdWString();
    BYTE data[32] = {};
    DWORD size = sizeof(data);
    DWORD type = 0;
    const LONG rc = RegGetValueW(HKEY_CURRENT_USER, sub.c_str(), val.c_str(),
                                 RRF_RT_REG_BINARY, &type, data, &size);
    if (rc != ERROR_SUCCESS || size == 0) {
        return false; // sin marca = habilitado
    }
    // Primer byte: par (0x02, 0x06) = habilitado, impar (0x03, 0x07) = deshabilitado.
    return (data[0] & 0x01) != 0;
}

bool isEnabled()
{
    const QString stored = storedCommand();
    if (stored.isEmpty()) {
        return false;
    }
    // Habilitado solo si el valor guardado apunta al ejecutable ACTUAL.
    return stored.contains(QDir::toNativeSeparators(QCoreApplication::applicationFilePath()),
                           Qt::CaseInsensitive);
}

bool setEnabled(bool enabled)
{
    if (enabled) {
        return RegistryHelper::writeString(HKEY_CURRENT_USER, kRunKey, kValueName, currentExeQuoted());
    }
    // Desactivar = borrar puntualmente ESTE valor, sin tocar el resto de la clave Run.
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

bool runsFromDevelopmentTree()
{
    const QString appPath =
        QDir::cleanPath(QDir::fromNativeSeparators(QCoreApplication::applicationFilePath()));
    // Solo la carpeta que CONTIENE al exe, nunca la ruta entera: `D:\Builds\Apps\X.exe` es una
    // instalacion legitima y su carpeta es `Apps`. `contains` y no `startsWith` para que entre
    // `build-release` y cualquier variante.
    const QString folder = QFileInfo(appPath).dir().dirName();
    if (folder.contains(QLatin1String("build"), Qt::CaseInsensitive)
        || folder.contains(QLatin1String("deploy"), Qt::CaseInsensitive)) {
        return true;
    }
#ifdef LGA_BUILD_TREE_DIR
    if (pathIsInside(appPath, LGA_BUILD_TREE_DIR)) {
        return true;
    }
#endif
#ifdef LGA_SOURCE_TREE_DIR
    if (pathIsInside(appPath, LGA_SOURCE_TREE_DIR)) {
        return true;
    }
#endif
    return false;
}

Availability availability()
{
    if (runsFromDevelopmentTree()) {
        return {false, Unavailability::DevelopmentTree,
                QStringLiteral("Not available from a development build")};
    }
    return {true, Unavailability::None, QString()};
}

} // namespace AutoStart
