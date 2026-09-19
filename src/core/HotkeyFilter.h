#ifndef FOLDERSWITCH_HOTKEYFILTER_H
#define FOLDERSWITCH_HOTKEYFILTER_H

#include <QAbstractNativeEventFilter>
#include <QObject>

// Registra los atajos globales (RegisterHotKey) y avisa cuando Windows manda WM_HOTKEY:
//  - Ctrl+Alt+O: aplica la ultima carpeta al dialogo (hotkeyPressed).
//  - Ctrl+Alt+Shift+O: abre el menu de carpetas recientes (recentHotkeyPressed).
// RegisterHotKey compara los modificadores exactos: uno no dispara al otro.
// Se instala con QCoreApplication::installNativeEventFilter(...).
class HotkeyFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit HotkeyFilter(QObject *parent = nullptr);
    ~HotkeyFilter() override;

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

    // False si RegisterHotKey fallo (otra app ya tiene el atajo). La UI lo muestra.
    bool isRegistered() const { return m_registered; }
    bool isRecentRegistered() const { return m_recentRegistered; }

signals:
    void hotkeyPressed();
    void recentHotkeyPressed();

private:
    bool m_registered = false;
    bool m_recentRegistered = false;
};

#endif // FOLDERSWITCH_HOTKEYFILTER_H
