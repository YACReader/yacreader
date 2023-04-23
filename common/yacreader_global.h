#ifndef __YACREADER_GLOBAL_H
#define __YACREADER_GLOBAL_H

#include <QStandardPaths>
#include <QDataStream>
#include <QMetaType>
#include <QAbstractItemModel>

#define VERSION "9.12.0"

#define REMOTE_BROWSE_PERFORMANCE_WORKAROUND "REMOTE_BROWSE_PERFORMANCE_WORKAROUND"
#define IMPORT_COMIC_INFO_XML_METADATA "IMPORT_COMIC_INFO_XML_METADATA"

#define NUM_DAYS_BETWEEN_VERSION_CHECKS "NUM_DAYS_BETWEEN_VERSION_CHECKS"
#define LAST_VERSION_CHECK "LAST_VERSION_CHECK"

#define YACREADERLIBRARY_GUID "ea343ff3-2005-4865-b212-7fa7c43999b8"

#define LIBRARIES "LIBRARIES"

#define MAX_LIBRARIES_WARNING_NUM 10

namespace YACReader {

enum YACReaderIPCMessages {
    RequestComicInfo = 0,
    SendComicInfo,
};

enum YACReaderComicReadStatus {
    Unread = 0,
    Read = 1,
    Opened = 2
};

enum YACReaderErrors {
    SevenZNotFound = 700
};

enum LabelColors {
    YRed = 1,
    YOrange,
    YYellow,
    YGreen,
    YCyan,
    YBlue,
    YViolet,
    YPurple,
    YPink,
    YWhite,
    YLight,
    YDark
};

struct OpenComicSource {
    enum Source {
        Folder = 0,
        ReadingList
    };

    Source source;
    qulonglong sourceId;
};

QDataStream &operator<<(QDataStream &stream, const OpenComicSource &source);
QDataStream &operator>>(QDataStream &stream, OpenComicSource &source);

QString getSettingsPath();
QString colorToName(LabelColors colors);
QString labelColorToRGBString(LabelColors color);

void iterate(const QModelIndex &index,
             const QAbstractItemModel *model,
             const std::function<bool(const QModelIndex &)> &iteration);

}

Q_DECLARE_METATYPE(YACReader::OpenComicSource::Source)
Q_DECLARE_METATYPE(YACReader::OpenComicSource)

#endif
