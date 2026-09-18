#ifndef FOLDERSWITCH_MAINWINDOW_H
#define FOLDERSWITCH_MAINWINDOW_H

#include <QMainWindow>

class AppState;
class Chip;
class ElidedLabel;
class QCheckBox;
class QFrame;
class QLabel;
class QPushButton;
class TabHeader;

// Ventana de Settings de LGA FolderSwitch: tarjeta de estado (On/Paused), ultima carpeta y
// opciones. Todo lo que muestra sale de AppState; lo que el usuario cambia se escribe en AppState
// (o en AutoStart, para el inicio con Windows). El boton X no cierra la app: oculta la ventana a
// la bandeja (closeEvent).
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Capture: la arma --ui-shot. No lee QSettings ni el registro, no hace NINGUN connect y el
    // estado sale solo del AppState de prueba y de applyAutoStartFixture().
    enum class Mode { Normal, Capture };

    MainWindow(AppState *state, Mode mode, QWidget *parent = nullptr);
    ~MainWindow() override;

    TabHeader *tabHeader() const { return m_header; }

    // Solo para Mode::Capture: estado del checkbox de inicio con Windows.
    void applyAutoStartFixture(bool enabled, bool available);
    // Vuelve a leer AppState y ajusta el alto de la ventana a su contenido.
    void refresh();

signals:
    void helpRequested();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onAutoStartToggled(bool checked);

private:
    void buildUi();
    // Escrituras de los controles. Se conectan DESPUES de cargar el estado inicial, asi reflejarlo
    // no reescribe QSettings ni la clave Run. Nunca en Mode::Capture.
    void connectWrites();
    // Refleja el estado REAL del inicio con Windows sin disparar toggled().
    void syncAutoStartCheck();
    void setAutoStartTooltip(bool available);
    void fitHeight();

    AppState *m_state = nullptr;
    Mode m_mode = Mode::Normal;

    TabHeader *m_header = nullptr;
    QLabel *m_statusDot = nullptr;
    QLabel *m_statusTitle = nullptr;
    QLabel *m_statusCaption = nullptr;
    QPushButton *m_toggleButton = nullptr;

    Chip *m_sourceChip = nullptr;
    Chip *m_resultChip = nullptr;
    QLabel *m_timeLabel = nullptr;
    ElidedLabel *m_folderValue = nullptr;
    QLabel *m_folderCaption = nullptr;

    QCheckBox *m_autoSwitchCheck = nullptr;
    Chip *m_hotkeyBusyChip = nullptr;
    QLabel *m_hotkeyCaption = nullptr;
    QCheckBox *m_autoStartCheck = nullptr;
};

#endif // FOLDERSWITCH_MAINWINDOW_H
