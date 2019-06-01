#include "volumes_model.h"

#include <QtScript>

VolumesModel::VolumesModel(QObject *parent)
    : JSONModel(parent)
{
}

VolumesModel::~VolumesModel()
{
    //std::for_each(_data.begin(), _data.end(), [](QList<QString> * ptr) { delete ptr; });
}

void VolumesModel::load(const QString &json)
{
    QScriptEngine engine;
    QScriptValue sc;
    sc = engine.evaluate("(" + json + ")");

    if (!sc.property("error").isValid() && sc.property("error").toString() != "OK") {
        qDebug("Error detected");
    } else {
        int numResults = sc.property("number_of_total_results").toString().toInt(); //fix to weird behaviour using hasNext
        QScriptValueIterator it(sc.property("results"));
        bool test;
        QScriptValue resultsValue;
        while (it.hasNext()) {
            it.next();
            resultsValue = it.value();
            QString numIssues = resultsValue.property("count_of_issues").toString();
            QString year = resultsValue.property("start_year").toString();
            QString name = resultsValue.property("name").toString();
            QString publisher = resultsValue.property("publisher").property("name").toString();
            QString url = resultsValue.property("image").property("medium_url").toString();
            QString deck = resultsValue.property("deck").toString();
            QString id = resultsValue.property("id").toString();
            QStringList l;
            l << name << year << numIssues << publisher << url << deck << id;
            test = name.isEmpty() && year.isEmpty() && numIssues.isEmpty() && url.isEmpty();
            if (numResults > 0 && !test)
                _data.push_back(l);
            numResults--;
        }
    }
}

QModelIndex VolumesModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex(); //no parent
}

int VolumesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _data.count();
}

int VolumesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (_data.isEmpty())
        return 0;
    else
        return 4; //_data.at(0)->count();
}

QVariant VolumesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DecorationRole) {
        return QVariant();
    }

    int row = index.row();
    int column = index.column();

    if (role == Qt::TextAlignmentRole) {
        switch (column) {
        case YEAR:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ISSUES:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    if (column == YEAR || column == ISSUES) {
        return _data[row][column].toInt();
    } else {
        return _data[row][column];
    }
}

Qt::ItemFlags VolumesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant VolumesModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) //TODO obtener esto de la query
        {
        case SERIES:
            return QVariant(QString("series"));
        case YEAR:
            return QVariant(QString(tr("year")));
        case ISSUES:
            return QVariant(QString(tr("issues")));
        case PUBLISHER:
            return QVariant(QString(tr("publisher")));
        }
    }

    if (orientation == Qt::Horizontal && role == Qt::TextAlignmentRole) {
        switch (section) //TODO obtener esto de la query
        {
        case YEAR:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ISSUES:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    return QVariant();
}

QModelIndex VolumesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QString VolumesModel::getVolumeId(const QModelIndex &index) const
{
    return _data[index.row()][ID];
}

int VolumesModel::getNumIssues(const QModelIndex &index) const
{
    return _data[index.row()][ISSUES].toInt();
}

QString VolumesModel::getPublisher(const QModelIndex &index) const
{
    return _data[index.row()][PUBLISHER];
}

QString VolumesModel::getCoverURL(const QModelIndex &index) const
{
    return _data[index.row()][COVER_URL];
}
