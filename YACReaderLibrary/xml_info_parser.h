#ifndef XMLINFOPARSER_H
#define XMLINFOPARSER_H

#include "comic_db.h"

namespace YACReader {

bool parseXMLIntoInfo(const QByteArray &xmlRawData, ComicInfo &info);

}

#endif // XMLINFOPARSER_H
