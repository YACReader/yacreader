#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H

#include <QProcess>

class PackageManager : public QObject
{
	Q_OBJECT
public:
	PackageManager();
	void createPackage(const QString & libraryPath,const QString & dest);
	void extractPackage(const QString & packagePath,const QString & destDir);
	public slots:
	void cancel();
private:
	bool creating;
	QProcess * _7z;

signals:
	void exported();
	void imported();
};

#endif
