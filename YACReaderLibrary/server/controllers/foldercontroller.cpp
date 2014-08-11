#include "foldercontroller.h"
#include "controllers/errorcontroller.h"

#include "db_helper.h"  //get libraries
#include "comic_db.h"

#include "folder.h"

#include "template.h"
#include "../static.h"

#include "qnaturalsorting.h"

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
	HttpSession session=Static::sessionStore->getSession(request,response,false);

	response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");
	response.setHeader("Connection","close");

	//QString y = session.get("xxx").toString();
	//response.writeText(QString("session xxx : %1 <br/>").arg(y));

	Template t=Static::templateLoader->getTemplate("folder_"+session.getDeviceType(),request.getHeader("Accept-Language"));
	t.enableWarnings();
	QString path = QUrl::fromPercentEncoding(request.getPath()).toLatin1();
	QStringList pathElements = path.split('/');
	int libraryId = pathElements.at(2).toInt();
	QString libraryName = DBHelper::getLibraryName(libraryId);
	qulonglong parentId = pathElements.at(4).toULongLong();

    parentId = qMax<qulonglong>(1,parentId);

	QString folderName = DBHelper::getFolderName(libraryName,parentId);
    if(folderName.isEmpty())
    {
        ErrorController(300).service(request,response);
        return;
    }

	if(parentId!=1)
		t.setVariable("folder.name",folderName);
	else
		t.setVariable("folder.name",libraryName);
	QList<LibraryItem *> folderContent = DBHelper::getFolderContentFromLibrary(libraryName,parentId);
	QList<LibraryItem *> folderComics = DBHelper::getFolderComicsFromLibrary(libraryName,parentId);

	//response.writeText(libraryName);

	folderContent.append(folderComics);

	qSort(folderContent.begin(),folderContent.end(),LibraryItemSorter());
	folderComics.clear();

	qulonglong backId = DBHelper::getParentFromComicFolderId(libraryName,parentId);

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
	
	int upPage = 0;

    if(parentId == 1)
        session.clearFoldersPath();
    else
    {
        if(fromUp)
            session.popFolder();
        else
            if(session.getFoldersPath().contains(parentId))
        {
            while(session.topFolder()!=parentId)
                session.popFolder();
        }
        else
            session.pushFolder(parentId);
    }

	if(backId == 1 && parentId == 1)
	{
		session.popPage();
		session.pushPage(page);
		t.setVariable(QString("upurl"),"/?page=0");
	}
	else
	{
		if(fromUp)
		{
			session.popPage();
            upPage = session.topPage();
			page = upPage;
		}
		else //este nivel puede haberse cargado por primera vez ó puede que estemos navegando horizontalmente
			if(p.length() == 0) // acabamos de entrar
			{
				upPage = session.topPage();
				session.pushPage(page);
			}
			else //navegación horizontal
			{
				session.popPage();
				upPage = session.topPage();
				session.pushPage(page);
			}
		t.setVariable(QString("upurl"),"/library/" + QString::number(libraryId) + "/folder/" +QString("%1?page=%2&up=true").arg(backId).arg(upPage));		
	}

	/*if(currentPath.length()>0)
	{
		if(currentPath.contains(QString("%1").arg(parentId))
		{

		}
		else
		{
			session.set("currentPath",currentPath+QString("/%1/%2").arg(parentId).arg(page);
		}
	}*/


	//t.loop("element",folderContent.length());

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
    QStack<int> foldersPath = session.getFoldersPath();
    t.setVariable(QString("library.name"),libraryName);
    t.setVariable(QString("library.url"),QString("/library/%1/folder/1").arg(libraryId));
    t.loop("path",foldersPath.length());
    for(int i = 0; i < foldersPath.length(); i++){

        t.setVariable(QString("path%1.url").arg(i),QString("/library/%1/folder/%2").arg(libraryId).arg(foldersPath[i]));
        t.setVariable(QString("path%1.name").arg(i),DBHelper::getFolderName(libraryName,foldersPath[i]));
    }

	t.loop("element",numFoldersAtCurrentPage);
	int i = 0;
	while(i<numFoldersAtCurrentPage)
	{
		LibraryItem * item = folderContent.at(i + (page*elementsPerPage));
		t.setVariable(QString("element%1.name").arg(i),folderContent.at(i + (page*elementsPerPage))->name);
		if(item->isDir())
		{
			t.setVariable(QString("element%1.class").arg(i),"folder");

            QList<LibraryItem *> children = DBHelper::getFolderComicsFromLibrary(libraryName, item->id);
            if(children.length()>0)
            {
               const ComicDB * comic = static_cast<ComicDB*>(children.at(0));
               t.setVariable(QString("element%1.image.url").arg(i),QString("/library/%1/cover/%2.jpg?folderCover=true").arg(libraryId).arg(comic->info.hash));
            }
            else
                t.setVariable(QString("element%1.image.url").arg(i),"/images/f.png");

			t.setVariable(QString("element%1.browse").arg(i),QString("<a class =\"browseButton\" href=\"%1\">browse</a>").arg(QString("/library/%1/folder/%2").arg(libraryId).arg(item->id)));

			//t.setVariable(QString("element%1.url").arg(i),"/library/"+libraryName+"/folder/"+QString("%1").arg(folderContent.at(i + (page*10))->id));
			//t.setVariable(QString("element%1.downloadurl").arg(i),"/library/"+libraryName+"/folder/"+QString("%1/info").arg(folderContent.at(i + (page*elementsPerPage))->id));
			
			t.setVariable(QString("element%1.download").arg(i),QString("<a onclick=\"this.innerHTML='importing';this.className='importedButton';\" class =\"importButton\" href=\"%1\">import</a>").arg("/library/"+QString::number(libraryId)+"/folder/"+QString("%1/info").arg(folderContent.at(i + (page*elementsPerPage))->id)));
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
				t.setVariable(QString("element%1.download").arg(i),QString("<a onclick=\"this.innerHTML='importing';this.className='importedButton';\" class =\"importButton\" href=\"%1\">import</a>").arg("/library/"+QString::number(libraryId)+"/comic/"+QString("%1").arg(comic->id)));
			else if (!session.isComicDownloaded(comic->info.hash))
					t.setVariable(QString("element%1.download").arg(i),QString("<div class=\"importedButton\">imported</div>"));
			else
				t.setVariable(QString("element%1.download").arg(i),QString("<div class=\"importedButton\">importing</div>"));
			
			//t.setVariable(QString("element%1.image.url").arg(i),"/images/f.png");

            t.setVariable(QString("element%1.read").arg(i),QString("<a class =\"readButton\" href=\"%1\">read</a>").arg("/library/"+QString::number(libraryId)+"/comic/"+QString("%1").arg(comic->id)+"/remote"));

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


		}
		i++;
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
				t.setVariable(QString("index%1.url").arg(i),QString("/library/%1/folder/%2?page=%3").arg(libraryId).arg(parentId).arg(indexPage));
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

			t.setVariable(QString("page%1.url").arg(z),QString("/library/%1/folder/%2?page=%3").arg(libraryId).arg(parentId).arg(z));
			t.setVariable(QString("page%1.number").arg(z),QString("%1").arg(z+1));
			if(page == z)
				t.setVariable(QString("page%1.current").arg(z),"current");
			else
				t.setVariable(QString("page%1.current").arg(z),"");
			z++;
		}

		t.setVariable("page.first",QString("/library/%1/folder/%2?page=%3").arg(libraryId).arg(parentId).arg(0));
		t.setVariable("page.previous",QString("/library/%1/folder/%2?page=%3").arg(libraryId).arg(parentId).arg((page==0)?page:page-1));
		t.setVariable("page.next",QString("/library/%1/folder/%2?page=%3").arg(libraryId).arg(parentId).arg((page==numPages-1)?page:page+1));
		t.setVariable("page.last",QString("/library/%1/folder/%2?page=%3").arg(libraryId).arg(parentId).arg(numPages-1));
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

	response.write(t.toLatin1(),true);

}
