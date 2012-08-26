#ifndef TABLEITEM_H
#define TABLEITEM_H

#include <QList>
#include <QVariant>
#include "comic.h"

//! [0]
class TableItem
{
public:
    TableItem(const QList<QVariant> &data);
    ~TableItem();
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
	unsigned long long int id; //TODO sustituir por una clase adecuada
	//Comic comic;
private:
    QList<QVariant> itemData;
    
	
};
//! [0]

#endif