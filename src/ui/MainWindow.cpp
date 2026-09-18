#include "ui/MainWindow.h"

#include "core/AppState.h"
#include "ui/TabHeader.h"
#include "ui/Theme.h"
#include "ui/UiWidgets.h"
#include "windows/AutoStart.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

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

} // namespace

MainWindow::MainWindow(AppState *state, Mode mode, QWidget *parent)
    : QMainWindow(parent)
    , m_state(state)
    , m_mode(mode)
{
    setWindowTitle(QStringLiteral("LGA FolderSwitch"));
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

    m_header = new TabHeader(QStringLiteral("SETTINGS"), central);
    root->addWidget(m_header);

    auto *content = new QWidget(central);
    content->setObjectName(QStringLiteral("content"));
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(14, 12, 14, 14);
    layout->setSpacing(10);
    root->addWidget(content, 1);

    // ---------- Estado: On / Paused ----------
    auto *statusCard = card(content);
    auto *statusRow = new QHBoxLayout(statusCard);
    statusRow->setContentsMargins(14, 12, 14, 12);
    statusRow->setSpacing(12);
    m_statusDot = label(QString(), "statusDot", statusCard);
    statusRow->addWidget(m_statusDot, 0, Qt::AlignVCenter);
    auto *statusTexts = new QVBoxLayout();
    statusTexts->setSpacing(2);
    m_statusTitle = label(QString(), "cardTitle", statusCard);
    m_statusCaption = caption(QString(), statusCard);
    statusTexts->addWidget(m_statusTitle);
    statusTexts->addWidget(m_statusCaption);
    statusRow->addLayout(statusTexts, 1);
    m_toggleButton = Ui::button(QString(), QString(), QString(), statusCard);
    m_toggleButton->setObjectName(QStringLiteral("toggleButton"));
    m_toggleButton->setMinimumWidth(78);
    statusRow->addWidget(m_toggleButton, 0, Qt::AlignVCenter);
    layout->addWidget(statusCard);

    // ---------- Ultima carpeta ----------
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

    // ---------- Opciones ----------
    auto *optionsCard = card(content);
    auto *options = new QVBoxLayout(optionsCard);
    options->setContentsMargins(14, 12, 14, 12);
    options->setSpacing(4);

    m_autoSwitchCheck = new QCheckBox(QStringLiteral("Switch automatically"), optionsCard);
    options->addWidget(m_autoSwitchCheck);
    auto *autoCaption = caption(QStringLiteral("When you return to a dialog from the file manager."), optionsCard);
    autoCaption->setContentsMargins(kCheckIndent, 0, 0, 0);
    options->addWidget(autoCaption);
    options->addSpacing(8);

    auto *hotkeyRow = new QHBoxLayout();
    hotkeyRow->setContentsMargins(kCheckIndent, 0, 0, 0);
    hotkeyRow->setSpacing(5);
    auto *hotkeyLabel = label(QStringLiteral("Manual shortcut"), "optionLabel", optionsCard);
    hotkeyRow->addWidget(hotkeyLabel);
    hotkeyRow->addSpacing(5);
    for (const QString &key : {QStringLiteral("Ctrl"), QStringLiteral("Alt"), QStringLiteral("O")}) {
        auto *chip = new Chip(optionsCard);
        chip->set(QStringLiteral("key"), key);
        hotkeyRow->addWidget(chip, 0, Qt::AlignVCenter);
    }
    hotkeyRow->addStretch(1);
    m_hotkeyBusyChip = new Chip(optionsCard);
    m_hotkeyBusyChip->set(QStringLiteral("err"), QStringLiteral("In use"));
    hotkeyRow->addWidget(m_hotkeyBusyChip, 0, Qt::AlignVCenter);
    options->addLayout(hotkeyRow);
    m_hotkeyCaption = caption(QString(), optionsCard);
    m_hotkeyCaption->setContentsMargins(kCheckIndent, 0, 0, 0);
    options->addWidget(m_hotkeyCaption);

    options->addSpacing(8);
    auto *divider = new QFrame(optionsCard);
    divider->setObjectName(QStringLiteral("divider"));
    options->addWidget(divider);
    options->addSpacing(8);

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
    m_autoStartCheck = new QCheckBox(QStringLiteral("Start with Windows"), optionsCard);
    options->addWidget(m_autoStartCheck);
    layout->addWidget(optionsCard);

    setCentralWidget(central);
}

void MainWindow::connectWrites()
{
    connect(m_header, &TabHeader::helpClicked, this, &MainWindow::helpRequested);
    connect(m_state, &AppState::changed, this, &MainWindow::refresh);
    connect(m_toggleButton, &QPushButton::clicked, this, [this]() {
        m_state->setEnabled(!m_state->enabled());
    });
    connect(m_autoSwitchCheck, &QCheckBox::toggled, m_state, &AppState::setAutoSwitch);
    connect(m_autoStartCheck, &QCheckBox::toggled, this, &MainWindow::onAutoStartToggled);
}

void MainWindow::refresh()
{
    const bool on = m_state->enabled();
    Ui::setStyleProperty(m_statusDot, "state", on ? QStringLiteral("on") : QStringLiteral("off"));
    m_statusTitle->setText(on ? QStringLiteral("Switching is on") : QStringLiteral("Switching is paused"));
    m_statusCaption->setText(on ? QStringLiteral("Dialogs jump to the last folder you used.")
                                : QStringLiteral("Dialogs keep their own folder."));
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
        m_folderCaption->setText(QStringLiteral("The dialog didn't take it. Retry with Ctrl+Alt+O."));
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

    const bool hotkeyOk = m_state->hotkeyRegistered();
    m_hotkeyBusyChip->setVisible(!hotkeyOk);
    m_hotkeyCaption->setText(hotkeyOk ? QStringLiteral("Press it inside a file dialog to jump right away.")
                                      : QStringLiteral("Another app took it. Automatic switching still works."));
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

void MainWindow::showEvent(QShowEvent *event)
{
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
