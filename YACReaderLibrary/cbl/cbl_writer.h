#ifndef CBL_WRITER_H
#define CBL_WRITER_H

#include "cbl_reader.h"

#include <QByteArray>
#include <QList>
#include <QString>

// a book to write, and the source it was imported from
struct CblExportBook {
    CblBook book;
    // the book as in its file, written as it is when the format is the same; empty when it was not imported
    QString source;
    // "xml" or "json"
    QString sourceFormat;
};

// the file of a reading list; a JSON file gets `uuid` as its file UUID. Values the library does not know are left out,
// also when the JSON schema requires them.
QByteArray writeCbl(const QString &name, const QList<CblExportBook> &books, CblReadingList::Format format, const QString &uuid);

#endif
