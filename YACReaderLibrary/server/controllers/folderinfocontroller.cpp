#include "folderinfocontroller.h"
#include "db_helper.h"  //get libraries

#include "folder.h"
#include "comic_db.h"

#include "template.h"
#include "../static.h"


FolderInfoController::FolderInfoController() {}

void FolderInfoController::service(HttpRequest& request, HttpResponse& response)
{
	response.setHeader("Content-Type", "plain/text; charset=ISO-8859-1");

	QString path = QUrl::fromPercentEncoding(request.getPath()).toLatin1();
	QStringList pathElements = path.split('/');
	QString libraryName = pathElements.at(2);
	qulonglong parentId = pathElements.at(4).toULongLong();
	QList<LibraryItem *> folderContent = DBHelper::getFolderContentFromLibrary(libraryName,parentId);
	QList<LibraryItem *> folderComics = DBHelper::getFolderComicsFromLibrary(libraryName,parentId);

	Folder * currentFolder;
	for(QList<LibraryItem *>::const_iterator itr = folderContent.constBegin();itr!=folderContent.constEnd();itr++)
	{
		currentFolder = (Folder *)(*itr);
		response.writeText(QString("/library/%1/folder/%2/info\n").arg(QString(QUrl::toPercentEncoding(libraryName))).arg(currentFolder->id));
	}

	ComicDB * currentComic;
	for(QList<LibraryItem *>::const_iterator itr = folderComics.constBegin();itr!=folderComics.constEnd();itr++)
	{
		currentComic = (ComicDB *)(*itr);
		response.writeText(QString("/library/%1/comic/%2\n").arg(QString(QUrl::toPercentEncoding(libraryName))).arg(currentComic->id));
	}

}