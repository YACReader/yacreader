#include "yacreader_http_session_store.h"

#include "yacreader_http_session.h"

YACReaderHttpSessionStore::YACReaderHttpSessionStore(QObject *parent)
    : QObject(parent)
{
}

void YACReaderHttpSessionStore::addYACReaderHttpSession(const QByteArray &httpSessionId, YACReaderHttpSession *yacreaderHttpSession)
{
    QMutexLocker locker(&mutex);

    sessions.insert(httpSessionId, yacreaderHttpSession);
}

YACReaderHttpSession *YACReaderHttpSessionStore::getYACReaderSessionHttpSession(const QByteArray &httpSessionId)
{
    QMutexLocker locker(&mutex);

    return sessions.value(httpSessionId, nullptr);
}

