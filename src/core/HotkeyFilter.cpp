#include "core/HotkeyFilter.h"

#include <QDebug>

#include <windows.h>

namespace {
constexpr int kHotkeyId = 1;
constexpr int kRecentHotkeyId = 2;
}

HotkeyFilter::HotkeyFilter(QObject *parent)
    : QObject(parent)
{
    m_registered = RegisterHotKey(nullptr, kHotkeyId, MOD_CONTROL | MOD_ALT, 'O');
    if (!m_registered) {
        qWarning() << "[HotkeyFilter] No se pudo registrar Ctrl+Alt+O (ya en uso por otra app?)";
    } else {
        qDebug() << "[HotkeyFilter] Ctrl+Alt+O registrado.";
    }
    // MOD_NOREPEAT: dejar apretado el atajo no abre un menu tras otro.
    m_recentRegistered = RegisterHotKey(nullptr, kRecentHotkeyId, MOD_CONTROL | MOD_ALT | MOD_SHIFT | MOD_NOREPEAT, 'O');
    if (!m_recentRegistered) {
        qWarning() << "[HotkeyFilter] No se pudo registrar Ctrl+Alt+Shift+O (ya en uso por otra app?)";
    } else {
        qDebug() << "[HotkeyFilter] Ctrl+Alt+Shift+O registrado.";
    }
}

HotkeyFilter::~HotkeyFilter()
{
    if (m_registered) {
        UnregisterHotKey(nullptr, kHotkeyId);
    }
    if (m_recentRegistered) {
        UnregisterHotKey(nullptr, kRecentHotkeyId);
    }
}

bool HotkeyFilter::nativeEventFilter(const QByteArray &eventType, void *message, qintptr * /*result*/)
{
    if (eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG") {
        return false;
    }
    MSG *msg = static_cast<MSG *>(message);
    if (msg->message != WM_HOTKEY) {
        return false;
    }
    if (static_cast<int>(msg->wParam) == kHotkeyId) {
        emit hotkeyPressed();
        return true;
    }
    if (static_cast<int>(msg->wParam) == kRecentHotkeyId) {
        emit recentHotkeyPressed();
        return true;
    }
    return false;
}
