#ifndef SELECT_COMIC_H
#define SELECT_COMIC_H

#include <QtWidgets>

#include "scraper_results_paginator.h"

class QLabel;
class VolumeComicsModel;
class QModelIndex;

class ScraperScrollLabel;
class ScraperTableView;

class SelectComic : public QWidget
{
    Q_OBJECT
public:
    SelectComic(QWidget *parent = nullptr);
    void load(const QString &json, const QString &volumeId);
    virtual ~SelectComic();

public slots:
    void loadComicInfo(const QModelIndex &mi);
    void setCover(const QByteArray &);
    void setDescription(const QString &jsonDetail);
    QString getSelectedComicId();

signals:
    void loadPage(QString, int);

private slots:
    void loadNextPage();
    void loadPreviousPage();

private:
    QLabel *cover;
    ScraperScrollLabel *detailLabel;
    ScraperTableView *tableComics;
    VolumeComicsModel *model;
    QString currentVolumeId;
    ScraperResultsPaginator *paginator;
};

#endif // SELECT_COMIC_H
