/**
  @file
  @author Stefan Frings
*/

#include "staticfilecontroller.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

//YACReader-----
#include "httpsession.h"
#include "yacreader_http_session.h"
#include "static.h"
//--

StaticFileController::StaticFileController(QSettings* settings, QObject* parent)
    :HttpRequestHandler(parent)
{
    maxAge=settings->value("maxAge","60000").toInt();
    encoding=settings->value("encoding","UTF-8").toString();
    docroot=settings->value("path",".").toString();
    if(!(docroot.startsWith(":/") || docroot.startsWith("qrc://")))
    {
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
    }
    qDebug("StaticFileController: docroot=%s, encoding=%s, maxAge=%i",qPrintable(docroot),qPrintable(encoding),maxAge);
    maxCachedFileSize=settings->value("maxCachedFileSize","65536").toInt();
    cache.setMaxCost(settings->value("cacheSize","1000000").toInt());
    cacheTimeout=settings->value("cacheTime","60000").toInt();
    qDebug("StaticFileController: cache timeout=%i, size=%i",cacheTimeout,cache.maxCost());
}


void StaticFileController::service(HttpRequest& request, HttpResponse& response)
{
    QByteArray path=request.getPath();
    // Check if we have the file in cache
    qint64 now=QDateTime::currentMSecsSinceEpoch();
    mutex.lock();
    CacheEntry* entry=cache.object(path);
    if (entry && (cacheTimeout==0 || entry->created>now-cacheTimeout))
    {
        QByteArray document=entry->document; //copy the cached document, because other threads may destroy the cached entry immediately after mutex unlock.
        QByteArray filename=entry->filename;
        mutex.unlock();
        qDebug("StaticFileController: Cache hit for %s",path.data());
        setContentType(filename,response);
        response.setHeader("Cache-Control","max-age="+QByteArray::number(maxAge/1000));
        response.write(document);
    }
    else
    {
        mutex.unlock();

        //TODO(DONE) carga sensible al dispositivo y a la localización
        QString stringPath = path;
        QStringList paths = QString(path).split('/');
        QString fileName = paths.last();
        stringPath.remove(fileName);
        HttpSession session=Static::sessionStore->getSession(request,response,false);
        YACReaderHttpSession *ySession = Static::yacreaderSessionStore->getYACReaderSessionHttpSession(session.getId());
        QString device = "ipad";
        QString display = "@2x";
        if (ySession != nullptr) {
            device = ySession->getDeviceType();
            display = ySession->getDisplayType();
        }
        
        if(fileName.endsWith(".png"))
            fileName = getDeviceAwareFileName(fileName, device, display, request.getHeader("Accept-Language"), stringPath);
        else
            fileName = getDeviceAwareFileName(fileName, device, request.getHeader("Accept-Language"), stringPath);
        QString newPath = stringPath.append(fileName);
        path = newPath.toLocal8Bit();

        //CAMBIADO
        //response.setHeader("Connection","close");
        //END_TODO

        // The file is not in cache.
        qDebug("StaticFileController: Cache miss for %s",path.data());
        // Forbid access to files outside the docroot directory
        if (path.contains("/.."))
        {
            qWarning("StaticFileController: detected forbidden characters in path %s",path.data());
            response.setStatus(403,"forbidden");
            response.write("403 forbidden",true);
            return;
        }
        // If the filename is a directory, append index.html.
        if (QFileInfo(docroot+path).isDir())
        {
            path+="/index.html";
        }
        // Try to open the file
        QFile file(docroot+path);
        qDebug("StaticFileController: Open file %s",qPrintable(file.fileName()));
        if (file.open(QIODevice::ReadOnly))
        {
            setContentType(path,response);
            response.setHeader("Cache-Control","max-age="+QByteArray::number(maxAge/1000));
            if (file.size()<=maxCachedFileSize)
            {
                // Return the file content and store it also in the cache
                entry=new CacheEntry();
                while (!file.atEnd() && !file.error())
                {
                    QByteArray buffer=file.read(65536);
                    response.write(buffer);
                    entry->document.append(buffer);
                }
                entry->created=now;
                entry->filename=path;
                mutex.lock();
                cache.insert(request.getPath(),entry,entry->document.size());
                mutex.unlock();
            }
            else
            {
                // Return the file content, do not store in cache
                while (!file.atEnd() && !file.error())
                {
                    response.write(file.read(65536));
                }
            }
            file.close();
        }
        else {
            if (file.exists())
            {
                qWarning("StaticFileController: Cannot open existing file %s for reading",qPrintable(file.fileName()));
                response.setStatus(403,"forbidden");
                response.write("403 forbidden",true);
            }
            else
            {
                response.setStatus(404,"not found");
                response.write("404 not found",true);
            }
        }
    }
}

