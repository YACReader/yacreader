#include "properties_dialog.h"

#include "data_base_management.h"
#include "initial_comic_info_extractor.h"
#include "yacreader_field_edit.h"
#include "yacreader_field_plain_text_edit.h"
#include "db_helper.h"

#include <QHBoxLayout>
#include <QApplication>
#include <QSizePolicy>
#include <QFormLayout>
#include <QCheckBox>
#include <QTabWidget>
#include <QIntValidator>
#include <QFileInfo>
#include <QLabel>
#include <QDialogButtonBox>
#include <QToolButton>
#include <QPushButton>
#include <QMessageBox>

using namespace YACReader;

PropertiesDialog::PropertiesDialog(QWidget *parent)
    : QDialog(parent), updated(false)
{
    createCoverBox();
    createGeneralInfoBox();
    createAuthorsBox();
    createPublishingBox();
    createButtonBox();
    createPlotBox();
    createNotesBox(); // review, notes, tags

    createTabBar();
    auto rootLayout = new QGridLayout;

    cover = new QLabel();

    mainLayout = new QGridLayout;
    mainLayout->addWidget(tabBar, 0, 0);
    mainLayout->addWidget(buttonBox, 1, 0, Qt::AlignBottom);

    mainWidget = new QWidget(this);
    mainWidget->setAutoFillBackground(true);
    mainWidget->setLayout(mainLayout);

    rootLayout->setHorizontalSpacing(0);
    rootLayout->setVerticalSpacing(0);

    rootLayout->addWidget(cover, 0, 0, Qt::AlignTop);
    rootLayout->addWidget(mainWidget, 0, 1);

    rootLayout->setColumnStretch(0, 0);
    rootLayout->setColumnStretch(1, 1);

    QScreen *screen = parent != nullptr ? parent->window()->screen() : nullptr;
    if (screen == nullptr) {
        screen = QApplication::screens().constFirst();
    }

    this->setLayout(rootLayout);

    this->setContentsMargins(0, 0, 0, 0);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    rootLayout->addWidget(coverBox, 0, 0, Qt::AlignBottom);

    setModal(true);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

QSize PropertiesDialog::sizeHint() const
{
    return QSize(800, 444);
}

void PropertiesDialog::createTabBar()
{
    tabBar = new QTabWidget;
    tabBar->addTab(generalInfoBox, tr("General info"));
    tabBar->addTab(plotBox, tr("Plot"));
    tabBar->addTab(authorsBox, tr("Authors"));
    tabBar->addTab(publishingBox, tr("Publishing"));
    tabBar->addTab(notesBox, tr("Notes"));
}

void PropertiesDialog::createCoverBox()
{
    coverBox = new QWidget;

    auto layout = new QHBoxLayout;

    QLabel *label = new QLabel(tr("Cover page"));
    label->setStyleSheet("QLabel {color: white; font-weight:bold; font-size:14px;}");
    layout->addWidget(label, 0, Qt::AlignVCenter);
    layout->addStretch();

    coverPageEdit = new YACReaderFieldEdit();

    showPreviousCoverPageButton = new QToolButton();
    showPreviousCoverPageButton->setIcon(QIcon(":/images/previousCoverPage.png"));
    showPreviousCoverPageButton->setStyleSheet("QToolButton {border:none;}");
    showNextCoverPageButton = new QToolButton();
    showNextCoverPageButton->setIcon(QIcon(":/images/nextCoverPage.png"));
    showNextCoverPageButton->setStyleSheet("QToolButton {border:none;}");

    coverPageNumberLabel = new QLabel("-");

    coverPageNumberLabel->setStyleSheet("QLabel {color: white; font-weight:bold; font-size:14px;}");

    layout->addWidget(showPreviousCoverPageButton, 0, Qt::AlignVCenter);
    layout->addSpacing(5);
    layout->addWidget(coverPageNumberLabel, 0, Qt::AlignVCenter);
    layout->addSpacing(5);
    layout->addWidget(showNextCoverPageButton, 0, Qt::AlignVCenter);

    coverPageEdit->setStyleSheet("QLineEdit {border:none;}");
    layout->setSpacing(0);

    coverBox->setLayout(layout);

    coverBox->setFixedHeight(28);
    layout->setContentsMargins(5, 2, 5, 2);

    QPalette p(palette());
    p.setColor(QPalette::Window, QColor(0, 0, 0, 200));
    coverBox->setAutoFillBackground(true);
    coverBox->setPalette(p);

    connect(showPreviousCoverPageButton, &QAbstractButton::clicked, this, &PropertiesDialog::loadPreviousCover);
    connect(showNextCoverPageButton, &QAbstractButton::clicked, this, &PropertiesDialog::loadNextCover);
}

void PropertiesDialog::createGeneralInfoBox()
{
    generalInfoBox = new QWidget;

    auto generalInfoLayout = new QFormLayout;

    generalInfoLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    generalInfoLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    generalInfoLayout->addRow(tr("Series:"), series = new YACReaderFieldEdit());
    generalInfoLayout->addRow(tr("Title:"), title = new YACReaderFieldEdit());

    auto number = new QHBoxLayout;
    number->addWidget(numberEdit = new YACReaderFieldEdit());
    number->addWidget(new QLabel(tr("of:")));
    number->addWidget(countEdit = new YACReaderFieldEdit());
    countValidator.setBottom(0);
    countEdit->setValidator(&countValidator);
    number->addStretch(1);
    generalInfoLayout->addRow(tr("Issue number:"), number);

    generalInfoLayout->addRow(tr("Volume:"), volumeEdit = new YACReaderFieldEdit());

    auto arc = new QHBoxLayout;
    arc->addWidget(storyArcEdit = new YACReaderFieldEdit(), 1);
    storyArcEdit->setMinimumWidth(200);
    arc->addWidget(new QLabel(tr("Arc number:")));
    arc->addWidget(arcNumberEdit = new YACReaderFieldEdit());
    arc->addWidget(new QLabel(tr("of:")));
    arc->addWidget(arcCountEdit = new YACReaderFieldEdit());
    arcCountValidator.setBottom(0);
    arcCountEdit->setValidator(&arcCountValidator);
    generalInfoLayout->addRow(tr("Story arc:"), arc);

    auto alternate = new QHBoxLayout;
    alternate->addWidget(alternateSeriesEdit = new YACReaderFieldEdit(), 1);
    alternateSeriesEdit->setMinimumWidth(200);
    alternate->addWidget(new QLabel(tr("alt. number:")));
    alternate->addWidget(alternateNumberEdit = new YACReaderFieldEdit());
    alternate->addWidget(new QLabel(tr("of:")));
    alternate->addWidget(alternateCountEdit = new YACReaderFieldEdit());
    arcCountValidator.setBottom(0);
    alternateCountEdit->setValidator(&arcCountValidator);
    generalInfoLayout->addRow(tr("Alternate series:"), alternate);

    generalInfoLayout->addRow(tr("Series Group:"), seriesGroupEdit = new YACReaderFieldEdit());

    generalInfoLayout->addRow(tr("Genre:"), genereEdit = new YACReaderFieldEdit());

    generalInfoLayout->addRow(tr("Size:"), size = new QLabel("size"));

    auto main = new QVBoxLayout;
    main->addLayout(generalInfoLayout);
    main->addStretch();
    main->addWidget(comicVineLink = new QLabel("Comic Vine link : ..."));
    comicVineLink->setOpenExternalLinks(true);

    generalInfoBox->setLayout(main);
}

void PropertiesDialog::createAuthorsBox()
{
    authorsBox = new QWidget;

    auto authorsLayout = new QVBoxLayout;

    auto h1 = new QHBoxLayout;
    auto vl1 = new QVBoxLayout;
    auto vr1 = new QVBoxLayout;
    vl1->addWidget(new QLabel(tr("Writer(s):")));
    vl1->addWidget(writer = new YACReaderFieldPlainTextEdit());
    h1->addLayout(vl1);
    vr1->addWidget(new QLabel(tr("Penciller(s):")));
    vr1->addWidget(penciller = new YACReaderFieldPlainTextEdit());
    h1->addLayout(vr1);

    auto h2 = new QHBoxLayout;
    auto vl2 = new QVBoxLayout;
    auto vr2 = new QVBoxLayout;
    vl2->addWidget(new QLabel(tr("Inker(s):")));
    vl2->addWidget(inker = new YACReaderFieldPlainTextEdit());
    h2->addLayout(vl2);
    vr2->addWidget(new QLabel(tr("Colorist(s):")));
    vr2->addWidget(colorist = new YACReaderFieldPlainTextEdit());
    h2->addLayout(vr2);

    auto h3 = new QHBoxLayout;
    auto vl3 = new QVBoxLayout;
    auto vr3 = new QVBoxLayout;
    vl3->addWidget(new QLabel(tr("Letterer(s):")));
    vl3->addWidget(letterer = new YACReaderFieldPlainTextEdit());
    h3->addLayout(vl3);
    vr3->addWidget(new QLabel(tr("Cover Artist(s):")));
    vr3->addWidget(coverArtist = new YACReaderFieldPlainTextEdit());
    h3->addLayout(vr3);

    auto h4 = new QHBoxLayout;
    auto vl4 = new QVBoxLayout;
    auto vr4 = new QVBoxLayout;
    vl4->addWidget(new QLabel(tr("Editor(s):")));
    vl4->addWidget(editor = new YACReaderFieldPlainTextEdit());
    h4->addLayout(vl4);
    vr4->addWidget(new QLabel(tr("Imprint:")));
    vr4->addWidget(imprint = new YACReaderFieldPlainTextEdit());
    h4->addLayout(vr4);

    authorsLayout->addLayout(h1);
    authorsLayout->addLayout(h2);
    authorsLayout->addLayout(h3);
    authorsLayout->addLayout(h4);
    authorsLayout->addStretch(1);
    authorsBox->setLayout(authorsLayout);
}

void PropertiesDialog::createPublishingBox()
{
    publishingBox = new QWidget;

    auto publishingLayout = new QFormLayout;

    publishingLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    auto date = new QHBoxLayout;
    date->addWidget(new QLabel(tr("Day:")));
    date->addWidget(dayEdit = new YACReaderFieldEdit());
    dayValidator.setRange(1, 31);
    dayEdit->setValidator(&dayValidator);
    date->addWidget(new QLabel(tr("Month:")));
    date->addWidget(monthEdit = new YACReaderFieldEdit());
    monthValidator.setRange(1, 12);
    monthEdit->setValidator(&monthValidator);
    date->addWidget(new QLabel(tr("Year:")));
    date->addWidget(yearEdit = new YACReaderFieldEdit());
    yearValidator.setRange(1, 9999);
    yearEdit->setValidator(&yearValidator);
    date->addStretch(1);

    publishingLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    publishingLayout->addRow(date);
    publishingLayout->addRow(tr("Publisher:"), publisherEdit = new YACReaderFieldEdit());
    publishingLayout->addRow(tr("Format:"), formatEdit = new YACReaderFieldEdit());
    publishingLayout->addRow(tr("Color/BW:"), colorCheck = new QCheckBox());
    publishingLayout->addRow(tr("Age rating:"), ageRatingEdit = new YACReaderFieldEdit());
    publishingLayout->addRow(tr("Type:"), typeCombo = new QComboBox());
    publishingLayout->addRow(tr("Language (ISO):"), languageEdit = new YACReaderFieldEdit());

    typeCombo->addItem("Comic");
    typeCombo->addItem("Manga");
    typeCombo->addItem("Western Manga");
    typeCombo->addItem("Web Comic");
    typeCombo->addItem("4koma");

    typeCombo->setCurrentIndex(-1);

    publishingBox->setLayout(publishingLayout);
}

void PropertiesDialog::createPlotBox()
{
    plotBox = new QWidget;

    auto plotLayout = new QVBoxLayout;

    auto h1 = new QHBoxLayout;
    auto vl1 = new QVBoxLayout;
    vl1->addWidget(new QLabel(tr("Synopsis:")));
    vl1->addWidget(synopsis = new YACReaderFieldPlainTextEdit());
    h1->addLayout(vl1);

    auto h2 = new QHBoxLayout;
    auto vl2 = new QVBoxLayout;
    auto vr2 = new QVBoxLayout;
    vl2->addWidget(new QLabel(tr("Characters:")));
    vl2->addWidget(characters = new YACReaderFieldPlainTextEdit());
    h2->addLayout(vl2);
    vr2->addWidget(new QLabel(tr("Teams:")));
    vr2->addWidget(teams = new YACReaderFieldPlainTextEdit());
    h2->addLayout(vr2);

    auto h3 = new QHBoxLayout;
    auto vl3 = new QVBoxLayout;
    vl3->addWidget(new QLabel(tr("Locations:")));
    vl3->addWidget(locations = new YACReaderFieldPlainTextEdit());
    h3->addLayout(vl3);

    auto h4 = new QHBoxLayout;
    auto vl4 = new QVBoxLayout;
    vl4->addWidget(new QLabel(tr("Main character or team:")));
    vl4->addWidget(mainCharacterOrTeamEdit = new YACReaderFieldEdit());
    h4->addLayout(vl4);

    plotLayout->addLayout(h1);
    plotLayout->addLayout(h2);
    plotLayout->addLayout(h3);
    plotLayout->addLayout(h4);
    plotLayout->addStretch(1);

    plotBox->setLayout(plotLayout);
}

void PropertiesDialog::createNotesBox()
{
    notesBox = new QWidget;

    auto notesLayout = new QFormLayout;
    notesLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    notesLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);

    notesLayout->addRow(tr("Review:"), review = new YACReaderFieldPlainTextEdit());
    notesLayout->addRow(tr("Notes:"), notes = new YACReaderFieldPlainTextEdit());
    notesLayout->addRow(tr("Tags:"), tags = new YACReaderFieldPlainTextEdit());

    notesBox->setLayout(notesLayout);
}

