# Changelog

## 2026-08-25 (4)

La primera marca fallaba en tres cosas medibles contra el resto del set: 1.37 de
proporcion W/H (los otros van 0.88-1.00), 7.9% de verde (los otros 3.7-4.8%) y solo
46% de core. La flecha gris encima se comia la masa oscura, y una forma ancha alarga
el borde inferior, que es justo donde cyan y amarillo se solapan en verde. Se
reemplazo por silueta maciza sin glifo, mas alta y con el lado derecho en punta:
0.93 / 5.0% / 58%, los tres en rango. El generador ahora mide esas tres cifras, asi
que cualquier variante se compara con datos y no a ojo.

[commit sugerido: "fix: marca mas alta, sin glifo claro y con el verde en rango"]

## 2026-08-25 (3)

La app usaba un icono prestado de LinkRedirector. Se diseñó marca propia dentro del
sistema de las apps LGA: silueta maciza de carpeta, tres planchas CMY desregistradas
a 120°, y flecha gris encima (no calada: calarla deja flecos, porque el hueco de cada
plancha cae desplazado). Paleta y offsets medidos de los PNG existentes, no estimados.
El glifo se ajusta al core — la intersección de las tres planchas — y no a la silueta.
Se agregó `tools/logo/` (geometría única en Python que emite PNG, ICO y SVG) y se
verificó la lectura a 16 px componiendo sobre capturas reales de la barra y el tray.

[commit sugerido: "feat: marca propia de LGA_FolderSwitch y generador de iconos"]

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
