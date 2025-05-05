#ifndef SELECT_VOLUME_H
#define SELECT_VOLUME_H

#include <QtWidgets>

#include "scraper_results_paginator.h"
#include "selected_volume_info.h"
#include "volume_search_query.h"

class QLabel;
class VolumesModel;
class QModelIndex;
class QToolButton;
class QSortFilterProxyModel;

class ScraperScrollLabel;
class ScraperTableView;
class ScraperLineEdit;

class SelectVolume : public QWidget
{
    Q_OBJECT
public:
    SelectVolume(QWidget *parent = nullptr);
    void load(const QString &json, const VolumeSearchQuery &searchQuery);
    void clearFilter();
    virtual ~SelectVolume();

public slots:
    void loadVolumeInfo(const QModelIndex &mi);
    void setCover(const QByteArray &);
    void setDescription(const QString &jsonDetail);
    SelectedVolumeInfo getSelectedVolumeInfo();

signals:
    void loadPage(VolumeSearchQuery);

private slots:
    void loadNextPage();
    void loadPreviousPage();

private:
    QLabel *cover;
    ScraperScrollLabel *detailLabel;
    ScraperTableView *tableVolumes;
    VolumesModel *model;
    QSortFilterProxyModel *proxyModel;
    ScraperLineEdit *filterEdit;
    QString selectedVolumeDescription;
    VolumeSearchQuery currentSearchQuery;
    ScraperResultsPaginator *paginator;
};

#endif // SELECT_VOLUME_H
