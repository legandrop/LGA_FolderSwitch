#ifndef FOLDERSWITCH_FOREGROUNDWATCHER_H
#define FOLDERSWITCH_FOREGROUNDWATCHER_H

#include <QObject>

#include <windows.h>

// Escucha EVENT_SYSTEM_FOREGROUND via SetWinEventHook y emite foregroundChanged()
// cada vez que cambia la ventana en primer plano.
class ForegroundWatcher : public QObject
{
    Q_OBJECT

public:
    explicit ForegroundWatcher(QObject *parent = nullptr);
    ~ForegroundWatcher() override;

signals:
    void foregroundChanged(quintptr hwnd);

private slots:
    void handleForegroundChanged(quintptr hwnd);

private:
    static void CALLBACK winEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
                                       LONG idObject, LONG idChild,
                                       DWORD eventThread, DWORD eventTime);

    HWINEVENTHOOK m_hook = nullptr;
    static ForegroundWatcher *s_instance;
};

#endif // FOLDERSWITCH_FOREGROUNDWATCHER_H
