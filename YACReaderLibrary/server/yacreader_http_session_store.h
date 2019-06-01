#ifndef YACREADERHTTPSESSIONSTORE_H
#define YACREADERHTTPSESSIONSTORE_H

#include <QObject>
#include <QtCore>

class HttpSessionStore;
class YACReaderHttpSession;

class YACReaderHttpSessionStore : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderHttpSessionStore(HttpSessionStore *sessionStore, QObject *parent = 0);

    void addYACReaderHttpSession(const QByteArray &httpSessionId, YACReaderHttpSession *yacreaderHttpSession);
    YACReaderHttpSession *getYACReaderSessionHttpSession(const QByteArray &httpSessionId);

signals:

public slots:

private:
    QMap<QByteArray, YACReaderHttpSession *> sessions;
    HttpSessionStore *sessionStore;
    QTimer cleanupTimer;

    QMutex mutex;

private slots:

    void sessionTimerEvent();
};

#endif // YACREADERHTTPSESSIONSTORE_H
