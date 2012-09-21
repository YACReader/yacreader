#include "librariescontroller.h"
#include "library_window.h"  //get libraries

#include "template.h"
#include "../static.h"

extern LibraryWindow * mw;

LibrariesController::LibrariesController() {}

void LibrariesController::service(HttpRequest& request, HttpResponse& response)
{
	response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");

	HttpSession session=Static::sessionStore->getSession(request,response);

	session.set("xxx","yyy");
	QString postData = QString::fromUtf8(request.getBody());
	response.writeText(postData);

	QList<QString> data = postData.split("\n");
	session.setDeviceType(data.at(0).split(":").at(1));
	session.setDisplayType(data.at(1).split(":").at(1));
	QList<QString> comics = data.at(2).split(":").at(1).split("\t");
	foreach(QString hash,comics)
	{
		session.setComicOnDevice(hash);
	}

	Template t=Static::templateLoader->getTemplate("libraries",request.getHeader("Accept-Language"));
	t.enableWarnings();

	QMap<QString,QString> libraries = mw->getLibraries();
	QList<QString> names = libraries.keys();
	
	t.loop("library",names.length());
	int i=0;
	while (i<names.length()) {
		t.setVariable(QString("library%1.name").arg(i),names.at(i));
		t.setVariable(QString("library%1.label").arg(i),names.at(i));
		i++;
	}

	response.write(t.toLatin1(),true);
	
}