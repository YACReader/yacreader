#ifndef __PROPERTIES_DIALOG_H
#define __PROPERTIES_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QTabWidget>
#include <QCheckBox>
#include <QPlainTextEdit>

#include "comic.h"

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
		QLineEdit * title;
		QLineEdit * pages;
		QLabel * size;

		QLineEdit * coverPageEdit;

		QLineEdit * numberEdit;
		QCheckBox * isBisCheck;
		QLineEdit * countEdit;

		QLineEdit * volumeEdit;
		QLineEdit * storyArcEdit;
		QLineEdit * arcNumberEdit;
		QLineEdit * arcCountEdit;

		QLineEdit * genereEdit;

		QPlainTextEdit * writer;
		QPlainTextEdit * penciller;
		QPlainTextEdit * inker;
		QPlainTextEdit * colorist;
		QPlainTextEdit * letterer;
		QPlainTextEdit * coverArtist;

		QLineEdit * dayEdit;
		QLineEdit * monthEdit;
		QLineEdit * yearEdit;
		QLineEdit * publisherEdit;
		QLineEdit * formatEdit;
		QCheckBox * colorCheck;
		QLineEdit * ageRatingEdit;

		QPlainTextEdit * synopsis;
		QPlainTextEdit * characters;
		QPlainTextEdit * notes;

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

	public:
		PropertiesDialog(QWidget * parent = 0);
		QSqlDatabase database;
	public slots:
		void setComics(QList<Comic> comics);
		void updateComics();

		//Deprecated
		void setCover(const QPixmap & cover);
		void setFilename(const QString & name);
		void setNumpages(int pages);
		void setSize(float size);
		void save();
	};
#endif

