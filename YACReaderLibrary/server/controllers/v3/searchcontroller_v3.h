#ifndef SEARCHCONTROLLER_V3_H
#define SEARCHCONTROLLER_V3_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

// the folders and the comics that match the `query` of the JSON body; the comics have the order of the desktop search
class SearchControllerV3 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchControllerV3)
public:
    explicit SearchControllerV3(int libraryId);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    int libraryId;
};

#endif // SEARCHCONTROLLER_V3_H
