#include "caption_buttons.h"

#include <QFontDatabase>
#include <QPainter>
#include <QVariantAnimation>

CaptionButtons::CaptionButtons(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFixedWidth(3 * ButtonWidth);

    const QStringList families = QFontDatabase::families();
    glyphFont.setFamily(families.contains(QStringLiteral("Segoe Fluent Icons")) ? QStringLiteral("Segoe Fluent Icons")
                                                                                : QStringLiteral("Segoe MDL2 Assets"));
    glyphFont.setPixelSize(10);

    for (int i = 0; i < 3; ++i) {
        animations[i] = new QVariantAnimation(this);
        connect(animations[i], &QVariantAnimation::valueChanged, this, [this, i](const QVariant &value) {
            hoverLevel[i] = value.toReal();
            update(buttonRect(Button(i)));
        });
    }
}

void CaptionButtons::setColors(const CaptionButtonColors &c)
{
    colors = c;
    update();
}

CaptionButtons::Button CaptionButtons::buttonAt(const QPoint &pos) const
{
    if (!rect().contains(pos))
        return None;
    return Button(qBound(0, pos.x() / ButtonWidth, 2));
}

void CaptionButtons::setHovered(Button button)
{
    if (button == hoveredButton)
        return;
    hoveredButton = button;
    for (int i = 0; i < 3; ++i) {
        const qreal target = i == button ? 1.0 : 0.0;
        if (qFuzzyCompare(hoverLevel[i] + 1, target + 1) && animations[i]->state() != QAbstractAnimation::Running)
            continue;
        const int duration = target > 0 ? HoverInMs : HoverOutMs;
        animations[i]->stop();
        if (duration <= 0) {
            hoverLevel[i] = target;
            update(buttonRect(Button(i)));
            continue;
        }
        animations[i]->setStartValue(hoverLevel[i]);
        animations[i]->setEndValue(target);
        animations[i]->setDuration(duration);
        animations[i]->start();
    }
}

void CaptionButtons::setPressed(Button button)
{
    if (button == pressedButton)
        return;
    pressedButton = button;
    update();
}

void CaptionButtons::setWindowActive(bool isActive)
{
    active = isActive;
    update();
}

void CaptionButtons::setMaximized(bool isMaximized)
{
    maximized = isMaximized;
    update(buttonRect(Maximize));
}

QRect CaptionButtons::buttonRect(Button button) const
{
    return QRect(int(button) * ButtonWidth, 0, ButtonWidth, height());
}

QString CaptionButtons::glyph(Button button) const
{
    switch (button) {
    case Minimize:
        return QString(QChar(0xE921));
    case Maximize:
        return QString(QChar(maximized ? 0xE923 : 0xE922));
    case Close:
        return QString(QChar(0xE8BB));
    default:
        return { };
    }
}

void CaptionButtons::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setFont(glyphFont);
    for (int i = 0; i < 3; ++i) {
        const Button button = Button(i);
        const QRect r = buttonRect(button);
        const bool isClose = button == Close;
        const bool isPressed = pressedButton == button && hoveredButton == button;

        QColor background = isClose ? colors.closeHover : colors.hover;
        if (isPressed)
            background = isClose ? colors.closePressed : colors.pressed;
        else
            background.setAlphaF(background.alphaF() * hoverLevel[i]);
        painter.fillRect(r, background);

        QColor foreground = active ? colors.foreground : colors.inactiveForeground;
        if (isClose && (isPressed || hoverLevel[i] > 0)) {
            const qreal level = isPressed ? 1.0 : hoverLevel[i];
            const QColor to = colors.closeForeground;
            foreground = QColor::fromRgbF(foreground.redF() + (to.redF() - foreground.redF()) * level,
                                          foreground.greenF() + (to.greenF() - foreground.greenF()) * level,
                                          foreground.blueF() + (to.blueF() - foreground.blueF()) * level,
                                          foreground.alphaF() + (to.alphaF() - foreground.alphaF()) * level);
        }
        painter.setPen(foreground);
        painter.drawText(r, Qt::AlignCenter, glyph(button));
    }
}
