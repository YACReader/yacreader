#include "select_volume.h"

#include <QDesktopServices>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLabel>
#include <QModelIndex>
#include <QScrollArea>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QAction>

#include "scraper_tableview.h"
#include "scraper_lineedit.h"

#include "volumes_model.h"
#include "comic_vine_client.h"
#include "scraper_scroll_label.h"

#include "response_parser.h"
#include "scraper_results_paginator.h"

#include "selected_volume_info.h"

SelectVolume::SelectVolume(QWidget *parent)
    : QWidget(parent), model(0)
{
    proxyModel = new QSortFilterProxyModel;
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    QString labelStylesheet = "QLabel {color:white; font-size:12px;font-family:Arial;}";

    QLabel *label = new QLabel(tr("Please, select the right series for your comic."));
    label->setStyleSheet(labelStylesheet);

    auto l = new QVBoxLayout;
    QWidget *leftWidget = new QWidget;
    auto left = new QVBoxLayout;
    auto content = new QGridLayout;
    auto top = new QHBoxLayout;

    // widgets
    cover = new QLabel();
    cover->setScaledContents(true);
    cover->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    cover->setMinimumSize(168, 168 * 5.0 / 3);
    cover->setStyleSheet("QLabel {background-color: #2B2B2B; color:white; font-size:12px; font-family:Arial; }");
    detailLabel = new ScraperScrollLabel();

    tableVolumes = new ScraperTableView();
    tableVolumes->setSortingEnabled(true);
    tableVolumes->horizontalHeader()->setSectionsClickable(true);

    filterEdit = new ScraperLineEdit(tr("Filter:"));
    filterEdit->setMaximumWidth(200);
    filterEdit->setClearButtonEnabled(true);

    connect(filterEdit, &QLineEdit::textChanged, proxyModel, &QSortFilterProxyModel::setFilterFixedString);
    connect(tableVolumes->horizontalHeader(), qOverload<int, Qt::SortOrder>(&QHeaderView::sortIndicatorChanged), tableVolumes, qOverload<int, Qt::SortOrder>(&QTableView::sortByColumn));
    connect(tableVolumes, &QAbstractItemView::clicked, this, &SelectVolume::loadVolumeInfo);

    paginator = new ScraperResultsPaginator;
    connect(paginator, &ScraperResultsPaginator::loadNextPage, this, &SelectVolume::loadNextPage);
    connect(paginator, &ScraperResultsPaginator::loadPreviousPage, this, &SelectVolume::loadPreviousPage);
    paginator->setCustomLabel(tr("volumes"));

    top->addWidget(label);
    top->addStretch();
    top->addWidget(filterEdit);

    left->addWidget(cover);
    left->addWidget(detailLabel, 1);
    leftWidget->setMaximumWidth(180);
    leftWidget->setLayout(left);
    left->setContentsMargins(0, 0, 0, 0);
    leftWidget->setContentsMargins(0, 0, 0, 0);

    content->addWidget(leftWidget, 0, 0);
    content->addWidget(tableVolumes, 0, 1);
    content->addWidget(paginator, 1, 1);

    content->setColumnStretch(1, 1);
    content->setRowStretch(0, 1);

    l->addSpacing(15);
    l->addLayout(top);
    l->addSpacing(5);
    l->addLayout(content);

    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    setContentsMargins(0, 0, 0, 0);
}

void SelectVolume::load(const QString &json, const VolumeSearchQuery &searchQuery)
{
    auto tempM = new VolumesModel();
    tempM->load(json);
    // tableVolumes->setModel(tempM);

    proxyModel->setSourceModel(tempM);
    tableVolumes->setModel(proxyModel);
    tableVolumes->sortByColumn(0, Qt::AscendingOrder);
    tableVolumes->resizeColumnsToContents();

    if (model != 0)
        delete model;

    model = tempM;

    if (model->rowCount() > 0) {
        tableVolumes->selectRow(0);
        loadVolumeInfo(proxyModel->index(0, 0));
    }

    tableVolumes->setColumnWidth(0, 350);

    currentSearchQuery = searchQuery;
    paginator->update(json);
}

