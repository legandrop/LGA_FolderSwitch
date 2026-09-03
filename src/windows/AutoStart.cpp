#include "windows/AutoStart.h"
#include "windows/RegistryHelper.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

namespace {

const QString kRunKey = "Software\Microsoft\Windows\CurrentVersion\Run";
// Donde Task Manager > Startup guarda lo que el usuario deshabilito. Un valor de Run
// con esta marca existe pero Windows no lo lanza: sin mirarla, la app diria "activo"
// y nunca arrancaria.
const QString kStartupApprovedKey =
    "Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\Run";
const QString kValueName = "LGA_FolderSwitch";

QString currentExeNative()
{
    return QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
}

// Entre comillas dobles para que sobreviva a rutas con espacios.
QString currentExeQuoted()
{
    return "\"" + currentExeNative() + "\"";
}

// Borra puntualmente el valor kValueName de una subclave de HKCU, sin tocar el resto.
bool deleteValue(const QString &subKey)
{
    const std::wstring sub = subKey.toStdWString();
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, sub.c_str(), 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        return true; // no existe la clave: nada que borrar
    }
    const std::wstring val = kValueName.toStdWString();
    LONG rc = RegDeleteValueW(hKey, val.c_str());
    RegCloseKey(hKey);
    return rc == ERROR_SUCCESS || rc == ERROR_FILE_NOT_FOUND;
}

// Si `appPath` (limpio, con '/') cuelga de `rootDirLiteral`. Se compara por
// componente de ruta: un startsWith pelado haria que C:/x/build2 cuente como
// adentro de C:/x/build.
bool pathIsInside(const QString &appPath, const char *rootDirLiteral)
{
    const QString rootDir = QDir::cleanPath(QDir::fromNativeSeparators(QLatin1String(rootDirLiteral)));
    if (rootDir.isEmpty()) {
        return false;
    }
    return appPath.startsWith(rootDir + QLatin1Char('/'), Qt::CaseInsensitive);
}

} // namespace

namespace AutoStart {

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
    if (!stored.contains(currentExeNative(), Qt::CaseInsensitive)) {
        return false;
    }
    return !disabledByTaskManager();
}

bool setEnabled(bool enabled)
{
    if (enabled) {
        if (!RegistryHelper::writeString(HKEY_CURRENT_USER, kRunKey, kValueName, currentExeQuoted())) {
            return false;
        }
        // Si el usuario lo habia apagado desde Task Manager, la marca sigue ahi y
        // Windows no lanzaria el valor recien escrito. Borrarla es lo mismo que hace
        // Task Manager al volver a habilitarlo.
        return deleteValue(kStartupApprovedKey);
    }
    // Desactivar = borrar puntualmente ESTE valor, sin tocar el resto de la clave Run.
    return deleteValue(kRunKey);
}

bool runsFromDevelopmentTree()
{
    const QString appPath =
        QDir::cleanPath(QDir::fromNativeSeparators(QCoreApplication::applicationFilePath()));
    // Solo la carpeta que CONTIENE al exe, no la ruta entera: D:\Builds\Apps\X.exe es
    // una instalacion legitima. `contains` y no `startsWith` para que entre
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

} // namespace AutoStart
