#include "folderinfocontroller_v2.h"
#include "db_helper.h"  //get libraries

#include "folder.h"
#include "comic_db.h"

#include "template.h"
#include "../static.h"


FolderInfoControllerV2::FolderInfoControllerV2() {}

void FolderInfoControllerV2::service(HttpRequest& request, HttpResponse& response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
	QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();
	QString libraryName = DBHelper::getLibraryName(libraryId);
    qulonglong parentId = pathElements.at(5).toULongLong();

    serviceComics(libraryId, parentId, response);

    response.write("",true);
}

void FolderInfoControllerV2::serviceComics(const int &library, const qulonglong &folderId, HttpResponse &response)
{
    QList<LibraryItem *> folderContent = DBHelper::getFolderSubfoldersFromLibrary(library,folderId);
    QList<LibraryItem *> folderComics = DBHelper::getFolderComicsFromLibrary(library,folderId);

    ComicDB * currentComic;
    for(QList<LibraryItem *>::const_iterator itr = folderComics.constBegin();itr!=folderComics.constEnd();itr++)
    {
        currentComic = (ComicDB *)(*itr);
        response.write(QString("/v2/library/%1/comic/%2:%3:%4:%5:%6\r\n")
                       .arg(library)
                       .arg(currentComic->id)
                       .arg(currentComic->getFileName())
                       .arg(currentComic->getFileSize())
                       .arg(currentComic->info.read ? 1 : 0)
                       .arg(currentComic->info.hash)
                       .toUtf8());
        delete currentComic;
    }

    Folder * currentFolder;
    for(QList<LibraryItem *>::const_iterator itr = folderContent.constBegin();itr!=folderContent.constEnd();itr++)
    {
        currentFolder = (Folder *)(*itr);
        serviceComics(library, currentFolder->id, response);
        delete currentFolder;
    }
}
