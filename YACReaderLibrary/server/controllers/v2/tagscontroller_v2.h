#ifndef TAGSCONTROLLER_V2_H
#define TAGSCONTROLLER_V2_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

class TagsControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(TagsControllerV2)
public:
    /** Constructor */
    TagsControllerV2();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // TAGSCONTROLLER_H