void SelectVolume::loadNextPage()
{
    emit loadPage({ currentSearchQuery.volume, paginator->getCurrentPage() + 1, currentSearchQuery.exactMatch });
}

void SelectVolume::loadPreviousPage()
{
    emit loadPage({ currentSearchQuery.volume, paginator->getCurrentPage() - 1, currentSearchQuery.exactMatch });
}

void SelectVolume::clearFilter()
{
    filterEdit->clear();
}

SelectVolume::~SelectVolume() { }

void SelectVolume::loadVolumeInfo(const QModelIndex &omi)
{
    QString msgStyle = "<font color='#AAAAAA'>%1</font>";

    QModelIndex mi = proxyModel->mapToSource(omi);
    if (!mi.isValid()) {
        cover->clear();
        detailLabel->setAltText(msgStyle.arg(tr("Nothing found, clear the filter if any.")));
        return;
    }
    QString coverURL = model->getCoverURL(mi);
    QString id = model->getVolumeId(mi);

    cover->setText(msgStyle.arg(tr("loading cover")));
    detailLabel->setAltText(msgStyle.arg(tr("loading description")));

    auto comicVineClient = new ComicVineClient;
    connect(comicVineClient, &ComicVineClient::seriesCover, this, &SelectVolume::setCover);
    connect(comicVineClient, &ComicVineClient::finished, comicVineClient, &QObject::deleteLater);
    comicVineClient->getSeriesCover(coverURL);

    auto comicVineClient2 = new ComicVineClient;
    connect(comicVineClient2, &ComicVineClient::seriesDetail, this, &SelectVolume::setDescription);
    connect(comicVineClient2, &ComicVineClient::finished, comicVineClient2, &QObject::deleteLater);
    comicVineClient2->getSeriesDetail(id);
}

void SelectVolume::setCover(const QByteArray &data)
{
    QPixmap p;
    p.loadFromData(data);
    int w = p.width();
    int h = p.height();

    cover->setPixmap(p);
    float aspectRatio = static_cast<float>(w) / h;

    cover->setFixedSize(180, static_cast<int>(180 / aspectRatio));

    cover->update();
}

void SelectVolume::setDescription(const QString &jsonDetail)
{
    QJsonParseError Err;
    QVariantMap sc = QJsonDocument::fromJson(jsonDetail.toUtf8(), &Err).toVariant().toMap();

    if (Err.error != QJsonParseError::NoError) {
        qDebug("Error detected");
        return;
    }

    auto resultMap = sc.value("results").toMap();
    QVariant descriptionValues = resultMap.value("description");
    auto description = descriptionValues.toString().trimmed();
    QVariant deckValues = resultMap.value("deck");
    auto deck = deckValues.toString().trimmed();
    bool valid = !descriptionValues.isNull() && descriptionValues.isValid() && !description.isEmpty();
    bool validDeck = !deckValues.isNull() && deckValues.isValid() && !deck.isEmpty();
    if (valid) {
        selectedVolumeDescription = description;
        detailLabel->setText(description.replace("<a", "<a style = 'color:#827A68; text-decoration:none;'"));
    } else if (validDeck) {
        selectedVolumeDescription = deck;
        detailLabel->setText(deck.replace("<a", "<a style = 'color:#827A68; text-decoration:none;'"));
    } else {
        detailLabel->setText(tr("volume description unavailable"));
    }
}

SelectedVolumeInfo SelectVolume::getSelectedVolumeInfo()
{
    auto volumeId = model->getVolumeId(proxyModel->mapToSource(tableVolumes->currentIndex()));
    auto numIssues = model->getNumIssues(proxyModel->mapToSource(tableVolumes->currentIndex()));
    auto publisher = model->getPublisher(proxyModel->mapToSource(tableVolumes->currentIndex()));

    return { volumeId, numIssues, publisher, selectedVolumeDescription };
}
