#include "httpconnectionhandlerpool.h"

HttpConnectionHandlerPool::HttpConnectionHandlerPool(QSettings* settings, HttpRequestHandler* requestHandler)
    : QObject()
{
    Q_ASSERT(settings!=0);
    this->settings=settings;
    this->requestHandler=requestHandler;
    cleanupTimer.start(settings->value("cleanupInterval",1000).toInt());
    connect(&cleanupTimer, SIGNAL(timeout()), SLOT(cleanup()));
}


HttpConnectionHandlerPool::~HttpConnectionHandlerPool() {    
    foreach(HttpConnectionHandler* handler, pool) {
        delete handler;
    }
}


HttpConnectionHandler* HttpConnectionHandlerPool::getConnectionHandler() {
    HttpConnectionHandler* freeHandler=0;
    foreach(HttpConnectionHandler* handler, pool) {
        if (!handler->isBusy()) {
            freeHandler=handler;
        }
    }
    if (!freeHandler) {
		//CAMBIADO
        int maxConnectionHandlers= 100;//settings->value("maxThreads",10).toInt();
        if (pool.count()<maxConnectionHandlers) {
            freeHandler=new HttpConnectionHandler(settings,requestHandler);
            pool.append(freeHandler);
        }
    }
    if (freeHandler) freeHandler->busy = true; // pdiener: set it to busy-state immediately
    return freeHandler;
}



void HttpConnectionHandlerPool::cleanup() {
    int maxIdleHandlers=settings->value("minThreads",1).toInt();
    int idleCounter=0;
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
}
