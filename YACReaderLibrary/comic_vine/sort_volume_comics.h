#ifndef SORT_VOLUME_COMICS_H
#define SORT_VOLUME_COMICS_H

#include <QModelIndex>
#include <QPushButton>
#include <QPainter>

#include "comic_db.h"
#include "scraper_results_paginator.h"
#include "themable.h"

class ScraperTableView;
class LocalComicListModel;
class VolumeComicsModel;
class QLabel;

class ScrapperToolButton : public QPushButton, protected Themable
{
    Q_OBJECT
public:
    enum Appearance {
        DEFAULT,
        LEFT,
        RIGHT
    };

    ScrapperToolButton(ScrapperToolButton::Appearance appearance = DEFAULT, QWidget *parent = nullptr);
    static QWidget *getSeparator();
    void setAppearance(ScrapperToolButton::Appearance appearance) { this->appearance = appearance; }
    virtual ~ScrapperToolButton() { }

protected:
    void paintEvent(QPaintEvent *e) override;
    void applyTheme(const Theme &theme) override;

private:
    Appearance appearance;
    QColor fillColor;
};

class SortVolumeComics : public QWidget, protected Themable
{
    Q_OBJECT
public:
    explicit SortVolumeComics(QWidget *parent = nullptr);

public slots:
    void setData(QList<ComicDB> &comics, const QString &json, const QString &vID);
    QList<QPair<ComicDB, QString>> getMatchingInfo();

protected slots:
    void synchronizeScroll(int pos);
    void moveUpCL();
    void moveDownCL();
    void moveUpIL();
    void moveDownIL();

    void removeSelectedComics();
    void restoreAllComics();
    void showRemovedComicsSelector();

signals:
    void loadPage(QString, int);

private slots:
    void loadNextPage();
    void loadPreviousPage();

private:
    QLabel *label;
    QLabel *sortLabel;
    ScraperTableView *tableFiles;
    ScraperTableView *tableVolumeComics;

    LocalComicListModel *localComicsModel;
    VolumeComicsModel *volumeComicsModel;

    ScrapperToolButton *moveUpButtonCL;
    ScrapperToolButton *moveDownButtonCL;
    ScrapperToolButton *moveUpButtonIL;
    ScrapperToolButton *moveDownButtonIL;

    QString currentVolumeId;
    ScraperResultsPaginator *paginator;

protected:
    void applyTheme(const Theme &theme) override;
};

#endif // SORT_VOLUME_COMICS_H
