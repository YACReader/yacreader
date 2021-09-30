#include "rounded_corners_dialog.h"

#include <QtWidgets>

YACReader::RoundedCornersDialog::RoundedCornersDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void YACReader::RoundedCornersDialog::paintEvent(QPaintEvent *)
{
    qreal radius = 36.0; // desired radius in absolute pixels

    if (!(windowFlags() & Qt::FramelessWindowHint) && !testAttribute(Qt::WA_TranslucentBackground))
        return; // nothing to do

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Paint thyself.
    QRectF rect(QPointF(0, 0), size());

    p.setPen(Qt::NoPen);

    // Set the brush from palette role.
    p.setBrush(palette().brush(backgroundRole()));
    // Got radius?  Otherwise draw a quicker rect.
    if (radius > 0.0)
        p.drawRoundedRect(rect, radius, radius, Qt::AbsoluteSize);
    else
        p.drawRect(rect);

    // C'est finí
    p.end();
}
