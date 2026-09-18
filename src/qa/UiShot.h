#ifndef FOLDERSWITCH_UISHOT_H
#define FOLDERSWITCH_UISHOT_H

#include <QStringList>

// Captura de QA sin escritorio: --ui-shot <estado> <out.png> [--dpr N].
// Arma la ventana real con datos de prueba y la dibuja a un PNG con QWidget::render, sin
// mostrar nada, sin bandeja, sin hotkey, sin updater y sin tocar QSettings ni el registro.
// Devuelve el codigo de salida del proceso.
int runUiShot(const QStringList &args);

#endif // FOLDERSWITCH_UISHOT_H
