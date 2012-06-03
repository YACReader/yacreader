#ifndef __DATA_BASE_MANAGEMENT_H
#define __DATA_BASE_MANAGEMENT_H

#include <QtCore>
#include <QtSql>
#include <QSqlDatabase>

#include "treemodel.h"

class DataBaseManagement : public QObject
{
	Q_OBJECT
private:
	QList<QString> dataBasesList;
public:
	DataBaseManagement();
	TreeModel * newTreeModel(QString path);
	//crea una base de datos y todas sus tablas
	static QSqlDatabase createDatabase(QString name, QString path);
	//carga una base de datos desde la ruta path
	static QSqlDatabase loadDatabase(QString path);
	static bool createTables(QSqlDatabase & database);
};

#endif