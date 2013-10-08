#ifndef SELECT_COMIC_H
#define SELECT_COMIC_H

#include <QWidget>

class SelectComic : public QWidget
{
	Q_OBJECT
public:
	SelectComic(QWidget * parent = 0);
	void load(const QString & json);
	virtual ~SelectComic();
};

#endif // SELECT_COMIC_H
