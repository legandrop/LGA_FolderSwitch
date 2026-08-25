#include "tray/TrayController.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QIcon>
#include <QLockFile>
#include <QMessageBox>
#include <QSystemTrayIcon>

#include <objbase.h>

#include <QDateTime>
#include <QTextStream>

// Misma resolucion de config/ que las otras apps LGA: el exe corre desde build/,
// los config y el debug.log viven en la raiz del repo.
static QString configBaseDir()
{
    const QString appPath = QCoreApplication::applicationDirPath();
    if (appPath.contains(QStringLiteral("build"), Qt::CaseInsensitive)) {
        return QDir(appPath).absoluteFilePath(QStringLiteral(".."));
    }
    return appPath;
}

static bool debugLogEnabled()
{
    QFile file(QDir::cleanPath(QDir(configBaseDir()).filePath(QStringLiteral("config/debug_flags.txt"))));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) {
            continue;
        }
        const QStringList parts = line.split(QLatin1Char('='));
        if (parts.size() == 2 && parts[0].trimmed() == QStringLiteral("log")) {
            const QString v = parts[1].trimmed().toLower();
            return v == QStringLiteral("true") || v == QStringLiteral("1") || v == QStringLiteral("yes");
        }
    }
    return false;
}

// Log a archivo: la app es WIN32 (sin consola), sin esto qDebug es invisible.
// Se activa con log=true en config/debug_flags.txt.
static void fileMessageHandler(QtMsgType, const QMessageLogContext &, const QString &msg)
{
    static const bool enabled = debugLogEnabled();
    if (!enabled) {
        return;
    }
    static QFile logFile(QDir::cleanPath(QDir(configBaseDir()).filePath(QStringLiteral("debug.log"))));
    static bool opened = logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    if (!opened) {
        return;
    }
    QTextStream out(&logFile);
    out << QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz")) << ' ' << msg << '\n';
    out.flush();
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(fileMessageHandler);
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    QApplication::setApplicationName("LGA_FolderSwitch");
    QApplication::setApplicationVersion(FOLDERSWITCH_VERSION);
    QApplication::setOrganizationName("LGA");
    QApplication::setQuitOnLastWindowClosed(false);

    // COM en modo apartment: lo necesita FolderResolver (IShellWindows/IWebBrowser2).
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    // Instancia unica.
    static QLockFile singleInstanceLock(
        QDir(QDir::tempPath()).filePath(QStringLiteral("com.lga.folderswitch.singleton.lock")));
    if (!singleInstanceLock.tryLock(100)) {
        qWarning() << "LGA_FolderSwitch ya esta corriendo; esta instancia sale para no duplicar.";
        CoUninitialize();
        return 0;
    }

    const QStringList fontFiles = {
        ":/fonts/Inter_18pt-Regular.ttf",
        ":/fonts/Inter_18pt-Medium.ttf",
    };
    for (const QString &f : fontFiles) {
        QFontDatabase::addApplicationFont(f);
    }

    app.setWindowIcon(QIcon(":/icons/LGA_FolderSwitch.png"));

    QFile qssFile(":/styles/dark_theme.qss");
    if (qssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(qssFile.readAll()));
        qssFile.close();
    }

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "LGA FolderSwitch",
                              "No system tray available in this session.");
        CoUninitialize();
        return 1;
    }

    TrayController tray;
    qInfo() << "LGA_FolderSwitch iniciado. Corriendo en bandeja.";

    const int rc = app.exec();
    CoUninitialize();
    return rc;
}
