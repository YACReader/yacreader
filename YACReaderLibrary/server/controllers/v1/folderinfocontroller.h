#ifndef FOLDERINFOCONTROLLER_H
#define FOLDERINFOCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class FolderInfoController : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(FolderInfoController);

public:
    /** Constructor */
    FolderInfoController();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    void serviceComics(const int &library, const qulonglong &folderId, stefanfrings::HttpResponse &response);
};

#endif // FOLDERINFOCONTROLLER_H
