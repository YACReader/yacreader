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
signals:

public slots:
	void loadJSONResponse(const QString & response);

protected:
	bool error;
	qint32 numResults;
};

#endif // RESPONSE_PARSER_H
