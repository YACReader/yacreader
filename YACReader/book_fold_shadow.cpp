#include "book_fold_shadow.h"

#include <QLinearGradient>
#include <QPainter>

void drawBookFoldShadow(QPainter &painter, const QRectF &pageRect, qreal seamRatio, Qt::Orientation seamOrientation)
{
    const bool verticalSeam = seamOrientation == Qt::Vertical;
    const qreal axisLength = verticalSeam ? pageRect.width() : pageRect.height();
    const qreal seamOffset = axisLength * seamRatio;
    const qreal radius = qMin(qreal(36), qMin(seamOffset, axisLength - seamOffset));
    if (radius <= 0 || pageRect.isEmpty()) {
        return;
    }

    const qreal seamPosition = (verticalSeam ? pageRect.left() : pageRect.top()) + seamOffset;
    const QRectF shadowRect = verticalSeam
            ? QRectF(seamPosition - radius, pageRect.top(), radius * 2, pageRect.height())
            : QRectF(pageRect.left(), seamPosition - radius, pageRect.width(), radius * 2);
    // Skip gradient setup when the exposed viewport or magnifier misses the fold.
    if (painter.hasClipping() && !painter.clipBoundingRect().intersects(shadowRect)) {
        return;
    }

    QLinearGradient gradient;
    if (verticalSeam) {
        gradient = QLinearGradient(seamPosition - radius, 0, seamPosition + radius, 0);
    } else {
        gradient = QLinearGradient(0, seamPosition - radius, 0, seamPosition + radius);
    }
    gradient.setColorAt(0.0, QColor(0, 0, 0, 0));
    gradient.setColorAt(0.32, QColor(0, 0, 0, 8));
    gradient.setColorAt(0.42, QColor(0, 0, 0, 35));
    gradient.setColorAt(0.48, QColor(0, 0, 0, 90));
    gradient.setColorAt(0.5, QColor(0, 0, 0, 125));
    gradient.setColorAt(0.54, QColor(0, 0, 0, 72));
    gradient.setColorAt(0.68, QColor(0, 0, 0, 18));
    gradient.setColorAt(1.0, QColor(0, 0, 0, 0));

    painter.save();
    painter.setClipRect(pageRect, Qt::IntersectClip);
    painter.fillRect(shadowRect, gradient);
    const QRectF creaseRect = verticalSeam
            ? QRectF(seamPosition - 0.5, pageRect.top(), 1, pageRect.height())
            : QRectF(pageRect.left(), seamPosition - 0.5, pageRect.width(), 1);
    painter.fillRect(creaseRect, QColor(0, 0, 0, 145));
    painter.restore();
}
