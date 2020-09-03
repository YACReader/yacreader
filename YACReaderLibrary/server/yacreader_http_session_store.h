#ifndef YACREADERHTTPSESSIONSTORE_H
#define YACREADERHTTPSESSIONSTORE_H

#include <QObject>
#include <QtCore>

namespace stefanfrings {
class HttpSessionStore;
}
class YACReaderHttpSession;

class YACReaderHttpSessionStore : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderHttpSessionStore(stefanfrings::HttpSessionStore *sessionStore, QObject *parent = nullptr);

    void addYACReaderHttpSession(const QByteArray &httpSessionId, YACReaderHttpSession *yacreaderHttpSession);
    YACReaderHttpSession *getYACReaderSessionHttpSession(const QByteArray &httpSessionId);

signals:

public slots:

private:
    QMap<QByteArray, YACReaderHttpSession *> sessions;
    stefanfrings::HttpSessionStore *sessionStore;
    QTimer cleanupTimer;

    QMutex mutex;

private slots:

    void sessionTimerEvent();
};

#endif // YACREADERHTTPSESSIONSTORE_H
