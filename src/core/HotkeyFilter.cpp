#include "core/HotkeyFilter.h"

#include <QDebug>

#include <windows.h>

namespace {
constexpr int kHotkeyId = 1;
}

HotkeyFilter::HotkeyFilter(QObject *parent)
    : QObject(parent)
    , m_hotkeyId(kHotkeyId)
{
    m_registered = RegisterHotKey(nullptr, m_hotkeyId, MOD_CONTROL | MOD_ALT, 'O');
    if (!m_registered) {
        qWarning() << "[HotkeyFilter] No se pudo registrar Ctrl+Alt+O (ya en uso por otra app?)";
    } else {
        qDebug() << "[HotkeyFilter] Ctrl+Alt+O registrado.";
    }
}

HotkeyFilter::~HotkeyFilter()
{
    if (m_registered) {
        UnregisterHotKey(nullptr, m_hotkeyId);
    }
}

bool HotkeyFilter::nativeEventFilter(const QByteArray &eventType, void *message, qintptr * /*result*/)
{
    if (eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG") {
        return false;
    }
    MSG *msg = static_cast<MSG *>(message);
    if (msg->message == WM_HOTKEY && static_cast<int>(msg->wParam) == m_hotkeyId) {
        emit hotkeyPressed();
        return true;
    }
    return false;
}
