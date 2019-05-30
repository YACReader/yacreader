#ifndef FOLDERCONTROLLER_H
#define FOLDERCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class FolderController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(FolderController);

public:
    /** Constructor */
    FolderController();

    /** Generates the response */
    void service(HttpRequest &request, HttpResponse &response);
};

#endif // FOLDERCONTROLLER_H
