#ifndef LOCAL_COMIC_LIST_MODEL_H
#define LOCAL_COMIC_LIST_MODEL_H

#include "comic_record.h"

#include <QAbstractItemModel>

#include <optional>

class LocalComicListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit LocalComicListModel(QObject *parent = nullptr);

    void load(const QList<ComicRecord> &comics);

    // QAbstractItemModel methods
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    // a padding row (no comic) has no record
    QList<std::optional<ComicRecord>> getData();

    void removeComics(const QList<QModelIndex> &selectedIndexes);
    void restoreAll();
signals:

public slots:
    void moveSelectionUp(const QList<QModelIndex> &selectedIndexes);
    void moveSelectionDown(const QList<QModelIndex> &selectedIndexes);
    void addExtraRows(int numRows);

private:
    int numExtraRows;
    QList<std::optional<ComicRecord>> _data;
    QList<std::optional<ComicRecord>> _removed;
};

#endif // LOCAL_COMIC_LIST_MODEL_H
