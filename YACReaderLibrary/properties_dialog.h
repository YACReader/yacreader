#ifndef __PROPERTIES_DIALOG_H
#define __PROPERTIES_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QTabWidget>
#include <QCheckBox>
#include <QIntValidator>

#include "comic.h"
#include "custom_widgets.h"

	class PropertiesDialog : public QDialog
	{
		Q_OBJECT
	private: 
		QGridLayout * mainLayout;
		
		QTabWidget * tabBar;

		QGroupBox * coverBox;
		QLabel * cover;
		QScrollArea * sa;

		QWidget * generalInfoBox;
		YACReaderFieldEdit * title;
		YACReaderFieldEdit * numPagesEdit;
		QLabel * size;

		YACReaderFieldEdit * coverPageEdit;
		QIntValidator coverPageValidator;

		YACReaderFieldEdit * numberEdit;
		QIntValidator numberValidator;
		QCheckBox * isBisCheck;
		YACReaderFieldEdit * countEdit;
		QIntValidator countValidator;

		YACReaderFieldEdit * volumeEdit;
		YACReaderFieldEdit * storyArcEdit;
		YACReaderFieldEdit * arcNumberEdit;
		QIntValidator arcNumberValidator;
		YACReaderFieldEdit * arcCountEdit;
		QIntValidator arcCountValidator;

		YACReaderFieldEdit * genereEdit;

		YACReaderFieldPlainTextEdit * writer;
		YACReaderFieldPlainTextEdit * penciller;
		YACReaderFieldPlainTextEdit * inker;
		YACReaderFieldPlainTextEdit * colorist;
		YACReaderFieldPlainTextEdit * letterer;
		YACReaderFieldPlainTextEdit * coverArtist;

		YACReaderFieldEdit * dayEdit;
		QIntValidator dayValidator;
		YACReaderFieldEdit * monthEdit;
		QIntValidator monthValidator;
		YACReaderFieldEdit * yearEdit;
		QIntValidator yearValidator;
		YACReaderFieldEdit * publisherEdit;
		YACReaderFieldEdit * formatEdit;
		QCheckBox * colorCheck;
		YACReaderFieldEdit * ageRatingEdit;

		YACReaderFieldPlainTextEdit * synopsis;
		YACReaderFieldPlainTextEdit * characters;
		YACReaderFieldPlainTextEdit * notes;

		QWidget * authorsBox;

		QWidget * publishingBox;

		QWidget * plotBox;

		QDialogButtonBox *buttonBox;
		QPushButton *closeButton;
		QPushButton *saveButton;
		QPushButton *restoreButton; //??

		void createTabBar();
		void createCoverBox();
		void createGeneralInfoBox();
		void createAuthorsBox();
		void createPublishingBox();
		void createPlotBox();

		void createButtonBox();

		void setDisableUniqueValues(bool disabled);

		QList<Comic> comics;
		void closeEvent ( QCloseEvent * e );
	public:
		PropertiesDialog(QWidget * parent = 0);
		QString databasePath;
		QString basePath;

	public slots:
		void setComics(QList<Comic> comics);
		void updateComics();
		void save();
		//Deprecated
		void setCover(const QPixmap & cover);
		void setFilename(const QString & name);
		void setNumpages(int pages);
		void setSize(float size);

	};
#endif

