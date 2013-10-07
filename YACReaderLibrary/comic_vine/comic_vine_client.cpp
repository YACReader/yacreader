#include "comic_vine_client.h"

//this is the API key used by YACReader to access Comic Vine
//please, do not use it in your own software, get one for free at Comic Vine
static const QString CV_API_KEY = "46680bebb358f1de690a5a365e15d325f9649f91";

static const QString CV_WEB_ADDRESS = "http://www.comicvine.com/api";

//gets any volumen containing any comic matching 'query'
static const QString CV_SEARCH = CV_WEB_ADDRESS + "/search/?api_key=" + CV_API_KEY +
								 "&format=json&limit=100&resources=volume"
								 "&field_list=name,start_year,publisher,id,image,count_of_issues"
								 "&query=%1&page=%2";
//http://www.comicvine.com/api/search/?api_key=46680bebb358f1de690a5a365e15d325f9649f91&format=json&limit=100&resources=volume&field_list=name,start_year,publisher,id,image,count_of_issues&query=superman

//gets the detail for a volume %1
static const QString CV_SERIES_DETAIL = CV_WEB_ADDRESS + "/volume/4050-%1/?api_key=" + CV_API_KEY +
										"&format=json&field_list=name,start_year,publisher,image,count_of_issues,id";

//gets ids for comics in a volume id %1
static const QString CV_COMIC_IDS = CV_WEB_ADDRESS + "/issues/?api_key=" + CV_API_KEY +
									"&format=json&field_list=name,issue_number,id,image&filter=volume:%1&page=%1";//offset??

//gets id for comic number %2 in a volume id %1
static const QString CV_COMIC_ID = CV_WEB_ADDRESS + "/issues/?api_key=" + CV_API_KEY +
									"&format=json&field_list=name,issue_number,id,image"
									"&filter=volume:%1,issue_number:%2";
//gets comic detail
static const QString CV_COMIC_DETAIL = CV_WEB_ADDRESS + "/issue/4000-%1/?api_key=" + CV_API_KEY + "&format=json";

//gets comic cover URL
static const QString CV_COVER_URL = CV_WEB_ADDRESS + "/issue/4000-%1/?api_key=" + CV_API_KEY + "&format=json&field_list=image";

//gets comics matching name %1 and number %2
//http://comicvine.com/api/issues/?api_key=46680bebb358f1de690a5a365e15d325f9649f91&limit=20&filter=name:super,issue_number:15

ComicVineClient::ComicVineClient(QObject *parent) :
	QObject(parent)
{

}

//CV_SEARCH
void ComicVineClient::search(const QString & query, int page)
{
	HttpWorker * search = new HttpWorker(CV_SEARCH.arg(query).arg(page));
	connect(search,SIGNAL(dataReady(const QByteArray &)),this,SLOT(proccessVolumesSearchData(const QByteArray &)));
	connect(search,SIGNAL(timeout()),this,SLOT(queryTimeOut()));
	connect(search,SIGNAL(finished()),search,SLOT(deleteLater()));
	search->get();
}
//CV_SEARCH result
void ComicVineClient::proccessVolumesSearchData(const QByteArray & data)
{
	QString json(data);
	emit searchResult(json);
}

void ComicVineClient::queryTimeOut()
{

}

//CV_SERIES_DETAIL
void ComicVineClient::getSeriesDetail(const QString & id)
{

}

//CV_COMIC_IDS
void ComicVineClient::getComicIds(const QString & id, int page)
{

}

//CV_COMIC_ID
void ComicVineClient::getComicId(const QString & id, int comicNumber)
{

}

//CV_COMIC_DETAIL
void ComicVineClient::getComicDetail(const QString & id)
{

}

//CV_COVER_DETAIL
void ComicVineClient::getCoverURL(const QString & id)
{

}
