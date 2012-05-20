#ifndef __DATA_BASE_MANAGEMENT_H
#define __DATA_BASE_MANAGEMENT_H

#include <QtCore>
#include <QtSql>

class DataBaseManagement : public QObject
{
	Q_OBJECT
private:
	QList<QString> dataBasesList;
public:
	DataBaseManagement();
	bool createDataBase(QString name, QString path);

};

#endif