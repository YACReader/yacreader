#ifndef YACREADERCOMICSSELECTIONHELPER_H
#define YACREADERCOMICSSELECTIONHELPER_H

#include <QItemSelectionModel>
#include <QModelIndex>
#include <QModelIndexList>
#include <QObject>
#include <QVariantMap>

class ComicModel;

class YACReaderComicsSelectionHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qulonglong selectionRevision READ selectionRevision NOTIFY selectionChanged)
public:
    explicit YACReaderComicsSelectionHelper(QObject *parent = nullptr);

    void setModel(ComicModel *model);

    Q_INVOKABLE void selectIndex(int index);
    Q_INVOKABLE void selectOnly(int index);
    Q_INVOKABLE void deselectIndex(int index);
    Q_INVOKABLE bool isSelectedIndex(int index) const;
    Q_INVOKABLE void clear();
    Q_INVOKABLE int numItemsSelected() const;
    Q_INVOKABLE int lastSelectedIndex() const;
    Q_INVOKABLE QModelIndex currentIndex();
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE QModelIndexList selectedIndexes() const;
    Q_INVOKABLE QModelIndexList selectedRows(int column = 0) const;
    QVariantMap selectionInfo() const;
    qulonglong selectionRevision() const;

    QItemSelectionModel *selectionModel();

signals:
    void selectionChanged();

public slots:

private:
    QItemSelectionModel *itemSelectionModel = nullptr;
    ComicModel *model = nullptr;
    qulonglong revision = 0;
};

#endif // YACREADERCOMICSSELECTIONHELPER_H
