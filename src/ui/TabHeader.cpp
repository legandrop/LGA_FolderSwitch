#include "ui/TabHeader.h"
#include "ui/Theme.h"
#include "ui/UiWidgets.h"

#include <QAbstractButton>
#include <QFontMetricsF>
#include <QHBoxLayout>
#include <QPainter>
#include <QtMath>

namespace {

constexpr int BAR_HEIGHT = 50;
constexpr int TAB_PADDING_X = 22;

QFont tabFont()
{
    QFont font = Theme::uiFont(15, QFont::Medium);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 0.2);
    return font;
}

// Boton de esquina (37x50) con el icono de ayuda; se aclara al pasar el mouse.
class CornerButton : public QAbstractButton
{
public:
    explicit CornerButton(QWidget *parent) : QAbstractButton(parent)
    {
        setObjectName(QStringLiteral("helpCorner"));
        setFixedSize(37, BAR_HEIGHT);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::TabFocus);
        setAttribute(Qt::WA_Hover);
        setToolTip(QStringLiteral("Help and updates"));
        setAccessibleName(QStringLiteral("Help"));
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        const QColor color = underMouse() || hasFocus() ? Theme::color(Theme::kText) : Theme::color(Theme::kIcon);
        Icons::paint(painter, Icon::Help, QRectF((width() - 20) / 2.0, (height() - 20) / 2.0, 20, 20), color);
    }
};

} // namespace

TabHeader::TabHeader(const QString &tabText, QWidget *parent)
    : QWidget(parent)
    , m_tabText(tabText)
{
    setObjectName(QStringLiteral("tabHeader"));
    setFixedHeight(BAR_HEIGHT);
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 8, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    auto *help = new CornerButton(this);
    layout->addWidget(help);
    connect(help, &QAbstractButton::clicked, this, &TabHeader::helpClicked);
}

void TabHeader::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Theme::color(Theme::kTabBar));
    // Linea inferior de la barra; la pestana activa la tapa.
    painter.fillRect(QRect(0, BAR_HEIGHT - 1, width(), 1), Theme::color(Theme::kBorder));

    const QFont font = tabFont();
    const int textWidth = qFloor(QFontMetricsF(font).horizontalAdvance(m_tabText));
    const QRectF tab(0.5, 0.5, textWidth + TAB_PADDING_X * 2 + 1, BAR_HEIGHT + 4);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Theme::color(Theme::kBorder), 1.0));
    painter.setBrush(Theme::color(Theme::kWindow));
    painter.setClipRect(QRect(0, 0, width(), BAR_HEIGHT));
    painter.drawRoundedRect(tab, 4, 4);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Theme::color(Theme::kAccent));
    painter.setFont(font);
    painter.drawText(QRectF(1, 0, textWidth + TAB_PADDING_X * 2, BAR_HEIGHT), Qt::AlignCenter, m_tabText);
}
