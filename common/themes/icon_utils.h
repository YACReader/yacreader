#ifndef ICON_UTILS_H
#define ICON_UTILS_H

#include <QtGui>

QString readSvg(const QString &resourcePath);
QString writeSvg(const QString &svg, const QString &resourcePath, const QString &themeName);

QString recolorSvgXML(QString &svg, const QString &placeHolder, const QColor &color);
QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color, // #f0f (magenta)
                                const QString &themeName,
                                const QString &suffix = QString());
QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color1, // #f0f (magenta)
                                const QColor &color2, // #0ff (cyan)
                                const QString &themeName,
                                const QString &suffix = QString());
QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color1, // #f0f (magenta)
                                const QColor &color2, // #0ff (cyan)
                                const QColor &color3, // #ff0 (yellow)
                                const QString &themeName,
                                const QString &suffix = QString());

#endif // ICON_UTILS_H
