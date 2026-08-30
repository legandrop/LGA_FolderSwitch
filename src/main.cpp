#include "tray/TrayController.h"
#include "core/DialogSwitcher.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QIcon>
#include <QLockFile>
#include <QSystemTrayIcon>
#include <QTimer>

#include <objbase.h>
#include <cstdlib>

#include <QDateTime>
#include <QTextStream>

#include <functional>

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

namespace {

/**
 * Marcador de version embebido en el binario. Lo lee un guard de instalador (findstr
 * sobre el exe) para verificar QUE VERSION quedo compilada antes de empaquetar,
 * taggear y publicar. Mismo mecanismo que LGA_MediaTools_v2 (ver kBuildVersionMarker
 * en su main.cpp).
 *
 * DOS condiciones que no se pueden romper sin romper el guard:
 *  - Literal NARROW (char[], no QStringLiteral/QString): un findstr sobre el exe
 *    busca bytes ASCII/UTF-8, y un literal UTF-16 no aparece.
 *  - REFERENCIADO: sin uso, el linker lo descarta. El qDebug() de abajo lo referencia
 *    y de paso deja la version compilada en el log.
 */
static const char kBuildVersionMarker[] = "LGA_FOLDERSWITCH_BUILD_VERSION=" FOLDERSWITCH_VERSION;

} // namespace

int main(int argc, char *argv[])
{
    qInstallMessageHandler(fileMessageHandler);
    qDebug() << kBuildVersionMarker;
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    QApplication::setApplicationName("LGA_FolderSwitch");
    QApplication::setApplicationVersion(FOLDERSWITCH_VERSION);
    QApplication::setOrganizationName("LGA");
    QApplication::setQuitOnLastWindowClosed(false);

    // COM en modo apartment: lo necesita FolderResolver (IShellWindows/IWebBrowser2).
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    // Modo de prueba oculto: --test-switch <hwnd_decimal> <folder>
    // No levanta el tray; hace el switch sobre ese HWND (Qt o Win32 segun
    // corresponda, decidido dentro de DialogSwitcher::switchDialog) y sale.
    if (argc >= 4 && QString::fromLocal8Bit(argv[1]) == QStringLiteral("--test-switch")) {
        bool okConv = false;
        const quintptr hwndValue = QString::fromLocal8Bit(argv[2]).toULongLong(&okConv);
        const QString folder = QString::fromLocal8Bit(argv[3]);
        HWND dlg = okConv ? reinterpret_cast<HWND>(hwndValue) : nullptr;

        qDebug() << "[test-switch] hwnd=" << argv[2] << "folder=" << folder;
        const bool ok = dlg ? DialogSwitcher::switchDialog(dlg, folder) : false;
        qDebug() << "[test-switch] resultado:" << (ok ? "OK" : "FALLO");

        CoUninitialize();
        return ok ? 0 : 1;
    }

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

    // Al arrancar con Windows (Run key), el shell suele no tener la bandeja lista
    // todavia: explorer.exe sigue inicializandose cuando ya nos lanzaron. Un loop
    // bloqueante ANTES de app.exec() (QThread::msleep en el hilo principal) dejaba
    // el proceso entero sin bombear mensajes de Windows durante toda la espera, y
    // el shell lo mostraba colgado/sin respuesta en el arranque de sesion -- esa
    // era la razon por la que la app no aparecia despues de reiniciar, aunque si
    // andaba al abrirla a mano. El reintento ahora vive DENTRO del event loop de
    // Qt: arranca junto con app.exec() y no bloquea nada.
    constexpr int kTrayWaitMs = 90000;
    constexpr int kTrayPollMs = 500;
    int trayWaitedMs = 0;
    std::function<void()> pollTray;
    pollTray = [&app, &trayWaitedMs, &pollTray]() {
        if (QSystemTrayIcon::isSystemTrayAvailable()) {
            if (trayWaitedMs > 0) {
                qInfo() << "La bandeja tardo" << trayWaitedMs << "ms en estar disponible.";
            }
            auto *tray = new TrayController(&app);
            Q_UNUSED(tray);
            qInfo() << "LGA_FolderSwitch iniciado. Corriendo en bandeja.";
            return;
        }
        if (trayWaitedMs >= kTrayWaitMs) {
            // Nada de QMessageBox: es una app de bandeja, un modal al arranque no
            // lo ve nadie y ademas dejaria el proceso colgado.
            qWarning() << "No hay bandeja del sistema despues de esperar"
                       << (kTrayWaitMs / 1000) << "s; se sale.";
            QCoreApplication::exit(1);
            return;
        }
        trayWaitedMs += kTrayPollMs;
        QTimer::singleShot(kTrayPollMs, qApp, pollTray);
    };
    QTimer::singleShot(0, qApp, pollTray);

    const int rc = app.exec();
    CoUninitialize();
    return rc;
}
