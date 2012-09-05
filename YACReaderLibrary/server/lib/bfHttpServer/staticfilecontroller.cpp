/**
  @file
  @author Stefan Frings
*/

#include "staticfilecontroller.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

StaticFileController::StaticFileController(QSettings* settings, QObject* parent)
    :HttpRequestHandler(parent)
{
    maxAge=settings->value("maxAge","60000").toInt();
    encoding=settings->value("encoding","UTF-8").toString();
    docroot=settings->value("path",".").toString();
    // Convert relative path to absolute, based on the directory of the config file.
#ifdef Q_OS_WIN32
    if (QDir::isRelativePath(docroot) && settings->format()!=QSettings::NativeFormat)
#else
        if (QDir::isRelativePath(docroot))
#endif
        {
        QFileInfo configFile(settings->fileName());
        docroot=QFileInfo(configFile.absolutePath(),docroot).absoluteFilePath();
    }
    qDebug("StaticFileController: docroot=%s, encoding=%s, maxAge=%i",qPrintable(docroot),qPrintable(encoding),maxAge);
    maxCachedFileSize=settings->value("maxCachedFileSize","65536").toInt();
    cache.setMaxCost(settings->value("cacheSize","1000000").toInt());
    cacheTimeout=settings->value("cacheTime","60000").toInt();
    qDebug("StaticFileController: cache timeout=%i, size=%i",cacheTimeout,cache.maxCost());
}


void StaticFileController::service(HttpRequest& request, HttpResponse& response) {
    QByteArray path=request.getPath();
    // Forbid access to files outside the docroot directory
    if (path.startsWith("/..")) {
        qWarning("StaticFileController: somebody attempted to access a file outside the docroot directory");
        response.setStatus(403,"forbidden");
        response.write("403 forbidden",true);
    }
    // Check if we have the file in cache
    qint64 now=QDateTime::currentMSecsSinceEpoch();
    CacheEntry* entry=cache.object(path);
    if (entry && (cacheTimeout==0 || entry->created>now-cacheTimeout)) {
        qDebug("StaticFileController: Cache hit for %s",path.data());
        setContentType(path,response);
        response.setHeader("Cache-Control","max-age="+QByteArray::number(maxAge/1000));
        response.write(entry->document);
    }
    else {
        qDebug("StaticFileController: Cache miss for %s",path.data());
        // The file is not in cache.
        // If the filename is a directory, append index.html.
        if (QFileInfo(docroot+path).isDir()) {
            path+="/index.html";
        }

		//TODO(DONE) carga sensible a la localización
		QString stringPath = path;
		QStringList paths = QString(path).split('/');
		QString fileName = paths.last();
		stringPath.remove(fileName);
		fileName = getLocalizedFileName(fileName, request.getHeader("Accept-Language"), stringPath);
		QString newPath = stringPath.append(fileName);
		//END_TODO
        QFile file(docroot+newPath);
        if (file.exists()) {
            qDebug("StaticFileController: Open file %s",qPrintable(file.fileName()));
            if (file.open(QIODevice::ReadOnly)) {
                setContentType(newPath,response);
                response.setHeader("Cache-Control","max-age="+QByteArray::number(maxAge/1000));
                if (file.size()<=maxCachedFileSize) {
                    // Return the file content and store it also in the cache
                    entry=new CacheEntry();
                    while (!file.atEnd() && !file.error()) {
                        QByteArray buffer=file.read(65536);
                        response.write(buffer);
                        entry->document.append(buffer);
                    }
                    entry->created=now;
                    cache.insert(request.getPath(),entry,entry->document.size());
                }
                else {
                    // Return the file content, do not store in cache
                    while (!file.atEnd() && !file.error()) {
                        response.write(file.read(65536));
                    }
                }
                file.close();
            }
            else {
                qWarning("StaticFileController: Cannot open existing file %s for reading",qPrintable(file.fileName()));
                response.setStatus(403,"forbidden");
                response.write("403 forbidden",true);
            }
        }
        else {
            response.setStatus(404,"not found");
            response.write("404 not found",true);
        }
    }
}

void StaticFileController::setContentType(QString fileName, HttpResponse& response) const {
    if (fileName.endsWith(".png")) {
        response.setHeader("Content-Type", "image/png");
    }
    else if (fileName.endsWith(".jpg")) {
        response.setHeader("Content-Type", "image/jpeg");
    }
    else if (fileName.endsWith(".gif")) {
        response.setHeader("Content-Type", "image/gif");
    }
    else if (fileName.endsWith(".txt")) {
        response.setHeader("Content-Type", qPrintable("text/plain; charset="+encoding));
    }
    else if (fileName.endsWith(".html") || fileName.endsWith(".htm")) {
        response.setHeader("Content-Type", qPrintable("text/html; charset=charset="+encoding));
    }
	else if (fileName.endsWith(".js"))
		response.setHeader("Content-Type", qPrintable("text/javascript; charset=charset="+encoding));
    // Todo: add all of your content types
}

bool StaticFileController::exists(QString localizedName, QString path) const
{
    QString fileName=docroot+"/"+path + localizedName;
    QFile file(fileName);
    return file.exists();
}

//retorna fileName si no se encontró alternativa traducida ó fileName-locale.extensión si se encontró
QString StaticFileController::getLocalizedFileName(QString fileName, QString locales, QString path) const
{
	QSet<QString> tried; // used to suppress duplicate attempts
	QStringList locs=locales.split(',',QString::SkipEmptyParts);
	QStringList fileNameParts = fileName.split('.');
	QString file = fileNameParts.first();
	QString extension = fileNameParts.last();
	// Search for exact match
	foreach (QString loc,locs) {
		loc.replace(QRegExp(";.*"),"");
		loc.replace('-','_');
		QString localizedName=file+"-"+loc.trimmed()+"."+extension;
		if (!tried.contains(localizedName)) {
			if(exists(localizedName, path))
				return localizedName;
			tried.insert(localizedName);
		}
	}

	// Search for correct language but any country
	foreach (QString loc,locs) {
		loc.replace(QRegExp("[;_-].*"),"");
		QString localizedName=file+"-"+loc.trimmed()+"."+extension;
		if (!tried.contains(localizedName)) {
			if(exists(localizedName, path))
				return localizedName;
			tried.insert(localizedName);
		}
	}

	return fileName;
}
	 