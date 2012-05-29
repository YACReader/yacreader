#ifndef __PROPERTIES_DIALOG_H
#define __PROPERTIES_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>

#include "comic.h"

	class PropertiesDialog : public QDialog
	{
		Q_OBJECT
	private: 
		QLabel * _cover;
		QLabel * _name;
		QLabel * _pages;
		QLabel * _size;
		QScrollArea * sa;

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

