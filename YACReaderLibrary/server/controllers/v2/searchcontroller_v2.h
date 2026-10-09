
#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QUuid>

class SearchController : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchController)
public:
    SearchController();

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    void serviceSearch(int libraryId, const QString &query, stefanfrings::HttpResponse &response);
    void getFolders(int libraryId, const QUuid &libraryUuid, QSqlQuery &sqlQuery, QJsonArray &items);
    void getComics(int libraryId, const QUuid &libraryUuid, QSqlDatabase &db, QSqlQuery &sqlQuery, QJsonArray &items);
};

#endif // SEARCHCONTROLLER_H
