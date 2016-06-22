#include "librariescontroller.h"
#include "db_helper.h"  //get libraries
#include "yacreader_libraries.h"
#include "yacreader_http_session.h"

#include "template.h"
#include "../static.h"

#include "QsLog.h"

LibrariesController::LibrariesController() {}

void LibrariesController::service(HttpRequest& request, HttpResponse& response)
{
    HttpSession session=Static::sessionStore->getSession(request,response,false);
    YACReaderHttpSession *ySession = Static::yacreaderSessionStore.value(session.getId());

    response.setHeader("Content-Type", "text/html; charset=utf-8");
    response.setHeader("Connection","close");

    ySession->clearNavigationPath();

    Template t=Static::templateLoader->getTemplate("libraries_"+ySession->getDeviceType(),request.getHeader("Accept-Language"));
	t.enableWarnings();

	YACReaderLibraries libraries = DBHelper::getLibraries();
	QList<QString> names = DBHelper::getLibrariesNames();

	t.loop("library",names.length());

	int currentId = 0;
	int i = 0;
	foreach (QString name,names) {
		currentId = libraries.getId(name);
		t.setVariable(QString("library%1.name").arg(i),QString::number(currentId));
		t.setVariable(QString("library%1.label").arg(i),name);
		i++;
	}

    response.setStatus(200,"OK");
    response.write(t.toUtf8(),true);
}
