#ifndef FAVORITESCONTROLLER_V3_H
#define FAVORITESCONTROLLER_V3_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

// the comics of the Favorites list of the desktop, in their order
class FavoritesControllerV3 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(FavoritesControllerV3)
public:
    explicit FavoritesControllerV3(int libraryId);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    int libraryId;
};

#endif // FAVORITESCONTROLLER_V3_H
