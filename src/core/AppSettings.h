#ifndef FOLDERSWITCH_APPSETTINGS_H
#define FOLDERSWITCH_APPSETTINGS_H

#include <QSettings>
#include <QString>

#include <memory>

// Donde vive la configuracion de FolderSwitch: %APPDATA%\LGA\LGA_FolderSwitch\settings.ini, el
// mismo lugar y formato que las otras apps LGA (FrameRev: AppDataLocation + settings.ini). El
// desinstalador borra esa carpeta entera.
namespace AppSettings {

QString filePath();
std::unique_ptr<QSettings> open();

// Antes la configuracion vivia en el registro (HKCU\Software\LGA\FolderSwitch). La primera
// vez que corre una version nueva, si el .ini todavia no existe, copia esas claves al .ini y borra
// la clave del registro para no dejar basura. Se llama una vez al arrancar, nunca en --ui-shot.
void migrateFromRegistry();

} // namespace AppSettings

#endif // FOLDERSWITCH_APPSETTINGS_H
