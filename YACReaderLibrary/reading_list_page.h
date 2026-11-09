#ifndef READING_LIST_PAGE_H
#define READING_LIST_PAGE_H

#include <QByteArray>
#include <QDate>
#include <QFuture>
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

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
// the distinct cover files of the rows of a page, each encoded once with readingListPageCover in the background
class ReadingListPageCovers
{
public:
    // an empty path is a row without a cover file
    explicit ReadingListPageCovers(const QStringList &paths);
    QFuture<QByteArray> start();
    int count() const;
    // after the work finished without a cancel; empty when the file cannot be read
    QByteArray cover(const QString &path) const;

private:
    QStringList paths;
    QHash<QString, int> indexes; // by path
    QFuture<QByteArray> future;
};

// "Series #Number (Volume) · Date"; a d/m/yyyy date shows its month and year, a year alone the year
QString readingListPageDetails(const QString &series, const QString &number, const QString &volume, const QString &date);

#endif
