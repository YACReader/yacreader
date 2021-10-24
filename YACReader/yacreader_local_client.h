#ifndef YACREADER_LOCAL_CLIENT_H
#define YACREADER_LOCAL_CLIENT_H

#include "yacreader_global.h"
#include "comic_db.h"

#include <QObject>

class QLocalSocket;

class YACReaderLocalClient : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderLocalClient(QObject *parent = nullptr);
    ~YACReaderLocalClient() override;
signals:
    void finished();
public slots:
    void readMessage();
    bool requestComicInfo(quint64 libraryId, ComicDB &comic, QList<ComicDB> &siblings, YACReader::OpenComicSource source);
    bool sendComicInfo(quint64 libraryId, ComicDB &comic);
    bool sendComicInfo(quint64 libraryId, ComicDB &comic, qulonglong nextComicId);

private:
    QLocalSocket *localSocket;
};

#endif // YACREADER_LOCAL_CLIENT_H
