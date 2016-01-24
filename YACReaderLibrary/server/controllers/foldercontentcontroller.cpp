#include "foldercontentcontroller.h"

#include <QUrl>

#include "db_helper.h"
#include "comic_db.h"
#include "folder.h"

#include "qnaturalsorting.h"

#include <ctime>
using namespace std;

struct LibraryItemSorter
{
    bool operator()(const LibraryItem * a,const LibraryItem * b) const
    {
        return naturalSortLessThanCI(a->name,b->name);
    }
};

FolderContentController::FolderContentController() {}

void FolderContentController::service(HttpRequest& request, HttpResponse& response)
{
    response.setHeader("Content-Type", "plain/text; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(2).toInt();
    qulonglong parentId = pathElements.at(4).toULongLong();

    serviceContent(libraryId, parentId, response);

    response.writeText("",true);
}

void FolderContentController::serviceContent(const int &library, const qulonglong &folderId, HttpResponse &response)
{
     clock_t begin = clock();

    QList<LibraryItem *> folderContent = DBHelper::getFolderSubfoldersFromLibrary(library,folderId);
    QList<LibraryItem *> folderComics = DBHelper::getFolderComicsFromLibrary(library,folderId);

    folderContent.append(folderComics);
    qSort(folderContent.begin(),folderContent.end(),LibraryItemSorter());

    folderComics.clear();

    ComicDB * currentComic;
    Folder * currentFolder;
    for(QList<LibraryItem *>::const_iterator itr = folderContent.constBegin();itr!=folderContent.constEnd();itr++)
    {
        if((*itr)->isDir())
        {
            currentFolder = (Folder *)(*itr);
            response.writeText(QString("f:%1:%2:%3:%4\r\n").arg(library).arg(currentFolder->id).arg(currentFolder->name).arg(currentFolder->numChildren));
        }
        else
        {
            currentComic = (ComicDB *)(*itr);
            response.writeText(QString("c:%1:%2:%3:%4:%5\r\n").arg(library).arg(currentComic->id).arg(currentComic->getFileName()).arg(currentComic->getFileSize()).arg(currentComic->info.hash));
        }
    }

    clock_t end = clock();
    double msecs = double(end - begin);

    response.writeText(QString("%1ms").arg(msecs));
}
