#include "comic_vine_client.h"
#include "yacreader_global_gui.h"

#include "comic_vine_all_volume_comics_retriever.h"

//this is the API key used by YACReader to access Comic Vine
//please, do not use it in your own software, get one for free at Comic Vine
static const QString CV_API_KEY = "%CV_API_KEY%"; //get from settings
static const QString CV_API_KEY_DEFAULT = "46680bebb358f1de690a5a365e15d325f9649f91";

static const QString CV_WEB_ADDRESS = "%CV_WEB_ADDRESS%"; //get from settings

//gets any volumen containing any comic matching 'query'
static const QString CV_SEARCH = CV_WEB_ADDRESS + "/search/?api_key=" + CV_API_KEY +
								 "&format=json&limit=100&resources=volume"
								 "&field_list=name,start_year,publisher,id,image,count_of_issues,deck"
								 "&sort=name:asc"
								 "&query=%1&page=%2";
//http://www.comicvine.com/api/search/?api_key=46680bebb358f1de690a5a365e15d325f9649f91&format=json&limit=100&resources=volume&field_list=name,start_year,publisher,id,image,count_of_issues,deck&query=superman

//gets the detail for a volume %1
static const QString CV_SERIES_DETAIL = CV_WEB_ADDRESS + "/volume/4050-%1/?api_key=" + CV_API_KEY +
										"&format=json&field_list=name,start_year,publisher,image,count_of_issues,id,description";

//gets info for comics in a volume id %1
static const QString CV_COMICS_INFO = CV_WEB_ADDRESS + "/issues/?api_key=" + CV_API_KEY +
                                    "&limit=1000&format=json&field_list=name,issue_number,id,image&filter=volume:%1"
									"&sort=cover_date:asc"  //sorting by cover_date, because comic vine doesn't use natural sorting (issue_number -> 1 10 11 ... 100 2 20 21....)
									"&offset=%2";

//"http://www.comicvine.com/api/issues/?api_key=46680bebb358f1de690a5a365e15d325f9649f91&format=json&field_list=name,issue_number,id,image&filter=volume:%1&page=%2

//gets id for comic number %2 in a volume id %1
static const QString CV_COMIC_ID = CV_WEB_ADDRESS + "/issues/?api_key=" + CV_API_KEY +
									"&format=json&field_list=name,issue_number,id,image"
									"&filter=volume:%1,issue_number:%2";
//gets comic detail
static const QString CV_COMIC_DETAIL = CV_WEB_ADDRESS + "/issue/4000-%1/?api_key=" + CV_API_KEY + "&format=json";
//http://www.comicvine.com/api/issue/4000-%1/?api_key=46680bebb358f1de690a5a365e15d325f9649f91&format=json

//gets comic cover URL
static const QString CV_COVER_URL = CV_WEB_ADDRESS + "/issue/4000-%1/?api_key=" + CV_API_KEY + "&format=json&field_list=image";

//gets comics matching name %1 and number %2
//http://comicvine.com/api/issues/?api_key=46680bebb358f1de690a5a365e15d325f9649f91&limit=20&filter=name:super,issue_number:15

ComicVineClient::ComicVineClient(QObject *parent) :
	QObject(parent)
{
    settings = new QSettings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
    settings->beginGroup("ComicVine");
    baseURL = settings->value(COMIC_VINE_BASE_URL, "https://comicvine.gamespot.com/api").toString();
}

ComicVineClient::~ComicVineClient()
{
    delete settings;
}

//CV_SEARCH
void ComicVineClient::search(const QString & query, int page)
{
    HttpWorker * search = new HttpWorker(QString(CV_SEARCH).replace(CV_WEB_ADDRESS, baseURL).replace(CV_API_KEY,settings->value(COMIC_VINE_API_KEY,CV_API_KEY_DEFAULT).toString()).arg(query).arg(page));
	connect(search,SIGNAL(dataReady(const QByteArray &)),this,SLOT(proccessVolumesSearchData(const QByteArray &)));
	connect(search,SIGNAL(timeout()),this,SIGNAL(timeOut()));
	connect(search,SIGNAL(finished()),search,SLOT(deleteLater()));
	search->get();
}
//CV_SEARCH result
void ComicVineClient::proccessVolumesSearchData(const QByteArray & data)
{
	QString json(data);
	emit searchResult(json);
	emit finished();
}

void ComicVineClient::proccessSeriesDetailData(const QByteArray &data)
{
	QString json(data);
	emit seriesDetail(json);
	emit finished();
}

void ComicVineClient::processVolumeComicsInfo(const QByteArray &data)
{
	QString json(data);
	emit volumeComicsInfo(json);
	emit finished();
}

void ComicVineClient::proccessComicDetailData(const QByteArray &data)
{
	QString json(data);
	emit comicDetail(json);
	emit finished();
}

