# Changelog

## 2026-08-25 (2)

El auto-switch fallaba al volver de XYplorer: la condición exigía que la ventana
inmediatamente anterior al diálogo fuera el file manager, y cualquier ventana
intermedia (task switcher de Alt+Tab) la rompía en silencio. Se reemplazó por
tracking de retorno: al pasar de un diálogo vivo al manager se marca ese diálogo
como pendiente, y al volver a ese mismo HWND se inyecta, sin importar ventanas
intermedias. Además: debug log opcional (`log=true` en `config/debug_flags.txt`
→ `debug.log` en la raíz) porque la app WIN32 no tiene consola, y `.gitignore`.

[commit sugerido: "fix: auto-switch robusto ante ventanas intermedias + debug log"]

## 2026-08-25

Implementación inicial de LGA_FolderSwitch: app de tray que detecta la carpeta activa
en Explorer o XYplorer y la inyecta en el diálogo Open/Save de Windows (`#32770`) al
volver a él, o a demanda con Ctrl+Alt+O. Usa `SetWinEventHook` para seguir la ventana
en foreground, COM (`IShellWindows`/`IWebBrowser2`) para leer el path de Explorer,
parseo de título para XYplorer, y la técnica QuickSwitch (inyectar en el combo de
nombre de archivo + `WM_COMMAND`/`IDOK`) para el switch. Settings con QSettings,
autostart por registro, y estética dark de LGA.

[commit sugerido: "feat: implementación inicial de LGA_FolderSwitch"]
