#ifndef TAGINFOCONTROLLER_V2_H
#define TAGINFOCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class TagInfoControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(TagInfoControllerV2)
public:
    TagInfoControllerV2();

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    void serviceComics(const int &library, const qulonglong &tagId, stefanfrings::HttpResponse &response);
};

#endif // TAGINFOCONTROLLER_V2_H
