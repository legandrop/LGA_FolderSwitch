#include "core/DialogSwitcher.h"

#include <QDebug>
#include <QDir>
#include <QTimer>

namespace {

HWND findFileNameEdit(HWND dlg)
{
    // Camino tipico: ComboBoxEx32 -> ComboBox -> Edit.
    HWND comboEx = FindWindowExW(dlg, nullptr, L"ComboBoxEx32", nullptr);
    if (comboEx) {
        HWND combo = FindWindowExW(comboEx, nullptr, L"ComboBox", nullptr);
        if (combo) {
            HWND edit = FindWindowExW(combo, nullptr, L"Edit", nullptr);
            if (edit) {
                return edit;
            }
        }
    }
    // Fallback: Edit directo hijo del dialogo.
    return FindWindowExW(dlg, nullptr, L"Edit", nullptr);
}

QString getEditText(HWND edit)
{
    const int len = static_cast<int>(SendMessageW(edit, WM_GETTEXTLENGTH, 0, 0));
    if (len <= 0) {
        return QString();
    }
    std::wstring buf(len + 1, L'\0');
    SendMessageW(edit, WM_GETTEXT, static_cast<WPARAM>(buf.size()),
                reinterpret_cast<LPARAM>(buf.data()));
    return QString::fromStdWString(buf.c_str());
}

void setEditText(HWND edit, const QString &text)
{
    const std::wstring w = text.toStdWString();
    SendMessageW(edit, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(w.c_str()));
}

bool looksLikeAbsolutePath(const QString &s)
{
    return s.contains(QStringLiteral(":\\")) || s.startsWith(QStringLiteral("\\\\"));
}

} // namespace

namespace DialogSwitcher {

bool switchDialog(HWND dlg, const QString &folder)
{
    if (!dlg || folder.isEmpty()) {
        return false;
    }

    HWND edit = findFileNameEdit(dlg);
    if (!edit) {
        qWarning() << "[DialogSwitcher] No se encontro el edit de nombre de archivo.";
        return false;
    }

    const QString previousText = getEditText(edit);
    const QString nativePath = QDir::toNativeSeparators(folder);

    setEditText(edit, nativePath);

    HWND okButton = GetDlgItem(dlg, IDOK);
    SendMessageW(dlg, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED),
                reinterpret_cast<LPARAM>(okButton));

    HWND editCapture = edit;
    QString restoreText = previousText;
    QTimer::singleShot(150, [editCapture, restoreText]() {
        if (!IsWindow(editCapture)) {
            return;
        }
        if (restoreText.isEmpty()) {
            setEditText(editCapture, QString());
        } else if (!looksLikeAbsolutePath(restoreText)) {
            setEditText(editCapture, restoreText);
        }
        // Si restoreText era un path absoluto, dejamos el que quedo (ya navegado).
    });

    qDebug() << "[DialogSwitcher] Inyectado:" << nativePath;
    return true;
}

} // namespace DialogSwitcher
