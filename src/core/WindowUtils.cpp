#include "core/WindowUtils.h"

#include <QDebug>

#include <psapi.h>

namespace {

QString classNameOf(HWND hwnd)
{
    wchar_t buf[256] = {0};
    GetClassNameW(hwnd, buf, static_cast<int>(std::size(buf)));
    return QString::fromWCharArray(buf);
}

struct FindDescendantContext {
    QStringList classNames;
    bool found = false;
};

BOOL CALLBACK enumChildProc(HWND hwnd, LPARAM lParam)
{
    auto *ctx = reinterpret_cast<FindDescendantContext *>(lParam);
    const QString cls = classNameOf(hwnd);
    for (const QString &target : ctx->classNames) {
        if (cls.compare(target, Qt::CaseInsensitive) == 0) {
            ctx->found = true;
            return FALSE; // dejar de enumerar
        }
    }
    return TRUE;
}

bool hasDescendantOfClass(HWND hwnd, const QStringList &classNames)
{
    FindDescendantContext ctx;
    ctx.classNames = classNames;
    EnumChildWindows(hwnd, enumChildProc, reinterpret_cast<LPARAM>(&ctx));
    return ctx.found;
}

} // namespace

namespace WindowUtils {

bool isExplorerWindow(HWND hwnd)
{
    if (!hwnd) {
        return false;
    }
    return classNameOf(hwnd).compare(QStringLiteral("CabinetWClass"), Qt::CaseInsensitive) == 0;
}

QString processExeName(HWND hwnd)
{
    if (!hwnd) {
        return QString();
    }
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0) {
        return QString();
    }
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) {
        return QString();
    }
    wchar_t path[MAX_PATH] = {0};
    DWORD size = static_cast<DWORD>(std::size(path));
    QString result;
    if (QueryFullProcessImageNameW(hProcess, 0, path, &size)) {
        const QString fullPath = QString::fromWCharArray(path, static_cast<int>(size));
        const int slash = fullPath.lastIndexOf(QLatin1Char('\\'));
        result = (slash >= 0) ? fullPath.mid(slash + 1) : fullPath;
    }
    CloseHandle(hProcess);
    return result;
}

bool isXYplorerWindow(HWND hwnd)
{
    if (!hwnd) {
        return false;
    }
    // El chequeo por exe es el definitivo: la clase VB6 es generica.
    const QString exeName = processExeName(hwnd);
    if (exeName.compare(QStringLiteral("xyplorer.exe"), Qt::CaseInsensitive) == 0) {
        return true;
    }
    return classNameOf(hwnd).compare(QStringLiteral("ThunderRT6FormDC"), Qt::CaseInsensitive) == 0;
}

bool isFileManagerWindow(HWND hwnd)
{
    return isExplorerWindow(hwnd) || isXYplorerWindow(hwnd);
}

bool isFileDialogWindow(HWND hwnd)
{
    if (!hwnd) {
        return false;
    }
    if (classNameOf(hwnd).compare(QStringLiteral("#32770"), Qt::CaseInsensitive) != 0) {
        return false;
    }
    if (hasDescendantOfClass(hwnd, {QStringLiteral("DUIViewWndClassName"),
                                    QStringLiteral("SHELLDLL_DefView"),
                                    QStringLiteral("ComboBoxEx32")})) {
        return true;
    }
    return false;
}

} // namespace WindowUtils
