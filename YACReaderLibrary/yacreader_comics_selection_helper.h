#ifndef YACREADERCOMICSSELECTIONHELPER_H
#define YACREADERCOMICSSELECTIONHELPER_H

#include <QObject>
#include <QtWidgets>

class ComicModel;

class YACReaderComicsSelectionHelper : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderComicsSelectionHelper(QObject *parent = 0);

    void setModel(ComicModel *model);

    Q_INVOKABLE void selectIndex(int index);
    Q_INVOKABLE void deselectIndex(int index);
    Q_INVOKABLE bool isSelectedIndex(int index) const;
    Q_INVOKABLE void clear();
    Q_INVOKABLE int numItemsSelected() const;
    Q_INVOKABLE int lastSelectedIndex() const;
    Q_INVOKABLE QModelIndex currentIndex();
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE QModelIndexList selectedIndexes() const;
    Q_INVOKABLE QModelIndexList selectedRows(int column = 0) const;

    QItemSelectionModel * selectionModel();

signals:
    void selectionChanged();

public slots:

protected:
    QItemSelectionModel * _selectionModel;

    ComicModel * model;
};

#endif // YACREADERCOMICSSELECTIONHELPER_H
