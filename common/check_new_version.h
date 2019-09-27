#ifndef __CHECKUPDATE_H
#define __CHECKUPDATE_H

#include "http_worker.h"
#include "yacreader_global.h"

#include <QByteArray>
#include <QThread>

class HttpVersionChecker : public HttpWorker
{
    Q_OBJECT
public:
    HttpVersionChecker();
public slots:

private:
    bool found;
private slots:
    bool checkNewVersion(QString sourceContent);
    void checkNewVersion(const QByteArray &data);
signals:
    void newVersionDetected();
};

#endif
