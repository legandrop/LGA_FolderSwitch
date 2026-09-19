#ifndef FOLDERSWITCH_TITLEBAR_H
#define FOLDERSWITCH_TITLEBAR_H

#include <QWidget>

// Barra de titulo propia de la ventana principal (la ventana va sin el marco de Windows): icono de
// la app, nombre, ayuda, minimizar y cerrar. Arrastrarla mueve la ventana con startSystemMove(),
// asi Windows sigue haciendo el movimiento (y el acomodo contra los bordes de la pantalla).
// Cerrar llama a close() de la ventana, que la oculta a la bandeja como antes.
class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);

signals:
    void helpClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // FOLDERSWITCH_TITLEBAR_H
