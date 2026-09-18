#ifndef FOLDERSWITCH_HOTKEYFILTER_H
#define FOLDERSWITCH_HOTKEYFILTER_H

#include <QAbstractNativeEventFilter>
#include <QObject>

// Registra el hotkey global Ctrl+Alt+O (RegisterHotKey) y emite hotkeyPressed()
// cuando Windows manda WM_HOTKEY. Se instala con
// QCoreApplication::installNativeEventFilter(...).
class HotkeyFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit HotkeyFilter(QObject *parent = nullptr);
    ~HotkeyFilter() override;

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

    // False si RegisterHotKey fallo (otra app ya tiene Ctrl+Alt+O). La UI lo muestra.
    bool isRegistered() const { return m_registered; }

signals:
    void hotkeyPressed();

private:
    int m_hotkeyId = 0;
    bool m_registered = false;
};

#endif // FOLDERSWITCH_HOTKEYFILTER_H
