#ifndef READING_LIST_MODEL_H
#define READING_LIST_MODEL_H

#include <QAbstractItemModel>

class ReadingListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ReadingListModel(QObject *parent = 0);

signals:

public slots:

};

#endif // READING_LIST_MODEL_H
