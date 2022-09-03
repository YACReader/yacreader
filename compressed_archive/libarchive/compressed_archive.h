#ifndef COMPRESSED_ARCHIVE_H
#define COMPRESSED_ARCHIVE_H

#include "extract_delegate.h"

#include <QObject>
#include <QDebug>

extern "C" {
#include <archive.h>
#include <archive_entry.h>
}

class CompressedArchive : public QObject
{
    Q_OBJECT
public:
    explicit CompressedArchive(const QString &filePath, QObject *parent = nullptr);
    ~CompressedArchive() override;

public slots:
    void getAllData(const QVector<quint32> &indexes, ExtractDelegate *delegate = nullptr);
    QByteArray getRawDataAtIndex(int index);

    int getNumFiles() { return num_entries; }
    QList<QString> getFileNames() { return entries; }
    bool isValid() { return valid; }
    bool toolsLoaded() { return true; }

private:
    archive *a;
    QStringList entries;
    int num_entries;
    bool valid;
    quint32 idx;
    QString filename;

    bool open_archive();
    void close_archive();
    bool archive_seek(quint32 index);
    QByteArray read_entry();
};

#endif // COMPRESSED_ARCHIVE_H
