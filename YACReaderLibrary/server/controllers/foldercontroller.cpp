#include "foldercontroller.h"
#include "controllers/errorcontroller.h"

#include "db_helper.h"  //get libraries
#include "comic_db.h"

#include "folder.h"

#include "template.h"
#include "../static.h"

#include "qnaturalsorting.h"
#include "yacreader_global.h"

#include "QsLog.h"

struct LibraryItemSorter
{
	bool operator()(const LibraryItem * a,const LibraryItem * b) const
	{
		return naturalSortLessThanCI(a->name,b->name);
	} 
};

FolderController::FolderController() {}

void FolderController::service(HttpRequest& request, HttpResponse& response)
{
    QSettings * settings = new QSettings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
    settings->beginGroup("libraryConfig");

    bool showlessInfoPerFolder = settings->value(REMOTE_BROWSE_PERFORMANCE_WORKAROUND,false).toBool();

	HttpSession session=Static::sessionStore->getSession(request,response,false);

    response.setHeader("Content-Type", "text/html; charset=utf-8");
	response.setHeader("Connection","close");

	//QString y = session.get("xxx").toString();
	//response.writeText(QString("session xxx : %1 <br/>").arg(y));

	Template t=Static::templateLoader->getTemplate("folder_"+session.getDeviceType(),request.getHeader("Accept-Language"));
	t.enableWarnings();
    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
	QStringList pathElements = path.split('/');
	int libraryId = pathElements.at(2).toInt();
	QString libraryName = DBHelper::getLibraryName(libraryId);
    qulonglong folderId = pathElements.at(4).toULongLong();

    folderId = qMax<qulonglong>(1,folderId);

    QString folderName = DBHelper::getFolderName(libraryId,folderId);
    if(folderName.isEmpty())
    {
        ErrorController(300).service(request,response);
        return;
    }

    if(folderId!=1)
        t.setVariable("folder.name",folderName);
	else
		t.setVariable("folder.name",libraryName);
    QList<LibraryItem *> folderContent = DBHelper::getFolderSubfoldersFromLibrary(libraryId,folderId);
    QList<LibraryItem *> folderComics = DBHelper::getFolderComicsFromLibrary(libraryId,folderId);

	//response.writeText(libraryName);

	folderContent.append(folderComics);

	qSort(folderContent.begin(),folderContent.end(),LibraryItemSorter());
	folderComics.clear();

    //qulonglong backId = DBHelper::getParentFromComicFolderId(libraryName,folderId);

	int page = 0;
	QByteArray p = request.getParameter("page");
	if(p.length() != 0)
		page = p.toInt();

	// /comicIdi/pagei/comicIdj/pagej/....../comicIdn/pagen
	//QString currentPath = session.get("currentPath").toString();
	//QStringList pathSize = currentPath.split("/").last().toInt;

	bool fromUp = false;

	QMultiMap<QByteArray,QByteArray> map = request.getParameterMap();
	if(map.contains("up"))
		fromUp = true;
	
    //int upPage = 0;

    if(folderId == 1)
    {
        session.clearNavigationPath();
        session.pushNavigationItem(QPair<qulonglong,quint32>(folderId,page));
        t.setVariable(QString("upurl"),"/");
    }
    else
    {
        if(fromUp)
            session.popNavigationItem();
        else //drill down or direct access
        {
            QStack<QPair<qulonglong, quint32> > path = session.getNavigationPath();
            bool found=false;
            for(QStack<QPair<qulonglong, quint32> >::const_iterator itr = path.begin(); itr!=path.end(); itr++)
                if(itr->first == folderId)
                {
                    found = true;
                    break;
                }

            if(found)
            {
                while(session.topNavigationItem().first != folderId)
                    session.popNavigationItem();

                session.updateTopItem(QPair<qulonglong,quint32>(folderId,page));
            }
            else
                session.pushNavigationItem(QPair<qulonglong,quint32>(folderId,page));
        }

        QStack<QPair<qulonglong, quint32> > path = session.getNavigationPath();
        if(path.count()>1)
        {
            QPair<qulonglong, quint32> parentItem = path.at(path.count()-2);
            qulonglong upParent = parentItem.first;
            quint32 upPage = parentItem.second;
            t.setVariable(QString("upurl"),"/library/" + QString::number(libraryId) + "/folder/" +QString("%1?page=%2&up=true").arg(upParent).arg(upPage));
        } else
            t.setVariable(QString("upurl"),"/");
    }

    int elementsPerPage = 24;

	int numFolders = folderContent.length();
	//int numComics = folderComics.length();
	int totalLength = folderContent.length() + folderComics.length();

//	int numFolderPages = numFolders / elementsPerPage + ((numFolders%elementsPerPage)>0?1:0);
	int numPages = totalLength / elementsPerPage + ((totalLength%elementsPerPage)>0?1:0);

	//response.writeText(QString("Number of pages : %1 <br/>").arg(numPages));

	if(page < 0)
		page = 0;
	else if(page >= numPages)
		page = numPages-1;

	int indexCurrentPage = page*elementsPerPage;
	int numFoldersAtCurrentPage = qMax(0,qMin(numFolders - indexCurrentPage, elementsPerPage));

    //PATH
    QStack<QPair<qulonglong,quint32> > foldersPath = session.getNavigationPath();
    t.setVariable(QString("library.name"),libraryName);
    t.setVariable(QString("library.url"),QString("/library/%1/folder/1").arg(libraryId));
    t.loop("path",foldersPath.count()-1);
    for(int i = 1; i < foldersPath.count(); i++){
        t.setVariable(QString("path%1.url").arg(i-1),QString("/library/%1/folder/%2").arg(libraryId).arg(foldersPath[i].first));
        t.setVariable(QString("path%1.name").arg(i-1),DBHelper::getFolderName(libraryId,foldersPath[i].first));
    }

    if(folderContent.length() > 0)
    {
        t.loop("element",numFoldersAtCurrentPage);
        int i = 0;
        while(i<numFoldersAtCurrentPage)
        {
            LibraryItem * item = folderContent.at(i + (page*elementsPerPage));
            t.setVariable(QString("element%1.name").arg(i),folderContent.at(i + (page*elementsPerPage))->name);
            if(item->isDir())
            {
                t.setVariable(QString("element%1.class").arg(i),"folder");

                if(showlessInfoPerFolder)
                {
                    t.setVariable(QString("element%1.image.url").arg(i),"/images/f.png");
                }
                else
                {
                    QList<LibraryItem *> children = DBHelper::getFolderComicsFromLibrary(libraryId, item->id);
                    if(children.length()>0)
                    {
                        const ComicDB * comic = static_cast<ComicDB*>(children.at(0));
                        t.setVariable(QString("element%1.image.url").arg(i),QString("/library/%1/cover/%2.jpg?folderCover=true").arg(libraryId).arg(comic->info.hash));
                    }
                    else
                        t.setVariable(QString("element%1.image.url").arg(i),"/images/f.png");
                }

                t.setVariable(QString("element%1.browse").arg(i),QString("<a class =\"browseButton\" href=\"%1\">BROWSE</a>").arg(QString("/library/%1/folder/%2").arg(libraryId).arg(item->id)));
                t.setVariable(QString("element%1.cover.browse").arg(i),QString("<a href=\"%1\">").arg(QString("/library/%1/folder/%2").arg(libraryId).arg(item->id)));
                t.setVariable(QString("element%1.cover.browse.end").arg(i),"</a>");
                //t.setVariable(QString("element%1.url").arg(i),"/library/"+libraryName+"/folder/"+QString("%1").arg(folderContent.at(i + (page*10))->id));
                //t.setVariable(QString("element%1.downloadurl").arg(i),"/library/"+libraryName+"/folder/"+QString("%1/info").arg(folderContent.at(i + (page*elementsPerPage))->id));

                t.setVariable(QString("element%1.download").arg(i),QString("<a onclick=\"this.innerHTML='IMPORTING';this.className='importedButton';\" class =\"importButton\" href=\"%1\">IMPORT</a>").arg("/library/"+QString::number(libraryId)+"/folder/"+QString("%1/info").arg(folderContent.at(i + (page*elementsPerPage))->id)));
                t.setVariable(QString("element%1.read").arg(i),"");

                t.setVariable(QString("element%1.size").arg(i),"");
                t.setVariable(QString("element%1.pages").arg(i),"");
                t.setVariable(QString("element%1.status").arg(i),"");
            }
            else
            {
                t.setVariable(QString("element%1.class").arg(i),"cover");
                const ComicDB * comic = (ComicDB *)item;
                t.setVariable(QString("element%1.browse").arg(i),"");
                //t.setVariable(QString("element%1.downloadurl").arg(i),"/library/"+libraryName+"/comic/"+QString("%1").arg(comic->id));
                if(!session.isComicOnDevice(comic->info.hash) && !session.isComicDownloaded(comic->info.hash))
                    t.setVariable(QString("element%1.download").arg(i),QString("<a onclick=\"this.innerHTML='IMPORTING';this.className='importedButton';\" class =\"importButton\" href=\"%1\">IMPORT</a>").arg("/library/"+QString::number(libraryId)+"/comic/"+QString("%1").arg(comic->id)));
                else if (session.isComicOnDevice(comic->info.hash))
                    t.setVariable(QString("element%1.download").arg(i),QString("<div class=\"importedButton\">IMPORTED</div>"));
                else
                    t.setVariable(QString("element%1.download").arg(i),QString("<div class=\"importedButton\">IMPORTING</div>"));

                //t.setVariable(QString("element%1.image.url").arg(i),"/images/f.png");

                t.setVariable(QString("element%1.read").arg(i),QString("<a class =\"readButton\" href=\"%1\">READ</a>").arg("/library/"+QString::number(libraryId)+"/comic/"+QString("%1").arg(comic->id)+"/remote"));

                t.setVariable(QString("element%1.image.url").arg(i),QString("/library/%1/cover/%2.jpg").arg(libraryId).arg(comic->info.hash));

                t.setVariable(QString("element%1.size").arg(i),"<span class=\"comicSize\">" + QString::number(comic->info.hash.right(comic->info.hash.length()-40).toInt()/1024.0/1024.0,'f',2)+"Mb</span>");
                if(comic->info.hasBeenOpened)
                    t.setVariable(QString("element%1.pages").arg(i),QString("<span class=\"numPages\">%1/%2 pages</span>").arg(comic->info.currentPage).arg(comic->info.numPages.toInt()));
                else
                    t.setVariable(QString("element%1.pages").arg(i),QString("<span class=\"numPages\">%1 pages</span>").arg(comic->info.numPages.toInt()));

                if(comic->info.read)
                    t.setVariable(QString("element%1.status").arg(i), QString("<div class=\"mark\"><img src=\"/images/readMark.png\" style = \"width: 15px\"/> </div>"));
                else if(comic->info.hasBeenOpened)
                    t.setVariable(QString("element%1.status").arg(i), QString("<div class=\"mark\"><img src=\"/images/readingMark.png\" style = \"width: 15px\"/> </div>"));
                else
                    t.setVariable(QString("element%1.status").arg(i),"");

                t.setVariable(QString("element%1.cover.browse").arg(i),"");
                t.setVariable(QString("element%1.cover.browse.end").arg(i),"");
            }
            i++;
        }
    } else
    {
        t.loop("element",0);
    }

	if(numPages > 1)
	{
		t.setCondition("pageIndex",true);

		QMap<QString,int> indexCount;

		QString firstChar;
		int xyz = 1;
		for(QList<LibraryItem *>::const_iterator itr=folderContent.constBegin();itr!=folderContent.constEnd();itr++)
		{
			firstChar = QString((*itr)->name[0]).toUpper();
			firstChar = firstChar.normalized(QString::NormalizationForm_D).at(0);//TODO _D or _KD??
			bool ok;
			/*int dec = */firstChar.toInt(&ok, 10);
			if(ok)
				firstChar = "#";
			//response.writeText(QString("%1 - %2 <br />").arg((*itr)->name).arg(xyz));
			if(indexCount.contains(firstChar))
				indexCount.insert(firstChar, indexCount.value(firstChar)+1);
			else
				indexCount.insert(firstChar, 1);

			xyz++;
		}

		QList<QString> index = indexCount.keys();
		if(index.length()>1)
		{
			t.setCondition("alphaIndex",true);

			qSort(index.begin(),index.end(),naturalSortLessThanCI);
			t.loop("index",index.length());
			int i=0;
			int count=0;
			int indexPage=0;
			for(QList<QString>::const_iterator itr=index.constBegin();itr!=index.constEnd();itr++)
			{
				//response.writeText(QString("%1 - %2 <br />").arg(*itr).arg(count));
				t.setVariable(QString("index%1.indexname").arg(i), *itr);
                t.setVariable(QString("index%1.url").arg(i),QString("/library/%1/folder/%2?page=%3").arg(libraryId).arg(folderId).arg(indexPage));
				i++;
				count += indexCount.value(*itr);
				indexPage = count/elementsPerPage;
			}
		}
		else
		{
			t.loop("index",0);
			t.setCondition("alphaIndex",false);

		}

		t.loop("page",numPages);
		int z = 0;
		while(z < numPages)
		{

            t.setVariable(QString("page%1.url").arg(z),QString("/library/%1/folder/%2?page=%3").arg(libraryId).arg(folderId).arg(z));
			t.setVariable(QString("page%1.number").arg(z),QString("%1").arg(z+1));
			if(page == z)
				t.setVariable(QString("page%1.current").arg(z),"current");
			else
				t.setVariable(QString("page%1.current").arg(z),"");
			z++;
		}

        t.setVariable("page.first",QString("/library/%1/folder/%2?page=%3").arg(libraryId).arg(folderId).arg(0));
        t.setVariable("page.previous",QString("/library/%1/folder/%2?page=%3").arg(libraryId).arg(folderId).arg((page==0)?page:page-1));
        t.setVariable("page.next",QString("/library/%1/folder/%2?page=%3").arg(libraryId).arg(folderId).arg((page==numPages-1)?page:page+1));
        t.setVariable("page.last",QString("/library/%1/folder/%2?page=%3").arg(libraryId).arg(folderId).arg(numPages-1));
        t.setCondition("index", true);
	}
	else
	{

		t.loop("page",0);
		t.loop("index",0);
        t.setCondition("index", false);
		t.setCondition("pageIndex",false);
		t.setCondition("alphaIndex",false);
	}

	t.setVariable("page",QString("%1").arg(page+1));
	t.setVariable("pages",QString("%1").arg(numPages));

    response.writeText(t, true);

}
