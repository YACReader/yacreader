#ifndef RESIZE_IMAGE_H
#define RESIZE_IMAGE_H

#include <QImage>
#include <QPixmap>

enum class ScaleMethod {
    Nearest = 0,
    Bilinear = 1,
    Lanczos = 2
};

// Base scaling API — callers are responsible for supplying the correct target dimensions.

QPixmap scalePixmap(const QPixmap &pixmap, int width, int height, ScaleMethod method = ScaleMethod::Lanczos);

QImage scaleImage(const QImage &image, int width, int height, ScaleMethod method = ScaleMethod::Lanczos);

#endif // RESIZE_IMAGE_H
