#include "core/ForegroundWatcher.h"

#include <QMetaObject>

ForegroundWatcher *ForegroundWatcher::s_instance = nullptr;

ForegroundWatcher::ForegroundWatcher(QObject *parent)
    : QObject(parent)
{
    s_instance = this;
    m_hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND,
                             nullptr, &ForegroundWatcher::winEventProc,
                             0, 0, WINEVENT_OUTOFCONTEXT);
}

ForegroundWatcher::~ForegroundWatcher()
{
    if (m_hook) {
        UnhookWinEvent(m_hook);
        m_hook = nullptr;
    }
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

void CALLBACK ForegroundWatcher::winEventProc(HWINEVENTHOOK /*hook*/, DWORD event, HWND hwnd,
                                              LONG idObject, LONG /*idChild*/,
                                              DWORD /*eventThread*/, DWORD /*eventTime*/)
{
    if (event != EVENT_SYSTEM_FOREGROUND || idObject != OBJID_WINDOW || !s_instance) {
        return;
    }
    QMetaObject::invokeMethod(s_instance, "handleForegroundChanged", Qt::QueuedConnection,
                              Q_ARG(quintptr, reinterpret_cast<quintptr>(hwnd)));
}

void ForegroundWatcher::handleForegroundChanged(quintptr hwnd)
{
    emit foregroundChanged(hwnd);
}
