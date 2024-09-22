#include "yacreader_global.h"

#include <QModelIndex>
#include <QLibrary>
#include <QFileInfo>
#include <QCoreApplication>

using namespace YACReader;

QString YACReader::getSettingsPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
}

QString YACReader::colorToName(LabelColors colors)
{
    switch (colors) {
    case YRed:
        return "red";
    case YOrange:
        return "orange";
    case YYellow:
        return "yellow";
    case YGreen:
        return "green";
    case YCyan:
        return "cyan";
    case YBlue:
        return "blue";
    case YViolet:
        return "violet";
    case YPurple:
        return "purple";
    case YPink:
        return "pink";
    case YWhite:
        return "white";
    case YLight:
        return "light";
    case YDark:
        return "dark";
    }

    return "";
}

QString YACReader::labelColorToRGBString(LabelColors color)
{
    switch (color) {
    case YRed:
        return "#FD777C";

    case YOrange:
        return "#FEBF34";

    case YYellow:
        return "#F5E934";

    case YGreen:
        return "#B6E525";

    case YCyan:
        return "#9FFFDD";

    case YBlue:
        return "#82C7FF";

    case YViolet:
        return "#8286FF";

    case YPurple:
        return "#E39FFF";

    case YPink:
        return "#FF9FDD";

#ifdef Y_MAC_UI
    case YWhite:
        return "#E3E3E3";
#else
    case YWhite:
        return "#FFFFFF";
#endif
    case YLight:
        return "#C8C8C8";
    case YDark:
        return "#ABABAB";
    }

    return "";
}

QDataStream &YACReader::operator<<(QDataStream &stream, const OpenComicSource &source)
{
    stream << (quint8)source.source;
    stream << source.sourceId;
    return stream;
}

QDataStream &YACReader::operator>>(QDataStream &stream, OpenComicSource &source)
{
    quint8 sourceRaw;
    stream >> sourceRaw;
    source.source = (OpenComicSource::Source)sourceRaw;
    stream >> source.sourceId;
    return stream;
}

void YACReader::iterate(const QModelIndex &index,
                        const QAbstractItemModel *model,
                        const std::function<bool(const QModelIndex &)> &iteration)
{
    if (index.isValid()) {
        auto continueIterating = iteration(index);
        if (!continueIterating) {
            return;
        }
    }
    if ((index.flags() & Qt::ItemNeverHasChildren) || !model->hasChildren(index))
        return;
    auto rows = model->rowCount(index);
    for (int i = 0; i < rows; ++i)
        iterate(model->index(i, 0, index), model, iteration);
}

QLibrary *YACReader::load7zLibrary()
{
#if defined Q_OS_UNIX && !defined Q_OS_MACOS
    auto yacreader7zPath = QString(LIBDIR) + "/yacreader/7z.so";
    QFileInfo sevenzlibrary(yacreader7zPath);
    if (sevenzlibrary.exists()) {
        return new QLibrary(yacreader7zPath);
    } else {
        return new QLibrary(QString(LIBDIR) + "/7zip/7z.so");
    }
#else
    return new QLibrary(QCoreApplication::applicationDirPath() + "/utils/7z");
#endif
}
