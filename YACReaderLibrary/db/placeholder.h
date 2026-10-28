#ifndef PLACEHOLDER_H
#define PLACEHOLDER_H

#include "descriptive_comic_metadata.h"
#include "library_ids.h"

#include <QString>
#include <QStringList>

#include <optional>

class QSqlQuery;
class QSqlRecord;

// an expected issue without a file; it has no file, reading, or reader data
struct Placeholder : DescriptiveComicMetadata {
    PlaceholderId id;
    std::optional<QString> coverKey;

    static const QStringList &columns();
    static QList<int> columnIndexes(const QSqlRecord &record, const QString &alias);
    static Placeholder fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes);
    void bindTo(QSqlQuery &query) const;
    // compares every column
    bool hasSameValues(const Placeholder &other) const;
};

#endif
