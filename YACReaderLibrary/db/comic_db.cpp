#include "comic_db.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

//-----------------------------------------------------------------------------
//COMIC------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ComicDB::ComicDB()
{

}

ComicDB::ComicDB(qulonglong cparentId, QString cname, QString cpath, QString chash, QSqlDatabase & database)
{
	parentId = cparentId;
	name = cname;
	path = cpath;

	if(!info.load(chash,database))
	{
		info.hash = chash;
		info.coverPage = new int(1);
		_hasCover = false;
	}
	else
		_hasCover = true;
}

QList<LibraryItem *> ComicDB::getComicsFromParent(qulonglong parentId, QSqlDatabase & db, bool sort)
{
	QList<LibraryItem *> list;

	QSqlQuery selectQuery(db);
	selectQuery.prepare("select c.id,c.parentId,c.fileName,c.path,ci.hash from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.parentId = :parentId");
	selectQuery.bindValue(":parentId", parentId);
	selectQuery.exec();

	ComicDB * currentItem;
	while (selectQuery.next()) 
	{
		QList<QVariant> data;
		QSqlRecord record = selectQuery.record();
		for(int i=0;i<record.count();i++)
			data << record.value(i);

		currentItem = new ComicDB();
		currentItem->id = record.value("id").toULongLong();
		currentItem->parentId = record.value(1).toULongLong();
		currentItem->name = record.value(2).toString();
		currentItem->path = record.value(3).toString();
		currentItem->info.load(record.value(4).toString(),db);
		int lessThan = 0;
		if(list.isEmpty() || !sort)
			list.append(currentItem);
		else
		{
			ComicDB * last = static_cast<ComicDB *>(list.back());
			QString nameLast = last->name; 
			QString nameCurrent = currentItem->name;
			QList<LibraryItem *>::iterator i;
			i = list.end();
			i--;
			while ((0 > (lessThan = nameCurrent.localeAwareCompare(nameLast))) && i != list.begin())  //se usa la misma ordenación que en QDir
			{
				i--;
				nameLast = (*i)->name;
			}
			if(lessThan>0) //si se ha encontrado un elemento menor que current, se inserta justo después
				list.insert(++i,currentItem);
			else
				list.insert(i,currentItem);

		}
	}
	//selectQuery.finish();
	return list;
}

bool ComicDB::load(qulonglong idc, QSqlDatabase & db)
{

	QSqlQuery selectQuery(db); 
	selectQuery.prepare("select c.id,c.parentId,c.fileName,c.path,ci.hash from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.id = :id");
	selectQuery.bindValue(":id", idc);
	selectQuery.exec();
	this->id = idc;
	if(selectQuery.next())
	{
		QSqlRecord record = selectQuery.record();
		//id = record.value("id").toULongLong();
		parentId = record.value("parentId").toULongLong();
		name = record.value("name").toString();
		path = record.value("path").toString();
		info.load(record.value("hash").toString(),db);
		
		return true;
	}
	//selectQuery.finish();
	return false;

}

qulonglong ComicDB::insert(QSqlDatabase & db)
{
	//TODO cambiar por info.insert(db)

	if(!info.existOnDb)
	{
		QSqlQuery comicInfoInsert(db);
		comicInfoInsert.prepare("INSERT INTO comic_info (hash,numPages) "
			"VALUES (:hash,:numPages)");
		comicInfoInsert.bindValue(":hash", info.hash);
		comicInfoInsert.bindValue(":numPages", *info.numPages);
		comicInfoInsert.exec();
		info.id =comicInfoInsert.lastInsertId().toULongLong();
		_hasCover = false;
	}
	else
		_hasCover = true;
	
	QSqlQuery query(db);
	query.prepare("INSERT INTO comic (parentId, comicInfoId, fileName, path) "
                   "VALUES (:parentId,:comicInfoId,:name, :path)");
    query.bindValue(":parentId", parentId);
	query.bindValue(":comicInfoId", info.id);
    query.bindValue(":name", name);
	query.bindValue(":path", path);
	query.exec();
	return query.lastInsertId().toULongLong();
}

void ComicDB::update(QSqlDatabase & db)
{

}

