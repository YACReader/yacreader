#ifndef SCRAPER_SCROLL_LABEL_H
#define SCRAPER_SCROLL_LABEL_H

#include <QScrollArea>

class QLabel;

class ScraperScrollLabel : public QScrollArea
{
	Q_OBJECT
public:
	explicit ScraperScrollLabel(QWidget *parent = 0);

signals:

public slots:
	void setText(const QString & text);
	void setAltText(const QString &text);

	void openLink(const QString &link);
private:
	QLabel * textLabel;
};

#endif // SCRAPER_SCROLL_LABEL_H
