#include "yacreader_libraries.h"
#include "yacreader_global.h"



YACReaderLibraries::YACReaderLibraries()
	:QObject()
{

}

YACReaderLibraries::YACReaderLibraries(const YACReaderLibraries &source)
	:QObject(),libraries(source.libraries)
{

}

QList<QString> YACReaderLibraries::getNames()
{
	return libraries.keys();
}

QString YACReaderLibraries::getPath(const QString &name)
{
	return libraries.value(name).second;
}

QString YACReaderLibraries::getPath(int id)
{
	foreach(QString name, libraries.keys())
		if(libraries.value(name).first == id)
			return libraries.value(name).second;
	return "";
}

QString YACReaderLibraries::getName(int id)
{
	foreach(QString name, libraries.keys())
		if(libraries.value(name).first == id)
			return name;
	return "";
}

bool YACReaderLibraries::isEmpty()
{
	return libraries.isEmpty();
}

bool YACReaderLibraries::contains(const QString &name)
{
	return libraries.contains(name);
}

bool YACReaderLibraries::contains(int id)
{
	foreach(QString name, libraries.keys())
		if(libraries.value(name).first == id)
			return true;
	return false;
}

void YACReaderLibraries::remove(const QString &name)
{
	libraries.remove(name);
}

void YACReaderLibraries::rename(const QString &oldName, const QString &newName)
{
	if(libraries.contains(oldName))
	{
		QPair<int,QString> value = libraries.value(oldName);
		libraries.remove(oldName);
		libraries.insert(newName,value);
	}
}

int YACReaderLibraries::getId(const QString &name)
{
	return libraries.value(name).first;
}

YACReaderLibraries &YACReaderLibraries::operator=(const YACReaderLibraries &source)
{
	libraries = source.libraries;
	return *this;
}

QMap<QString, QPair<int, QString> > YACReaderLibraries::getLibraries()
{
	return libraries;
}


void YACReaderLibraries::addLibrary(const QString &name, const QString &path)
{
	int newID=0;
	foreach(QString name, libraries.keys())
		newID = qMax(0,libraries.value(name).first);
	newID++;
	libraries.insert(name,QPair<int,QString>(newID,path));
}

void YACReaderLibraries::load()
{
	QSettings settings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat);

	if(settings.value(LIBRARIES).isValid())
	{
		QByteArray data = settings.value(LIBRARIES).toByteArray();
		QDataStream in(&data, QIODevice::ReadOnly);
		in >> libraries;
	}
	else //only for compatibility with old versions (<7.0)
	{
		QFile f(QCoreApplication::applicationDirPath()+"/libraries.yacr");
		f.open(QIODevice::ReadOnly);
		QTextStream txtS(&f);
		QString content = txtS.readAll();
		QStringList lines = content.split('\n');
		QString line,name;
		int i=0;

		foreach(line,lines)
		{
			if((i%2)==0)
				name = line;
			else
				addLibrary(name.trimmed(),line.trimmed());
			i++;
		}
		f.close();
		if(save())
			f.remove();
	}
}

bool YACReaderLibraries::save()
{
	QSettings settings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat);

	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	out << libraries;
	settings.setValue(LIBRARIES, data);

	return settings.isWritable();
}
