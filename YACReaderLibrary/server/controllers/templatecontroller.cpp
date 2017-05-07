/**
  @file
  @author Stefan Frings
*/

#include "templatecontroller.h"
#include "template.h"
#include "../static.h"

TemplateController::TemplateController(){}

void TemplateController::service(HttpRequest& request, HttpResponse& response) {

    response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");

    Template t=Static::templateLoader->getTemplate("demo",request.getHeader("Accept-Language"));
    t.enableWarnings();
    t.setVariable("path",request.getPath());
    QMap<QByteArray,QByteArray> headers=request.getHeaderMap();
    QMapIterator<QByteArray,QByteArray> iterator(headers);
    t.loop("header",headers.size());
    int i=0;
    while (iterator.hasNext()) {
        iterator.next();
        t.setVariable(QString("header%1.name").arg(i),QString(iterator.key()));
        t.setVariable(QString("header%1.value").arg(i),QString(iterator.value()));
        ++i;
    }

    response.write(t.toLatin1(),true);
}
