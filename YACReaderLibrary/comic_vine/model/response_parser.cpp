#include "response_parser.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonParseError>

ResponseParser::ResponseParser(QObject *parent)
    : QObject(parent), error(false), errorTxt("None"), numResults(-1), currentPage(-1), totalPages(-1)
{
}

bool ResponseParser::responseError()
{
    return error;
}

QString ResponseParser::errorDescription()
{
    return errorTxt;
}

qint32 ResponseParser::getNumResults()
{
    return numResults;
}

qint32 ResponseParser::getCurrentPage()
{
    return currentPage;
}

qint32 ResponseParser::getTotalPages()
{
    return totalPages;
}

bool ResponseParser::isError(qint32 error)
{
    switch (error) {
    case 100:
        return true;

    default:
        return false;
    }
}

void ResponseParser::loadJSONResponse(const QString &response)
{
    QJsonParseError Err;
    QVariantMap sc = QJsonDocument::fromJson(response.toUtf8(), &Err).toVariant().toMap();

    errorTxt = "None";

    if (Err.error != QJsonParseError::NoError) {
        errorTxt = "Json syntax error";
        error = true;
        return;
    }

    if (!sc.value("status_code").isValid() || isError(sc.value("status_code").toInt())) {
        error = true;
        if (sc.value("error").isValid()) {
            errorTxt = sc.value("error").toString();

        } else {
            errorTxt = "Unknown error";
        }
        return;
    }

    error = false;
    if (sc.value("number_of_total_results").isValid()) {
        numResults = sc.value("number_of_total_results").toInt(); // sc.property("number_of_total_results").toInt32();

    } else {
        qDebug() << sc.value("oops").toString();
    }

    auto limit = sc.value("limit").toInt();
    auto offset = sc.value("offset").toInt();
    auto total = sc.value("number_of_total_results").toInt();
    if (limit > 0) {
        totalPages = (total / limit) + (total % limit > 0 ? 1 : 0);
        currentPage = (offset / limit) + 1;
    } else {
        totalPages = currentPage = 1;
    }
}
