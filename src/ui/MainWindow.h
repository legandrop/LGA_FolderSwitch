#ifndef FOLDERSWITCH_MAINWINDOW_H
#define FOLDERSWITCH_MAINWINDOW_H

#include <QMainWindow>

class QCheckBox;
class QLabel;

// Ventana de Settings de LGA FolderSwitch. Chica, dark, con chrome del SO.
// El boton X no cierra la app: oculta la ventana a la bandeja (closeEvent).
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    bool autoSwitchEnabled() const;
    bool masterEnabled() const;

public slots:
    // Actualiza el label de estado con la ultima carpeta detectada.
    void setLastDetectedFolder(const QString &folder);

signals:
    void autoSwitchToggled(bool enabled);
    void masterEnabledToggled(bool enabled);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onAutoStartToggled(bool checked);

private:
    void buildUi();
    void loadSettings();
    // Refleja el estado REAL del inicio con Windows sin disparar toggled().
    void syncAutoStartCheck();

    QLabel *m_statusLabel = nullptr;
    QCheckBox *m_autoSwitchCheck = nullptr;
    QCheckBox *m_autoStartCheck = nullptr;
    QCheckBox *m_enabledCheck = nullptr;
};

#endif // FOLDERSWITCH_MAINWINDOW_H
