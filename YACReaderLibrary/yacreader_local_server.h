#ifndef YACREADER_LOCAL_SERVER_H
#define YACREADER_LOCAL_SERVER_H

#include <QObject>

class QLocalServer;

class YACReaderLocalServer : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderLocalServer(QObject *parent = 0);
    
signals:
    
public slots:
    bool isListening();
    void sendResponse();
	static bool isRunning();

private:
    QLocalServer * localServer;
    
};

#endif // YACREADER_LOCAL_SERVER_H
