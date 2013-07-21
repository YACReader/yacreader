#ifndef TABLEITEM_H
#define TABLEITEM_H

#include <QList>
#include <QVariant>

//! [0]
class TableItem : public QObject
{
	Q_OBJECT
public:
    TableItem(const QList<QVariant> &data);
    ~TableItem();
    int columnCount() const;
    QVariant data(int column) const;
	void setData(int column,const QVariant & value);
    int row() const;
	//unsigned long long int id; //TODO sustituir por una clase adecuada
	//Comic comic;
private:
    QList<QVariant> itemData;
    
	
};
//! [0]

#endif