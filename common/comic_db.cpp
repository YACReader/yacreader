#include "comic_db.h"

#include <QVariant>
#include <QFileInfo>

//-----------------------------------------------------------------------------
//COMIC------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ComicDB::ComicDB()
{

}

ComicDB::ComicDB(const ComicDB &comicDB)
{
    operator=(comicDB);
}

bool ComicDB::isDir() const
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
	txt.append(QString("numpages:%1\r\n").arg(info.numPages.toString()));

	//new 7.0
	txt.append(QString("rating:%1\r\n").arg(info.rating));
	txt.append(QString("currentPage:%1\r\n").arg(info.currentPage));
	txt.append(QString("contrast:%1\r\n").arg(info.contrast));

	//Informaci�n general
	if(!info.coverPage.isNull())
		txt.append(QString("coverPage:%1\r\n").arg(info.coverPage.toString()));

	if(!info.title.isNull())
		txt.append(QString("title:%1\r\n").arg(info.title.toString()));

	if(!info.number.isNull())
		txt.append(QString("number:%1\r\n").arg(info.number.toString()));

	if(!info.isBis.isNull())
		txt.append(QString("isBis:%1\r\n").arg(info.isBis.toBool()?"1":"0"));

	if(!info.count.isNull())
		txt.append(QString("count:%1\r\n").arg(info.count.toString()));

	if(!info.volume.isNull())
		txt.append(QString("volume:%1\r\n").arg(info.volume.toString()));

	if(!info.storyArc.isNull())
		txt.append(QString("storyArc:%1\r\n").arg(info.storyArc.toString()));

	if(!info.arcNumber.isNull())
		txt.append(QString("arcNumber:%1\r\n").arg(info.arcNumber.toString()));

	if(!info.arcCount.isNull())
		txt.append(QString("arcCount:%1\r\n").arg(info.arcCount.toString()));

	if(!info.genere.isNull())
		txt.append(QString("genere:%1\r\n").arg(info.genere.toString()));

	//Autores
	if(!info.writer.isNull())
		txt.append(QString("writer:%1\r\n").arg(info.writer.toString()));

	if(!info.penciller.isNull())
		txt.append(QString("penciller:%1\r\n").arg(info.penciller.toString()));

	if(!info.inker.isNull())
		txt.append(QString("inker:%1\r\n").arg(info.inker.toString()));

	if(!info.colorist.isNull())
		txt.append(QString("colorist:%1\r\n").arg(info.colorist.toString()));

	if(!info.letterer.isNull())
		txt.append(QString("letterer:%1\r\n").arg(info.letterer.toString()));

	if(!info.coverArtist.isNull())
		txt.append(QString("coverArtist:%1\r\n").arg(info.coverArtist.toString()));
	//Publicaci�n
	if(!info.date.isNull())
		txt.append(QString("date:%1\r\n").arg(info.date.toString()));

	if(!info.publisher.isNull())
		txt.append(QString("publisher:%1\r\n").arg(info.publisher.toString()));

	if(!info.format.isNull())
		txt.append(QString("format:%1\r\n").arg(info.format.toString()));

	if(!info.color.isNull())
		txt.append(QString("color:%1\r\n").arg(info.color.toString()));

	if(!info.ageRating.isNull())
		txt.append(QString("ageRating:%1\r\n").arg(info.ageRating.toString()));
	//Argumento
	if(!info.synopsis.isNull())
		txt.append(QString("synopsis:%1\r\n").arg(info.synopsis.toString()));

	if(!info.characters.isNull())
		txt.append(QString("characters:%1\r\n").arg(info.characters.toString()));

	if(!info.notes.isNull())
		txt.append(QString("notes:%1\r\n").arg(info.notes.toString()));

	return txt;
}

ComicDB &ComicDB::operator=(const ComicDB &other)
{
    LibraryItem::operator =(other);

    this->_hasCover = other._hasCover;

    this->info = other.info;

    return *this;
}

QString ComicDB::getFileName() const
{
	return QFileInfo(path).fileName();
}

QString ComicDB::getTitleOrFileName() const
{
    if(!info.title.isNull() && !info.title.toString().isEmpty())
        return info.title.toString();
	else
		return QFileInfo(path).fileName();
}

