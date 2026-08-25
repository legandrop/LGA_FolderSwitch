#ifndef FOLDERSWITCH_AUTOSTART_H
#define FOLDERSWITCH_AUTOSTART_H

// Maneja el inicio automatico en Windows via
// HKCU\Software\Microsoft\Windows\CurrentVersion\Run
namespace AutoStart {

// True si LGA_FolderSwitch esta registrado para iniciar con el sistema
// (y apunta al ejecutable actual).
bool isEnabled();

// Activa o desactiva el inicio automatico. Usa la ruta del ejecutable actual.
bool setEnabled(bool enabled);

} // namespace AutoStart

#endif // FOLDERSWITCH_AUTOSTART_H
