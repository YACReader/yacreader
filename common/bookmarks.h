#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <QObject>
#include <QMap>
#include <QImage>
#include <QString>
#include <QMap>
#include <QDateTime>
class BookmarksList
{
public:
	struct Bookmark {
		int lastPage;
		QList<int> bookmarks;
		QDateTime added;
		Bookmark():lastPage(0){};
		friend QDataStream & operator<< ( QDataStream & out, const Bookmark & bm )
		{
					out << bm.lastPage;
					out << bm.bookmarks;
					out << bm.added;
					return out;
				}
		friend QDataStream & operator>> ( QDataStream & in, Bookmark & bm )
		{
					in >> bm.lastPage;
					in >> bm.bookmarks;
					in >> bm.added;
					return in;
				}

	};
		BookmarksList():numMaxBookmarks(400){}
	void load();
	void save();
	void add(const QString & comicID, const Bookmark & b);
	Bookmark get(const QString & comicID);
protected:
	QMap<QString,Bookmark> list;
	void deleteOldest(int num);
private:
	int numMaxBookmarks;

};

class Bookmarks : public QObject
{
	Q_OBJECT

	protected:
	QString comicPath;
	//bookmarks setted by the user
	QMap<int,QImage> bookmarks;
	QList<int> latestBookmarks;
	//last page readed
	int lastPageIndex;
	QImage lastPage;
	BookmarksList list;
	QDateTime added;

	public:
	Bookmarks();
	void setLastPage(int index,const QImage & page);
	void setBookmark(int index,const QImage & page);
	void removeBookmark(int index);
	QList<int> getBookmarkPages() const;
	QImage getBookmarkPixmap(int page) const;
	QImage getLastPagePixmap() const;
	int getLastPage() const;
	bool isBookmark(int page);
	bool imageLoaded(int page);
	void newComic(const QString & path);
	void clear();
	void save();
	bool load(const QList<int> & bookmarkIndexes, int lastPage); 

};

#endif // BOOKMARKS_H
