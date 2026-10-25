#ifndef RECENTCONTROLLER_V3_H
#define RECENTCONTROLLER_V3_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

// the comics of the Recent list of the desktop: the `days` parameter (7 when it is missing) replaces its setting
class RecentControllerV3 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(RecentControllerV3)
public:
    explicit RecentControllerV3(int libraryId);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    int libraryId;
};

#endif // RECENTCONTROLLER_V3_H
