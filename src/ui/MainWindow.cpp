#include "ui/MainWindow.h"

#include "core/AppState.h"
#include "ui/Theme.h"
#include "ui/TitleBar.h"
#include "ui/UiWidgets.h"
#include "windows/AutoStart.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

#include <utility>

#include <windows.h>
#include <dwmapi.h>

namespace {

constexpr int kWindowWidth = 440;
// Sangria de lo que va debajo de un checkbox: indicador (16) + spacing (10).
constexpr int kCheckIndent = 26;

QLabel *label(const QString &text, const char *name, QWidget *parent)
{
    auto *l = new QLabel(text, parent);
    l->setObjectName(QLatin1String(name));
    return l;
}

QFrame *card(QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName(QStringLiteral("card"));
    return frame;
}

QLabel *caption(const QString &text, QWidget *parent)
{
    auto *l = label(text, "caption", parent);
    l->setWordWrap(true);
    return l;
}

// Linea divisoria entre filas de una tarjeta, con el mismo aire arriba y abajo.
void addDivider(QVBoxLayout *layout, QWidget *parent)
{
    layout->addSpacing(8);
    auto *divider = new QFrame(parent);
    divider->setObjectName(QStringLiteral("divider"));
    layout->addWidget(divider);
    layout->addSpacing(8);
}

} // namespace

