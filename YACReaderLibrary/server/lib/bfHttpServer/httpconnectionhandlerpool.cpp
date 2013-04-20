#include "httpconnectionhandlerpool.h"

HttpConnectionHandlerPool::HttpConnectionHandlerPool(QSettings* settings, HttpRequestHandler* requestHandler)
    : QObject()
{
    Q_ASSERT(settings!=0);
    this->settings=settings;
    this->requestHandler=requestHandler;
    cleanupTimer.start(settings->value("cleanupInterval",10000).toInt());
    connect(&cleanupTimer, SIGNAL(timeout()), SLOT(cleanup()));
}


HttpConnectionHandlerPool::~HttpConnectionHandlerPool() {    
    foreach(HttpConnectionHandler* handler, pool) {
        connect(handler,SIGNAL(finished()),handler,SLOT(deleteLater()));
        handler->quit();
    }
}


HttpConnectionHandler* HttpConnectionHandlerPool::getConnectionHandler() {   
    HttpConnectionHandler* freeHandler=0;
    mutex.lock();
    // find a free handler in pool
    foreach(HttpConnectionHandler* handler, pool) {
        if (!handler->isBusy()) {
            freeHandler=handler;
            freeHandler->setBusy();
            break;
        }
    }
    // create a new handler, if necessary
    if (!freeHandler) {
        int maxConnectionHandlers=settings->value("maxThreads",1000).toInt();
        if (pool.count()<maxConnectionHandlers) {
            freeHandler=new HttpConnectionHandler(settings,requestHandler);
            freeHandler->setBusy();
            pool.append(freeHandler);
        }
    }
    mutex.unlock();
    return freeHandler;
}



void HttpConnectionHandlerPool::cleanup() {
    int maxIdleHandlers=settings->value("minThreads",50).toInt();
    int idleCounter=0;
    mutex.lock();
    foreach(HttpConnectionHandler* handler, pool) {
        if (!handler->isBusy()) {
            if (++idleCounter > maxIdleHandlers) {
                pool.removeOne(handler);
                qDebug("HttpConnectionHandlerPool: Removed connection handler (%p), pool size is now %i",handler,pool.size());
                connect(handler,SIGNAL(finished()),handler,SLOT(deleteLater()));
                handler->quit();
                break; // remove only one handler in each interval
            }
        }
    }
    mutex.unlock();
}
