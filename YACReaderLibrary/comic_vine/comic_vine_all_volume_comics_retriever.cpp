#include "comic_vine_all_volume_comics_retriever.h"

#include "http_worker.h"
#include "response_parser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

ComicVineAllVolumeComicsRetriever::ComicVineAllVolumeComicsRetriever(const QString &volumeURLString, QObject *parent)
    : QObject(parent), volumeURLString(volumeURLString)
{
}

void ComicVineAllVolumeComicsRetriever::getAllVolumeComics()
{
    getAllVolumeComics(0);
}

void ComicVineAllVolumeComicsRetriever::getAllVolumeComics(int range)
{
    HttpWorker *search = new HttpWorker(volumeURLString.arg(range));
    connect(search, &HttpWorker::dataReady, this, &ComicVineAllVolumeComicsRetriever::appendVolumeComicsInfo);
    connect(search, &HttpWorker::timeout, this, &ComicVineAllVolumeComicsRetriever::timeOut);
    connect(search, &HttpWorker::timeout, this, &ComicVineAllVolumeComicsRetriever::finished);
    connect(search, &QThread::finished, search, &QObject::deleteLater);
    search->get();
}

void ComicVineAllVolumeComicsRetriever::appendVolumeComicsInfo(const QByteArray &data)
{
    QString json(data);

    jsonResponses.append(data);

    ResponseParser rp;
    rp.loadJSONResponse(json);

    qint32 currentPage = rp.getCurrentPage();
    qint32 totalPages = rp.getTotalPages();

    bool isLastResponse = currentPage == totalPages;

    if (!isLastResponse) {
        getAllVolumeComics(currentPage * 100);
    } else {
        emit allVolumeComicsInfo(consolidateJSON());
        emit finished();
    }
}

QString ComicVineAllVolumeComicsRetriever::consolidateJSON()
{
    QJsonObject consolidatedJSON;
    QJsonArray comicsInfo;

    foreach (QByteArray json, jsonResponses) {
        QJsonDocument doc = QJsonDocument::fromJson(json);

        if (doc.isNull() || !doc.isObject() || doc.isEmpty()) {
            continue;
        }

        QJsonObject main = doc.object();
        QJsonValue error = main["error"];

        if (error.isUndefined() || error.toString() != "OK") {
            continue;
        } else {
            QJsonValue results = main["results"];
            if (results.isUndefined() || !results.isArray()) {
                continue;
            }

            QJsonArray resultsArray = results.toArray();
            foreach (const QJsonValue &v, resultsArray)
                comicsInfo.append(v);
        }
    }

    consolidatedJSON["error"] = "OK";
    consolidatedJSON["status_code"] = 1;
    consolidatedJSON["number_of_total_results"] = comicsInfo.size();
    consolidatedJSON["offset"] = 0;
    consolidatedJSON["results"] = comicsInfo;

    QJsonDocument doc(consolidatedJSON);
    return doc.toJson(QJsonDocument::Compact);
}
