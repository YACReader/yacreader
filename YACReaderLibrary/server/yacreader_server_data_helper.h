#ifndef YACREADERSERVERDATAHELPER_H
#define YACREADERSERVERDATAHELPER_H

#include "comic_db.h"
#include "comic_item.h"
#include "folder.h"
#include "list_folder.h"
#include "reading_list.h"
#include "smart_list.h"

#include <QtCore>

void variantToJson(const QString &name, QMetaType::Type type, const QVariant &value, QJsonObject &json);

class YACReaderServerDataHelper
{
public:
    static QString folderToYSFormat(const qulonglong libraryId, const Folder &folder);
    static QString comicToYSFormat(const qulonglong libraryId, const ComicDB &comic);

    static QJsonObject folderToJSON(const qulonglong libraryId, const QUuid libraryUuid, const Folder &folder);
    static QJsonObject comicToJSON(const qulonglong libraryId, const QUuid libraryUuid, const ComicDB &comic);
    static QJsonObject fullComicToJSON(const qulonglong libraryId, const QUuid libraryUuid, const ComicDB &comic);
    static QJsonObject readingListToJSON(const qulonglong libraryId, const QUuid libraryUuid, const ReadingList &readingList);
    static QJsonObject labelToJSON(const qulonglong libraryId, const QUuid libraryUuid, const Label &label);

    static QJsonObject deletedComicToJSON(const qulonglong libraryId, const QUuid libraryUuid, const ComicInfo &info);
    static QJsonObject placeholderToJSON(const qulonglong libraryId, const QUuid libraryUuid, const Placeholder &placeholder);
    // v3 content: comics, deleted comics, and placeholders, with their entry in lists
    static QJsonObject comicItemToJSON(const qulonglong libraryId, const QUuid libraryUuid, const ComicItem &item);
    // v3 lists sections
    static QJsonObject listFolderToJSON(const qulonglong libraryId, const QUuid libraryUuid, const ListFolder &folder);
    // firstComicHash: empty when the list has no comic
    static QJsonObject sectionReadingListToJSON(const qulonglong libraryId, const QUuid libraryUuid, const ReadingList &readingList, const QString &firstComicHash);
    static QJsonObject smartListToJSON(const qulonglong libraryId, const QUuid libraryUuid, const SmartList &smartList);

    // v2 clients send only the comic id, so a repeated comic is navigated at its first occurrence
    static QList<ComicDB> firstOccurrences(const QList<ComicDB> &comics);

private:
    YACReaderServerDataHelper();
};

#endif // YACREADERSERVERDATAHELPER_H
