#include "ui/MainWindow.h"
#include "windows/AutoStart.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QFontMetrics>
#include <QLabel>
#include <QSettings>
#include <QVBoxLayout>

namespace {
constexpr int kWindowWidth = 380;
constexpr int kWindowHeight = 240;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("LGA FolderSwitch"));
    buildUi();
    setFixedSize(kWindowWidth, kWindowHeight);
    loadSettings();
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    auto *statusTitle = new QLabel(QStringLiteral("Última carpeta detectada:"), central);
    layout->addWidget(statusTitle);

    m_statusLabel = new QLabel(QStringLiteral("(ninguna todavía)"), central);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
    m_statusLabel->setWordWrap(false);
    layout->addWidget(m_statusLabel);

    layout->addSpacing(8);

    m_autoSwitchCheck = new QCheckBox(QStringLiteral("Auto-switch al volver al diálogo"), central);
    connect(m_autoSwitchCheck, &QCheckBox::toggled, this, [this](bool checked) {
        QSettings settings(QStringLiteral("LGA"), QStringLiteral("FolderSwitch"));
        settings.setValue(QStringLiteral("autoSwitch"), checked);
        emit autoSwitchToggled(checked);
    });
    layout->addWidget(m_autoSwitchCheck);

    m_autoStartCheck = new QCheckBox(QStringLiteral("Iniciar con Windows"), central);
    connect(m_autoStartCheck, &QCheckBox::toggled, this, &MainWindow::onAutoStartToggled);
    layout->addWidget(m_autoStartCheck);

    m_enabledCheck = new QCheckBox(QStringLiteral("Activado"), central);
    connect(m_enabledCheck, &QCheckBox::toggled, this, [this](bool checked) {
        QSettings settings(QStringLiteral("LGA"), QStringLiteral("FolderSwitch"));
        settings.setValue(QStringLiteral("enabled"), checked);
        emit masterEnabledToggled(checked);
    });
    layout->addWidget(m_enabledCheck);

    layout->addStretch(1);

    auto *hotkeyLabel = new QLabel(QStringLiteral("Hotkey manual: Ctrl+Alt+O"), central);
    hotkeyLabel->setObjectName(QStringLiteral("hotkeyLabel"));
    layout->addWidget(hotkeyLabel);

    setCentralWidget(central);
}

void MainWindow::loadSettings()
{
    QSettings settings(QStringLiteral("LGA"), QStringLiteral("FolderSwitch"));
    const bool autoSwitch = settings.value(QStringLiteral("autoSwitch"), true).toBool();
    const bool enabled = settings.value(QStringLiteral("enabled"), true).toBool();

    m_autoSwitchCheck->setChecked(autoSwitch);
    m_enabledCheck->setChecked(enabled);
    m_autoStartCheck->setChecked(AutoStart::isEnabled());
}

bool MainWindow::autoSwitchEnabled() const
{
    return m_autoSwitchCheck && m_autoSwitchCheck->isChecked();
}

bool MainWindow::masterEnabled() const
{
    return m_enabledCheck && m_enabledCheck->isChecked();
}

void MainWindow::setLastDetectedFolder(const QString &folder)
{
    if (!m_statusLabel) {
        return;
    }
    if (folder.isEmpty()) {
        m_statusLabel->setText(QStringLiteral("(ninguna todavía)"));
        return;
    }
    const QFontMetrics fm(m_statusLabel->font());
    const QString elided = fm.elidedText(folder, Qt::ElideMiddle, kWindowWidth - 32 - 16);
    m_statusLabel->setText(elided);
    m_statusLabel->setToolTip(folder);
}

void MainWindow::onAutoStartToggled(bool checked)
{
    AutoStart::setEnabled(checked);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // No cerramos la app: ocultamos a la bandeja. Salir solo desde "Quit" del tray.
    hide();
    event->ignore();
}
