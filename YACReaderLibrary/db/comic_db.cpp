#include "comic_db.h"

#include <QVariant>

//-----------------------------------------------------------------------------
//COMIC------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ComicDB::ComicDB()
{

}

bool ComicDB::isDir()
{
	return false;
}

QString ComicDB::toTXT()
{
	QString txt;

	//Legacy info
	txt.append(QString("comicid:%1\r\n").arg(id));
	txt.append(QString("hash:%1\r\n").arg(info.hash));
	txt.append(QString("path:%1\r\n").arg(path));
	txt.append(QString("numpages:%1\r\n").arg(*info.numPages));

	//Información general
	if(info.coverPage != NULL)
		txt.append(QString("coverPage:%1\r\n").arg(*info.coverPage));

	if(info.title != NULL)
		txt.append(QString("title:%1\r\n").arg(*info.title));

	if(info.number != NULL)
		txt.append(QString("number:%1\r\n").arg(*info.number));

	if(info.isBis != NULL)
		txt.append(QString("isBis:%1\r\n").arg(*info.isBis));

	if(info.count != NULL)
		txt.append(QString("count:%1\r\n").arg(*info.count));

	if(info.volume != NULL)
		txt.append(QString("volume:%1\r\n").arg(*info.volume));

	if(info.storyArc != NULL)
		txt.append(QString("storyArc:%1\r\n").arg(*info.storyArc));

	if(info.arcNumber != NULL)
		txt.append(QString("arcNumber:%1\r\n").arg(*info.arcNumber));

	if(info.arcCount != NULL)
		txt.append(QString("arcCount:%1\r\n").arg(*info.arcCount));

	if(info.genere != NULL)
		txt.append(QString("genere:%1\r\n").arg(*info.genere));

	//Autores
	if(info.writer != NULL)
		txt.append(QString("writer:%1\r\n").arg(*info.writer));

	if(info.penciller != NULL)
		txt.append(QString("penciller:%1\r\n").arg(*info.penciller));

	if(info.inker != NULL)
		txt.append(QString("inker:%1\r\n").arg(*info.inker));

	if(info.colorist != NULL)
		txt.append(QString("colorist:%1\r\n").arg(*info.colorist));

	if(info.letterer != NULL)
		txt.append(QString("letterer:%1\r\n").arg(*info.letterer));

	if(info.coverArtist != NULL)
		txt.append(QString("coverArtist:%1\r\n").arg(*info.coverArtist));
	//Publicación
	if(info.date != NULL)
		txt.append(QString("date:%1\r\n").arg(*info.date));
	
	if(info.publisher != NULL)
		txt.append(QString("publisher:%1\r\n").arg(*info.publisher));

	if(info.format != NULL)
		txt.append(QString("format:%1\r\n").arg(*info.format));

	if(info.color != NULL)
		txt.append(QString("color:%1\r\n").arg(*info.color));

	if(info.ageRating != NULL)
		txt.append(QString("ageRating:%1\r\n").arg(*info.ageRating));
	//Argumento
	if(info.synopsis != NULL)
		txt.append(QString("synopsis:%1\r\n").arg(*info.synopsis));

	if(info.characters != NULL)
		txt.append(QString("characters:%1\r\n").arg(*info.characters));

	if(info.notes != NULL)
		txt.append(QString("notes:%1\r\n").arg(*info.notes));

	return txt;
}
//-----------------------------------------------------------------------------
//COMIC_INFO-------------------------------------------------------------------
//-----------------------------------------------------------------------------
ComicInfo::ComicInfo()
	:existOnDb(false),
	title(NULL), 
	coverPage(NULL), 
	numPages(NULL), 
	number(NULL), 
	isBis(NULL), 
	count(NULL),
	volume(NULL),
	storyArc(NULL),
	arcNumber(NULL),
	arcCount(NULL),
	genere(NULL),
	writer(NULL),
	penciller(NULL),
	inker(NULL),
	colorist(NULL),
	letterer(NULL),
	coverArtist(NULL),
	date(NULL),
	publisher(NULL),
	format(NULL),
	color(NULL),
	ageRating(NULL),
	synopsis(NULL),
	characters(NULL),
	notes(NULL)
{

}

ComicInfo::ComicInfo(const ComicInfo & comicInfo)
:	title(NULL), 
	coverPage(NULL), 
	numPages(NULL), 
	number(NULL), 
	isBis(NULL), 
	count(NULL),
	volume(NULL),
	storyArc(NULL),
	arcNumber(NULL),
	arcCount(NULL),
	genere(NULL),
	writer(NULL),
	penciller(NULL),
	inker(NULL),
	colorist(NULL),
	letterer(NULL),
	coverArtist(NULL),
	date(NULL),
	publisher(NULL),
	format(NULL),
	color(NULL),
	ageRating(NULL),
	synopsis(NULL),
	characters(NULL),
	notes(NULL)
{
	operator=(comicInfo);
}

