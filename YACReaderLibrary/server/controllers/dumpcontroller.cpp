/**
  @file
  @author Stefan Frings
*/

#include "dumpcontroller.h"
#include <QVariant>
#include <QDateTime>

DumpController::DumpController(){}

void DumpController::service(HttpRequest& request, HttpResponse& response) {

    response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");
    response.setCookie(HttpCookie("firstCookie","hello",600));
    response.setCookie(HttpCookie("secondCookie","world",600));

    QByteArray body("<html><body>");
    body.append("<b>Request:</b>");
    body.append("<br>Method: ");
    body.append(request.getMethod());
    body.append("<br>Path: ");
    body.append(request.getPath());
    body.append("<br>Version: ");
    body.append(request.getVersion());

    body.append("<p><b>Headers:</b>");
    QMapIterator<QByteArray,QByteArray> i(request.getHeaderMap());
    while (i.hasNext()) {
        i.next();
        body.append("<br>");
        body.append(i.key());
        body.append("=");
        body.append(i.value());
    }

    body.append("<p><b>Parameters:</b>");
    i=QMapIterator<QByteArray,QByteArray>(request.getParameterMap());
    while (i.hasNext()) {
        i.next();
        body.append("<br>");
        body.append(i.key());
        body.append("=");
        body.append(i.value());
    }

    body.append("<p><b>Cookies:</b>");
    i=QMapIterator<QByteArray,QByteArray>(request.getCookieMap());
    while (i.hasNext()) {
        i.next();
        body.append("<br>");
        body.append(i.key());
        body.append("=");
        body.append(i.value());
    }

    body.append("<p><b>Body:</b><br>");
    body.append(request.getBody());

    body.append("</body></html>");
    response.write(body,true);
}
