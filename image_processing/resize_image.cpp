#include "resize_image.h"

// include opencv
#include <opencv2/opencv.hpp>

QPixmap scalePixmapBicubic(const QPixmap &pixmap, int width, int height);
QPixmap scalePixmapLanczos(const QPixmap &pixmap, int width, int height);
QPixmap scalePixmapArea(const QPixmap &pixmap, int width, int height);
QPixmap scalePixmapOpenCV(const QPixmap &pixmap, int width, int height, cv::InterpolationFlags flags);

QPixmap smartScalePixmap(const QPixmap &pixmap, int width, int height)
{
    const int w = pixmap.width();
    const int h = pixmap.height();
    if ((w == width && h == height) || pixmap.isNull()) {
        return pixmap;
    }

    if (w <= width && h <= height) { // upscaling
        return scalePixmapLanczos(pixmap, width, height);
    }

    return pixmap;
}

QPixmap scalePixmap(const QPixmap &pixmap, int width, int height, ScaleMethod method)
{
    const int w = pixmap.width();
    const int h = pixmap.height();
    if (w == width && h == height) {
        return pixmap;
    }

    switch (method) {
    case ScaleMethod::QtFast:
        return pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::FastTransformation);
    case ScaleMethod::QtSmooth:
        return pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    case ScaleMethod::Bicubic:
        return scalePixmapBicubic(pixmap, width, height);
    case ScaleMethod::Lanczos:
        return scalePixmapLanczos(pixmap, width, height);
    case ScaleMethod::Area:
        return scalePixmapArea(pixmap, width, height);
    }
}

QPixmap scalePixmapBicubic(const QPixmap &pixmap, int width, int height)
{
    return scalePixmapOpenCV(pixmap, width, height, cv::INTER_CUBIC);
}

QPixmap scalePixmapLanczos(const QPixmap &pixmap, int width, int height)
{
    return scalePixmapOpenCV(pixmap, width, height, cv::INTER_LANCZOS4);
}

QPixmap scalePixmapArea(const QPixmap &pixmap, int width, int height)
{
    return scalePixmapOpenCV(pixmap, width, height, cv::INTER_AREA);
}

QPixmap scalePixmapOpenCV(const QPixmap &pixmap, int width, int height, cv::InterpolationFlags flags)
{
    QImage qimage = pixmap.toImage();
    cv::Mat mat = cv::Mat(qimage.height(), qimage.width(), CV_8UC4, (void *)qimage.bits(), qimage.bytesPerLine());

    cv::Mat resizedMat;
    cv::resize(mat, resizedMat, cv::Size(width, height), 0, 0, flags);

    QImage resizedQImage((uchar *)resizedMat.data, resizedMat.cols, resizedMat.rows, resizedMat.step, QImage::Format_ARGB32);
    QPixmap resizedPixmap = QPixmap::fromImage(resizedQImage);

    return resizedPixmap;
}
