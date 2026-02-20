#ifndef YACREADERHTTPSESSIONSTORE_H
#define YACREADERHTTPSESSIONSTORE_H

#include <QObject>
#include <QtCore>

class YACReaderHttpSession;

class YACReaderHttpSessionStore : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderHttpSessionStore(QObject *parent = nullptr);

    void addYACReaderHttpSession(const QByteArray &httpSessionId, YACReaderHttpSession *yacreaderHttpSession);
    YACReaderHttpSession *getYACReaderSessionHttpSession(const QByteArray &httpSessionId);

private:
    QMap<QByteArray, YACReaderHttpSession *> sessions;
    QMutex mutex;
};

#endif // YACREADERHTTPSESSIONSTORE_H
