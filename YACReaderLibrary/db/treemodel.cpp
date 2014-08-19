/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

/*
	treemodel.cpp

	Provides a simple tree model to show how to create and use hierarchical
	models.
*/

#include <QtGui>


#include "treeitem.h"
#include "treemodel.h"
#include "data_base_management.h"
#include "folder.h"
#include "db_helper.h"
#include "qnaturalsorting.h"

#ifdef Q_OS_MAC
#include <QFileIconProvider>
QIcon finishedFolderIcon;
void drawMacOSXFinishedFolderIcon()
{
    QIcon ico = QFileIconProvider().icon(QFileIconProvider::Folder);
    QPixmap pixNormalOff = ico.pixmap(16,16, QIcon::Normal, QIcon::Off);
    QPixmap pixNormalOn = ico.pixmap(16,16, QIcon::Normal, QIcon::On);
    QPixmap pixSelectedOff = ico.pixmap(16,16, QIcon::Selected, QIcon::Off);
    QPixmap pixSelectedOn = ico.pixmap(16,16, QIcon::Selected, QIcon::On);
    QPixmap tick(":/images/folder_finished_macosx.png");


    {
        QPainter p(&pixNormalOff);
        p.drawPixmap(4,7,tick);
    }
    finishedFolderIcon.addPixmap(pixNormalOff, QIcon::Normal, QIcon::Off);

    {
        QPainter p(&pixNormalOn);
        p.drawPixmap(4,7,tick);
    }
    finishedFolderIcon.addPixmap(pixNormalOn, QIcon::Normal, QIcon::On);

    {
        QPainter p(&pixSelectedOff);
        p.drawPixmap(4,7,tick);
    }
    finishedFolderIcon.addPixmap(pixSelectedOff, QIcon::Selected, QIcon::Off);

    {
        QPainter p(&pixSelectedOn);
        p.drawPixmap(4,7,tick);
    }
    finishedFolderIcon.addPixmap(pixSelectedOn, QIcon::Selected, QIcon::On);
}
#endif

#define ROOT 1

TreeModel::TreeModel(QObject *parent)
	: QAbstractItemModel(parent),rootItem(0),rootBeforeFilter(0),filterEnabled(false),includeComics(false)
{
	connect(this,SIGNAL(beforeReset()),this,SIGNAL(modelAboutToBeReset()));
	connect(this,SIGNAL(reset()),this,SIGNAL(modelReset()));
}

//! [0]
TreeModel::TreeModel( QSqlQuery &sqlquery, QObject *parent)
	: QAbstractItemModel(parent),rootItem(0),rootBeforeFilter(0),filterEnabled(false),includeComics(false)
{
	//lo m�s probable es que el nodo ra�z no necesite tener informaci�n
	QList<QVariant> rootData;
	rootData << "root"; //id 0, padre 0, title "root" (el id, y el id del padre van a ir en la clase TreeItem)
	rootItem = new TreeItem(rootData);
	rootItem->id = ROOT;
	rootItem->parentItem = 0;
	setupModelData(sqlquery, rootItem);
	//sqlquery.finish();
}
//! [0]

//! [1]
TreeModel::~TreeModel()
{
	if(rootItem != 0)
		delete rootItem;
}
//! [1]

//! [2]
int TreeModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return rootItem->columnCount();
}
//! [2]

//! [3]
QVariant TreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

	if (role == Qt::DecorationRole)

#ifdef Q_OS_MAC
        if(item->data(TreeModel::Finished).toBool()){
            if(finishedFolderIcon.isNull()){
                drawMacOSXFinishedFolderIcon();
            }

            return QVariant(finishedFolderIcon);
        }
        else {
            return QVariant(QFileIconProvider().icon(QFileIconProvider::Folder));
        }
#else
        if(item->data(TreeModel::Finished).toBool())
            return QVariant(QIcon(":/images/folder_finished.png"));
        else
            return QVariant(QIcon(":/images/folder.png"));
#endif

	if (role != Qt::DisplayRole)
		return QVariant();



	return item->data(index.column());
}
//! [3]

//! [4]
Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//! [4]

//! [5]
QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
							   int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}
//! [5]

//! [6]
QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent)
			const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	TreeItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	TreeItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}
//! [6]

//! [7]
QModelIndex TreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
	TreeItem *parentItem = childItem->parent();

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}
//! [7]

QModelIndex TreeModel::indexFromItem(TreeItem * item,int column)
{
	//if(item->parent() != 0)
	//	return index(item->row(),column,parent(indexFromItem(item->parent(),column-1)));
	//else
	//	return index(item->row(),0,QModelIndex());
	return createIndex(item->row(), column, item);
}


//! [8]
int TreeModel::rowCount(const QModelIndex &parent) const
{
	TreeItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	return parentItem->childCount();
}
//! [8]