void PropertiesDialog::createButtonBox()
{
    buttonBox = new QDialogButtonBox;

    closeButton = buttonBox->addButton(QDialogButtonBox::Close);
    saveButton = buttonBox->addButton(QDialogButtonBox::Save);
    previousButton = buttonBox->addButton("<", QDialogButtonBox::ButtonRole::NoRole);
    nextButton = buttonBox->addButton(">", QDialogButtonBox::ButtonRole::NoRole);

    connect(closeButton, &QAbstractButton::clicked, this, &PropertiesDialog::close);
    connect(saveButton, &QAbstractButton::clicked, this, &PropertiesDialog::saveAndClose);
    connect(previousButton, &QAbstractButton::clicked, this, &PropertiesDialog::saveAndOpenPrevious);
    connect(nextButton, &QAbstractButton::clicked, this, &PropertiesDialog::saveAndOpenNext);
}

QImage blurred(const QImage &image, const QRect &rect, int radius, bool alphaOnly = false)
{
    int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
    int alpha = (radius < 1) ? 16 : ((radius > 17) ? 1 : tab[radius - 1]);

    QImage result = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int r1 = rect.top();
    int r2 = rect.bottom();
    int c1 = rect.left();
    int c2 = rect.right();

    int bpl = result.bytesPerLine();
    int rgba[4];
    unsigned char *p;

    int i1 = 0;
    int i2 = 3;

    if (alphaOnly)
        i1 = i2 = (QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3);

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r1) + col * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p += bpl;
        for (int j = r1; j < r2; j++, p += bpl)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c1 * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p += 4;
        for (int j = c1; j < c2; j++, p += 4)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r2) + col * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p -= bpl;
        for (int j = r1; j < r2; j++, p -= bpl)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c2 * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p -= 4;
        for (int j = c1; j < c2; j++, p -= 4)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    return result;
}

