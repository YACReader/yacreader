#ifndef RESPONSE_PARSER_H
#define RESPONSE_PARSER_H

#include <QObject>

class ResponseParser : public QObject
{
	Q_OBJECT
public:
	explicit ResponseParser(QObject *parent = 0);
	bool responseError();
	qint32 getNumResults();
	qint32 getCurrentPage();
	qint32 getTotalPages();
signals:

public slots:
	void loadJSONResponse(const QString & response);

protected:
	bool error;
	qint32 numResults;
	qint32 currentPage;
	qint32 totalPages;
};

#endif // RESPONSE_PARSER_H
