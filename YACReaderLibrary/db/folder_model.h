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

#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlDatabase>

#include "yacreader_global.h"

class FolderItem;

class FolderModelProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FolderModelProxy(QObject *parent = 0);
    ~FolderModelProxy();

    void setFilter(const YACReader::SearchModifiers modifier, QString filter, bool includeComics);
    void setupFilteredModelData( QSqlQuery &sqlquery, FolderItem *parent);
    void setupFilteredModelData();
    void clear();

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

protected:
    FolderItem *rootItem;
    QMap<unsigned long long int, FolderItem *> filteredItems; //relación entre folders

    bool includeComics;
    QString filter;
    bool filterEnabled;

    YACReader::SearchModifiers modifier;
};

class FolderModel : public QAbstractItemModel
{

	Q_OBJECT

    friend class FolderModelProxy;

public:
    FolderModel(QObject *parent = 0);
    FolderModel( QSqlQuery &sqlquery, QObject *parent = 0);
    ~FolderModel();

    //QAbstractItemModel methods
	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
					  const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

    //Convenience methods
    void setupModelData(QString path);
    QString getDatabase();
	QString getFolderPath(const QModelIndex &folder);
    //QModelIndex indexFromItem(FolderItem * item, int column);


    //bool isFilterEnabled(){return filterEnabled;};

    void updateFolderCompletedStatus(const QModelIndexList & list, bool status);
    void updateFolderFinishedStatus(const QModelIndexList & list, bool status);

    QStringList getSubfoldersNames(const QModelIndex & mi);

    void fetchMoreFromDB(const QModelIndex & parent);

    QModelIndex addFolderAtParent(const QString & folderName, const QModelIndex & parent);

    enum Columns {
        Name = 0,
        Path = 1,
        Finished = 2,
        Completed = 3
    };//id INTEGER PRIMARY KEY, parentId INTEGER NOT NULL, name TEXT NOT NULL, path TEXT NOT NULL

    enum Roles {
        FinishedRole = Qt::UserRole + 1,
        CompletedRole,
        IdRole
    };

public slots:
    void deleteFolder(const QModelIndex & mi);
    void updateFolderChildrenInfo(qulonglong folderId);

private:
	void setupModelData( QSqlQuery &sqlquery, FolderItem *parent);
    void updateFolderModelData( QSqlQuery &sqlquery, FolderItem *parent);

	FolderItem *rootItem; //el árbol
	QMap<unsigned long long int, FolderItem *> items; //relación entre folders

	QString _databasePath;

signals:
	void beforeReset();
	void reset();
};
//! [0]

#endif