QString ComicDB::getParentFolderName() const
{
	QStringList paths = path.split('/');
	if(paths.length()<2)
		return "";
	else
        return paths[paths.length()-2];
}

qulonglong ComicDB::getFileSize() const
{
    //the size is encoded in the hash after the SHA-1
    return info.hash.right(info.hash.length()-40).toLongLong();
}

QString ComicDB::getTitleIncludingNumber() const
{
    if(!info.number.isNull())
    {
        return "#" + info.number.toString() + " - " + getTitleOrFileName();
    }

    return getTitleOrFileName();
}

//-----------------------------------------------------------------------------
//COMIC_INFO-------------------------------------------------------------------
//-----------------------------------------------------------------------------
ComicInfo::ComicInfo()
	:existOnDb(false),
	rating(0),
	hasBeenOpened(false),
	currentPage(1),
	bookmark1(-1),
	bookmark2(-1),
	bookmark3(-1),
	brightness(-1),
	contrast(-1),
    gamma(-1)
{

}

ComicInfo::ComicInfo(const ComicInfo & comicInfo)
{
	operator=(comicInfo);
}

ComicInfo::~ComicInfo()
{

}
//the default operator= should work
ComicInfo & ComicInfo::operator=(const ComicInfo & comicInfo)
{
	hash = comicInfo.hash;
	id = comicInfo.id;
	existOnDb = comicInfo.existOnDb;
	read = comicInfo.read;
	edited = comicInfo.edited;

	hasBeenOpened = comicInfo.hasBeenOpened;
	rating = comicInfo.rating;
	currentPage = comicInfo.currentPage;
	bookmark1 = comicInfo.bookmark1;
	bookmark2 = comicInfo.bookmark2;
	bookmark3 = comicInfo.bookmark3;
	brightness = comicInfo.brightness;
	contrast = comicInfo.contrast;
	gamma = comicInfo.gamma;

    title = comicInfo.title;
    coverPage = comicInfo.coverPage;
    numPages = comicInfo.numPages;
    number = comicInfo.number;
    isBis = comicInfo.isBis;
    count = comicInfo.count;
    volume = comicInfo.volume;
    storyArc = comicInfo.storyArc;
    arcNumber = comicInfo.arcNumber;
    arcCount = comicInfo.arcCount;
    genere = comicInfo.genere;
    writer = comicInfo.writer;
    penciller = comicInfo.penciller;
    inker = comicInfo.inker;
    colorist = comicInfo.colorist;
    letterer = comicInfo.letterer;
    coverArtist = comicInfo.coverArtist;
    date = comicInfo.date;
    publisher = comicInfo.publisher;
    format = comicInfo.format;
    color = comicInfo.color;
    ageRating = comicInfo.ageRating;
    synopsis = comicInfo.synopsis;
    characters = comicInfo.characters;
    notes = comicInfo.notes;
    comicVineID = comicInfo.comicVineID;

    lastTimeOpened = comicInfo.lastTimeOpened;

    coverSizeRatio = comicInfo.coverSizeRatio;
    originalCoverSize = comicInfo.originalCoverSize;

	return *this;
}

//set fields
/*
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
}*/

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

