#ifndef __BOOKMARKS_DIALOG_H
#define __BOOKMARKS_DIALOG_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QEvent>
#include <QKeyEvent>
#include <QPropertyAnimation>

#include "bookmarks.h"

		class BookmarksDialog : public QDialog
		{
			Q_OBJECT

			protected:
				QList<QLabel *> pages;
				QList<QLabel *> images;

				int lastPage;

				QPushButton * accept;
				QPushButton * cancel;

				QSize coverSize;

				bool eventFilter(QObject *obj, QEvent *event);
				void keyPressEvent(QKeyEvent * event);
				//QPropertyAnimation * animation;

			public:
				BookmarksDialog(QWidget * parent = 0);
			
			public slots:
				void setBookmarks(const Bookmarks & bookmarks);
				//void show();
				//void hide();
			
			signals:
				void goToPage(unsigned int page);
		 };

#endif // BOOKMARKS_DIALOG_H
