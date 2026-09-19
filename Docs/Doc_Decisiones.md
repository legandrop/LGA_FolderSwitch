# Decisiones

Decisiones que solo Lega puede tomar. Una abierta no se resuelve por cuenta propia: mientras tanto se
sigue con la opción reversible que se indica. Al decidirse, pasa a "Decididas" con fecha y opción.
El trabajo pendiente está en [`Doc_Roadmap.md`](Doc_Roadmap.md).

## Abiertas

### D-01 · Cómo responde "Check now" (abierta el 2026-09-18)

- **Qué se decide:** si el resultado del chequeo manual de updates se muestra en la misma fila de la
  ventana o con los cuadros de diálogo de siempre.
- **Opciones:**
  - A. Diálogos (`QMessageBox`), como hoy.
  - B. En la fila: "Checking…" con el botón deshabilitado, "v0.10 is the latest version" en verde,
    "v0.11 is available" con el botón convertido en "Update". El diálogo de confirmación y la descarga
    siguen iguales. Hay mockup de los cuatro estados.
- **Mientras tanto:** A. El menú del tray ("Check for Updates...") sigue con diálogos en cualquier caso,
  porque la ventana puede estar oculta.

## Decididas

- **2026-09-18 · Ventana sin pestañas: barra de título propia (opción B).** Entre pie con versión (A),
  barra propia (B) y franja superior (C). Se eligió B: la app dibuja su barra con el `?`, minimizar y
  cerrar.
- **2026-09-18 · Updates a la vista, con checkbox.** Fuera de la ayuda, debajo de "Start with Windows",
  con "Check for updates at startup" apagable y "Check now".
- **2026-09-18 · Sin foco de teclado en toda la app.** Solo hover; Tab no recorre controles.
- **2026-09-18 · Tres tarjetas.** Cambio de carpeta (estado, automático, atajos), última carpeta, y la
  app (inicio con Windows, updates).
- **2026-09-18 · Carpetas recientes con `Ctrl+Alt+Shift+O`.** Popup en el puntero con las 5 últimas.
  Diseño: número en caja violeta en lugar del ícono (A1), tras probar la variante con carpeta grande.
- **2026-09-18 · Configuración en AppData y desinstalación sin restos.** `%APPDATA%\LGA\LGA_FolderSwitch`,
  como las otras apps LGA, con migración desde el registro.
