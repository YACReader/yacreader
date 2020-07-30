#ifndef FAVORITESCONTROLLER_V2_H
#define FAVORITESCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class FavoritesControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(FavoritesControllerV2)
public:
    FavoritesControllerV2();

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    void serviceContent(const int library, stefanfrings::HttpResponse &response);
};

#endif // FAVORITESCONTROLLER_H
