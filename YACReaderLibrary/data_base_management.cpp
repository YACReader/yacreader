#include "data_base_management.h"

#include <QtCore>

DataBaseManagement::DataBaseManagement()
	:QObject(),dataBasesList()
{

}

bool DataBaseManagement::createDataBase(QString name, QString path)
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(name + ".ydb");
    if (!db.open())
        qDebug() << db.lastError();
    else {
        qDebug() << db.tables();
        db.close();
    }
	return true;
}