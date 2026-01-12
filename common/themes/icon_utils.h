#ifndef ICON_UTILS_H
#define ICON_UTILS_H

#include <QtGui>

QString recolorSvgXML(QString &svg, const QString &placeHolder, const QColor &color);
QString readSvg(const QString &resourcePath);
QString writeSvg(const QString &svg, const QString &resourcePath, const QString &themeName);
QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color1, // #f0f (magenta)
                                const QColor &color2, // #0ff (cyan)
                                const QString &themeName,
                                const QString &suffix = QString());
QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color, // #f0f (magenta)
                                const QString &themeName,
                                const QString &suffix = QString());
QString createMenuArrowSvg(const QColor &color, const QString &themeName);

#endif // ICON_UTILS_H