void StaticFileController::setContentType(QString fileName, HttpResponse& response) const
{
    if (fileName.endsWith(".png"))
    {
        response.setHeader("Content-Type", "image/png");
    }
    else if (fileName.endsWith(".jpg"))
    {
        response.setHeader("Content-Type", "image/jpeg");
    }
    else if (fileName.endsWith(".gif"))
    {
        response.setHeader("Content-Type", "image/gif");
    }
    else if (fileName.endsWith(".pdf"))
    {
        response.setHeader("Content-Type", "application/pdf");
    }
    else if (fileName.endsWith(".txt"))
    {
        response.setHeader("Content-Type", qPrintable("text/plain; charset="+encoding));
    }
    else if (fileName.endsWith(".html") || fileName.endsWith(".htm"))
    {
        response.setHeader("Content-Type", qPrintable("text/html; charset="+encoding));
    }
    else if (fileName.endsWith(".css"))
    {
        response.setHeader("Content-Type", "text/css");
    }
    else if (fileName.endsWith(".js"))
    {
        response.setHeader("Content-Type", "text/javascript");
    }
    else if (fileName.endsWith(".svg"))
    {
        response.setHeader("Content-Type", "image/svg+xml");
    }
    else if (fileName.endsWith(".woff"))
    {
        response.setHeader("Content-Type", "font/woff");
    }
    else if (fileName.endsWith(".woff2"))
    {
        response.setHeader("Content-Type", "font/woff2");
    }
    else if (fileName.endsWith(".ttf"))
    {
        response.setHeader("Content-Type", "application/x-font-ttf");
    }
    else if (fileName.endsWith(".eot"))
    {
        response.setHeader("Content-Type", "application/vnd.ms-fontobject");
    }
    else if (fileName.endsWith(".otf"))
    {
        response.setHeader("Content-Type", "application/font-otf");
    }
    // Todo: add all of your content types
    else
    {
        qDebug("StaticFileController: unknown MIME type for filename '%s'", qPrintable(fileName));
    }
}

//YACReader------------------------------------------------------------------------

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

QString StaticFileController::getDeviceAwareFileName(QString fileName, QString device, QString locales, QString path) const
{
    QFileInfo fi(fileName);
    QString baseName = fi.baseName();
    QString extension = fi.completeSuffix();

    QString completeFileName = getLocalizedFileName(baseName+"_"+device+"."+extension,locales,path);

    if(QFile(docroot+"/"+path+completeFileName).exists())
        return completeFileName; //existe un archivo específico para este dispositivo y locales
    else
        return getLocalizedFileName(fileName,locales,path); //no hay archivo específico para el dispositivo, pero puede haberlo para estas locales
}

QString StaticFileController::getDeviceAwareFileName(QString fileName, QString device, QString display, QString /* locales */, QString path) const
{
    QFileInfo fi(fileName);
    QString baseName = fi.baseName();
    QString extension = fi.completeSuffix();

    QString completeFileName = baseName+display+"."+extension;
    if(QFile(docroot+"/"+path+completeFileName).exists())
        return completeFileName;
    else
    {
        completeFileName = baseName+"_"+device+display+"."+extension;
        if((QFile(docroot+"/"+path+completeFileName).exists()))
            return completeFileName;
    }

    return fileName;
}
