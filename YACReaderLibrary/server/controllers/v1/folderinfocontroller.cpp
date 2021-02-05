#include "folderinfocontroller.h"
#include "db_helper.h" //get libraries

#include "folder.h"
#include "comic_db.h"

#include "template.h"
#include "../static.h"

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

FolderInfoController::FolderInfoController() { }

void FolderInfoController::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(2).toInt();
    qulonglong parentId = pathElements.at(4).toULongLong();

    serviceComics(libraryId, parentId, response);

    response.write("", true);
}

void FolderInfoController::serviceComics(const int &library, const qulonglong &folderId, HttpResponse &response)
{
    QList<LibraryItem *> folderContent = DBHelper::getFolderSubfoldersFromLibrary(library, folderId);
    QList<LibraryItem *> folderComics = DBHelper::getFolderComicsFromLibrary(library, folderId);

    ComicDB *currentComic;
    for (QList<LibraryItem *>::const_iterator itr = folderComics.constBegin(); itr != folderComics.constEnd(); itr++) {
        currentComic = (ComicDB *)(*itr);
        response.write(QString("/library/%1/comic/%2:%3:%4\r\n").arg(library).arg(currentComic->id).arg(currentComic->getFileName()).arg(currentComic->getFileSize()).toUtf8());
        delete currentComic;
    }

    Folder *currentFolder;
    for (QList<LibraryItem *>::const_iterator itr = folderContent.constBegin(); itr != folderContent.constEnd(); itr++) {
        currentFolder = (Folder *)(*itr);
        serviceComics(library, currentFolder->id, response);
        delete currentFolder;
    }
}
