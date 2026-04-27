#include "resize_image.h"

#include "lancir.h"

static QImage scaleImageLancir(const QImage &image, int width, int height)
{
    QImage src = (image.format() == QImage::Format_ARGB32)
            ? image
            : image.convertToFormat(QImage::Format_ARGB32);

    QImage dst(width, height, QImage::Format_ARGB32);

    // SrcSSize / NewSSize are in elements; for uint8_t that equals bytes,
    // so bytesPerLine() covers any Qt row-alignment padding correctly.
    avir::CLancIRParams params(src.bytesPerLine(), dst.bytesPerLine());
    params.la = 4.0; // Lanczos4

    avir::CLancIR lancir;
    lancir.resizeImage<uint8_t, uint8_t>(
            src.constBits(), src.width(), src.height(),
            dst.bits(), width, height, 4, &params);

    return dst;
}

// ---- QPixmap API ------------------------------------------------------------

QPixmap scalePixmap(const QPixmap &pixmap, int width, int height, ScaleMethod method)
{
    if ((pixmap.width() == width && pixmap.height() == height) || pixmap.isNull())
        return pixmap;

    switch (method) {
    case ScaleMethod::Nearest:
        return pixmap.scaled(width, height, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    case ScaleMethod::Bilinear:
        return pixmap.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    case ScaleMethod::Lanczos:
        return QPixmap::fromImage(scaleImageLancir(pixmap.toImage(), width, height));
    }
    return pixmap;
}

// ---- QImage API (avoids QPixmap round-trip in ContinuousPageWidget) ---------

QImage scaleImage(const QImage &image, int width, int height, ScaleMethod method)
{
    if ((image.width() == width && image.height() == height) || image.isNull())
        return image;

    switch (method) {
    case ScaleMethod::Nearest:
        return image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    case ScaleMethod::Bilinear:
        return image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    case ScaleMethod::Lanczos:
        return scaleImageLancir(image, width, height);
    }
    return image;
}