ComicInfo::~ComicInfo()
{
	delete title;
	delete coverPage;
	delete numPages;
	delete number;
	delete isBis;
	delete count;
	delete volume;
	delete storyArc;
	delete arcNumber;
	delete arcCount;
	delete genere;
	delete writer;
	delete penciller;
	delete inker;
	delete colorist;
	delete letterer;
	delete coverArtist;
	delete date;
	delete publisher;
	delete format;
	delete color;
	delete ageRating;
	delete synopsis;
	delete characters;
	delete notes;
}
ComicInfo & ComicInfo::operator=(const ComicInfo & comicInfo)
{
	copyField(title,comicInfo.title);
	copyField(coverPage,comicInfo.coverPage);
	copyField(numPages,comicInfo.numPages);
	copyField(number,comicInfo.number);
	copyField(isBis,comicInfo.isBis);
	copyField(count,comicInfo.count);
	copyField(volume,comicInfo.volume);
	copyField(storyArc,comicInfo.storyArc);
	copyField(arcNumber,comicInfo.arcNumber);
	copyField(arcCount,comicInfo.arcCount);
	copyField(genere,comicInfo.genere);
	copyField(writer,comicInfo.writer);
	copyField(penciller,comicInfo.penciller);
	copyField(inker,comicInfo.inker);
	copyField(colorist,comicInfo.colorist);
	copyField(letterer,comicInfo.letterer);
	copyField(coverArtist,comicInfo.coverArtist);
	copyField(date,comicInfo.date);
	copyField(publisher,comicInfo.publisher);
	copyField(format,comicInfo.format);
	copyField(color,comicInfo.color);
	copyField(ageRating,comicInfo.ageRating);
	copyField(synopsis,comicInfo.synopsis);
	copyField(characters,comicInfo.characters);
	copyField(notes,comicInfo.notes);

	hash = comicInfo.hash;
	id = comicInfo.id;
	existOnDb = comicInfo.existOnDb;
	read = comicInfo.read;
	edited = comicInfo.edited;

	return *this;
}


void ComicInfo::setValue(QString * & field, const QString & value)
{
	if(field == NULL)
		field = new QString;
	*field = value;
}

void ComicInfo::setValue(int * & field, int value)
{
	if(field == NULL)
		field = new int;
	*field = value;
}

void ComicInfo::setValue(bool * & field, bool value)
{
	if(field == NULL)
		field = new bool;
	*field = value;
}

void ComicInfo::copyField(QString * & field, const QString * value)
{
	if(value != NULL)
		field = new QString(*value);
}

void ComicInfo::copyField(int * & field, int * value)
{
	if(value != NULL)
		field = new int(*value);
}

void ComicInfo::copyField(bool * & field, bool * value)
{
	if(value != NULL)
		field = new bool(*value);
}


//set fields
void ComicInfo::setTitle(QString value)
{
	setValue(title,value);
}

void ComicInfo::setCoverPage(int value)
{
	setValue(coverPage,value);
}
void ComicInfo::setNumPages(int value)
{
	setValue(numPages,value);
}

void ComicInfo::setNumber(int value)
{
	setValue(number,value);
}

void ComicInfo::setIsBis(bool value)
{
	setValue(isBis,value);
}

void ComicInfo::setCount(int value)
{
	setValue(count,value);
}

void ComicInfo::setVolume(QString value)
{
	setValue(volume,value);
}

void ComicInfo::setStoryArc(QString value)
{
	setValue(storyArc,value);
}

void ComicInfo::setArcNumber(int value)
{
	setValue(arcNumber,value);
}

void ComicInfo::setArcCount(int value)
{
	setValue(arcCount,value);
}

void ComicInfo::setGenere(QString value)
{
	setValue(genere,value);
}

void ComicInfo::setWriter(QString value)
{
	setValue(writer,value);
}

void ComicInfo::setPenciller(QString value)	
{
	setValue(penciller,value);
}

void ComicInfo::setInker(QString value)
{
	setValue(inker,value);
}

void ComicInfo::setColorist(QString value)
{
	setValue(colorist,value);
}

void ComicInfo::setLetterer(QString value)
{
	setValue(letterer,value);
}

void ComicInfo::setCoverArtist(QString value)
{
	setValue(coverArtist,value);
}

void ComicInfo::setDate(QString value)
{
	setValue(date,value);
}

void ComicInfo::setPublisher(QString value)
{
	setValue(publisher,value);
}

void ComicInfo::setFormat(QString value)
{
	setValue(format,value);
}

void ComicInfo::setColor(bool value)
{
	setValue(color,value);
}

void ComicInfo::setAgeRating(QString value)
{
	setValue(ageRating,value);
}

void ComicInfo::setSynopsis(QString value)
{
	setValue(synopsis,value);
}

void ComicInfo::setCharacters(QString value)
{
	setValue(characters,value);
}

void ComicInfo::setNotes(QString value)
{
	setValue(notes,value);
}

QPixmap ComicInfo::getCover(const QString & basePath)
{
	if(cover.isNull())
	{
		cover.load(basePath + "/.yacreaderlibrary/covers/" + hash + ".jpg");
	}
	QPixmap c;
	c.convertFromImage(cover);
	return c;
}