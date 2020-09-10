#include "librariescontroller.h"
#include "db_helper.h" //get libraries
#include "yacreader_libraries.h"
#include "yacreader_http_session.h"

#include "template.h"
#include "../static.h"

#include "QsLog.h"

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;
using stefanfrings::HttpSession;
using stefanfrings::Template;

LibrariesController::LibrariesController() { }

void LibrariesController::service(HttpRequest &request, HttpResponse &response)
{
    HttpSession session = Static::sessionStore->getSession(request, response, false);
    YACReaderHttpSession *ySession = Static::yacreaderSessionStore->getYACReaderSessionHttpSession(session.getId());

    response.setHeader("Content-Type", "text/html; charset=utf-8");
    response.setHeader("Connection", "close");

    ySession->clearNavigationPath();

    Template t = Static::templateLoader->getTemplate("libraries", request.getHeader("Accept-Language"));
    t.enableWarnings();

    // set device type and display
    t.setVariable("device", ySession->getDeviceType());
    t.setVariable("display", ySession->getDisplayType());

    YACReaderLibraries libraries = DBHelper::getLibraries();
    QList<QString> names = DBHelper::getLibrariesNames();

    t.loop("library", names.length());

    int currentId = 0;
    int i = 0;
    foreach (QString name, names) {
        currentId = libraries.getId(name);
        t.setVariable(QString("library%1.name").arg(i), QString::number(currentId));
        t.setVariable(QString("library%1.label").arg(i), name);
        i++;
    }

    response.setStatus(200, "OK");
    response.write(t.toUtf8(), true);
}
