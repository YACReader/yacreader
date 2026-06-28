#if !defined PDF_RENDER_SIZE_H
#define PDF_RENDER_SIZE_H

#include <QSize>
#include <QSizeF>
#include <QtGlobal>

#include <algorithm>
#include <cmath>

namespace YACReaderPdfRender {

// Keep ordinary PDF pages readable while allowing tall/narrow pages to exceed
// 4096px on one side when the total pixel cost stays bounded.
constexpr double TargetDpi = 150.0;
constexpr int MinimumLongestSide = 2560;
constexpr qint64 MaximumPixelArea = 4096LL * 4096LL;
constexpr int MaximumSide = 32768;

inline bool isValidSize(const QSizeF &size)
{
    return std::isfinite(size.width()) && std::isfinite(size.height()) && size.width() >= 1.0 && size.height() >= 1.0;
}

inline QSize renderSizeFromPixelSize(const QSizeF &naturalPixelSize)
{
    if (!isValidSize(naturalPixelSize)) {
        return QSize();
    }

    double width = naturalPixelSize.width();
    double height = naturalPixelSize.height();

    const double longestSide = std::max(width, height);
    if (longestSide < MinimumLongestSide) {
        const double scale = MinimumLongestSide / longestSide;
        width *= scale;
        height *= scale;
    }

    const double area = width * height;
    const double sideScale = std::min(1.0, MaximumSide / std::max(width, height));
    const double areaScale = area > MaximumPixelArea ? std::sqrt(MaximumPixelArea / area) : 1.0;
    const double scale = std::min(sideScale, areaScale);

    width *= scale;
    height *= scale;

    return QSize(std::max(1, static_cast<int>(std::floor(width))),
                 std::max(1, static_cast<int>(std::floor(height))));
}

inline QSize renderSizeFromPagePoints(const QSizeF &pageSizePoints, double targetDpi = TargetDpi)
{
    if (!isValidSize(pageSizePoints) || !std::isfinite(targetDpi) || targetDpi <= 0.0) {
        return QSize();
    }

    const double scale = targetDpi / 72.0;
    return renderSizeFromPixelSize(QSizeF(pageSizePoints.width() * scale, pageSizePoints.height() * scale));
}

inline double renderDpiForWidth(const QSizeF &pageSizePoints, int renderWidth)
{
    if (!isValidSize(pageSizePoints) || renderWidth < 1) {
        return 0.0;
    }

    return (static_cast<double>(renderWidth) / pageSizePoints.width()) * 72.0;
}

}

#endif // PDF_RENDER_SIZE_H
