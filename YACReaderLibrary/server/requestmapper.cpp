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
#include "controllers/comiccontroller.h"
#include "controllers/folderinfocontroller.h"
#include "controllers/pagecontroller.h"
#include "controllers/errorcontroller.h"

#include "db_helper.h"

RequestMapper::RequestMapper(QObject* parent)
    :HttpRequestHandler(parent) {}

void RequestMapper::service(HttpRequest& request, HttpResponse& response) {
	QByteArray path=request.getPath();
	qDebug("RequestMapper: path=%s",path.data());

	QRegExp folder("/library/.+/folder/[0-9]+/?");//get comic content
	QRegExp folderInfo("/library/.+/folder/[0-9]+/info/?"); //get folder info
	QRegExp comic("/library/.+/comic/[0-9]+/?"); //get comic info
	QRegExp comicOpen("/library/.+/comic/[0-9]+/open/?"); //the server will open for reading the comic
	QRegExp comicClose("/library/.+/comic/[0-9]+/close/?"); //the server will close the comic and free memory
	QRegExp cover("/library/.+/cover/[0-9a-f]+.jpg"); //get comic cover (navigation)
	QRegExp comicPage("/library/.+/comic/[0-9]+/page/[0-9]+/?"); //get comic page

	QRegExp library("/library/([0-9]+)/.+"); //permite verificar que la biblioteca solicitada existe

	path = QUrl::fromPercentEncoding(path).toLatin1();

	//primera petición, se ha hecho un post, se sirven las bibliotecas si la seguridad mediante login no está habilitada
	if(path == "/")
	{
		LibrariesController().service(request, response);
	}

	else 
	{
		//se comprueba que la sesión sea la correcta con el fin de evitar accesos no autorizados
		HttpSession session=Static::sessionStore->getSession(request,response,false);
		if(!session.isNull() && session.contains("ySession"))
		{
			if(library.indexIn(path)!=-1 && DBHelper::getLibraries().count() > library.cap(1).toInt() )
			{
				//listar el contenido del folder
				if(folder.exactMatch(path))
				{
					FolderController().service(request, response);
				}
				else if (folderInfo.exactMatch(path))
				{
					FolderInfoController().service(request, response);
				}
				else if(cover.exactMatch(path))
				{
					CoverController().service(request, response);
				}
				else if(comic.exactMatch(path))
				{
					ComicController().service(request, response);
				}
				else if(comicPage.exactMatch(path))
				{
					PageController().service(request,response);
				}
			}
			else
			{
				//response.writeText(library.cap(1));
				Static::staticFileController->service(request, response);
			}
		}
		else //acceso no autorizado, redirección
		{
			ErrorController(300).service(request,response);
		}
	}

}