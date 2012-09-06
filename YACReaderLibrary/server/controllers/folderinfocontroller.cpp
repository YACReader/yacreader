#include "folderinfocontroller.h"
#include "library_window.h"  //get libraries

#include "folder.h"
#include "comic_db.h"

#include "template.h"
#include "../static.h"

extern LibraryWindow * mw;

FolderInfoController::FolderInfoController() {}

void FolderInfoController::service(HttpRequest& request, HttpResponse& response)
{
	response.setHeader("Content-Type", "plain/text; charset=ISO-8859-1");

	QString path = request.getPath();
	QStringList pathElements = path.split('/');
	QString libraryName = pathElements.at(2);
	qulonglong parentId = pathElements.at(4).toULongLong();
	QList<LibraryItem *> folderContent = mw->getFolderContentFromLibrary(libraryName,parentId);
	QList<LibraryItem *> folderComics = mw->getFolderComicsFromLibrary(libraryName,parentId);

	Folder * currentFolder;
	for(QList<LibraryItem *>::const_iterator itr = folderContent.constBegin();itr!=folderContent.constEnd();itr++)
	{
		currentFolder = (Folder *)(*itr);
		response.writeText(QString("/library/%1/folder/%2/info\n").arg(libraryName).arg(currentFolder->id));
	}

	ComicDB * currentComic;
	for(QList<LibraryItem *>::const_iterator itr = folderComics.constBegin();itr!=folderComics.constEnd();itr++)
	{
		currentComic = (ComicDB *)(*itr);
		response.writeText(QString("/library/%1/comic/%2\n").arg(libraryName).arg(currentComic->id));
	}

}