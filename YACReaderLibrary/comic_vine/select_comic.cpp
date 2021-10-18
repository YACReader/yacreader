#include "select_comic.h"

#include "comic_vine_client.h"
#include "scraper_scroll_label.h"
#include "scraper_tableview.h"
#include "volume_comics_model.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QLabel>
#include <QLayout>

SelectComic::SelectComic(QWidget *parent)
    : ScraperSelector(parent), model(0)
{
    QString labelStylesheet = "QLabel {color:white; font-size:12px;font-family:Arial;}";

    QLabel *label = new QLabel(tr("Please, select the right comic info."));
    label->setStyleSheet(labelStylesheet);

    auto l = new QVBoxLayout;
    QWidget *leftWidget = new QWidget;
    auto left = new QVBoxLayout;
    auto content = new QGridLayout;

    // widgets
    cover = new QLabel();
    cover->setScaledContents(true);
    cover->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    cover->setMinimumSize(168, 168 * 5.0 / 3);
    cover->setStyleSheet("QLabel {background-color: #2B2B2B; color:white; font-size:12px; font-family:Arial; }");
    detailLabel = new ScraperScrollLabel(this);

    tableComics = new ScraperTableView(this);
    // connections
    connect(tableComics, &QAbstractItemView::clicked, this, &SelectComic::loadComicInfo);

    paginator->setCustomLabel(tr("comics"));

    left->addWidget(cover);
    left->addWidget(detailLabel, 1);
    leftWidget->setMaximumWidth(180);
    leftWidget->setLayout(left);
    left->setContentsMargins(0, 0, 0, 0);
    leftWidget->setContentsMargins(0, 0, 0, 0);

    content->addWidget(leftWidget, 0, 0);
    content->addWidget(tableComics, 0, 1);
    content->addWidget(paginator, 1, 1);

    content->setColumnStretch(1, 1);
    content->setRowStretch(0, 1);
    ;

    l->addSpacing(15);
    l->addWidget(label);
    l->addSpacing(5);
    l->addLayout(content);

    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    setContentsMargins(0, 0, 0, 0);
}

void SelectComic::load(const QString &json, const QString &searchString)
{
    auto tempM = new VolumeComicsModel();
    tempM->load(json);
    tableComics->setModel(tempM);

    if (model != 0)
        delete model;

    model = tempM;

    if (model->rowCount() > 0) {
        tableComics->selectRow(0);
        loadComicInfo(model->index(0, 0));
    }

    tableComics->resizeColumnToContents(0);

    ScraperSelector::load(json, searchString);
}

SelectComic::~SelectComic() { }

void SelectComic::loadComicInfo(const QModelIndex &mi)
{
    QString coverURL = model->getCoverURL(mi);
    QString id = model->getComicId(mi);

    QString loadingStyle = "<font color='#AAAAAA'>%1</font>";
    cover->setText(loadingStyle.arg(tr("loading cover")));
    detailLabel->setAltText(loadingStyle.arg(tr("loading description")));

    auto comicVineClient = new ComicVineClient;
    connect(comicVineClient, &ComicVineClient::comicCover, this, &SelectComic::setCover);
    connect(comicVineClient, &ComicVineClient::finished, comicVineClient, &QObject::deleteLater);
    comicVineClient->getComicCover(coverURL);

    auto comicVineClient2 = new ComicVineClient;
    connect(comicVineClient2, &ComicVineClient::comicDetail, this, &SelectComic::setDescription);
    connect(comicVineClient2, &ComicVineClient::finished, comicVineClient2, &QObject::deleteLater);
    comicVineClient2->getComicDetailAsync(id);
}

void SelectComic::setCover(const QByteArray &data)
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

void SelectComic::setDescription(const QString &jsonDetail)
{
    QJsonParseError Err;
    QVariantMap sc = QJsonDocument::fromJson(jsonDetail.toUtf8(), &Err).toVariant().toMap();

    if (Err.error != QJsonParseError::NoError) {
        qDebug("Error detected");
        return;
    }

    QVariant descriptionValues = sc.value("results").toMap().value("description");
    bool valid = !descriptionValues.isNull() && descriptionValues.isValid();
    detailLabel->setText(valid ? descriptionValues.toString().replace("<a", "<a style = 'color:#827A68; text-decoration:none;'") : tr("description unavailable"));
}

QString SelectComic::getSelectedComicId()
{
    return model->getComicId(tableComics->currentIndex());
}
