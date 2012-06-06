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

	public:
		PropertiesDialog(QWidget * parent = 0);
	public slots:
		void setComics(QList<Comic> comics);
		void updateComics(QList<Comic> comics);

		//Deprecated
		void setCover(const QPixmap & cover);
		void setFilename(const QString & name);
		void setNumpages(int pages);
		void setSize(float size);
	};
#endif

