# Roadmap

Lo que falta, ordenado por importancia. Lo hecho se borra de acá: la historia queda en
[`Changelog.md`](Changelog.md). Las decisiones que dependen de Lega están en
[`Doc_Decisiones.md`](Doc_Decisiones.md).

## Pendiente

1. **Registro compartido de LGA.** La app no escribe su `LGA_FolderSwitch.json` en `%APPDATA%\LGA\`
   (`LgaRegistry::registerThisApp`, convención de la Base). Sin eso, las otras apps LGA (el card de
   LGA Updates de PipeSync) no la ven. Copiar `LgaRegistry` de la Base tal cual y cablearlo en `main()`;
   el desinstalador tiene que borrar ese `.json`.
2. **Checklist de app nueva antes del próximo release.** Recorrer
   `LGA_Base_QT_C_Py/docs/Doc_Checklist_App_Nueva.md` completo sobre esta app (rutas, borrado,
   instalador, casos negativos de cada guarda).
3. **Carpeta de un manager cerrado con la X.** Si el usuario cierra la ventana de Explorer/XYplorer en
   vez de cambiar a otra, esa carpeta no entra a las recientes: cuando llega el cambio de ventana, la
   ventana ya no existe. Arreglo posible: resolver la carpeta mientras el manager está en primer plano
   (al navegar), no solo al salir.
4. **Resultado de "Check now" en la fila de updates.** Depende de D-01.
5. **Marco nativo en otros entornos.** Probar la ventana sin marco (sombra, esquinas, minimizar desde la
   barra de tareas) en Windows 10 y con monitores de distinto DPI.
6. **Popup de recientes en pantallas muy angostas.** En un monitor más angosto que la tarjeta (~420 px)
   queda pegada al borde y cortada. Improbable en escritorio; bajo.
