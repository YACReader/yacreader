#ifndef YACREADER_SOCIAL_DIALOG_H
#define YACREADER_SOCIAL_DIALOG_H

#include <QWidget>

class QPixmap;
class QTextEdit;
class ComicDB;

class YACReaderSocialDialog : public QWidget
{
    Q_OBJECT
public:
    explicit YACReaderSocialDialog(QWidget *parent = 0);
    QSize sizeHint() const;
signals:
    
public slots:
	void setComic(ComicDB & comic,QString & basePath);
protected:
	void paintEvent(QPaintEvent *);

private:
	QPixmap cover;
	QTextEdit * plainText;
};

#endif // YACREADER_SOCIAL_DIALOG_H
