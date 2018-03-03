#include "foldercontentcontroller.h"

#include <QUrl>

#include "db_helper.h"
#include "comic_db.h"
#include "folder.h"

#include "yacreader_server_data_helper.h"

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
    //clock_t begin = clock();

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
            response.write(YACReaderServerDataHelper::folderToYSFormat(library, *currentFolder).toUtf8());
        }
        else
        {
            currentComic = (ComicDB *)(*itr);
            response.write(YACReaderServerDataHelper::comicToYSFormat(library, *currentComic).toUtf8());
        }
    }

    /*clock_t end = clock();
    double msecs = double(end - begin);

    response.write(QString("%1ms").arg(msecs).toUtf8());*/
}