MainWindow::MainWindow(AppState *state, Mode mode, QWidget *parent)
    : QMainWindow(parent)
    , m_state(state)
    , m_mode(mode)
{
    setWindowTitle(QStringLiteral("LGA FolderSwitch"));
    // La barra de titulo la dibuja la app (TitleBar); applyNativeFrame() le devuelve a la ventana
    // la sombra y las esquinas de Windows.
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    buildUi();
    if (m_mode == Mode::Normal) {
        m_autoStartCheck->setChecked(AutoStart::isEnabled());
        setAutoStartTooltip(AutoStart::availability().available);
        refresh();
        connectWrites();
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    central->setObjectName(QStringLiteral("central"));
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_titleBar = new TitleBar(central);
    root->addWidget(m_titleBar);

    auto *content = new QWidget(central);
    content->setObjectName(QStringLiteral("content"));
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(14, 12, 14, 14);
    layout->setSpacing(10);
    root->addWidget(content, 1);

    // Tres tarjetas: el cambio de carpeta (estado, automatico y atajo), la ultima carpeta y las
    // opciones de la app (inicio con Windows y updates).

    // ---------- Tarjeta 1: cambio de carpeta ----------
    auto *switchCard = card(content);
    auto *switching = new QVBoxLayout(switchCard);
    switching->setContentsMargins(14, 12, 14, 12);
    switching->setSpacing(4);

    auto *statusRow = new QHBoxLayout();
    statusRow->setContentsMargins(0, 0, 0, 0);
    statusRow->setSpacing(12);
    m_statusDot = label(QString(), "statusDot", switchCard);
    statusRow->addWidget(m_statusDot, 0, Qt::AlignVCenter);
    auto *statusTexts = new QVBoxLayout();
    statusTexts->setSpacing(2);
    m_statusTitle = label(QString(), "cardTitle", switchCard);
    m_statusCaption = caption(QString(), switchCard);
    statusTexts->addWidget(m_statusTitle);
    statusTexts->addWidget(m_statusCaption);
    statusRow->addLayout(statusTexts, 1);
    m_toggleButton = Ui::button(QString(), QString(), QString(), switchCard);
    m_toggleButton->setObjectName(QStringLiteral("toggleButton"));
    m_toggleButton->setMinimumWidth(78);
    statusRow->addWidget(m_toggleButton, 0, Qt::AlignVCenter);
    switching->addLayout(statusRow);

    addDivider(switching, switchCard);
    m_autoSwitchCheck = new QCheckBox(QStringLiteral("Switch automatically"), switchCard);
    switching->addWidget(m_autoSwitchCheck);
    auto *autoCaption = caption(QStringLiteral("When you return to a dialog from the file manager."), switchCard);
    autoCaption->setContentsMargins(kCheckIndent, 0, 0, 0);
    switching->addWidget(autoCaption);

    // Los atajos son independientes del auto-switch (no miran ni autoSwitch ni la pausa): van en
    // su propio bloque, separado y sin sangria. Las teclas de los dos arrancan en la misma columna.
    addDivider(switching, switchCard);
    const auto shortcutRow = [switchCard](const QString &name, const QStringList &keys, Chip **busyChip) {
        auto *row = new QHBoxLayout();
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(5);
        auto *nameLabel = label(name, "optionLabel", switchCard);
        row->addWidget(nameLabel);
        row->addSpacing(5);
        for (const QString &key : keys) {
            auto *chip = new Chip(switchCard);
            chip->set(QStringLiteral("key"), key);
            row->addWidget(chip, 0, Qt::AlignVCenter);
        }
        // "In use" pegado a las teclas, no flotando contra el borde.
        row->addSpacing(3);
        *busyChip = new Chip(switchCard);
        (*busyChip)->set(QStringLiteral("err"), QStringLiteral("In use"));
        row->addWidget(*busyChip, 0, Qt::AlignVCenter);
        row->addStretch(1);
        return std::make_pair(row, nameLabel);
    };
    const auto [hotkeyRow, hotkeyLabel] = shortcutRow(
        QStringLiteral("Manual shortcut"), {QStringLiteral("Ctrl"), QStringLiteral("Alt"), QStringLiteral("O")},
        &m_hotkeyBusyChip);
    switching->addLayout(hotkeyRow);
    m_hotkeyCaption = caption(QString(), switchCard);
    switching->addWidget(m_hotkeyCaption);

    switching->addSpacing(8);
    const auto [recentRow, recentLabel] = shortcutRow(
        QStringLiteral("Recent folders"),
        {QStringLiteral("Ctrl"), QStringLiteral("Alt"), QStringLiteral("Shift"), QStringLiteral("O")},
        &m_recentBusyChip);
    switching->addLayout(recentRow);
    m_recentCaption = caption(QString(), switchCard);
    switching->addWidget(m_recentCaption);
    hotkeyLabel->ensurePolished();
    recentLabel->ensurePolished();
    const int labelWidth = qMax(hotkeyLabel->sizeHint().width(), recentLabel->sizeHint().width());
    hotkeyLabel->setFixedWidth(labelWidth);
    recentLabel->setFixedWidth(labelWidth);
    layout->addWidget(switchCard);

    // ---------- Tarjeta 2: ultima carpeta ----------
    auto *folderCard = card(content);
    auto *folderLayout = new QVBoxLayout(folderCard);
    folderLayout->setContentsMargins(14, 12, 14, 12);
    folderLayout->setSpacing(8);
    auto *folderHead = new QHBoxLayout();
    folderHead->setSpacing(6);
    folderHead->addWidget(label(QStringLiteral("Last folder"), "cardTitle", folderCard));
    folderHead->addStretch(1);
    m_sourceChip = new Chip(folderCard);
    m_sourceChip->setObjectName(QStringLiteral("chip"));
    m_resultChip = new Chip(folderCard);
    m_timeLabel = label(QString(), "meta", folderCard);
    folderHead->addWidget(m_sourceChip, 0, Qt::AlignVCenter);
    folderHead->addWidget(m_resultChip, 0, Qt::AlignVCenter);
    folderHead->addSpacing(2);
    folderHead->addWidget(m_timeLabel, 0, Qt::AlignVCenter);
    folderLayout->addLayout(folderHead);

    auto *field = new QFrame(folderCard);
    field->setObjectName(QStringLiteral("field"));
    auto *fieldRow = new QHBoxLayout(field);
    fieldRow->setContentsMargins(8, 0, 8, 0);
    fieldRow->setSpacing(8);
    fieldRow->addWidget(new IconWidget(Icon::Folder, Theme::color(Theme::kIcon), 14, field), 0, Qt::AlignVCenter);
    m_folderValue = new ElidedLabel(field);
    m_folderValue->setObjectName(QStringLiteral("fieldValue"));
    m_folderValue->setElideMode(Qt::ElideMiddle);
    fieldRow->addWidget(m_folderValue, 1);
    folderLayout->addWidget(field);
    m_folderCaption = caption(QString(), folderCard);
    folderLayout->addWidget(m_folderCaption);
    layout->addWidget(folderCard);

    // ---------- Tarjeta 3: la app (inicio con Windows y updates) ----------
    auto *appCard = card(content);
    auto *appOptions = new QVBoxLayout(appCard);
    appOptions->setContentsMargins(14, 12, 14, 12);
    appOptions->setSpacing(4);

    // Cableado tomado de FrameRev (src/ui/mainwindow/MainWindow.cpp): estado REAL del sistema
    // leido en vivo, y el connect DESPUES del setChecked inicial para que ese primer estado no
    // dispare una escritura al registro.
    //
    // A diferencia de FrameRev, el checkbox queda HABILITADO tambien desde una salida de
    // desarrollo. La distincion que importa no es quien corre, sino QUIEN DECIDE:
    //  - Escritura AUTOMATICA desde un build (primer arranque, reflejar estado): PROHIBIDA.
    //    Es la que puede pisar o borrar sola la entrada -- el bug que dejo a esta app sin
    //    arrancar. La bloquea AutoStart::availability() en runFirstLaunchSetupIfNeeded().
    //  - Click EXPLICITO del usuario: permitido siempre. Ademas es la UNICA forma de
    //    registrar una copia de `build/`: escribir la clave Run desde una terminal que corra
    //    dentro de un contenedor MSIX (una app empaquetada de la Store, por ejemplo) va a
    //    un registro VIRTUALIZADO por paquete, invisible para Windows -- la entrada parece
    //    puesta y el sistema nunca la ejecuta. Solo la app, que corre fuera de ese
    //    contenedor, escribe donde Windows lee. Medido el 2026-09-04; ver
    //    ../LGA_Base_QT_C_Py/docs/Doc_Autostart_Windows.md.
    m_autoStartCheck = new QCheckBox(QStringLiteral("Start with Windows"), appCard);
    appOptions->addWidget(m_autoStartCheck);

    // Updates a la vista y no en la ayuda. El checkbox solo decide el chequeo al ARRANCAR;
    // "Check now" busca en el momento, con las mismas respuestas que el menu del tray.
    addDivider(appOptions, appCard);
    auto *updatesRow = new QHBoxLayout();
    updatesRow->setContentsMargins(0, 0, 0, 0);
    updatesRow->setSpacing(12);
    m_updatesCheck = new QCheckBox(QStringLiteral("Check for updates at startup"), appCard);
    updatesRow->addWidget(m_updatesCheck, 1);
    m_checkNowButton = Ui::button(QStringLiteral("Check now"), QString(), QStringLiteral("sm"), appCard);
    m_checkNowButton->setObjectName(QStringLiteral("checkNowButton"));
    updatesRow->addWidget(m_checkNowButton, 0, Qt::AlignVCenter);
    appOptions->addLayout(updatesRow);
    auto *versionCaption = caption(QStringLiteral("Installed version: v" FOLDERSWITCH_VERSION), appCard);
    versionCaption->setContentsMargins(kCheckIndent, 0, 0, 0);
    appOptions->addWidget(versionCaption);
    layout->addWidget(appCard);

    setCentralWidget(central);
}

void MainWindow::connectWrites()
{
    connect(m_titleBar, &TitleBar::helpClicked, this, &MainWindow::helpRequested);
    connect(m_state, &AppState::changed, this, &MainWindow::refresh);
    connect(m_toggleButton, &QPushButton::clicked, this, [this]() {
        m_state->setEnabled(!m_state->enabled());
    });
    connect(m_autoSwitchCheck, &QCheckBox::toggled, m_state, &AppState::setAutoSwitch);
    connect(m_autoStartCheck, &QCheckBox::toggled, this, &MainWindow::onAutoStartToggled);
    connect(m_updatesCheck, &QCheckBox::toggled, m_state, &AppState::setCheckUpdatesAtStartup);
    connect(m_checkNowButton, &QPushButton::clicked, this, &MainWindow::checkUpdatesRequested);
}

void MainWindow::refresh()
{
    const bool on = m_state->enabled();
    const bool hotkeyOk = m_state->hotkeyRegistered();
    const bool recentOk = m_state->recentHotkeyRegistered();
    Ui::setStyleProperty(m_statusDot, "state", on ? QStringLiteral("on") : QStringLiteral("off"));
    m_statusTitle->setText(on ? QStringLiteral("Switching is on") : QStringLiteral("Switching is paused"));
    m_statusCaption->setText(on ? QStringLiteral("Dialogs jump to the last folder you used.")
                                : (hotkeyOk || recentOk ? QStringLiteral("Only the shortcuts work while paused.")
                                                        : QStringLiteral("Dialogs keep their own folder.")));
    m_toggleButton->setText(on ? QStringLiteral("Pause") : QStringLiteral("Resume"));
    Ui::setStyleProperty(m_toggleButton, "variant", on ? QString() : QStringLiteral("primary"));

    const AppState::LastSwitch last = m_state->lastSwitch();
    if (last.isValid()) {
        m_sourceChip->set(QStringLiteral("src"), last.source);
        m_resultChip->set(last.applied ? QStringLiteral("ok") : QStringLiteral("err"),
                          last.applied ? QStringLiteral("Applied") : QStringLiteral("Not applied"));
        m_timeLabel->setText(last.when.toString(QStringLiteral("HH:mm")));
        m_folderValue->setText(last.path);
        Ui::setStyleProperty(m_folderValue, "empty", false);
        // Reintentar con el atajo solo sirve si el atajo quedo registrado.
        m_folderCaption->setText(hotkeyOk ? QStringLiteral("The dialog didn't take it. Retry with Ctrl+Alt+O.")
                                          : QStringLiteral("The dialog didn't take it. Pick the folder by hand."));
        Ui::setStyleProperty(m_folderCaption, "tone", QStringLiteral("err"));
        m_folderCaption->setVisible(!last.applied);
    } else {
        m_folderValue->setText(QStringLiteral("No folder yet"));
        m_folderValue->setToolTip(QString());
        Ui::setStyleProperty(m_folderValue, "empty", true);
        m_folderCaption->setText(QStringLiteral("Open a folder in Explorer, then go to a file dialog."));
        Ui::setStyleProperty(m_folderCaption, "tone", QString());
        m_folderCaption->setVisible(true);
    }
    m_sourceChip->setVisible(last.isValid());
    m_resultChip->setVisible(last.isValid());
    m_timeLabel->setVisible(last.isValid());

    // Reflejar no es escribir: sin senales, setChecked no llega a AppState.
    m_autoSwitchCheck->blockSignals(true);
    m_autoSwitchCheck->setChecked(m_state->autoSwitch());
    m_autoSwitchCheck->blockSignals(false);
    m_updatesCheck->blockSignals(true);
    m_updatesCheck->setChecked(m_state->checkUpdatesAtStartup());
    m_updatesCheck->blockSignals(false);

    m_hotkeyBusyChip->setVisible(!hotkeyOk);
    m_hotkeyCaption->setText(hotkeyOk ? QStringLiteral("Press it inside a file dialog to jump right away.")
                                      : QStringLiteral("Another app took it. Automatic switching isn't affected."));
    Ui::setStyleProperty(m_hotkeyCaption, "tone", hotkeyOk ? QString() : QStringLiteral("err"));
    m_recentBusyChip->setVisible(!recentOk);
    m_recentCaption->setText(recentOk ? QStringLiteral("Press it inside a file dialog to pick a recent folder.")
                                      : QStringLiteral("Another app took it. The other shortcut isn't affected."));
    Ui::setStyleProperty(m_recentCaption, "tone", recentOk ? QString() : QStringLiteral("err"));
    fitHeight();
}

void MainWindow::fitHeight()
{
    // Ancho fijo y alto segun el contenido ya pulido (mismo criterio que el HelpDialog de VD:
    // adjustSize() calcula antes de que la hoja de estilo le ponga la fuente a los labels).
    ensurePolished();
    for (QWidget *child : findChildren<QWidget *>()) {
        child->ensurePolished();
    }
    QLayout *rootLayout = centralWidget()->layout();
    rootLayout->invalidate();
    rootLayout->activate();
    const int height = rootLayout->hasHeightForWidth() ? rootLayout->totalHeightForWidth(kWindowWidth)
                                                       : rootLayout->totalSizeHint().height();
    setFixedSize(kWindowWidth, height);
}

void MainWindow::applyAutoStartFixture(bool enabled, bool available)
{
    m_autoStartCheck->setChecked(enabled);
    setAutoStartTooltip(available);
}

void MainWindow::setAutoStartTooltip(bool available)
{
    m_autoStartCheck->setToolTip(available ? QStringLiteral("Start LGA FolderSwitch when you sign in to Windows")
                                           : QStringLiteral("Registers THIS development copy to start when you sign in"));
}

void MainWindow::syncAutoStartCheck()
{
    if (!m_autoStartCheck || m_mode != Mode::Normal) {
        return;
    }
    // Con las senales bloqueadas: reflejar el estado no es activarlo. Sin esto,
    // setChecked(true) disparaba onAutoStartToggled y reescribia el registro en
    // cada arranque de la app.
    m_autoStartCheck->blockSignals(true);
    m_autoStartCheck->setChecked(AutoStart::isEnabled());
    m_autoStartCheck->blockSignals(false);
}

void MainWindow::applyNativeFrame()
{
    // Una ventana Qt sin marco es un WS_POPUP: Windows no le da sombra, ni esquinas redondeadas,
    // ni la minimiza desde la barra de tareas. Se le agregan los estilos de una ventana con titulo
    // (WS_CAPTION, WS_SYSMENU, WS_MINIMIZEBOX) y WM_NCCALCSIZE (nativeEvent) deja el area no-cliente
    // en cero: el titulo nativo no se dibuja, pero Windows sigue tratandola como ventana normal.
    // Sin WS_THICKFRAME ni WS_MAXIMIZEBOX: tamano fijo, sin bordes de estirar ni maximizar.
    const HWND hwnd = reinterpret_cast<HWND>(winId());
    const LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    SetWindowLongPtrW(hwnd, GWL_STYLE, style | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
    // Un pixel de "marco" metido en el cliente es lo que hace que DWM pinte la sombra.
    const MARGINS margins{0, 0, 1, 0};
    DwmExtendFrameIntoClientArea(hwnd, &margins);
    // Esquinas redondeadas de Windows 11 (DWMWA_WINDOW_CORNER_PREFERENCE = 33, DWMWCP_ROUND = 2).
    // En Windows 10 el atributo no existe y la llamada falla sin efecto.
    const DWORD corners = 2;
    DwmSetWindowAttribute(hwnd, 33, &corners, sizeof(corners));
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    qInfo() << "[MainWindow] Marco nativo aplicado, estilo:" << Qt::hex
            << static_cast<qulonglong>(GetWindowLongPtrW(hwnd, GWL_STYLE));
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    const MSG *msg = static_cast<const MSG *>(message);
    if (m_nativeFrameApplied && msg->message == WM_NCCALCSIZE && msg->wParam == TRUE) {
        // Todo el rectangulo de la ventana es cliente: la barra de titulo es TitleBar.
        *result = 0;
        return true;
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::showEvent(QShowEvent *event)
{
    // La primera vez que se muestra ya existe el HWND. Nunca en la captura: no hay ventana real.
    if (!m_nativeFrameApplied && m_mode == Mode::Normal
        && QGuiApplication::platformName() == QLatin1String("windows")) {
        m_nativeFrameApplied = true;
        applyNativeFrame();
    }
    // Estado real en cada apertura: un cambio hecho por fuera (Task Manager >
    // Startup, otra copia de la app, un instalador) tiene que verse sin reiniciar.
    syncAutoStartCheck();
    QMainWindow::showEvent(event);
}

void MainWindow::onAutoStartToggled(bool checked)
{
    const bool ok = AutoStart::setEnabled(checked);
    // Se loguea el valor que QUEDO en el registro, no solo el resultado de la escritura: es
    // la unica evidencia de que la entrada aterrizo donde Windows la lee.
    qInfo() << "[MainWindow] AutoStart" << (checked ? "ON" : "OFF") << "ok:" << ok
            << "| valor en Run ahora:"
            << (AutoStart::storedCommand().isEmpty() ? QStringLiteral("(ninguno)")
                                                     : AutoStart::storedCommand());
    if (!ok) {
        syncAutoStartCheck(); // revertir el visual si fallo
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // No cerramos la app: ocultamos a la bandeja. Salir solo desde "Quit" del tray.
    hide();
    event->ignore();
}
