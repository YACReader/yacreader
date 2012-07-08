#include "properties_dialog.h"
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QSizePolicy>
#include <QFormLayout>
#include <QCheckBox>
#include <QTabWidget>
#include <QIntValidator>

#include "data_base_management.h"
#include "library_creator.h"

PropertiesDialog::PropertiesDialog(QWidget * parent)
:QDialog(parent)
{

	createCoverBox();
    createGeneralInfoBox();
    createAuthorsBox();
	createPublishingBox();
	createButtonBox();
	createPlotBox();

	createTabBar();

	mainLayout = new QGridLayout;
	mainLayout->addWidget(coverBox,0,0);
	mainLayout->addWidget(tabBar,0,1);
	mainLayout->setColumnStretch(1,1);
	/*mainLayout->addWidget(authorsBox,1,1);
	mainLayout->addWidget(publishingBox,2,1);*/
	mainLayout->addWidget(buttonBox,1,1,Qt::AlignBottom);

	this->setLayout(mainLayout);
	mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

	int heightDesktopResolution = QApplication::desktop()->screenGeometry().height();
	int widthDesktopResolution = QApplication::desktop()->screenGeometry().width();
	int sHeight,sWidth;
	sHeight = static_cast<int>(heightDesktopResolution*0.65);
	sWidth = static_cast<int>(sHeight*1.4);
	setCover(QPixmap(":/images/notCover.png"));
	this->resize(sWidth,this->height());
	this->move(QPoint((widthDesktopResolution-sWidth)/2,((heightDesktopResolution-sHeight)-40)/2));
	setModal(true);
	repaint();

}

void PropertiesDialog::createTabBar()
{
	tabBar = new QTabWidget;
	tabBar->addTab(generalInfoBox,tr("General info"));
	tabBar->addTab(authorsBox,tr("Authors"));
	tabBar->addTab(publishingBox,tr("Publishing"));
	tabBar->addTab(plotBox,tr("Plot"));
}

void PropertiesDialog::createCoverBox()
{
	coverBox = new QGroupBox(tr("Cover"));

	sa = new QScrollArea();
	cover = new QLabel();
	cover->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	cover->setScaledContents(false);
	cover->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
	

	/*sa->setWidget(cover);
	sa->setBackgroundRole(QPalette::Dark);
	sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	sa->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	sa->setFrameStyle(QFrame::NoFrame);
	sa->setAlignment(Qt::AlignCenter);*/

	QVBoxLayout * coverLayout = new QVBoxLayout();
	coverLayout->addWidget(cover);
	
	QHBoxLayout * coverPageLayout = new QHBoxLayout;
	coverPageLayout->addWidget(new QLabel(tr("Cover page : ")));
	coverPageLayout->addWidget(coverPageEdit = new YACReaderFieldEdit());
	coverPageLayout->setStretch(1,0);

	coverLayout->addLayout(coverPageLayout);

	coverBox->setLayout(coverLayout);
}

QFrame * createLine()
{
	QFrame * line = new QFrame();
    line->setObjectName(QString::fromUtf8("line"));
    //line->setGeometry(QRect(320, 150, 118, 3));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

	return line;
}

