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
    // Capture: la arma --ui-shot. No lee QSettings ni el registro, no conecta ninguna escritura
    // y el estado sale solo de applyFixture().
    enum class Mode { Normal, Capture };

    explicit MainWindow(Mode mode = Mode::Normal, QWidget *parent = nullptr);
    ~MainWindow() override;

    bool autoSwitchEnabled() const;
    bool masterEnabled() const;

    // Solo para Mode::Capture: estado de prueba de la captura.
    void applyFixture(bool enabled, bool autoSwitch, bool autoStart, const QString &folder);

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
    // Escrituras de los checkboxes: se conectan DESPUES de cargar el estado inicial, asi
    // reflejarlo no reescribe QSettings ni la clave Run. Nunca en Mode::Capture.
    void connectWrites();
    // Refleja el estado REAL del inicio con Windows sin disparar toggled().
    void syncAutoStartCheck();

    Mode m_mode = Mode::Normal;
    QLabel *m_statusLabel = nullptr;
    QCheckBox *m_autoSwitchCheck = nullptr;
    QCheckBox *m_autoStartCheck = nullptr;
    QCheckBox *m_enabledCheck = nullptr;
};

#endif // FOLDERSWITCH_MAINWINDOW_H
