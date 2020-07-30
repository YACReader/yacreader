#ifndef TAGCONTENTCONTROLLER_V2_H
#define TAGCONTENTCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class TagContentControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(TagContentControllerV2)
public:
    /** Constructor */
    TagContentControllerV2();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    void serviceContent(const int &library, const qulonglong &tagId, stefanfrings::HttpResponse &response);
};

#endif // TAGCONTENTCONTROLLER_H
