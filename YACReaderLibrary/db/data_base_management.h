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
	static QSqlDatabase createDatabase(QString name, QString path);
	static QSqlDatabase loadDatabase(QString path);
};

#endif