void TreeModel::setupModelData(QString path)
{
	beginResetModel();
	if(rootItem != 0)
		delete rootItem; //TODO comprobar que se libera bien la memoria
	filterEnabled = false;
	rootItem = 0;
	rootBeforeFilter = 0;
	//inicializar el nodo ra�z
	QList<QVariant> rootData;
	rootData << "root"; //id 0, padre 0, title "root" (el id, y el id del padre van a ir en la clase TreeItem)
	rootItem = new TreeItem(rootData);
	rootItem->id = ROOT;
	rootItem->parentItem = 0;

	//cargar la base de datos
	_databasePath = path;
	QSqlDatabase db = DataBaseManagement::loadDatabase(path);
	//crear la consulta
	{
	QSqlQuery selectQuery("select * from folder where id <> 1 order by parentId,name",db);

	setupModelData(selectQuery,rootItem);
	}
	//selectQuery.finish();
	db.close();
	QSqlDatabase::removeDatabase(path);
	endResetModel();

}


void TreeModel::setupModelData(QSqlQuery &sqlquery, TreeItem *parent)
{
	//64 bits para la primary key, es decir la misma precisi�n que soporta sqlit 2^64
	//el diccionario permitir� encontrar cualquier nodo del �rbol r�pidamente, de forma que a�adir un hijo a un padre sea O(1)
	items.clear();
	//se a�ade el nodo 0
	items.insert(parent->id,parent);

	while (sqlquery.next()) {
		QList<QVariant> data;
		QSqlRecord record = sqlquery.record();

		data << record.value("name").toString();
		data << record.value("path").toString();
        data << record.value("finished").toBool();
        data << record.value("completed").toBool();
		TreeItem * item = new TreeItem(data);

		item->id = record.value("id").toULongLong();
		//la inserci�n de hijos se hace de forma ordenada
		TreeItem * parent = items.value(record.value("parentId").toULongLong());
		if(parent !=0) //TODO if parent==0 the parent of item was removed from the DB and delete on cascade didn't work, ERROR.
			parent->appendChild(item);
		//se a�ade el item al map, de forma que se pueda encontrar como padre en siguientes iteraciones
		items.insert(item->id,item);
	}
}

void TreeModel::setupFilteredModelData()
{
	beginResetModel();
	
	//TODO hay que liberar memoria de anteriores filtrados

	//inicializar el nodo ra�z

	if(rootBeforeFilter == 0)
		rootBeforeFilter = rootItem;
	else
		delete rootItem;//los resultados de la b�squeda anterior deben ser borrados

	QList<QVariant> rootData;
	rootData << "root"; //id 1, padre 1, title "root" (el id, y el id del padre van a ir en la clase TreeItem)
	rootItem = new TreeItem(rootData);
	rootItem->id = ROOT;
	rootItem->parentItem = 0;

	//cargar la base de datos
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	//crear la consulta
	{
	QSqlQuery selectQuery(db); //TODO check
	if(!includeComics)
	{
		selectQuery.prepare("select * from folder where id <> 1 and upper(name) like upper(:filter) order by parentId,name ");
		selectQuery.bindValue(":filter", "%%"+filter+"%%");
	}
	else
	{
        selectQuery.prepare("SELECT DISTINCT f.id, f.parentId, f.name, f.path, f.finished, f.completed FROM folder f INNER JOIN comic c ON (f.id = c.parentId) WHERE f.id <> 1 AND ((UPPER(c.fileName) like UPPER(:filter)) OR (UPPER(f.name) like UPPER(:filter2))) ORDER BY f.parentId,f.name");
		selectQuery.bindValue(":filter", "%%"+filter+"%%");
		selectQuery.bindValue(":filter2", "%%"+filter+"%%");
	}
		selectQuery.exec();
		
	setupFilteredModelData(selectQuery,rootItem);
	}
	//selectQuery.finish();
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

    endResetModel();
}

