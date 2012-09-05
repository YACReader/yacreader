/**
  @file
  @author Stefan Frings
*/

#include "requestmapper.h"
#include "static.h"
#include "staticfilecontroller.h"
#include "controllers/dumpcontroller.h"
#include "controllers/templatecontroller.h"
#include "controllers/formcontroller.h"
#include "controllers/fileuploadcontroller.h"
#include "controllers/sessioncontroller.h"

#include "controllers/librariescontroller.h"
#include "controllers/foldercontroller.h"
#include "controllers/covercontroller.h"

RequestMapper::RequestMapper(QObject* parent)
    :HttpRequestHandler(parent) {}

void RequestMapper::service(HttpRequest& request, HttpResponse& response) {
    QByteArray path=request.getPath();
    qDebug("RequestMapper: path=%s",path.data());

	//primera petición, se ha hecho un post, se sirven las bibliotecas si la seguridad mediante login no está habilitada
	if(path == "/")
	{
		LibrariesController().service(request, response);
	}

	//listar el contenido del folder
	if(path.contains("folder") && !path.contains("info"))
	{
		FolderController().service(request, response);
	}

	if(path.contains("cover") )
	{
		CoverController().service(request, response);
	}
	else
	{
		if (path.startsWith("/dump")) {
			DumpController().service(request, response);
		}

		else if (path.startsWith("/template")) {
			TemplateController().service(request, response);
		}

		else if (path.startsWith("/form")) {
			FormController().service(request, response);
		}

		else if (path.startsWith("/file")) {
			FileUploadController().service(request, response);
		}

		else if (path.startsWith("/session")) {
			SessionController().service(request, response);
		}

		// All other pathes are mapped to the static file controller.
		else {
			Static::staticFileController->service(request, response);
		}
	}
}
