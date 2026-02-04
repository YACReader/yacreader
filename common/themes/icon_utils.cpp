#include "icon_utils.h"

#include "yacreader_global.h"

QString readSvg(const QString &resourcePath)
{
    QFile in(resourcePath);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open SVG resource:" << resourcePath;
        return {};
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
        return {};
    }

    out.write(svg.toUtf8());
    out.close();

    return outPath;
}

QString recolorSvgXML(QString &svg,
                      const QString &placeHolder,
                      const QColor &color)
{
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
