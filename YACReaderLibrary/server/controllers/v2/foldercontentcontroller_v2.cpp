#include "foldercontentcontroller_v2.h"

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

FolderContentControllerV2::FolderContentControllerV2() {}

void FolderContentControllerV2::service(HttpRequest& request, HttpResponse& response)
{
    response.setHeader("Content-Type", "application/json");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();
    qulonglong parentId = pathElements.at(5).toULongLong();

    serviceContent(libraryId, parentId, response);

    response.setStatus(200,"OK");
    response.write("",true);
}

void FolderContentControllerV2::serviceContent(const int &library, const qulonglong &folderId, HttpResponse &response)
{
    QList<LibraryItem *> folderContent = DBHelper::getFolderSubfoldersFromLibrary(library,folderId);
    QList<LibraryItem *> folderComics = DBHelper::getFolderComicsFromLibrary(library,folderId);

    folderContent.append(folderComics);
    qSort(folderContent.begin(),folderContent.end(),LibraryItemSorter());

    folderComics.clear();

    QJsonArray items;

    ComicDB * currentComic;
    Folder * currentFolder;
    for(QList<LibraryItem *>::const_iterator itr = folderContent.constBegin();itr!=folderContent.constEnd();itr++)
    {
        if((*itr)->isDir())
        {
            currentFolder = (Folder *)(*itr);
            items.append(YACReaderServerDataHelper::folderToJSON(library, *currentFolder));
        }
        else
        {
            currentComic = (ComicDB *)(*itr);
            items.append(YACReaderServerDataHelper::comicToJSON(library, *currentComic));
        }
    }

    QJsonDocument output(items);

    response.write(output.toJson());
}
