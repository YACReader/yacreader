#include "yacreader_comics_selection_helper.h"

#include "comic_model.h"

#include <QLocale>
#include <QSet>
#include <QVariantList>

#include <algorithm>
#include <utility>

YACReaderComicsSelectionHelper::YACReaderComicsSelectionHelper(QObject *parent)
    : QObject(parent)
{
}

void YACReaderComicsSelectionHelper::setModel(ComicModel *model)
{
    if (model == nullptr)
        return;

    this->model = model;

    delete itemSelectionModel;

    itemSelectionModel = new QItemSelectionModel(model, this);
    connect(itemSelectionModel, &QItemSelectionModel::selectionChanged, this, [this]() {
        ++revision;
        emit selectionChanged();
    });

    ++revision;
    emit selectionChanged();
}

void YACReaderComicsSelectionHelper::selectIndex(int index)
{
    if (itemSelectionModel != nullptr && model != nullptr && index >= 0 && index < model->rowCount())
        itemSelectionModel->select(model->index(index, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void YACReaderComicsSelectionHelper::selectOnly(int index)
{
    if (itemSelectionModel != nullptr && model != nullptr && index >= 0 && index < model->rowCount())
        itemSelectionModel->select(model->index(index, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void YACReaderComicsSelectionHelper::deselectIndex(int index)
{
    if (itemSelectionModel != nullptr && model != nullptr && index >= 0 && index < model->rowCount())
        itemSelectionModel->select(model->index(index, 0), QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
}

bool YACReaderComicsSelectionHelper::isSelectedIndex(int index) const
{
    if (itemSelectionModel != nullptr && model != nullptr) {
        QModelIndex mi = model->index(index, 0);
        return itemSelectionModel->isSelected(mi);
    }
    return false;
}

void YACReaderComicsSelectionHelper::clear()
{
    if (itemSelectionModel != nullptr)
        itemSelectionModel->clear();
}

QModelIndex YACReaderComicsSelectionHelper::currentIndex()
{
    if (!itemSelectionModel)
        return QModelIndex();

    QModelIndexList indexes = itemSelectionModel->selectedRows();
    if (indexes.length() > 0)
        return indexes[0];

    return QModelIndex();
}

void YACReaderComicsSelectionHelper::selectAll()
{
    if (!itemSelectionModel || !model || model->rowCount() == 0)
        return;

    QModelIndex top = model->index(0, 0);
    QModelIndex bottom = model->index(model->rowCount() - 1, 0);
    QItemSelection selection(top, bottom);
    itemSelectionModel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

QModelIndexList YACReaderComicsSelectionHelper::selectedRows(int column) const
{
    return itemSelectionModel ? itemSelectionModel->selectedRows(column) : QModelIndexList();
}

QList<QModelIndex> YACReaderComicsSelectionHelper::selectedIndexes() const
{
    return itemSelectionModel ? itemSelectionModel->selectedIndexes() : QModelIndexList();
}

QVariantMap YACReaderComicsSelectionHelper::selectionInfo() const
{
    QVariantMap info;
    if (!itemSelectionModel || !model)
        return info;

    // selectedRows() groups the indexes by selection range, so sort them to get
    // the covers of the first comics of the selection, in the order they are
    // shown in the grid.
    auto rows = itemSelectionModel->selectedRows();
    std::sort(rows.begin(), rows.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() < b.row();
    });

    int readCount = 0;
    int inProgressCount = 0;
    int pageCount = 0;
    int unknownPageCount = 0;
    qint64 totalSize = 0;
    QSet<QString> series;
    QVariantList covers;

    for (const auto &index : std::as_const(rows)) {
        const bool read = index.data(ComicModel::ReadColumnRole).toBool();
        const bool inProgress = !read && index.data(ComicModel::HasBeenOpenedRole).toBool() && index.data(ComicModel::CurrentPageRole).toInt() > 0;
        readCount += read ? 1 : 0;
        inProgressCount += inProgress ? 1 : 0;

        const auto pages = index.data(ComicModel::NumPagesRole);
        if (pages.isValid() && pages.toInt() > 0)
            pageCount += pages.toInt();
        else
            ++unknownPageCount;

        // The size in bytes is stored at the end of the hash, right after the 40
        // characters of the SHA1 digest.
        totalSize += index.data(ComicModel::HashRole).toString().mid(40).toLongLong();

        const QString seriesName = index.data(ComicModel::SeriesRole).toString().trimmed();
        if (!seriesName.isEmpty())
            series.insert(seriesName);

        if (covers.size() < 3)
            covers.append(index.data(ComicModel::CoverPathRole));
    }

    info.insert(QStringLiteral("count"), rows.size());
    info.insert(QStringLiteral("readCount"), readCount);
    info.insert(QStringLiteral("inProgressCount"), inProgressCount);
    info.insert(QStringLiteral("unreadCount"), rows.size() - readCount - inProgressCount);
    info.insert(QStringLiteral("pageCount"), pageCount);
    info.insert(QStringLiteral("unknownPageCount"), unknownPageCount);
    info.insert(QStringLiteral("size"), QLocale().formattedDataSize(totalSize, 2, QLocale::DataSizeTraditionalFormat));
    info.insert(QStringLiteral("seriesCount"), series.size());
    if (series.size() == 1)
        info.insert(QStringLiteral("seriesName"), *series.cbegin());
    info.insert(QStringLiteral("covers"), covers);
    return info;
}

int YACReaderComicsSelectionHelper::numItemsSelected() const
{
    if (itemSelectionModel != nullptr) {
        return itemSelectionModel->selectedRows().length();
    }

    return 0;
}

int YACReaderComicsSelectionHelper::lastSelectedIndex() const
{
    if (itemSelectionModel != nullptr) {
        const auto selectedRows = itemSelectionModel->selectedRows();
        return selectedRows.isEmpty() ? -1 : selectedRows.last().row();
    }

    return -1;
}

QItemSelectionModel *YACReaderComicsSelectionHelper::selectionModel()
{
    return itemSelectionModel;
}

qulonglong YACReaderComicsSelectionHelper::selectionRevision() const
{
    return revision;
}
