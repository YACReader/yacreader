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
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(2).toInt();
    qulonglong parentId = pathElements.at(4).toULongLong();

    serviceContent(libraryId, parentId, response);

    response.write("",true);
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
            response.write(QString("f\t%1\t%2\t%3\t%4\t%5\r\n").arg(library).arg(currentFolder->id).arg(currentFolder->name).arg(currentFolder->getNumChildren()).arg(currentFolder->getFirstChildHash()).toUtf8());
        }
        else
        {
            currentComic = (ComicDB *)(*itr);
            response.write(QString("c\t%1\t%2\t%3\t%4\t%5\r\n").arg(library).arg(currentComic->id).arg(currentComic->getFileName()).arg(currentComic->getFileSize()).arg(currentComic->info.hash).toUtf8());
        }
    }

    clock_t end = clock();
    double msecs = double(end - begin);

    response.write(QString("%1ms").arg(msecs).toUtf8());
}
