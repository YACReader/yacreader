#ifndef COMICCONTROLLER_H
#define COMICCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

#include <QThread>
class Comic;
class QString;

class ComicController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ComicController);

public:
    /** Constructor */
    ComicController();

    /** Generates the response */
    void service(HttpRequest &request, HttpResponse &response);
};

#endif // COMICCONTROLLER_H
