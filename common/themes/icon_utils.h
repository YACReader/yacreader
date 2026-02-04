#ifndef ICON_UTILS_H
#define ICON_UTILS_H

#include <QtGui>

struct RecolorOptions {
    QString suffix;
    QString fileName;
};

QString readSvg(const QString &resourcePath);
QString writeSvg(const QString &svg, const QString &resourcePath, const QString &themeName, const RecolorOptions &options = {});

QString recolorSvgXML(QString &svg, const QString &placeHolder, const QColor &color);
QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color, // #f0f (magenta)
                                const QString &themeName,
                                const RecolorOptions &options = {});
QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color1, // #f0f (magenta)
                                const QColor &color2, // #0ff (cyan)
                                const QString &themeName,
                                const RecolorOptions &options = {});
QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color1, // #f0f (magenta)
                                const QColor &color2, // #0ff (cyan)
                                const QColor &color3, // #ff0 (yellow)
                                const QString &themeName,
                                const RecolorOptions &options = {});

#endif // ICON_UTILS_H
