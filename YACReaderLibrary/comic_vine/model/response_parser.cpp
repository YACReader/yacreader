#include "response_parser.h"

#include <QtScript>
#include <QDebug>

ResponseParser::ResponseParser(QObject *parent) :
	QObject(parent),error(false),numResults(-1),currentPage(-1),totalPages(-1)
{
}

bool ResponseParser::responseError()
{
	return error;
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

void ResponseParser::loadJSONResponse(const QString &response)
{
	QScriptEngine engine;
	QScriptValue sc;
	sc = engine.evaluate("(" + response + ")");

	if (!sc.property("error").isValid() && sc.property("error").toString() != "OK")
	{
		error = true;
		qDebug("Error detected");
	}
	else
	{
		error = false;
		if(sc.property("number_of_total_results").isValid())
			numResults = sc.property("number_of_total_results").toString().toInt();// sc.property("number_of_total_results").toInt32();
		else
			qDebug() << sc.property("oops").toString();

		int limit = sc.property("limit").toInt32();
		int offset = sc.property("offset").toInt32();
		int total = sc.property("number_of_total_results").toInt32();
		if(limit > 0)
		{
			totalPages = (total / limit) + (total%limit>0?1:0);
			currentPage = (offset / limit) + 1;
		}
		else
			totalPages = currentPage = 1;
	}
}
