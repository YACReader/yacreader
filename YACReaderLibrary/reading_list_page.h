#ifndef READING_LIST_PAGE_H
#define READING_LIST_PAGE_H

#include <QByteArray>
#include <QDate>
#include <QList>
#include <QString>

// a row of the web page export of a reading list
struct ReadingListPageEntry {
    QString title;
    // series #number (volume) · date
    QString details;
    // a JPEG; empty when it has no cover
    QByteArray cover;
    bool missing = false;
};

// one self-contained HTML page: the covers are inside it
QByteArray writeReadingListPage(const QString &name, const QList<ReadingListPageEntry> &entries, const QDate &exported);
// a cover file scaled for the page, as JPEG; empty when it cannot be read
QByteArray readingListPageCover(const QString &path);
// "Series #Number (Volume) · Date"; a d/m/yyyy date shows its month and year, a year alone the year
QString readingListPageDetails(const QString &series, const QString &number, const QString &volume, const QString &date);

#endif
