#include "yacreader_http_session_store.h"

#include "yacreader_http_session.h"

#include "httpsessionstore.h"



YACReaderHttpSessionStore::YACReaderHttpSessionStore(HttpSessionStore *sessionStore, QObject *parent)
    : QObject(parent), sessionStore(sessionStore)
{
    connect(&cleanupTimer,SIGNAL(timeout()),this,SLOT(sessionTimerEvent()));
    cleanupTimer.start(60000);
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

void YACReaderHttpSessionStore::sessionTimerEvent()
{
    QMutexLocker locker(&mutex);
    for(const QByteArray &id : sessions.keys())
    {
        if(sessionStore->getSession(id).isNull())
        {
            YACReaderHttpSession *session = sessions.value(id, nullptr);
            if(session != nullptr)
                delete session;

            sessions.remove(id);
        }
    }
}