void PropertiesDialog::createGeneralInfoBox()
{
	generalInfoBox = new QWidget;

	QFormLayout *generalInfoLayout = new QFormLayout;

	//generalInfoLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
	generalInfoLayout->addRow(tr("Title:"), title = new YACReaderFieldEdit());
	

	QHBoxLayout * number = new QHBoxLayout;
	number->addWidget(numberEdit = new YACReaderFieldEdit());
	numberValidator.setBottom(0);
	numberEdit->setValidator(&numberValidator);
	number->addWidget(new QLabel("Bis:"));
	number->addWidget(isBisCheck = new QCheckBox());
	number->addWidget(new QLabel("of:"));
	number->addWidget(countEdit = new YACReaderFieldEdit());
	countValidator.setBottom(0);
	countEdit->setValidator(&countValidator);
	number->addStretch(1);
	/*generalInfoLayout->addRow(tr("&Issue number:"), );
	generalInfoLayout->addRow(tr("&Bis:"), );*/
	generalInfoLayout->addRow(tr("Issue number:"), number);
	
	generalInfoLayout->addRow(tr("Volume:"), volumeEdit = new YACReaderFieldEdit());

	QHBoxLayout * arc = new QHBoxLayout;
	arc->addWidget(storyArcEdit = new YACReaderFieldEdit());
	arc->addWidget(new QLabel("Arc number:"));
	arc->addWidget(arcNumberEdit = new YACReaderFieldEdit());
	arcNumberValidator.setBottom(0);
	arcNumberEdit->setValidator(&arcNumberValidator);
	arc->addWidget(new QLabel("of:"));
	arc->addWidget(arcCountEdit = new YACReaderFieldEdit());
	arcCountValidator.setBottom(0);
	arcCountEdit->setValidator(&arcCountValidator);
	arc->addStretch(1);
	generalInfoLayout->addRow(tr("Story arc:"), arc);
	
	generalInfoLayout->addRow(tr("Genere:"),  genereEdit = new YACReaderFieldEdit());
	
	generalInfoLayout->addRow(tr("Size:"), size = new QLabel("size"));

	generalInfoBox->setLayout(generalInfoLayout);
}

void PropertiesDialog::createAuthorsBox()
{
	authorsBox = new QWidget;
	
	QVBoxLayout *authorsLayout = new QVBoxLayout;

	//authorsLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
	QHBoxLayout * h1 = new QHBoxLayout;
	QVBoxLayout * vl1 = new QVBoxLayout;
	QVBoxLayout * vr1 = new QVBoxLayout;
	vl1->addWidget(new QLabel(tr("Writer(s):")));
	vl1->addWidget(writer = new YACReaderFieldPlainTextEdit());
	h1->addLayout(vl1);
	vr1->addWidget(new QLabel(tr("Penciller(s):")));
	vr1->addWidget(penciller = new YACReaderFieldPlainTextEdit());
	h1->addLayout(vr1);
	//authorsLayout->addRow(tr("Writer(s):"), new YACReaderFieldPlainTextEdit());
	//authorsLayout->addRow(tr("Penciller(s):"), new YACReaderFieldPlainTextEdit());
	QHBoxLayout * h2 = new QHBoxLayout;
	QVBoxLayout * vl2 = new QVBoxLayout;
	QVBoxLayout * vr2 = new QVBoxLayout;
	vl2->addWidget(new QLabel(tr("Inker(s):")));
	vl2->addWidget(inker = new YACReaderFieldPlainTextEdit());
	h2->addLayout(vl2);
	vr2->addWidget(new QLabel(tr("Colorist(s):")));
	vr2->addWidget(colorist = new YACReaderFieldPlainTextEdit());
	h2->addLayout(vr2);
	
	//authorsLayout->addRow(tr("Inker(s):"), new YACReaderFieldPlainTextEdit());
	//authorsLayout->addRow(tr("Colorist(s):"), new YACReaderFieldPlainTextEdit());

	QHBoxLayout * h3 = new QHBoxLayout;
	QVBoxLayout * vl3 = new QVBoxLayout;
	QVBoxLayout * vr3 = new QVBoxLayout;
	vl3->addWidget(new QLabel(tr("Letterer(es):")));
	vl3->addWidget(letterer = new YACReaderFieldPlainTextEdit());
	h3->addLayout(vl3);
	vr3->addWidget(new QLabel(tr("Cover Artist(s):")));
	vr3->addWidget(coverArtist = new YACReaderFieldPlainTextEdit());
	h3->addLayout(vr3);
	//authorsLayout->addRow(tr("Letterer(es):"), new YACReaderFieldPlainTextEdit());
	//authorsLayout->addRow(tr("Cover Artist(s):"), new YACReaderFieldPlainTextEdit());

	authorsLayout->addLayout(h1);
	authorsLayout->addLayout(h2);
	authorsLayout->addLayout(h3);
	authorsLayout->addStretch(1);
	authorsBox->setLayout(authorsLayout);

}

