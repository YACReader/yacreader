#ifndef NO_SEARCH_RESULTS_WIDGET_H
#define NO_SEARCH_RESULTS_WIDGET_H

#include <QWidget>

class QLabel;

class NoSearchResultsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NoSearchResultsWidget(QWidget *parent = 0);

signals:

public slots:

protected:
    QLabel *iconLabel;
    QLabel *titleLabel;
    void paintEvent(QPaintEvent *);
    QString backgroundColor;
};

#endif // NO_SEARCH_RESULTS_WIDGET_H