void PropertiesDialog::loadComic(ComicDB &comic)
{
    if (!comic.info.series.isNull())
        series->setText(comic.info.series.toString());
    if (!comic.info.title.isNull())
        title->setText(comic.info.title.toString());
    if (!comic.info.comicVineID.isNull()) {
        comicVineLink->setHidden(false);
        comicVineLink->setText(QString(tr("Comic Vine link: <a style='color: #FFCB00; text-decoration:none; font-weight:bold;' href=\"http://www.comicvine.com/comic/4000-%1/\"> view </a>").arg(comic.info.comicVineID.toString())));
    } else
        comicVineLink->setHidden(true);

    if (!comic.info.coverPage.isNull()) {
        coverPageEdit->setText(comic.info.coverPage.toString());
        coverPageValidator.setRange(1, comic.info.numPages.toInt());
        coverPageEdit->setValidator(&coverPageValidator);
        //----------
        int coverPage = comic.info.coverPage.toInt();
        coverPageNumberLabel->setText(QString::number(coverPage));
        coverPageNumberLabel->adjustSize();

        showPreviousCoverPageButton->setEnabled(true);
        showNextCoverPageButton->setEnabled(true);

        if (coverPage == 1)
            showPreviousCoverPageButton->setDisabled(true);
        if (coverPage == comic.info.numPages.toInt())
            showNextCoverPageButton->setDisabled(true);

        coverChanged = false;
        coverBox->show();

        if (!QFileInfo(basePath + comic.path).exists()) {
            QMessageBox::warning(this, tr("Not found"), tr("Comic not found. You should update your library."));
            showPreviousCoverPageButton->setDisabled(true);
            showNextCoverPageButton->setDisabled(true);
        }
    }
    /*if(comic.info.numPages != NULL)
        numPagesEdit->setText(QString::number(*comic.info.numPages));*/

    if (!comic.info.number.isNull())
        numberEdit->setText(comic.info.number.toString());
    if (!comic.info.count.isNull())
        countEdit->setText(comic.info.count.toString());

    if (!comic.info.volume.isNull())
        volumeEdit->setText(comic.info.volume.toString());
    if (!comic.info.storyArc.isNull())
        storyArcEdit->setText(comic.info.storyArc.toString());
    if (!comic.info.arcNumber.isNull())
        arcNumberEdit->setText(comic.info.arcNumber.toString());
    if (!comic.info.arcCount.isNull())
        arcCountEdit->setText(comic.info.arcCount.toString());
    if (!comic.info.alternateSeries.isNull())
        alternateSeriesEdit->setText(comic.info.alternateSeries.toString());
    if (!comic.info.alternateNumber.isNull())
        alternateNumberEdit->setText(comic.info.alternateNumber.toString());
    if (!comic.info.alternateCount.isNull())
        alternateCountEdit->setText(comic.info.alternateCount.toString());
    if (!comic.info.seriesGroup.isNull())
        seriesGroupEdit->setText(comic.info.seriesGroup.toString());

    if (!comic.info.genere.isNull())
        genereEdit->setText(comic.info.genere.toString());

    if (!comic.info.writer.isNull())
        writer->setPlainText(comic.info.writer.toString());
    if (!comic.info.penciller.isNull())
        penciller->setPlainText(comic.info.penciller.toString());
    if (!comic.info.inker.isNull())
        inker->setPlainText(comic.info.inker.toString());
    if (!comic.info.colorist.isNull())
        colorist->setPlainText(comic.info.colorist.toString());
    if (!comic.info.letterer.isNull())
        letterer->setPlainText(comic.info.letterer.toString());
    if (!comic.info.coverArtist.isNull())
        coverArtist->setPlainText(comic.info.coverArtist.toString());
    if (!comic.info.editor.isNull())
        editor->setPlainText(comic.info.editor.toString());
    if (!comic.info.imprint.isNull())
        imprint->setPlainText(comic.info.imprint.toString());

    size->setText(QString::number(comic.info.hash.right(comic.info.hash.length() - 40).toInt() / 1024.0 / 1024.0, 'f', 2) + "Mb");

    if (!comic.info.date.isNull()) {
        QStringList date = (comic.info.date.toString()).split("/");

        if (date.length() == 3) {
            dayEdit->setText(date[0]);
            monthEdit->setText(date[1]);
            yearEdit->setText(date[2]);
        }
    }

    if (!comic.info.publisher.isNull())
        publisherEdit->setText(comic.info.publisher.toString());
    if (!comic.info.format.isNull())
        formatEdit->setText(comic.info.format.toString());
    if (!comic.info.color.isNull())
        colorCheck->setChecked(comic.info.color.toBool());
    else
        colorCheck->setCheckState(Qt::PartiallyChecked);

    typeCombo->setCurrentIndex(comic.info.type.toInt());

    if (!comic.info.languageISO.isNull())
        languageEdit->setText(comic.info.languageISO.toString());

    if (!comic.info.ageRating.isNull())
        ageRatingEdit->setText(comic.info.ageRating.toString());

    if (!comic.info.synopsis.isNull())
        synopsis->setPlainText(comic.info.synopsis.toString());
    if (!comic.info.characters.isNull())
        characters->setPlainText(comic.info.characters.toString());
    if (!comic.info.teams.isNull())
        teams->setPlainText(comic.info.teams.toString());
    if (!comic.info.locations.isNull())
        locations->setPlainText(comic.info.locations.toString());
    if (!comic.info.mainCharacterOrTeam.isNull())
        mainCharacterOrTeamEdit->setText(comic.info.mainCharacterOrTeam.toString());

    if (!comic.info.review.isNull())
        review->setPlainText(comic.info.review.toString());
    if (!comic.info.notes.isNull())
        notes->setPlainText(comic.info.notes.toString());
    if (!comic.info.tags.isNull())
        tags->setPlainText(comic.info.tags.toString());

    this->setWindowTitle(tr("Edit comic information"));
    setCover(comic.info.getCover(basePath));
}

