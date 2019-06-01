#ifndef RESPONSE_PARSER_H
#define RESPONSE_PARSER_H

#include <QObject>

class ResponseParser : public QObject
{
    Q_OBJECT
public:
    explicit ResponseParser(QObject *parent = nullptr);
    bool responseError();
    QString errorDescription();
    qint32 getNumResults();
    qint32 getCurrentPage();
    qint32 getTotalPages();
    bool isError(qint32 error);
signals:

public slots:
    void loadJSONResponse(const QString &response);

protected:
    bool error;
    QString errorTxt;
    qint32 numResults;
    qint32 currentPage;
    qint32 totalPages;
};

#endif // RESPONSE_PARSER_H
