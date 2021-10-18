#include "volume_comics_model.h"
#include "qnaturalsorting.h"

#include <QJsonDocument>
#include <QJsonParseError>

bool lessThan(const QList<QString> &left, const QList<QString> &right)
{
    if ((left.count() > 0) && (right.count() > 0))
        return naturalSortLessThanCI(left.at(0), right.at(0));
    else
        return true;
}

VolumeComicsModel::VolumeComicsModel(QObject *parent)
    : JSONModel(parent), numExtraRows(0)
{
}

void VolumeComicsModel::load(const QString &json)
{
    QJsonParseError Err;
    QVariantMap sc = QJsonDocument::fromJson(json.toUtf8(), &Err).toVariant().toMap();

    if (Err.error != QJsonParseError::NoError) {
        qDebug("Error detected");
        return;
    }

    QListIterator<QVariant> it(sc.value("results").toList());
    QVariantMap resultsValue;
    while (it.hasNext()) {
        resultsValue = it.next().toMap();
        QString issueNumber = resultsValue.value("issue_number").toString();
        QVariant propertyName = resultsValue.value("name");
        QString name = propertyName.isNull() ? "-" : propertyName.toString();
        QString coverURL = resultsValue.value("image").toMap().value("medium_url").toString();
        QString id = resultsValue.value("id").toString();
        QStringList l;
        l << issueNumber << name << coverURL << id;
        _data.push_back(l);
    }

    std::sort(_data.begin(), _data.end(), lessThan);
}

/*void VolumeComicsModel::load(const QStringList &jsonList)
{
    foreach (QString json, jsonList) {
        load(json);
    }
}*/

QModelIndex VolumeComicsModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex(); // no parent
}

int VolumeComicsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _data.count() + numExtraRows;
}

int VolumeComicsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (_data.isEmpty())
        return 0;
    else
        return 2;
}

QVariant VolumeComicsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();

    if (role == Qt::DecorationRole) {
        return QVariant();
    }
    if (role == Qt::TextAlignmentRole) {
        switch (column) // TODO obtener esto de la query
        {
        case ISSUE:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case TITLE:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    if (row < _data.count())
        return _data[row][column];
    else
        return QVariant();
}

Qt::ItemFlags VolumeComicsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant VolumeComicsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) // TODO obtener esto de la query
        {
        case ISSUE:
            return QVariant(QString("issue"));
        case TITLE:
            return QVariant(QString(tr("title")));
        }
    }

    if (orientation == Qt::Horizontal && role == Qt::TextAlignmentRole) {
        switch (section) // TODO obtener esto de la query
        {
        case ISSUE:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case TITLE:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    return QVariant();
}

QModelIndex VolumeComicsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QString VolumeComicsModel::getComicId(const QModelIndex &index) const
{
    int row = index.row();
    if (row >= _data.count())
        return "";
    return _data[row][ID];
}

QString VolumeComicsModel::getComicId(int row) const
{
    if (row >= _data.count())
        return "";
    return _data[row][ID];
}

QString VolumeComicsModel::getCoverURL(const QModelIndex &index) const
{
    return _data[index.row()][COVER_URL];
}

void VolumeComicsModel::addExtraRows(int numRows)
{
    numExtraRows = numRows;
}
