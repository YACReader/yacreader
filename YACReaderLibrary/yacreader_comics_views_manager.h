#ifndef YACREADERCOMICSVIEWSMANAGER_H
#define YACREADERCOMICSVIEWSMANAGER_H

#include <QtWidgets>

class YACReaderComicsViewsManager : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderComicsViewsManager(QObject *parent = 0);

    QWidget * containerWidget();

protected:
    QStackedWidget * comicsViewStack;

    ComicsView * comicsView;
    ClassicComicsView * classicComicsView;
    GridComicsView * gridComicsView;

    ComicsViewTransition * comicsViewTransition;

    EmptyFolderWidget * emptyFolderWidget;
    EmptyLabelWidget * emptyLabelWidget;
    EmptySpecialListWidget * emptySpecialList;
    EmptyReadingListWidget * emptyReadingList;

    NoSearchResultsWidget * noSearchResultsWidget;

signals:

public slots:
};

#endif // COMICSVIEWSMANAGER_H
