#ifndef BOOK_FOLD_SHADOW_H
#define BOOK_FOLD_SHADOW_H

#include <QRectF>
#include <Qt>

class QPainter;

// Coordinates are in displayed page units. The painter may transform them into
// a magnifier crop, keeping the same fold geometry in both views.
void drawBookFoldShadow(QPainter &painter, const QRectF &pageRect, qreal seamRatio, Qt::Orientation seamOrientation);

#endif
