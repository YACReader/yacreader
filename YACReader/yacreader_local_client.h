#ifndef YACREADER_LOCAL_CLIENT_H
#define YACREADER_LOCAL_CLIENT_H

#include <QObject>

class QLocalSocket;
class ComicDB;

class YACReaderLocalClient : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderLocalClient(QObject *parent = 0);
    
signals:
    
public slots:
    void readMessage();
    void requestComicInfo(QString library, ComicDB & comic);
    void sendComicInfo(QString library, ComicDB & comic);

private:
    QLocalSocket * localSocket;
    
};

#endif // YACREADER_LOCAL_CLIENT_H