void PropertiesDialog::createPublishingBox()
{
	publishingBox = new QWidget;
	
	QFormLayout *publishingLayout = new QFormLayout;

	QHBoxLayout * date = new QHBoxLayout;
	date->addWidget(new QLabel(tr("Day:")));
	date->addWidget(dayEdit = new YACReaderFieldEdit());
	dayValidator.setRange(1,31);
	dayEdit->setValidator(&dayValidator);
	date->addWidget(new QLabel(tr("Month:")));
	date->addWidget(monthEdit = new YACReaderFieldEdit());
	monthValidator.setRange(1,12);
	monthEdit->setValidator(&monthValidator);
	date->addWidget(new QLabel(tr("Year:")));
	date->addWidget(yearEdit = new YACReaderFieldEdit());
	yearValidator.setRange(1,9999);
	yearEdit->setValidator(&yearValidator);
	date->addStretch(1);

	publishingLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
	publishingLayout->addRow(date);
	publishingLayout->addRow(tr("Publisher:"), publisherEdit = new YACReaderFieldEdit());
	publishingLayout->addRow(tr("Format:"), formatEdit = new YACReaderFieldEdit());
	publishingLayout->addRow(tr("Color/BW:"), colorCheck = new QCheckBox());
	publishingLayout->addRow(tr("Age rating:"), ageRatingEdit = new YACReaderFieldEdit());

	publishingBox->setLayout(publishingLayout);
}

void PropertiesDialog::createPlotBox()
{
	plotBox = new QWidget;

	QFormLayout *plotLayout = new QFormLayout;

	plotLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
	plotLayout->addRow(tr("Synopsis:"), synopsis = new YACReaderFieldPlainTextEdit());
	plotLayout->addRow(tr("Characters:"), characters = new YACReaderFieldPlainTextEdit());
	plotLayout->addRow(tr("Notes:"), notes = new YACReaderFieldPlainTextEdit());

	plotBox->setLayout(plotLayout);

}

void PropertiesDialog::createButtonBox()
{
    buttonBox = new QDialogButtonBox;

    closeButton = buttonBox->addButton(QDialogButtonBox::Close);
	saveButton = buttonBox->addButton(QDialogButtonBox::Save);
    //rotateWidgetsButton = buttonBox->addButton(tr("Rotate &Widgets"),QDialogButtonBox::ActionRole);

    //connect(rotateWidgetsButton, SIGNAL(clicked()), this, SLOT(rotateWidgets()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
}



