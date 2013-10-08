#include "volumes_model.h"

#include <QtScript>


VolumesModel::VolumesModel(QObject *parent) :
	JSONModel(parent)
{
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
		QScriptValueIterator it(sc.property("results"));
		while (it.hasNext()) {
			it.next();
			qDebug("Nick %s",it.value().property("nick").toString().toStdString().c_str());
		}
			/*
		if(sc.property("number_of_total_results").isValid())
			sc.property("number_of_total_results").toString().toInt();// sc.property("number_of_total_results").toInt32();
		else
			qDebug() << sc.property("oops").toString();*/
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
		return _data.at(0).count();
}

QVariant VolumesModel::data(const QModelIndex &index, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	int row = index.row();
	int column = index.column();
	return _data[row][column];
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
	return QModelIndex();
}