void PropertiesDialog::updateButtons()
{
    if (sequentialEditing) {
        previousButton->setDisabled(currentComicIndex == 0);
        nextButton->setDisabled(currentComicIndex == comics.length() - 1);
        previousButton->setHidden(false);
        nextButton->setHidden(false);
    } else {
        previousButton->setHidden(true);
        nextButton->setHidden(true);
    }
}

void PropertiesDialog::setComics(QList<ComicDB> comics)
{
    updated = false;
    sequentialEditing = false;

    this->comics = comics;

    ComicDB comic = comics[0];

    loadComic(comic);

    updateButtons();

    if (comics.length() > 1) {
        coverBox->hide();

        setDisableUniqueValues(true);
        this->setWindowTitle(tr("Edit selected comics information"));
        setMultipleCover();

        QList<ComicDB>::iterator itr;
        for (itr = ++comics.begin(); itr != comics.end(); itr++) {
            if (itr->info.series.isNull() || itr->info.series.toString() != series->text())
                series->clear();

            if (itr->info.title.isNull() || itr->info.title.toString() != title->text())
                title->clear();

            if (itr->info.count.isNull() || itr->info.count.toString() != countEdit->text())
                countEdit->clear();

            if (itr->info.volume.isNull() || itr->info.volume.toString() != volumeEdit->text())
                volumeEdit->clear();
            if (itr->info.storyArc.isNull() || itr->info.storyArc.toString() != storyArcEdit->text())
                storyArcEdit->clear();
            if (itr->info.arcCount.isNull() || itr->info.arcCount.toString() != storyArcEdit->text())
                arcCountEdit->clear();

            if (itr->info.alternateSeries.isNull() || itr->info.alternateSeries.toString() != alternateSeriesEdit->text())
                alternateSeriesEdit->clear();
            if (itr->info.alternateCount.isNull() || itr->info.alternateCount.toString() != alternateCountEdit->text())
                alternateCountEdit->clear();

            if (itr->info.genere.isNull() || itr->info.genere.toString() != genereEdit->text())
                genereEdit->clear();
            if (itr->info.seriesGroup.isNull() || itr->info.seriesGroup.toString() != seriesGroupEdit->text())
                seriesGroupEdit->clear();

            if (itr->info.writer.isNull() || itr->info.writer.toString() != writer->toPlainText())
                writer->clear();
            if (itr->info.penciller.isNull() || itr->info.penciller.toString() != penciller->toPlainText())
                penciller->clear();
            if (itr->info.inker.isNull() || itr->info.inker.toString() != inker->toPlainText())
                inker->clear();
            if (itr->info.colorist.isNull() || itr->info.colorist.toString() != colorist->toPlainText())
                colorist->clear();
            if (itr->info.letterer.isNull() || itr->info.letterer.toString() != letterer->toPlainText())
                letterer->clear();
            if (itr->info.coverArtist.isNull() || itr->info.coverArtist.toString() != coverArtist->toPlainText())
                coverArtist->clear();
            if (itr->info.editor.isNull() || itr->info.editor.toString() != editor->toPlainText())
                editor->clear();
            if (itr->info.imprint.isNull() || itr->info.imprint.toString() != imprint->toPlainText())
                imprint->clear();

            if (itr->info.date.isNull()) {
                dayEdit->clear();
                monthEdit->clear();
                yearEdit->clear();
            } else {
                QStringList date = itr->info.date.toString().split("/");
                if (dayEdit->text() != date[0])
                    dayEdit->clear();
                if (monthEdit->text() != date[1])
                    monthEdit->clear();
                if (yearEdit->text() != date[2])
                    yearEdit->clear();
            }

            if (itr->info.publisher.isNull() || itr->info.publisher.toString() != publisherEdit->text())
                publisherEdit->clear();
            if (itr->info.format.isNull() || itr->info.format.toString() != formatEdit->text())
                formatEdit->clear();
            if (itr->info.color.isNull() || itr->info.color.toBool() != colorCheck->isChecked())
                colorCheck->setCheckState(Qt::PartiallyChecked);

            if (itr->info.type.toInt() != typeCombo->currentIndex()) {
                typeCombo->setCurrentIndex(-1);
            } else {
                typeCombo->setCurrentIndex(itr->info.type.toInt());
            }

            if (itr->info.languageISO.toString() != languageEdit->text())
                languageEdit->clear();

            if (itr->info.ageRating.isNull() || itr->info.ageRating.toString() != ageRatingEdit->text())
                ageRatingEdit->clear();

            if (itr->info.synopsis.isNull() || itr->info.synopsis.toString() != synopsis->toPlainText())
                synopsis->clear();
            if (itr->info.characters.isNull() || itr->info.characters.toString() != characters->toPlainText())
                characters->clear();
            if (itr->info.teams.isNull() || itr->info.teams.toString() != teams->toPlainText())
                teams->clear();
            if (itr->info.locations.isNull() || itr->info.locations.toString() != locations->toPlainText())
                locations->clear();
            if (itr->info.mainCharacterOrTeam.isNull() || itr->info.mainCharacterOrTeam.toString() != mainCharacterOrTeamEdit->text())
                mainCharacterOrTeamEdit->clear();

            if (itr->info.review.isNull() || itr->info.review.toString() != review->toPlainText())
                review->clear();
            if (itr->info.notes.isNull() || itr->info.notes.toString() != notes->toPlainText())
                notes->clear();
            if (itr->info.tags.isNull() || itr->info.tags.toString() != tags->toPlainText())
                tags->clear();
        }
    }
}