void PropertiesDialog::setComics(QList<Comic> comics)
{
	this->comics = comics;

	Comic comic = comics.at(0);

	if(comic.info.title != NULL)
		title->setText(*comic.info.title);

	if(comic.info.coverPage != NULL)
	{
		coverPageEdit->setText(QString::number(*comic.info.coverPage));
		coverPageValidator.setRange(1,*comic.info.numPages);
		coverPageEdit->setValidator(&coverPageValidator);
	}
	/*if(comic.info.numPages != NULL)
	numPagesEdit->setText(QString::number(*comic.info.numPages));*/


	if(comic.info.number != NULL)
		numberEdit->setText(QString::number(*comic.info.number));
	if(comic.info.isBis != NULL)
		isBisCheck->setChecked(*comic.info.isBis);
	if(comic.info.count != NULL)
		countEdit->setText(QString::number(*comic.info.count));

	if(comic.info.volume != NULL)
		volumeEdit->setText(*comic.info.volume);
	if(comic.info.storyArc != NULL)
		storyArcEdit->setText(*comic.info.storyArc);
	if(comic.info.arcNumber != NULL)
		arcNumberEdit->setText(QString::number(*comic.info.arcNumber));
	if(comic.info.arcCount != NULL)
		arcCountEdit->setText(QString::number(*comic.info.arcCount));

	if(comic.info.genere != NULL)
		genereEdit->setText(*comic.info.genere);

	if(comic.info.writer != NULL)
		writer->setPlainText(*comic.info.writer);
	if(comic.info.penciller != NULL)
		penciller->setPlainText(*comic.info.penciller);
	if(comic.info.inker != NULL)
		inker->setPlainText(*comic.info.inker);
	if(comic.info.colorist != NULL)
		colorist->setPlainText(*comic.info.colorist);
	if(comic.info.letterer != NULL)
		letterer->setPlainText(*comic.info.letterer);
	if(comic.info.coverArtist != NULL)
		coverArtist->setPlainText(*comic.info.coverArtist);

	size->setText(QString::number(comic.info.hash.right(comic.info.hash.length()-40).toInt()/1024.0/1024.0,'f',2)+"Mb");

	if(comic.info.date != NULL)
	{
		QStringList date = (*comic.info.date).split("/");
		dayEdit->setText(date[0]);
		monthEdit->setText(date[1]);
		yearEdit->setText(date[2]);
	}
	if(comic.info.publisher != NULL)
		publisherEdit->setText(*comic.info.publisher);
	if(comic.info.format != NULL)
		formatEdit->setText(*comic.info.format);
	if(comic.info.color != NULL)
		colorCheck->setChecked(*comic.info.color);
	else
		colorCheck->setCheckState(Qt::PartiallyChecked);

	if(comic.info.ageRating != NULL)
		ageRatingEdit->setText(*comic.info.ageRating);

	if(comic.info.synopsis != NULL)
		synopsis->setPlainText(*comic.info.synopsis);
	if(comic.info.characters != NULL)
		characters->setPlainText(*comic.info.characters);
	if(comic.info.notes != NULL)
		notes->setPlainText(*comic.info.notes);


	if(comics.length() > 1)
	{
		setDisableUniqueValues(true);
		this->setWindowTitle(tr("Edit selected comics information"));
		setCover(QPixmap(":/images/editComic.png"));

		QList<Comic>::iterator itr;
		for(itr = ++comics.begin();itr!=comics.end();itr++)
		{
			if(itr->info.title == NULL || *(itr->info.title) != title->text())
				title->clear();
			
			if(itr->info.count == NULL || *(itr->info.count) != countEdit->text().toInt()) //TODO esto está mal
				countEdit->clear();

			if(itr->info.volume == NULL || *(itr->info.volume) != volumeEdit->text())
				volumeEdit->clear();
			if(itr->info.storyArc == NULL || *(itr->info.storyArc) != storyArcEdit->text())
				storyArcEdit->clear();
			if(itr->info.arcCount == NULL || *(itr->info.arcCount) != storyArcEdit->text().toInt())
				arcCountEdit->clear();

			if(itr->info.genere == NULL || *(itr->info.genere) != genereEdit->text())
				genereEdit->clear();

			if(itr->info.writer == NULL || *(itr->info.writer) != writer->toPlainText())
				writer->clear();
			if(itr->info.penciller == NULL || *(itr->info.penciller) != penciller->toPlainText())
				penciller->clear();
			if(itr->info.inker == NULL || *(itr->info.inker) != inker->toPlainText())
				inker->clear();
			if(itr->info.colorist == NULL || *(itr->info.colorist) != colorist->toPlainText())
				colorist->clear();
			if(itr->info.letterer == NULL || *(itr->info.letterer) != letterer->toPlainText())
				letterer->clear();
			if(itr->info.coverArtist == NULL || *(itr->info.coverArtist) != coverArtist->toPlainText())
				coverArtist->clear();

			if(itr->info.date == NULL)
			{
				dayEdit->clear();
				monthEdit->clear();
				yearEdit->clear();
			}
			else
			{
				QStringList date = itr->info.date->split("/");
				if(dayEdit->text() != date[0])
					dayEdit->clear();
				if(monthEdit->text() != date[1])
					monthEdit->clear();
				if(yearEdit->text() != date[2])
					yearEdit->clear();
			}

			if(itr->info.publisher == NULL || *(itr->info.publisher) != publisherEdit->text())
				publisherEdit->clear();
			if(itr->info.format == NULL || *(itr->info.format) != formatEdit->text())
				formatEdit->clear();
			if(itr->info.color == NULL || *(itr->info.color) != colorCheck->isChecked())
				colorCheck->setCheckState(Qt::PartiallyChecked);
			if(itr->info.ageRating == NULL || *(itr->info.ageRating) != ageRatingEdit->text())
				ageRatingEdit->clear();

			if(itr->info.synopsis == NULL || *(itr->info.synopsis) != synopsis->toPlainText())
				synopsis->clear();
			if(itr->info.characters == NULL || *(itr->info.characters) != characters->toPlainText())
				characters->clear();
			if(itr->info.notes == NULL || *(itr->info.notes) != notes->toPlainText())
				notes->clear();
		}
	}
	else
	{
		this->setWindowTitle(tr("Edit comic information"));
		setCover(comic.info.getCover(basePath));
	}

}