//CV_SERIES_DETAIL
void ComicVineClient::getSeriesDetail(const QString & id)
{
    HttpWorker * search = new HttpWorker(QString(CV_SERIES_DETAIL).replace(CV_WEB_ADDRESS, baseURL).replace(CV_API_KEY,settings->value(COMIC_VINE_API_KEY,CV_API_KEY_DEFAULT).toString()).arg(id));
	connect(search,SIGNAL(dataReady(const QByteArray &)),this,SLOT(proccessSeriesDetailData(const QByteArray &)));
	connect(search,SIGNAL(timeout()),this,SIGNAL(timeOut()));
	connect(search,SIGNAL(finished()),search,SLOT(deleteLater()));
	search->get();
}

void ComicVineClient::getSeriesCover(const QString & url)
{
	HttpWorker * search = new HttpWorker(url);
	connect(search,SIGNAL(dataReady(const QByteArray &)),this,SIGNAL(seriesCover(const QByteArray &)));
	connect(search,SIGNAL(timeout()),this,SIGNAL(timeOut())); //TODO
	connect(search,SIGNAL(finished()),search,SLOT(deleteLater()));
	search->get();
}

//CV_COMIC_IDS
void ComicVineClient::getVolumeComicsInfo(const QString & idVolume, int page)
{
    HttpWorker * search = new HttpWorker(QString(CV_COMICS_INFO).replace(CV_WEB_ADDRESS, baseURL).replace(CV_API_KEY,settings->value(COMIC_VINE_API_KEY,CV_API_KEY_DEFAULT).toString()).arg(idVolume).arg((page-1)*100)); //page doesn't work for search, using offset instead
	connect(search,SIGNAL(dataReady(const QByteArray &)),this,SLOT(processVolumeComicsInfo(const QByteArray &)));
	connect(search,SIGNAL(timeout()),this,SIGNAL(timeOut())); //TODO
	connect(search,SIGNAL(finished()),search,SLOT(deleteLater()));
    search->get();
}

void ComicVineClient::getAllVolumeComicsInfo(const QString &idVolume)
{
    QString url = QString(CV_COMICS_INFO).replace(CV_WEB_ADDRESS, baseURL).replace(CV_API_KEY,settings->value(COMIC_VINE_API_KEY,CV_API_KEY_DEFAULT).toString()).arg(idVolume);
    ComicVineAllVolumeComicsRetriever * comicsRetriever = new ComicVineAllVolumeComicsRetriever(url);

    connect(comicsRetriever, &ComicVineAllVolumeComicsRetriever::allVolumeComicsInfo, this, &ComicVineClient::volumeComicsInfo);
    connect(comicsRetriever, &ComicVineAllVolumeComicsRetriever::finished, this, &ComicVineClient::finished);
    connect(comicsRetriever, &ComicVineAllVolumeComicsRetriever::finished, this, &ComicVineAllVolumeComicsRetriever::deleteLater);
    connect(comicsRetriever, &ComicVineAllVolumeComicsRetriever::timeOut, this, &ComicVineClient::timeOut);

    comicsRetriever->getAllVolumeComics();
}

//CV_COMIC_ID
void ComicVineClient::getComicId(const QString & id, int comicNumber)
{
    Q_UNUSED(id);
    Q_UNUSED(comicNumber);
}

//CV_COMIC_DETAIL
QByteArray ComicVineClient::getComicDetail(const QString & id, bool & outError, bool & outTimeout)
{
    HttpWorker * search = new HttpWorker(QString(CV_COMIC_DETAIL).replace(CV_WEB_ADDRESS, baseURL).replace(CV_API_KEY,settings->value(COMIC_VINE_API_KEY,CV_API_KEY_DEFAULT).toString()).arg(id));

	//connect(search,SIGNAL(dataReady(const QByteArray &)),this,SLOT(proccessComicDetailData(const QByteArray &)));
	//connect(search,SIGNAL(timeout()),this,SIGNAL(timeOut()));
	//connect(search,SIGNAL(finished()),search,SLOT(deleteLater()));
	search->get();
	search->wait();
	outError = !(search->wasValid());
	outTimeout = search->wasTimeout();
	QByteArray result = search->getResult();
	delete search;

	return result;
}

//CV_COMIC_DETAIL
void ComicVineClient::getComicDetailAsync(const QString & id)
{
    HttpWorker * search = new HttpWorker(QString(CV_COMIC_DETAIL).replace(CV_WEB_ADDRESS, baseURL).replace(CV_API_KEY,settings->value(COMIC_VINE_API_KEY,CV_API_KEY_DEFAULT).toString()).arg(id));

	connect(search,SIGNAL(dataReady(const QByteArray &)),this,SLOT(proccessComicDetailData(const QByteArray &)));
	connect(search,SIGNAL(timeout()),this,SIGNAL(timeOut()));
	connect(search,SIGNAL(finished()),search,SLOT(deleteLater()));
	search->get();
}

void ComicVineClient::getComicCover(const QString &url)
{
	HttpWorker * search = new HttpWorker(url);
	connect(search,SIGNAL(dataReady(const QByteArray &)),this,SIGNAL(comicCover(QByteArray)));
	connect(search,SIGNAL(timeout()),this,SIGNAL(timeOut())); //TODO
	connect(search,SIGNAL(finished()),search,SLOT(deleteLater()));
	search->get();
}

//CV_COVER_DETAIL
void ComicVineClient::getCoverURL(const QString & id)
{
    Q_UNUSED(id);
}
