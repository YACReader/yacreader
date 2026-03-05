#include "icon_utils.h"

#include <QSvgRenderer>

#include "yacreader_global.h"

QPixmap renderSvgToPixmap(const QString &svgPath, int logicalSize, qreal devicePixelRatio)
{
    return renderSvgToPixmap(svgPath, logicalSize, logicalSize, devicePixelRatio);
}

QPixmap renderSvgToPixmap(const QString &svgPath, int logicalWidth, int logicalHeight, qreal devicePixelRatio)
{
    const int pixelWidth = qRound(logicalWidth * devicePixelRatio);
    const int pixelHeight = qRound(logicalHeight * devicePixelRatio);
    QPixmap pixmap(pixelWidth, pixelHeight);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QSvgRenderer renderer(svgPath);
    renderer.render(&painter);
    painter.end();
    pixmap.setDevicePixelRatio(devicePixelRatio);
    return pixmap;
}

QString readSvg(const QString &resourcePath)
{
    QFile in(resourcePath);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open SVG resource:" << resourcePath;
        return { };
    }

    QString svg = QString::fromUtf8(in.readAll());
    in.close();

    return svg;
}

QString writeSvg(const QString &svg, const QString &resourcePath, const QString &themeName, const RecolorOptions &options)
{
    const QString basePath = YACReader::getSettingsPath() + "/themes/" + themeName;

    QDir().mkpath(basePath);

    QString outFileName = options.fileName.isEmpty() ? QFileInfo(resourcePath).completeBaseName() : options.fileName;
    if (!options.suffix.isEmpty()) {
        outFileName += options.suffix;
    }
    outFileName += "." + QFileInfo(resourcePath).suffix();
    const QString outPath = basePath + "/" + outFileName;

    QFile out(outPath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "Failed to write SVG:" << outPath;
        return { };
    }

    out.write(svg.toUtf8());
    out.close();

    return outPath;
}

QString recolorSvgXML(QString &svg,
                      const QString &placeHolder,
                      const QColor &color)
{
    // TODO: svg colors can work without ;, we need to update all the svg files to inlude ;
    // Include the trailing ';' so e.g. "#ff0;" never accidentally matches
    // inside a previously-substituted value like "#ff0000;".
    return svg.replace(placeHolder, color.name(QColor::HexRgb), Qt::CaseInsensitive);
}

QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color, // #f0f (magenta)
                                const QString &themeName,
                                const RecolorOptions &options)
{
    auto svg = readSvg(resourcePath);

    recolorSvgXML(svg, "#f0f", color);

    return writeSvg(svg, resourcePath, themeName, options);
}

QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color1, // #f0f (magenta)
                                const QColor &color2, // #0ff (cyan)
                                const QString &themeName,
                                const RecolorOptions &options)
{
    auto svg = readSvg(resourcePath);

    recolorSvgXML(svg, "#f0f", color1);
    recolorSvgXML(svg, "#0ff", color2);

    return writeSvg(svg, resourcePath, themeName, options);
}

QString recoloredSvgToThemeFile(const QString &resourcePath,
                                const QColor &color1, // #f0f (magenta)
                                const QColor &color2, // #0ff (cyan)
                                const QColor &color3, // #ff0 (yellow)
                                const QString &themeName,
                                const RecolorOptions &options)
{
    auto svg = readSvg(resourcePath);

    recolorSvgXML(svg, "#f0f", color1);
    recolorSvgXML(svg, "#0ff", color2);
    recolorSvgXML(svg, "#ff0", color3);

    return writeSvg(svg, resourcePath, themeName, options);
}
