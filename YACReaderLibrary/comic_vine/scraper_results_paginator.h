#ifndef SCRAPER_RESULTS_PAGINATOR_H
#define SCRAPER_RESULTS_PAGINATOR_H

#include <QWidget>

class QToolButton;
class QLabel;

class ScraperResultsPaginator : public QWidget
{
	Q_OBJECT
public:
	explicit ScraperResultsPaginator(QWidget *parent = 0);
	void update(const QString & json);
	int getCurrentPage();
	void setCustomLabel(const QString & label);
signals:
	void loadNextPage();
	void loadPreviousPage();

public slots:

private:
	QToolButton * nextPage;
	QToolButton * previousPage;
	QLabel * numElements;
	QLabel * numPages;

	int currentPage;

	QString customLabel;
};

#endif // SCRAPER_RESULTS_PAGINATOR_H
