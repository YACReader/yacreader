#ifndef RESIZE_IMAGE_H
#define RESIZE_IMAGE_H

#include <cmath>
#include <QPixmap>
#include <QImage>

enum class ScaleMethod {
    QtFast,
    QtSmooth, // Bilinear
    Bicubic,
    Lanczos,
    Area

};

QPixmap smartScalePixmap(const QPixmap &pixmap, int width, int height);
QPixmap scalePixmap(const QPixmap &pixmap, int width, int height, ScaleMethod method = ScaleMethod::QtSmooth);

#endif // RESIZE_IMAGE_H