void PropertiesDialog::updateComics()
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
	db.open();
	db.transaction();
	QList<Comic>::iterator itr;
	for(itr = comics.begin();itr!=comics.end();itr++)
	{
		if(itr->info.edited)
			itr->info.update(db);
	}
	db.commit();
	db.close();
	QSqlDatabase::removeDatabase(databasePath);
}
//Deprecated
void PropertiesDialog::setCover(const QPixmap & coverImage)
{
	cover->setPixmap(coverImage.scaledToWidth(125,Qt::SmoothTransformation));
	//cover->repaint();

	//float aspectRatio = (float)coverImage.width()/coverImage.height();
	//int heightDesktopResolution = QApplication::desktop()->screenGeometry().height();
	//int widthDesktopResolution = QApplication::desktop()->screenGeometry().width();
	//int sHeight,sWidth;
	//sHeight = static_cast<int>(heightDesktopResolution*0.65);

	//if(aspectRatio<1)
	//{
	//	sWidth = static_cast<int>(sHeight*1.4);
	//	//this->resize(sWidth,sHeight);
	//	this->move(QPoint((widthDesktopResolution-sWidth)/2,((heightDesktopResolution-sHeight)-40)/2));
	//	//sa->resize(sa->width(),sa->width()*1.333);
	//	/*cover->resize(static_cast<int>((sa->height())*aspectRatio),
	//		(sa->height()));*/

	//}
	//else
	//{
	//	sWidth = static_cast<int>(sHeight/1.16);
	//	//this->resize(sWidth,sHeight);
	//	this->move(QPoint((widthDesktopResolution-sWidth)/2,((heightDesktopResolution-sHeight)-40)/2));
	//	cover->resize((width()-25),
	//		static_cast<int>((width()-25)/aspectRatio));
	//}
	
}
void PropertiesDialog::setFilename(const QString & nameString)
{
	title->setText(nameString);
}
void PropertiesDialog::setNumpages(int pagesNum)
{
	numPagesEdit->setText(QString::number(pagesNum));
}
void PropertiesDialog::setSize(float sizeFloat)
{
	 
	size->setText(QString::number(sizeFloat,'f',2) + " MB");
}

