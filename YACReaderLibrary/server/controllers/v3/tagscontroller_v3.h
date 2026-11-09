#ifndef TAGSCONTROLLER_V3_H
#define TAGSCONTROLLER_V3_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

// the labels of a library, in the order of the sidebar
class TagsControllerV3 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(TagsControllerV3)
public:
    explicit TagsControllerV3(int libraryId);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    int libraryId;
};

#endif // TAGSCONTROLLER_V3_H
