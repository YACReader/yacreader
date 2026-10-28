#include "folderinfocontroller_v2.h"

#include "comic_db.h"
#include "db_helper.h" //get libraries
#include "folder.h"

#include <QUrl>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

FolderInfoControllerV2::FolderInfoControllerV2() { }

void FolderInfoControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();
    qulonglong parentId = pathElements.at(5).toULongLong();

    serviceComics(libraryId, parentId, response);

    response.write("", true);
}

void FolderInfoControllerV2::serviceComics(const int &library, const qulonglong &folderId, HttpResponse &response)
{
    const auto folderContent = DBHelper::getFolderSubfoldersFromLibrary(library, folderId);
    const auto folderComics = DBHelper::getFolderComicsFromLibrary(library, folderId);

    for (const auto &comic : folderComics) {
        response.write(QString("/v2/library/%1/comic/%2:%3:%4:%5:%6\r\n")
                               .arg(library)
                               .arg(comic.file.id.value())
                               .arg(comic.getFileName())
                               .arg(comic.getFileSize())
                               .arg(comic.info.read ? 1 : 0)
                               .arg(comic.info.hash)
                               .toUtf8());
    }

    for (const auto &folder : folderContent) {
        serviceComics(library, folder.id.value(), response);
    }
}
