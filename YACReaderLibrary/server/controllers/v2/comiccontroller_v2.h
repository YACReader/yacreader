#ifndef COMICCONTROLLER_V2_H
#define COMICCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

#include <QThread>
class Comic;
class QString;

class ComicControllerV2 : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ComicControllerV2)
public:
    /** Constructor */
    ComicControllerV2();

    /** Generates the response */
    void service(HttpRequest &request, HttpResponse &response) override;
};

#endif // COMICCONTROLLER_H
