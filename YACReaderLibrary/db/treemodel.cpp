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

#ifdef Q_OS_MAC
#include <QFileIconProvider>
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
	//lo más probable es que el nodo raíz no necesite tener información
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
        return QVariant(QFileIconProvider().icon(QFileIconProvider::Folder)); //TODO draw a tick on top when it is needed
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
	//inicializar el nodo raíz
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
	//64 bits para la primary key, es decir la misma precisión que soporta sqlit 2^64
	//el diccionario permitirá encontrar cualquier nodo del árbol rápidamente, de forma que añadir un hijo a un padre sea O(1)
	items.clear();
	//se añade el nodo 0
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
		//la inserción de hijos se hace de forma ordenada
		TreeItem * parent = items.value(record.value("parentId").toULongLong());
		if(parent !=0) //TODO if parent==0 the parent of item was removed from the DB and delete on cascade didn't work, ERROR.
			parent->appendChild(item);
		//se añade el item al map, de forma que se pueda encontrar como padre en siguientes iteraciones
		items.insert(item->id,item);
	}
}

void TreeModel::setupFilteredModelData()
{
	beginResetModel();
	
	//TODO hay que liberar memoria de anteriores filtrados

	//inicializar el nodo raíz

	if(rootBeforeFilter == 0)
		rootBeforeFilter = rootItem;
	else
		delete rootItem;//los resultados de la búsqueda anterior deben ser borrados

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
	//64 bits para la primary key, es decir la misma precisión que soporta sqlit 2^64
	filteredItems.clear();

	//se añade el nodo 0 al modelo que representa el arbol de elementos que cumplen con el filtro
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

		//se añade el item al map, de forma que se pueda encontrar como padre en siguientes iteraciones
		if(!filteredItems.contains(item->id))
			filteredItems.insert(item->id,item);

		//es necesario conocer las coordenadas de origen para poder realizar scroll automático en la vista
		item->originalItem = items.value(item->id);

		//si el padre ya existe en el modelo, el item se añade como hijo
		if(filteredItems.contains(parentId))
			filteredItems.value(parentId)->appendChild(item);
		else//si el padre aún no se ha añadido, hay que añadirlo a él y todos los padres hasta el nodo raíz
		{
			//comprobamos con esta variable si el último de los padres (antes del nodo raíz) ya existía en el modelo
			bool parentPreviousInserted = false;

			//mientras no se alcance el nodo raíz se procesan todos los padres (de abajo a arriba)
			while(parentId != ROOT )
			{
				//el padre no estaba en el modelo filtrado, así que se rescata del modelo original
				TreeItem * parentItem = items.value(parentId);
				//se debe crear un nuevo nodo (para no compartir los hijos con el nodo original)
				TreeItem * newparentItem = new TreeItem(parentItem->getData()); //padre que se añadirá a la estructura de directorios filtrados
				newparentItem->id = parentId;

				newparentItem->originalItem = parentItem;

				//si el modelo contiene al padre, se añade el item actual como hijo
				if(filteredItems.contains(parentId))
				{
					filteredItems.value(parentId)->appendChild(item);
					parentPreviousInserted = true;
				}
				//sino se registra el nodo para poder encontrarlo con posterioridad y se añade el item actual como hijo
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

			//si el nodo es hijo de 1 y no había sido previamente insertado como hijo, se añade como tal
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
	rootItem = rootBeforeFilter; //TODO si no se aplica el filtro previamente, esto invalidaría en modelo
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
