#include "ui/TitleBar.h"
#include "ui/Theme.h"
#include "ui/UiWidgets.h"

#include <QAbstractButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QWindow>

namespace {

// Medidas del diseno aprobado (opcion B del canvas "FolderSwitch sin tabs"): 36 px de alto con el
// borde inferior adentro, asi los botones ocupan los 35 de arriba.
constexpr int kBarHeight = 36;
constexpr int kButtonHeight = kBarHeight - 1;

// Icono de la app a 16 px desde el PNG de 512. QIcon::pixmap lo reduce con filtro de area a la escala
// del dispositivo; un drawPixmap directo con SmoothPixmapTransform muestrea de a puntos y se come
// trazos finos del logo.
class AppIcon : public QWidget
{
public:
    explicit AppIcon(QWidget *parent) : QWidget(parent), m_icon(QStringLiteral(":/icons/LGA_FolderSwitch.png"))
    {
        setFixedSize(16, 16);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.drawPixmap(rect(), m_icon.pixmap(size(), devicePixelRatioF()));
    }

private:
    QIcon m_icon;
};

// Boton de la barra: solo un glifo. Con el mouse encima se aclara el glifo y aparece un fondo.
class TitleButton : public QAbstractButton
{
public:
    TitleButton(Icon icon, const QSize &size, int glyph, const char *normalColor, int radius, const QString &name,
                QWidget *parent)
        : QAbstractButton(parent)
        , m_icon(icon)
        , m_glyph(glyph)
        , m_normalColor(Theme::color(normalColor))
        , m_radius(radius)
    {
        setFixedSize(size);
        setFocusPolicy(Qt::NoFocus);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover);
        setToolTip(name);
        setAccessibleName(name);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const bool hot = underMouse();
        if (hot) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(isDown() ? QColor(0x24, 0x24, 0x24) : QColor(0x2a, 0x2a, 0x2a));
            painter.drawRoundedRect(QRectF(rect()), m_radius, m_radius);
        }
        const QRectF glyph((width() - m_glyph) / 2.0, (height() - m_glyph) / 2.0, m_glyph, m_glyph);
        Icons::paint(painter, m_icon, glyph, hot ? Theme::color(Theme::kTextStrong) : m_normalColor);
    }

private:
    Icon m_icon;
    int m_glyph;
    QColor m_normalColor;
    int m_radius;
};

} // namespace

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("titleBar"));
    setFixedHeight(kBarHeight);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 0, 1);
    layout->setSpacing(0);
    layout->addWidget(new AppIcon(this), 0, Qt::AlignVCenter);
    layout->addSpacing(9);
    auto *title = new QLabel(QStringLiteral("LGA FolderSwitch"), this);
    title->setObjectName(QStringLiteral("titleBarTitle"));
    title->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(title, 0, Qt::AlignVCenter);
    layout->addStretch(1);

    auto *help = new TitleButton(Icon::Help, QSize(34, 34), 16, Theme::kIcon, 5, QStringLiteral("Help"), this);
    layout->addWidget(help, 0, Qt::AlignVCenter);

    layout->addSpacing(4);
    auto *separator = new QFrame(this);
    separator->setObjectName(QStringLiteral("titleBarSeparator"));
    separator->setFixedSize(1, 16);
    layout->addWidget(separator, 0, Qt::AlignVCenter);
    layout->addSpacing(4);

    auto *minimize = new TitleButton(Icon::Minimize, QSize(40, kButtonHeight), 10, Theme::kTextMuted, 0,
                                     QStringLiteral("Minimize"), this);
    layout->addWidget(minimize, 0, Qt::AlignTop);
    auto *close = new TitleButton(Icon::Close, QSize(40, kButtonHeight), 10, Theme::kTextMuted, 0,
                                  QStringLiteral("Close"), this);
    layout->addWidget(close, 0, Qt::AlignTop);

    connect(help, &QAbstractButton::clicked, this, &TitleBar::helpClicked);
    connect(minimize, &QAbstractButton::clicked, this, [this]() { window()->showMinimized(); });
    connect(close, &QAbstractButton::clicked, this, [this]() { window()->close(); });
}

void TitleBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Theme::color(Theme::kTitleBar));
    painter.fillRect(QRect(0, height() - 1, width(), 1), Theme::color(Theme::kDivider));
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    // Los botones se quedan con su propio click; lo que llega aca es la zona libre de la barra.
    if (event->button() == Qt::LeftButton && window()->windowHandle()) {
        window()->windowHandle()->startSystemMove();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}
