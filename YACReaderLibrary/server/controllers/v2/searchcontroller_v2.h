
#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

#include <QSqlQuery>

class SearchController : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchController)
public:
    SearchController();

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    void serviceSearch(int libraryId, const QString &query, stefanfrings::HttpResponse &response);
    void getFolders(int libraryId, QSqlQuery &sqlQuery, QJsonArray &items);
    void getComics(int libraryId, QSqlQuery &sqlQuery, QJsonArray &items);
};

#endif // SEARCHCONTROLLER_H
