#ifndef FAVORITESCONTROLLER_H
#define FAVORITESCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class FavoritesController : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(FavoritesController);
public:
    FavoritesController();

    void service(HttpRequest& request, HttpResponse& response);

private:
    void serviceContent(const int library, HttpResponse &response);
};


#endif // FAVORITESCONTROLLER_H