void PropertiesDialog::setComicsForSequentialEditing(int currentComicIndex, QList<ComicDB> comics)
{
    updated = false;
    sequentialEditing = true;

    this->comics = comics;
    this->currentComicIndex = currentComicIndex;

    loadComic(comics[currentComicIndex]);

    updateButtons();
}

void PropertiesDialog::updateComics()
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        db.open();
        db.transaction();
        QList<ComicDB>::iterator itr;
        QList<ComicDB>::iterator begin = sequentialEditing ? comics.begin() + currentComicIndex : comics.begin();
        QList<ComicDB>::iterator end = sequentialEditing ? comics.begin() + currentComicIndex + 1 : comics.end();
        for (itr = begin; itr != end; itr++) {
            if (itr->info.edited) {
                DBHelper::update(&(itr->info), db);
                updated = true;
            }
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void PropertiesDialog::setMultipleCover()
{
    ComicDB lastComic = comics.last();
    QPixmap last = lastComic.info.getCover(basePath);
    last = last.scaledToHeight(444, Qt::SmoothTransformation);

    coverImage = QPixmap::fromImage(blurred(last.toImage(), QRect(0, 0, last.width(), last.height()), 15));

    cover->setPixmap(coverImage);
}

void PropertiesDialog::setCover(const QPixmap &coverI)
{
    coverImage = coverI.scaledToHeight(575, Qt::SmoothTransformation);

    cover->setPixmap(coverImage);
}

void PropertiesDialog::setFilename(const QString &nameString)
{
    title->setText(nameString);
}
void PropertiesDialog::setNumpages(int pagesNum)
{
    numPagesEdit->setText(QString::number(pagesNum));
}
void PropertiesDialog::setSize(float sizeFloat)
{

    size->setText(QString::number(sizeFloat, 'f', 2) + " MB");
}

void PropertiesDialog::save()
{
    QList<ComicDB>::iterator itr;
    QList<ComicDB>::iterator begin = sequentialEditing ? comics.begin() + currentComicIndex : comics.begin();
    QList<ComicDB>::iterator end = sequentialEditing ? comics.begin() + currentComicIndex + 1 : comics.end();
    for (itr = begin; itr != end; itr++) {
        bool edited = false;

        if (series->isModified()) {
            auto seriesString = series->text();
            itr->info.series = seriesString.isEmpty() ? QVariant() : series->text();
            edited = true;
        }

        if (title->isModified()) {
            auto titleString = title->text();
            itr->info.title = titleString.isEmpty() ? QVariant() : title->text();
            edited = true;
        }

        if (sequentialEditing)
            if (coverChanged) {
                itr->info.coverPage = coverPageNumberLabel->text().toInt();
                edited = true;
            }

        if (sequentialEditing)
            if (numberEdit->isModified()) {
                if (numberEdit->text().isEmpty())
                    itr->info.number = QVariant();
                else
                    itr->info.number = numberEdit->text();
                edited = true;
            }

        if (countEdit->isModified()) {
            itr->info.count = countEdit->text();
            edited = true;
        }

        if (volumeEdit->isModified()) {
            itr->info.volume = volumeEdit->text();
            edited = true;
        }
        if (storyArcEdit->isModified()) {
            itr->info.storyArc = storyArcEdit->text();
            edited = true;
        }
        if (sequentialEditing)
            if (arcNumberEdit->isModified() && !arcNumberEdit->text().isEmpty()) {
                itr->info.arcNumber = arcNumberEdit->text();
                edited = true;
            }
        if (arcCountEdit->isModified()) {
            itr->info.arcCount = arcCountEdit->text();
            edited = true;
        }

        if (alternateSeriesEdit->isModified()) {
            itr->info.alternateSeries = alternateSeriesEdit->text();
            edited = true;
        }

        if (sequentialEditing) {
            if (alternateNumberEdit->isModified()) {
                itr->info.alternateNumber = alternateNumberEdit->text();
                edited = true;
            }
        }

        if (alternateCountEdit->isModified()) {
            itr->info.alternateCount = alternateCountEdit->text();
            edited = true;
        }

        if (genereEdit->isModified()) {
            itr->info.genere = genereEdit->text();
            edited = true;
        }

        if (writer->document()->isModified()) {
            itr->info.writer = writer->toPlainText();
            edited = true;
        }
        if (penciller->document()->isModified()) {
            itr->info.penciller = penciller->toPlainText();
            edited = true;
        }
        if (inker->document()->isModified()) {
            itr->info.inker = inker->toPlainText();
            edited = true;
        }
        if (colorist->document()->isModified()) {
            itr->info.colorist = colorist->toPlainText();
            edited = true;
        }
        if (letterer->document()->isModified()) {
            itr->info.letterer = letterer->toPlainText();
            edited = true;
        }
        if (coverArtist->document()->isModified()) {
            itr->info.coverArtist = coverArtist->toPlainText();
            edited = true;
        }
        if (editor->document()->isModified()) {
            itr->info.editor = editor->toPlainText();
            edited = true;
        }
        if (imprint->document()->isModified()) {
            itr->info.imprint = imprint->toPlainText();
            edited = true;
        }

        if (dayEdit->isModified() || monthEdit->isModified() || yearEdit->isModified()) {
            itr->info.date = dayEdit->text() + "/" + monthEdit->text() + "/" + yearEdit->text();
            edited = true;
        }
        if (publisherEdit->isModified()) {
            itr->info.publisher = publisherEdit->text();
            edited = true;
        }
        if (formatEdit->isModified()) {
            itr->info.format = formatEdit->text();
            edited = true;
        }
        if (colorCheck->checkState() != Qt::PartiallyChecked) {
            itr->info.color = colorCheck->isChecked();
            edited = true;
        }

        if (typeCombo->currentIndex() != -1 && itr->info.type.toInt() != typeCombo->currentIndex()) {
            itr->info.type = typeCombo->currentIndex();
            edited = true;
        }

        if (languageEdit->isModified()) {
            itr->info.languageISO = languageEdit->text();
            edited = true;
        }

        if (ageRatingEdit->isModified()) {
            itr->info.ageRating = ageRatingEdit->text();
            edited = true;
        }

        if (synopsis->document()->isModified()) {
            itr->info.synopsis = synopsis->toPlainText();
            edited = true;
        }

        if (characters->document()->isModified()) {
            itr->info.characters = characters->toPlainText();
            edited = true;
        }
        if (locations->document()->isModified()) {
            itr->info.locations = locations->toPlainText();
            edited = true;
        }
        if (teams->document()->isModified()) {
            itr->info.teams = teams->toPlainText();
            edited = true;
        }
        if (mainCharacterOrTeamEdit->isModified()) {
            itr->info.mainCharacterOrTeam = mainCharacterOrTeamEdit->text();
            edited = true;
        }
        if (seriesGroupEdit->isModified()) {
            itr->info.seriesGroup = seriesGroupEdit->text();
            edited = true;
        }

        if (review->document()->isModified()) {
            itr->info.review = review->toPlainText();
            edited = true;
        }
        if (notes->document()->isModified()) {
            itr->info.notes = notes->toPlainText();
            edited = true;
        }
        if (tags->document()->isModified()) {
            itr->info.tags = tags->toPlainText();
            edited = true;
        }

        itr->info.edited = edited;
    }

    if (sequentialEditing) {
        if (coverChanged) {
            InitialComicInfoExtractor ie(basePath + comics[currentComicIndex].path, basePath + "/.yacreaderlibrary/covers/" + comics[currentComicIndex].info.hash + ".jpg", comics[currentComicIndex].info.coverPage.toInt());
            ie.extract();

            if (ie.getOriginalCoverSize().second > 0) {
                comics[currentComicIndex].info.originalCoverSize = QString("%1x%2").arg(ie.getOriginalCoverSize().first).arg(ie.getOriginalCoverSize().second);
                comics[currentComicIndex].info.coverSizeRatio = static_cast<float>(ie.getOriginalCoverSize().first) / ie.getOriginalCoverSize().second;
            }
        }
    }
}

void PropertiesDialog::saveAndOpenPrevious()
{
    save();

    updateComics();

    coverChanged = false;

    currentComicIndex--;

    loadComic(comics[currentComicIndex]);

    updateButtons();

    repaint();
}

void PropertiesDialog::saveAndOpenNext()
{
    save();

    updateComics();

    coverChanged = false;

    currentComicIndex++;

    loadComic(comics[currentComicIndex]);

    updateButtons();

    repaint();
}

void PropertiesDialog::saveAndClose()
{
    save();

    updateComics();

    close();
    emit accepted();
}

void PropertiesDialog::setDisableUniqueValues(bool disabled)
{
    coverPageEdit->setDisabled(disabled);
    coverPageEdit->clear();
    numberEdit->setDisabled(disabled);
    numberEdit->clear();
    arcNumberEdit->setDisabled(disabled);
    arcNumberEdit->clear();
    alternateNumberEdit->setDisabled(disabled);
    alternateNumberEdit->clear();
}

void PropertiesDialog::closeEvent(QCloseEvent *e)
{

    title->clear();
    title->setModified(false);
    coverPageEdit->clear();
    numberEdit->clear();
    countEdit->clear();
    volumeEdit->clear();
    storyArcEdit->clear();
    arcNumberEdit->clear();
    arcCountEdit->clear();
    genereEdit->clear();
    writer->clear();
    penciller->clear();
    inker->clear();
    colorist->clear();
    letterer->clear();
    coverArtist->clear();
    dayEdit->clear();
    monthEdit->clear();
    yearEdit->clear();
    publisherEdit->clear();
    formatEdit->clear();
    colorCheck->setCheckState(Qt::PartiallyChecked);
    ageRatingEdit->clear();
    synopsis->clear();
    characters->clear();
    notes->clear();

    // 9.13 fields
    typeCombo->setCurrentIndex(-1);
    editor->clear();
    imprint->clear();
    teams->clear();
    locations->clear();
    series->clear();
    alternateSeriesEdit->clear();
    alternateNumberEdit->clear();
    alternateCountEdit->clear();
    languageEdit->clear();
    seriesGroupEdit->clear();
    mainCharacterOrTeamEdit->clear();
    review->clear();
    tags->clear();

    setDisableUniqueValues(false);

    tabBar->setCurrentIndex(0);

    coverPageEdit->setFocus();

    QDialog::closeEvent(e);
}

void PropertiesDialog::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);
}

