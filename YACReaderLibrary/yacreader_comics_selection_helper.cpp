#include "yacreader_comics_selection_helper.h"

#include "comic_model.h"

YACReaderComicsSelectionHelper::YACReaderComicsSelectionHelper(QObject *parent) : QObject(parent), _selectionModel(nullptr)
{

}

void YACReaderComicsSelectionHelper::setModel(ComicModel *model)
{
    if(model == NULL)
        return;

    this->model = model;

    if(_selectionModel != nullptr)
        delete _selectionModel;

    _selectionModel = new QItemSelectionModel(model);
}

void YACReaderComicsSelectionHelper::selectIndex(int index)
{
    if(_selectionModel != nullptr && model!=NULL)
    {
        _selectionModel->select(model->index(index,0),QItemSelectionModel::Select | QItemSelectionModel::Rows);

        emit selectionChanged();
    }
}

void YACReaderComicsSelectionHelper::deselectIndex(int index)
{
    if(_selectionModel != nullptr && model!=NULL)
    {
        _selectionModel->select(model->index(index,0),QItemSelectionModel::Deselect | QItemSelectionModel::Rows);

        emit selectionChanged();
    }
}

bool YACReaderComicsSelectionHelper::isSelectedIndex(int index) const
{
    if(_selectionModel != nullptr && model!=NULL)
    {
        QModelIndex mi = model->index(index,0);
        return _selectionModel->isSelected(mi);
    }
    return false;
}

void YACReaderComicsSelectionHelper::clear()
{
    if(_selectionModel != nullptr)
    {
        _selectionModel->clear();

        emit selectionChanged();
    }
}

QModelIndex YACReaderComicsSelectionHelper::currentIndex()
{
    if(!_selectionModel)
        return QModelIndex();

    QModelIndexList indexes = _selectionModel->selectedRows();
    if(indexes.length()>0)
        return indexes[0];

    this->selectIndex(0);
    indexes = _selectionModel->selectedRows();
    if(indexes.length()>0)
        return indexes[0];
    else
        return QModelIndex();
}

void YACReaderComicsSelectionHelper::selectAll()
{
    QModelIndex top = model->index(0, 0);
    QModelIndex bottom = model->index(model->rowCount()-1, 0);
    QItemSelection selection(top, bottom);
    _selectionModel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);

    emit selectionChanged();
}

QModelIndexList YACReaderComicsSelectionHelper::selectedRows(int column) const
{
    return _selectionModel->selectedRows(column);
}

QList<QModelIndex> YACReaderComicsSelectionHelper::selectedIndexes() const
{
    return _selectionModel->selectedIndexes();
}

int YACReaderComicsSelectionHelper::numItemsSelected() const
{
    if(_selectionModel != nullptr)
    {
        return _selectionModel->selectedRows().length();
    }

    return 0;
}

int YACReaderComicsSelectionHelper::lastSelectedIndex() const
{
    if(_selectionModel != nullptr)
    {
        return _selectionModel->selectedRows().last().row();
    }

    return -1;
}

QItemSelectionModel *YACReaderComicsSelectionHelper::selectionModel()
{
    QModelIndexList indexes = _selectionModel->selectedRows();
    if(indexes.length()==0)
        this->selectIndex(0);

    return _selectionModel;
}
