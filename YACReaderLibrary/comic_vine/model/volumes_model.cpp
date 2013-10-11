#include "volumes_model.h"

#include <QtScript>


VolumesModel::VolumesModel(QObject *parent) :
	JSONModel(parent)
{
}

VolumesModel::~VolumesModel()
{
	std::for_each(_data.begin(), _data.end(), [](QList<QString> * ptr) { delete ptr; });
}

void VolumesModel::load(const QString &json)
{
	QScriptEngine engine;
	QScriptValue sc;
	sc = engine.evaluate("(" + json + ")");

	if (!sc.property("error").isValid() && sc.property("error").toString() != "OK")
	{
		qDebug("Error detected");
	}
	else
	{
		int numResults = sc.property("number_of_total_results").toString().toInt(); //fix to weird behaviour using hasNext
		QScriptValueIterator it(sc.property("results"));
		bool test;
		while (it.hasNext()) {
			it.next();
			QString numIssues = it.value().property("count_of_issues").toString();
			QString year = it.value().property("start_year").toString();
			QString name = it.value().property("name").toString();
			QString publisher = it.value().property("publisher").property("name").toString();
			QString url = it.value().property("image").property("screen_url").toString();
			QStringList & l = *(new QStringList);
			l << name << year << numIssues << publisher << url;
			test = name.isEmpty() && year.isEmpty() && numIssues.isEmpty() && url.isEmpty();
			if(numResults>0 && !test)
				_data.push_back(&l);
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
	if(_data.isEmpty())
		return 0;
	else
		return 4;//_data.at(0)->count();
}

QVariant VolumesModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DecorationRole)
	{
			return QVariant();
	}
	if (role == Qt::TextAlignmentRole)
	{
		//TODO
	}

	if(role != Qt::DisplayRole)
		return QVariant();

	int row = index.row();
	int column = index.column();
	return _data[row]->at(column);
}

Qt::ItemFlags VolumesModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant VolumesModel::headerData(int section, Qt::Orientation orientation, int role) const
{

	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section)//TODO obtener esto de la query
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

	if (orientation == Qt::Horizontal && role == Qt::TextAlignmentRole)
	{
		switch(section)//TODO obtener esto de la query
		{
		case YEAR:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		case ISSUES:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		}
	}

	return QVariant();
}

QModelIndex VolumesModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	return createIndex(row, column, _data[row]);
}

