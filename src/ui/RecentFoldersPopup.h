#ifndef FOLDERSWITCH_RECENTFOLDERSPOPUP_H
#define FOLDERSWITCH_RECENTFOLDERSPOPUP_H

#include <QStringList>
#include <QWidget>

class QEventLoop;

// Menu de carpetas recientes de Ctrl+Alt+Shift+O (opcion A1 del canvas "FolderSwitch sin tabs"):
// una tarjeta con esquinas redondeadas y sombra propia, un titulo con "Press 1-5", y una fila por
// carpeta con su numero en una caja violeta (como los badges del Shot Player), el nombre y la
// carpeta que la contiene. Sin carpetas, un estado vacio que explica que hacer.
//
// Lo dibuja entero la app, sin QMenu: el de Qt trae la sombra dura de Windows y un margen de icono
// que no se puede sacar. Se elige con el mouse, con 1-5, o con flechas + Enter; Esc o un click
// afuera lo cierran sin elegir.
class RecentFoldersPopup : public QWidget
{
    Q_OBJECT
public:
    explicit RecentFoldersPopup(const QStringList &folders, QWidget *parent = nullptr);

    // Lo muestra con la tarjeta pegada a pos (el puntero), dentro de la pantalla, y espera. Devuelve
    // la carpeta elegida, o vacio si se cerro sin elegir.
    QString exec(const QPoint &pos);

    // Fila resaltada (-1: ninguna). Publico para la captura de QA.
    void setCurrentIndex(int index);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    QRect cardRect() const;
    QRect rowRect(int index) const;
    int rowAt(const QPoint &pos) const;
    void choose(int index);

    QStringList m_folders;
    int m_current = -1;
    QString m_chosen;
    QEventLoop *m_loop = nullptr;
};

#endif // FOLDERSWITCH_RECENTFOLDERSPOPUP_H
