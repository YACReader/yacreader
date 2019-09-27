#include "response_parser.h"

#include <QtScript>
#include <QDebug>

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
    QScriptEngine engine;
    QScriptValue sc;
    sc = engine.evaluate("(" + response + ")");

    errorTxt = "None";

    if (!sc.property("status_code").isValid() || isError(sc.property("status_code").toInt32())) {
        error = true;
        if (sc.property("error").isValid())
            errorTxt = sc.property("error").toString();
        else
            errorTxt = "Unknown error";
    } else {
        error = false;
        if (sc.property("number_of_total_results").isValid())
            numResults = sc.property("number_of_total_results").toString().toInt(); // sc.property("number_of_total_results").toInt32();
        else
            qDebug() << sc.property("oops").toString();

        int limit = sc.property("limit").toInt32();
        int offset = sc.property("offset").toInt32();
        int total = sc.property("number_of_total_results").toInt32();
        if (limit > 0) {
            totalPages = (total / limit) + (total % limit > 0 ? 1 : 0);
            currentPage = (offset / limit) + 1;
        } else
            totalPages = currentPage = 1;
    }
}
