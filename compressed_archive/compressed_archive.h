#ifndef COMPRESSED_ARCHIVE_H
#define COMPRESSED_ARCHIVE_H

#include <QObject>

class ExtractDelegate;

class QLibrary;
#include <QString>
#include <QList>
#include <QMap>

struct SevenZipInterface;

class MyCodecs;

class CompressedArchive : public QObject
{
    Q_OBJECT
public:
    explicit CompressedArchive(const QString &filePath, QObject *parent = 0);
    ~CompressedArchive();

signals:

public slots:
    int getNumFiles();
    int getNumEntries();
    QList<QByteArray> getAllData(const QVector<quint32> &indexes, ExtractDelegate *delegate = 0);
    QByteArray getRawDataAtIndex(int index);
    QList<QString> getFileNames();
    bool isValid();
    bool toolsLoaded();

private:
    SevenZipInterface *szInterface;

    QLibrary *sevenzLib;
    bool loadFunctions();
    bool tools;
    bool valid;
    QList<QString> files;
    QList<qint32> offsets;
    QMap<qint32, qint32> indexesToPages;

    void setupFilesNames();
    QVector<quint32> translateIndexes(const QVector<quint32> &indexes);

    friend class MyCodecs;
};

#endif // COMPRESSED_ARCHIVE_H
