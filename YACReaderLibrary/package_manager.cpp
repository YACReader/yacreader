#include "package_manager.h"
#include <QCoreApplication>

PackageManager::PackageManager()
:_7z(0)
{

}

void PackageManager::createPackage(const QString & libraryPath,const QString & dest)
{
	QStringList attributes;
	attributes << "a" << "-y" << "-ttar" << dest+".clc" << libraryPath  ;
	_7z = new QProcess();
	connect(_7z,SIGNAL(error(QProcess::ProcessError)),this,SLOT(openingError(QProcess::ProcessError)));
	connect(_7z,SIGNAL(finished(int,QProcess::ExitStatus)),this,SIGNAL(exported()));
	_7z->start(QCoreApplication::applicationDirPath()+"/utils/7z",attributes);
}

void PackageManager::extractPackage(const QString & packagePath,const QString & destDir)
{
	QStringList attributes;
	QString output = "-o";
	output += destDir;
	attributes << "x" << "-y" << output << packagePath;
	_7z = new QProcess();
	connect(_7z,SIGNAL(error(QProcess::ProcessError)),this,SLOT(openingError(QProcess::ProcessError)));
	connect(_7z,SIGNAL(finished(int,QProcess::ExitStatus)),this,SIGNAL(imported()));
	_7z->start(QCoreApplication::applicationDirPath()+"/utils/7z",attributes);
}

void PackageManager::cancel()
{
	if(_7z!=0)
	{
		_7z->disconnect();
		_7z->kill();
		if(creating)
		{
			//TODO remove dest+".clc"
		}
		else
		{
		    //TODO fixed: is done by libraryWindow
		}
	}
}
