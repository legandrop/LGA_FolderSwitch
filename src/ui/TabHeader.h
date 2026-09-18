#ifndef FOLDERSWITCH_TABHEADER_H
#define FOLDERSWITCH_TABHEADER_H

#include <QWidget>

// Barra superior de 50 px con una sola pestana y el boton de ayuda en la esquina. Copia del
// TabHeader de LGA_VideoDownloader (mismas medidas y la misma pintura), con el texto como
// parametro.
class TabHeader : public QWidget
{
    Q_OBJECT
public:
    explicit TabHeader(const QString &tabText, QWidget *parent = nullptr);

signals:
    void helpClicked();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_tabText;
};

#endif // FOLDERSWITCH_TABHEADER_H