void PropertiesDialog::save()
{
	QList<Comic>::iterator itr;
	for(itr = comics.begin();itr!=comics.end();itr++)
	{
		//Comic & comic = comics[0];
		bool edited = false;

		if(title->isModified())
		{
			itr->info.setTitle(title->text());
			edited = true;
		}

		if(comics.size()==1)
		if(coverPageEdit->isModified() && !coverPageEdit->text().isEmpty() && coverPageEdit->text().toInt() != 0)
		{
			itr->info.setCoverPage(coverPageEdit->text().toInt());
			edited = true;
		}

		/*if(comic.info.numPages != NULL)
		numPagesEdit->setText(QString::number(*comic.info.numPages));*/
		if(comics.size()==1)
		if(numberEdit->isModified()  && !numberEdit->text().isEmpty())
		{
			itr->info.setNumber(numberEdit->text().toInt());
			edited = true;
		}
		if(comics.size()==1)
		if(itr->info.isBis != NULL || isBisCheck->isChecked())
		{
			itr->info.setIsBis(isBisCheck->isChecked());
			edited = true;
		}

		if(countEdit->isModified())
		{
			itr->info.setCount(countEdit->text().toInt());
			edited = true;
		}

		if(volumeEdit->isModified())
		{
			itr->info.setVolume(volumeEdit->text());
			edited = true;
		}
		if(storyArcEdit->isModified())
		{
			itr->info.setStoryArc(storyArcEdit->text());
			edited = true;
		}
		if(comics.size()==1)
		if(arcNumberEdit->isModified() && !arcNumberEdit->text().isEmpty())
		{
			itr->info.setArcNumber(arcNumberEdit->text().toInt());
			edited = true;
		}
		if(arcCountEdit->isModified())
		{
			itr->info.setArcCount(arcCountEdit->text().toInt());
			edited = true;
		}

		if(genereEdit->isModified())
		{
			itr->info.setGenere(genereEdit->text());
			edited = true;
		}

		if(writer->document()->isModified())
		{
			itr->info.setWriter(writer->toPlainText());
			edited = true;
		}
		if(penciller->document()->isModified())
		{
			itr->info.setPenciller(penciller->toPlainText());
			edited = true;
		}
		if(inker->document()->isModified())
		{
			itr->info.setInker(inker->toPlainText());
			edited = true;
		}
		if(colorist->document()->isModified())
		{
			itr->info.setColorist(colorist->toPlainText());
			edited = true;
		}
		if(letterer->document()->isModified())
		{
			itr->info.setLetterer(letterer->toPlainText());
			edited = true;
		}
		if(coverArtist->document()->isModified())
		{
			itr->info.setCoverArtist(coverArtist->toPlainText());
			edited = true;
		}

		if(dayEdit->isModified() || monthEdit->isModified()  || yearEdit->isModified() )
		{
			itr->info.setDate(dayEdit->text()+"/"+monthEdit->text()+"/"+yearEdit->text());
			edited = true;
		}
		if(publisherEdit->isModified())
		{
			itr->info.setPublisher(publisherEdit->text());
			edited = true;
		}
		if(formatEdit->isModified())
		{
			itr->info.setFormat(formatEdit->text());
			edited = true;
		}
		if(colorCheck->checkState() != Qt::PartiallyChecked)
		{
			itr->info.setColor(colorCheck->isChecked());
			edited = true;
		}
		if(ageRatingEdit->isModified())
		{
			itr->info.setAgeRating(ageRatingEdit->text());
			edited = true;
		}

		if(synopsis->document()->isModified())
		{
			itr->info.setSynopsis(synopsis->toPlainText());
			edited = true;
		}
		if(characters->document()->isModified())
		{
			itr->info.setCharacters(characters->toPlainText());
			edited = true;
		}
		if(notes->document()->isModified())
		{
			itr->info.setNotes(notes->toPlainText());
			edited = true;
		}

		itr->info.edited = edited;
	}
	updateComics();
	if(comics.count() == 1)
	{
		if(coverPageEdit->isModified())// && coverPageEdit->text().toInt() != *comics[0].info.coverPage)
		{
			ThumbnailCreator tc(basePath+comics[0].path,basePath+"/.yacreaderlibrary/covers/"+comics[0].info.hash+".jpg",*comics[0].info.coverPage);
			tc.create();
		}
	}
	close();
	emit(accepted());
}

void PropertiesDialog::setDisableUniqueValues(bool disabled)
{
	coverPageEdit->setDisabled(disabled);
	coverPageEdit->clear();
	numberEdit->setDisabled(disabled);
	numberEdit->clear();
	isBisCheck->setDisabled(disabled);
	isBisCheck->setChecked(false);
	arcNumberEdit->setDisabled(disabled);
	arcNumberEdit->clear();
}

void PropertiesDialog::closeEvent ( QCloseEvent * e )
{

	title->clear();
	title->setModified(false);
	coverPageEdit->clear();
	//			numPagesEdit->setText(QString::number(*comic.info.numPages));
	numberEdit->clear();
	isBisCheck->setChecked(false);
	countEdit->clear();
	volumeEdit->clear();
	storyArcEdit->clear();
	arcNumberEdit->clear();
	arcCountEdit->clear();
	genereEdit->clear();
	writer->clear();
	penciller->clear();
	inker->clear();
	colorist->clear();
	letterer->clear();
	coverArtist->clear();
	dayEdit->clear();
	monthEdit->clear();
	yearEdit->clear();
	publisherEdit->clear();
	formatEdit->clear();
	colorCheck->setCheckState(Qt::PartiallyChecked);
	ageRatingEdit->clear();
	synopsis->clear();
	characters->clear();
	notes->clear();
	
	setDisableUniqueValues(false);

	tabBar->setCurrentIndex(0);

	QDialog::closeEvent(e);
}