QStringList ComicInfo::getWriters()
{
    if(writer.toString().length()>0)
    {
        return writer.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getPencillers()
{
    if(penciller.toString().length()>0)
    {
        return penciller.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getInkers()
{
    if(inker.toString().length()>0)
    {
        return inker.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getColorists()
{
    if(colorist.toString().length()>0)
    {
        return colorist.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getLetterers()
{
    if(letterer.toString().length()>0)
    {
        return letterer.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getCoverArtists()
{
    if(coverArtist.toString().length()>0)
    {
        return coverArtist.toString().split("\n");
    }

    return QStringList();
}

QStringList ComicInfo::getCharacters()
{
    if(characters.toString().length()>0)
    {
       return characters.toString().split("\n");
    }

    return QStringList();
}

void ComicInfo::setRead(bool r)
{
    if(r != read)
    {
        read = r;
        emit readChanged();
    }
}

void ComicInfo::setRating(int r)
{
    if(r != rating)
    {
        rating = r;
        emit ratingChanged();
    }
}

void ComicInfo::setFavorite(bool f)
{
    if(f != isFavorite)
    {
        isFavorite = f;
        emit favoriteChanged();
    }
}

QDataStream &operator<<(QDataStream & stream, const ComicDB & comic)
{
	stream << comic.id;
	stream << comic.name;
	stream << comic.parentId;
	stream << comic.path;
	stream << comic._hasCover;
	stream << comic.info;
	return stream;
}

QDataStream &operator>>(QDataStream & stream, ComicDB & comic)
{
	stream >> comic.id;
	stream >> comic.name;
	stream >> comic.parentId;
	stream >> comic.path;
	stream >> comic._hasCover;
	stream >> comic.info;
	return stream;
}

QDataStream &operator<<(QDataStream & stream, const ComicInfo & comicInfo)
{
	stream << comicInfo.id;
	stream << comicInfo.read;
	stream << comicInfo.edited;
	stream << comicInfo.hash;
	stream << comicInfo.existOnDb;

	stream << comicInfo.hasBeenOpened;
	stream << comicInfo.rating;
	stream << comicInfo.currentPage;
	stream << comicInfo.bookmark1;
	stream << comicInfo.bookmark2;
	stream << comicInfo.bookmark3;
	stream << comicInfo.brightness; 
	stream << comicInfo.contrast;
	stream << comicInfo.gamma;

    stream << comicInfo.title;

	stream << comicInfo.coverPage;
	stream << comicInfo.numPages;

	stream << comicInfo.number;
	stream << comicInfo.isBis;
	stream << comicInfo.count;

	stream << comicInfo.volume;
	stream << comicInfo.storyArc;
	stream << comicInfo.arcNumber;
	stream << comicInfo.arcCount;

	stream << comicInfo.genere;

	stream << comicInfo.writer;
	stream << comicInfo.penciller;
	stream << comicInfo.inker;
	stream << comicInfo.colorist;
	stream << comicInfo.letterer;
	stream << comicInfo.coverArtist;

	stream << comicInfo.date;
	stream << comicInfo.publisher;
	stream << comicInfo.format;
	stream << comicInfo.color;
	stream << comicInfo.ageRating;

	stream << comicInfo.synopsis;
	stream << comicInfo.characters;
	stream << comicInfo.notes;

    stream << comicInfo.comicVineID;

    stream << comicInfo.lastTimeOpened;

    stream << comicInfo.coverSizeRatio;
    stream << comicInfo.originalCoverSize;

	return stream;
}

QDataStream &operator>>(QDataStream & stream, ComicInfo & comicInfo)
{
	stream >> comicInfo.id;
	stream >> comicInfo.read;
	stream >> comicInfo.edited;
	stream >> comicInfo.hash;
	stream >> comicInfo.existOnDb;

	stream >> comicInfo.hasBeenOpened;
	stream >> comicInfo.rating;
	stream >> comicInfo.currentPage;
	stream >> comicInfo.bookmark1;
	stream >> comicInfo.bookmark2;
	stream >> comicInfo.bookmark3;
	stream >> comicInfo.brightness; 
	stream >> comicInfo.contrast;
	stream >> comicInfo.gamma;

	stream >> comicInfo.title;

	stream >> comicInfo.coverPage;
	stream >> comicInfo.numPages;

	stream >> comicInfo.number;
	stream >> comicInfo.isBis;
	stream >> comicInfo.count;

	stream >> comicInfo.volume;
	stream >> comicInfo.storyArc;
	stream >> comicInfo.arcNumber;
	stream >> comicInfo.arcCount;

	stream >> comicInfo.genere;

	stream >> comicInfo.writer;
	stream >> comicInfo.penciller;
	stream >> comicInfo.inker;
	stream >> comicInfo.colorist;
	stream >> comicInfo.letterer;
	stream >> comicInfo.coverArtist;

	stream >> comicInfo.date;
	stream >> comicInfo.publisher;
	stream >> comicInfo.format;
	stream >> comicInfo.color;
	stream >> comicInfo.ageRating;

	stream >> comicInfo.synopsis;
	stream >> comicInfo.characters;
	stream >> comicInfo.notes;

    stream >> comicInfo.comicVineID;

    stream >> comicInfo.lastTimeOpened;

    stream >> comicInfo.coverSizeRatio;
    stream >> comicInfo.originalCoverSize;
	
	return stream;
}
