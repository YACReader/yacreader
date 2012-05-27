#include "data_base_management.h"

#include <QtCore>

DataBaseManagement::DataBaseManagement()
	:QObject(),dataBasesList()
{

}

TreeModel * DataBaseManagement::newTreeModel(QString path)
{
	//la consulta se ejecuta...
	QSqlQuery selectQuery(loadDatabase(path));
	selectQuery.setForwardOnly(true);
	selectQuery.exec("select * from folder order by parentId,name");
	return new TreeModel(selectQuery);
}

QSqlDatabase DataBaseManagement::createDatabase(QString name, QString path)
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(QDir::cleanPath(path) + "/" + name + ".ydb");
	if (!db.open())
		qDebug() << db.lastError();
	else {
		qDebug() << db.tables();
		db.close();
	}
	return db;
}

QSqlDatabase DataBaseManagement::loadDatabase(QString path)
{
	//TODO check path
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(path+"/library.ydb");
	if (!db.open()) {
		//se devuelve una base de datos vacía e inválida
		return QSqlDatabase();
	}

	//devuelve la base de datos
	return db;
}