void PropertiesDialog::updateCoverPageNumberLabel(int n)
{
    coverPageNumberLabel->setText(QString::number(n));
    coverPageNumberLabel->adjustSize();
}

void PropertiesDialog::loadNextCover()
{
    int current = coverPageNumberLabel->text().toInt();
    if (current < comics[currentComicIndex].info.numPages.toInt()) {
        updateCoverPageNumberLabel(current + 1);

        InitialComicInfoExtractor ie(basePath + comics[currentComicIndex].path, "", current + 1);
        ie.extract();
        setCover(ie.getCover());
        repaint();

        if ((current + 1) == comics[currentComicIndex].info.numPages.toInt()) {
            showNextCoverPageButton->setDisabled(true);
        }

        showPreviousCoverPageButton->setEnabled(true);
        if (current + 1 != comics[currentComicIndex].info.coverPage)
            coverChanged = true;
        else
            coverChanged = false;
    }
}

void PropertiesDialog::loadPreviousCover()
{
    int current = coverPageNumberLabel->text().toInt();
    if (current != 1) {
        updateCoverPageNumberLabel(current - 1);
        InitialComicInfoExtractor ie(basePath + comics[currentComicIndex].path, "", current - 1);
        ie.extract();
        setCover(ie.getCover());
        repaint();

        if ((current - 1) == 1) {
            showPreviousCoverPageButton->setDisabled(true);
        }

        showNextCoverPageButton->setEnabled(true);
        if (current - 1 != comics[currentComicIndex].info.coverPage.toInt())
            coverChanged = true;
        else
            coverChanged = false;
    }
}

bool PropertiesDialog::close()
{
    if (updated) {
        emit accepted();
    }

    return QDialog::close();
}
