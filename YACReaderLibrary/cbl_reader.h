#ifndef CBL_READER_H
#define CBL_READER_H

#include <QList>
#include <QString>

struct CblBook
{
    QString series;
    QString number;
    QString volume;
    QString year;
    QString id;
    int ordering = 0;
};

struct CblReadingList
{
    QString name;
    QList<CblBook> books;
};

struct CblReadResult
{
    bool success = false;
    CblReadingList readingList;
    QString errorMessage;
    qint64 errorLine = 0;
    qint64 errorColumn = 0;
};

class CblReader
{
public:
    static CblReadResult read(const QString &filePath);
};

#endif // CBL_READER_H
