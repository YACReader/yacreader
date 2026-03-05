#ifndef ICON_UTILS_H
#define ICON_UTILS_H

#include <QtGui>

// Render an SVG file to a QPixmap at a specific logical size with HiDPI support.
// Uses QSvgRenderer to rasterize directly at the target resolution (no upscaling).
QPixmap renderSvgToPixmap(const QString &svgPath, int logicalSize, qreal devicePixelRatio);
QPixmap renderSvgToPixmap(const QString &svgPath, int logicalWidth, int logicalHeight, qreal devicePixelRatio);

struct RecolorOptions {
    QString suffix;
    QString fileName;
};

QString readSvg(const QString &resourcePath);
QString writeSvg(const QString &svg, const QString &resourcePath, const QString &themeName, const RecolorOptions &options = { });

QString recolorSvgXML(QString &svg, const QString &placeHolder, const QColor &color);
QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color, // #f0f (magenta)
                                const QString &themeName,
                                const RecolorOptions &options = { });
QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color1, // #f0f (magenta)
                                const QColor &color2, // #0ff (cyan)
                                const QString &themeName,
                                const RecolorOptions &options = { });
QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color1, // #f0f (magenta)
                                const QColor &color2, // #0ff (cyan)
                                const QColor &color3, // #ff0 (yellow)
                                const QString &themeName,
                                const RecolorOptions &options = { });

#endif // ICON_UTILS_H