void ComicDB::removeFromDB(QSqlDatabase & db)
{
	QSqlQuery query(db);
	query.prepare("DELETE FROM comic WHERE id = :id");
    query.bindValue(":id", id);
	query.exec();
	//query.finish();
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

void ComicInfo::setField(const QString & name, QString * & field, QSqlRecord & record)
{
	if(!record.value(name).isNull())
	{
		field = new QString();
		*field = record.value(name).toString();
	}
}

void ComicInfo::setField(const QString & name, int * & field, QSqlRecord & record)
{
	if(!record.value(name).isNull())
	{
		field = new int;
		*field = record.value(name).toInt();
	}
}

void ComicInfo::setField(const QString & name, bool * & field, QSqlRecord & record)
{
	if(!record.value(name).isNull())
	{
		field = new bool;
		*field = record.value(name).toBool();
	}
}


void ComicInfo::bindField(const QString & name, QString * field, QSqlQuery & query)
{
	if(field != NULL)
	{
		query.bindValue(name,*field);
	}
}

void ComicInfo::bindField(const QString & name, int * field, QSqlQuery & query)
{
	if(field != NULL)
	{
		query.bindValue(name,*field);
	}
}

void ComicInfo::bindField(const QString & name, bool * field, QSqlQuery & query)
{
	if(field != NULL)
	{
		query.bindValue(name,*field);
	}
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

bool ComicInfo::load(QString hash, QSqlDatabase & db)
{
	QSqlQuery findComicInfo(db);
	findComicInfo.prepare("SELECT * FROM comic_info WHERE hash = :hash");
	findComicInfo.bindValue(":hash", hash);
	findComicInfo.exec();


	if(findComicInfo.next())
	{
	this->hash = hash;
	QSqlRecord record = findComicInfo.record();

	hash = hash;
	id = record.value("id").toULongLong();
	read = record.value("read").toBool();
	edited = record.value("edited").toBool();

	setField("title",title,record);
	setField("numPages",numPages,record);
		
	setField("coverPage",coverPage,record);

	setField("number",number,record);
	setField("isBis",isBis,record);
	setField("count",count,record);

	setField("volume",volume,record);
	setField("storyArc",storyArc,record);
	setField("arcNumber",arcNumber,record);
	setField("arcCount",arcCount,record);

	setField("genere",genere,record);

	setField("writer",writer,record);
	setField("penciller",penciller,record);
	setField("inker",inker,record);
	setField("colorist",colorist,record);
	setField("letterer",letterer,record);
	setField("coverArtist",coverArtist,record);

	setField("date",date,record);
	setField("publisher",publisher,record);
	setField("format",format,record);
	setField("color",color,record);
	setField("ageRating",ageRating,record);

	setField("synopsis",synopsis,record);
	setField("characters",characters,record);
	setField("notes",notes,record);

	return existOnDb = true;
	}

	return existOnDb = false;
}

qulonglong ComicInfo::insert(QSqlDatabase & db)
{
	return 0;
}
void ComicInfo::removeFromDB(QSqlDatabase & db)
{

}
void ComicInfo::update(QSqlDatabase & db)
{
	//db.open();
	QSqlQuery updateComicInfo(db);
	updateComicInfo.prepare("UPDATE comic_info SET "
		"title = :title,"
		
		"coverPage = :coverPage,"
		"numPages = :numPages,"

		"number = :number,"
		"isBis = :isBis,"
		"count = :count,"

		"volume = :volume,"
		"storyArc = :storyArc,"
		"arcNumber = :arcNumber,"
		"arcCount = :arcCount,"

		"genere = :genere,"
		
		"writer = :writer,"
		"penciller = :penciller,"
		"inker = :inker,"
		"colorist = :colorist,"
		"letterer = :letterer,"
		"coverArtist = :coverArtist,"

		"date = :date,"
		"publisher = :publisher,"
		"format = :format,"
		"color = :color,"
		"ageRating = :ageRating,"

		"synopsis = :synopsis,"
		"characters = :characters,"
		"notes = :notes,"
		
		"read = :read,"
		"edited = :edited"
		
		" WHERE id = :id ");
	bindField(":title",title,updateComicInfo);

	bindField(":coverPage",coverPage,updateComicInfo);
	bindField(":numPages",numPages,updateComicInfo);

	bindField(":number",number,updateComicInfo);
	bindField(":isBis",isBis,updateComicInfo);
	bindField(":count",count,updateComicInfo);

	bindField(":volume",volume,updateComicInfo);
	bindField(":storyArc",storyArc,updateComicInfo);
	bindField(":arcNumber",arcNumber,updateComicInfo);
	bindField(":arcCount",arcCount,updateComicInfo);

	bindField(":genere",genere,updateComicInfo);

	bindField(":writer",writer,updateComicInfo);
	bindField(":penciller",penciller,updateComicInfo);
	bindField(":inker",inker,updateComicInfo);
	bindField(":colorist",colorist,updateComicInfo);
	bindField(":letterer",letterer,updateComicInfo);
	bindField(":coverArtist",coverArtist,updateComicInfo);

	bindField(":date",date,updateComicInfo);
	bindField(":publisher",publisher,updateComicInfo);
	bindField(":format",format,updateComicInfo);
	bindField(":color",color,updateComicInfo);
	bindField(":ageRating",ageRating,updateComicInfo);

	bindField(":synopsis",synopsis,updateComicInfo);
	bindField(":characters",characters,updateComicInfo);
	bindField(":notes",notes,updateComicInfo);

	updateComicInfo.bindValue(":read", read?1:0);
	updateComicInfo.bindValue(":id", id);
	updateComicInfo.bindValue(":edited", edited?1:0);
	updateComicInfo.exec();
	//updateComicInfo.finish();
	//db.close();
}

void ComicInfo::updateRead(QSqlDatabase & db)
{
		QSqlQuery findComicInfo(db);
	findComicInfo.prepare("UPDATE comic_info SET "
		"read = :read"
		" WHERE id = :id ");

	findComicInfo.bindValue(":read", read?1:0);
	findComicInfo.bindValue(":id", id);
	findComicInfo.exec();
	//findComicInfo.finish();
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