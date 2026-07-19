#ifndef XMLINFOPARSER_H
#define XMLINFOPARSER_H

#include "comic_db.h"

#include <QList>

namespace YACReader {

bool parseXMLIntoInfo(const QByteArray &xmlRawData, ComicInfo &info);

// Every comic_info column that parseXMLIntoInfo can set, so code that writes
// parsed metadata to the DB stays in sync with the parser. When the parser
// learns a new field, add its column here too.
struct XmlMetadataField {
    const char *column;
    QVariant ComicInfo::*member;
};

const QList<XmlMetadataField> &xmlMetadataFields();

}

#endif // XMLINFOPARSER_H