void TreeModel::setupFilteredModelData(QSqlQuery &sqlquery, TreeItem *parent)
{
	//64 bits para la primary key, es decir la misma precisi�n que soporta sqlit 2^64
	filteredItems.clear();

	//se a�ade el nodo 0 al modelo que representa el arbol de elementos que cumplen con el filtro
	filteredItems.insert(parent->id,parent);

	while (sqlquery.next()) {  //se procesan todos los folders que cumplen con el filtro
		//datos de la base de datos
		QList<QVariant> data;
		QSqlRecord record = sqlquery.record();

		data << record.value("name").toString();
		data << record.value("path").toString();
        data << record.value("finished").toBool();
        data << record.value("completed").toBool();

		TreeItem * item = new TreeItem(data);
		item->id = sqlquery.value(0).toULongLong();

		//id del padre
		quint64 parentId = record.value("parentId").toULongLong();

		//se a�ade el item al map, de forma que se pueda encontrar como padre en siguientes iteraciones
		if(!filteredItems.contains(item->id))
			filteredItems.insert(item->id,item);

		//es necesario conocer las coordenadas de origen para poder realizar scroll autom�tico en la vista
		item->originalItem = items.value(item->id);

		//si el padre ya existe en el modelo, el item se a�ade como hijo
		if(filteredItems.contains(parentId))
			filteredItems.value(parentId)->appendChild(item);
		else//si el padre a�n no se ha a�adido, hay que a�adirlo a �l y todos los padres hasta el nodo ra�z
		{
			//comprobamos con esta variable si el �ltimo de los padres (antes del nodo ra�z) ya exist�a en el modelo
			bool parentPreviousInserted = false;

			//mientras no se alcance el nodo ra�z se procesan todos los padres (de abajo a arriba)
			while(parentId != ROOT )
			{
				//el padre no estaba en el modelo filtrado, as� que se rescata del modelo original
				TreeItem * parentItem = items.value(parentId);
				//se debe crear un nuevo nodo (para no compartir los hijos con el nodo original)
				TreeItem * newparentItem = new TreeItem(parentItem->getData()); //padre que se a�adir� a la estructura de directorios filtrados
				newparentItem->id = parentId;

				newparentItem->originalItem = parentItem;

				//si el modelo contiene al padre, se a�ade el item actual como hijo
				if(filteredItems.contains(parentId))
				{
					filteredItems.value(parentId)->appendChild(item);
					parentPreviousInserted = true;
				}
				//sino se registra el nodo para poder encontrarlo con posterioridad y se a�ade el item actual como hijo
				else
				{
					newparentItem->appendChild(item);
					filteredItems.insert(newparentItem->id,newparentItem);
					parentPreviousInserted = false;
				}

				//variables de control del bucle, se avanza hacia el nodo padre
				item = newparentItem;
				parentId = parentItem->parentItem->id;
			}

			//si el nodo es hijo de 1 y no hab�a sido previamente insertado como hijo, se a�ade como tal
			if(!parentPreviousInserted)
				filteredItems.value(ROOT)->appendChild(item);
		}
	}
}



QString TreeModel::getDatabase()
{
	return _databasePath;
}

QString TreeModel::getFolderPath(const QModelIndex &folder)
{
    return static_cast<TreeItem*>(folder.internalPointer())->data(TreeModel::Path).toString();
}

void TreeModel::setFilter(QString filter, bool includeComics)
{
	this->filter = filter;
	this->includeComics = includeComics;
	filterEnabled = true;
	setupFilteredModelData();
}

void TreeModel::resetFilter()
{
	beginResetModel();
	filter = "";
	includeComics = false;
	//TODO hay que liberar la memoria reservada para el filtrado
	//items.clear();
	filteredItems.clear();
	TreeItem * root = rootItem;
	rootItem = rootBeforeFilter; //TODO si no se aplica el filtro previamente, esto invalidar�a en modelo
	if(root !=0)
		delete root;

	rootBeforeFilter = 0;
	filterEnabled = false;
	endResetModel();


}

void TreeModel::updateFolderCompletedStatus(const QModelIndexList &list, bool status)
{
    QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
    db.transaction();
    foreach (QModelIndex mi, list)
    {
        TreeItem * item = static_cast<TreeItem*>(mi.internalPointer());
        item->setData(TreeModel::Completed,status);

        Folder f = DBHelper::loadFolder(item->id,db);
        f.setCompleted(status);
        DBHelper::update(f,db);
    }
    db.commit();
    db.close();
    QSqlDatabase::removeDatabase(_databasePath);

    emit dataChanged(index(list.first().row(),TreeModel::Name),index(list.last().row(),TreeModel::Completed));
}

void TreeModel::updateFolderFinishedStatus(const QModelIndexList &list, bool status)
{
    QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
    db.transaction();
    foreach (QModelIndex mi, list)
    {
        TreeItem * item = static_cast<TreeItem*>(mi.internalPointer());
        item->setData(TreeModel::Finished,status);

        Folder f = DBHelper::loadFolder(item->id,db);
        f.setFinished(status);
        DBHelper::update(f,db);
    }
    db.commit();
    db.close();
    QSqlDatabase::removeDatabase(_databasePath);

    emit dataChanged(index(list.first().row(),TreeModel::Name),index(list.last().row(),TreeModel::Completed));
}

QStringList TreeModel::getSubfoldersNames(const QModelIndex &mi)
{
    QStringList result;
    qulonglong id = 1;
    if(mi.isValid()){
        TreeItem * item = static_cast<TreeItem*>(mi.internalPointer());
        id = item->id;
    }

    QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
    db.transaction();

    result = DBHelper::loadSubfoldersNames(id,db);

    db.commit();
    db.close();
    QSqlDatabase::removeDatabase(_databasePath);

    //TODO sort result))
    qSort(result.begin(),result.end(),naturalSortLessThanCI);
    return result;
}
