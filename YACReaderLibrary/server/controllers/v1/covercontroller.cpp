#include "covercontroller.h"
#include "db_helper.h"  //get libraries
#include "yacreader_libraries.h"
#include "yacreader_http_session.h"

#include "template.h"
#include "../static.h"

CoverController::CoverController() {}

void CoverController::service(HttpRequest& request, HttpResponse& response)
{
    HttpSession session=Static::sessionStore->getSession(request,response,false);
    YACReaderHttpSession *ySession = Static::yacreaderSessionStore->getYACReaderSessionHttpSession(session.getId());

    response.setHeader("Content-Type", "image/jpeg");
    response.setHeader("Connection","close");
    //response.setHeader("Content-Type", "plain/text; charset=ISO-8859-1");

    YACReaderLibraries libraries = DBHelper::getLibraries();

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    QString libraryName = DBHelper::getLibraryName(pathElements.at(2).toInt());
    QString fileName = pathElements.at(4);

    bool folderCover = request.getParameter("folderCover").length()>0;

    //response.writeText(path+"<br/>");
    //response.writeText(libraryName+"<br/>");
    //response.writeText(libraries.value(libraryName)+"/.yacreaderlibrary/covers/"+fileName+"<br/>");

    //QFile file(libraries.value(libraryName)+"/.yacreaderlibrary/covers/"+fileName);
    //if (file.exists()) {
    //	if (file.open(QIODevice::ReadOnly))
    //	{
    //		qDebug("StaticFileController: Open file %s",qPrintable(file.fileName()));
    //		// Return the file content, do not store in cache
    //		while (!file.atEnd() && !file.error()) {
    //			response.write(file.read(131072));
    //		}
    //	}

    //	file.close();
    //}

    QImage img(libraries.getPath(libraryName)+"/.yacreaderlibrary/covers/"+fileName);
    if (!img.isNull()) {

        int width = 80, height = 120;
        if(ySession->getDisplayType()=="@2x")
        {
            width = 160;
            height = 240;
        }

        if(float(img.width())/img.height() < 0.66666)
            img = img.scaledToWidth(width,Qt::SmoothTransformation);
        else
            img = img.scaledToHeight(height,Qt::SmoothTransformation);

        QImage destImg(width,height,QImage::Format_RGB32);
        destImg.fill(Qt::black);
        QPainter p(&destImg);

        p.drawImage((width-img.width())/2,(height-img.height())/2,img);

        if(folderCover)
        {
             if(ySession->getDisplayType()=="@2x")
                p.drawImage(0,0,QImage(":/images/f_overlayed_retina.png"));
             else
                p.drawImage(0,0,QImage(":/images/f_overlayed.png"));
        }

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        destImg.save(&buffer, "JPG");
        response.write(ba,true);
    }
    //DONE else, hay que devolver un 404
    else
    {
        response.setStatus(404,"not found");
        response.write("404 not found",true);
    }
}

