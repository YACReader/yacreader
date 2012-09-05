#include "librariescontroller.h"
#include "library_window.h"  //get libraries

#include "template.h"
#include "../static.h"

extern LibraryWindow * mw;

LibrariesController::LibrariesController() {}

void LibrariesController::service(HttpRequest& request, HttpResponse& response)
{
	response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");

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