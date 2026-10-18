#include "properties_dialog.h"

#include "cover_utils.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "initial_comic_info_extractor.h"
#include "staged_cover.h"
#include "yacreader_cover_label.h"
#include "yacreader_field_edit.h"
#include "yacreader_field_plain_text_edit.h"
#include "yacreader_global_gui.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSet>
#include <QSizePolicy>
#include <QTabWidget>
#include <QToolButton>

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

    cover = new YACReader::CoverLabel();
    connect(cover, &YACReader::CoverLabel::imageDropped, this, &PropertiesDialog::loadCustomCoverImageFromPath);

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

    deletedComicNote = new QLabel;
    deletedComicNote->setWordWrap(true);
    deletedComicNote->hide();
    rootLayout->addWidget(deletedComicNote, 1, 0);

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
    showPreviousCoverPageButton->setIcon(QIcon(":/images/metadata_dialog/previousCoverPage.svg"));
    showPreviousCoverPageButton->setToolTip(tr("Load previous page as cover"));
    showPreviousCoverPageButton->setStyleSheet("QToolButton {border:none;}");
    showNextCoverPageButton = new QToolButton();
    showNextCoverPageButton->setIcon(QIcon(":/images/metadata_dialog/nextCoverPage.svg"));
    showNextCoverPageButton->setToolTip(tr("Load next page as cover"));
    showNextCoverPageButton->setStyleSheet("QToolButton {border:none;}");

    resetCoverButton = new QToolButton();
    resetCoverButton->setIcon(QIcon(":/images/metadata_dialog/resetCover.svg"));
    resetCoverButton->setToolTip(tr("Reset cover to the default image"));
    resetCoverButton->setStyleSheet("QToolButton {border:none;}");

    loadCustomCoverImageButton = new QToolButton();
    loadCustomCoverImageButton->setIcon(QIcon(":/images/metadata_dialog/loadCustomCover.svg"));
    loadCustomCoverImageButton->setToolTip(tr("Load custom cover image"));
    loadCustomCoverImageButton->setStyleSheet("QToolButton {border:none;}");

    coverPageNumberLabel = new QLabel("-");

    coverPageNumberLabel->setStyleSheet("QLabel {color: white; font-weight:bold; font-size:14px;}");

    layout->addWidget(showPreviousCoverPageButton, 0, Qt::AlignVCenter);
    layout->addSpacing(5);
    layout->addWidget(coverPageNumberLabel, 0, Qt::AlignVCenter);
    layout->addSpacing(5);
    layout->addWidget(showNextCoverPageButton, 0, Qt::AlignVCenter);
    layout->addSpacing(5);
    layout->addWidget(resetCoverButton, 0, Qt::AlignVCenter);
    layout->addSpacing(5);
    layout->addWidget(loadCustomCoverImageButton, 0, Qt::AlignVCenter);

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
    connect(resetCoverButton, &QAbstractButton::clicked, this, &PropertiesDialog::resetCover);
    connect(loadCustomCoverImageButton, &QAbstractButton::clicked, this, &PropertiesDialog::loadCustomCoverImage);
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

void PropertiesDialog::loadComic(const ComicRecord &record)
{
    customCover = QImage();

    const auto &info = *recordInfo(record);
    const auto *comic = recordComic(record);

    series->setText(info.series.isNull() ? QString() : info.series.toString());
    title->setText(info.title.isNull() ? QString() : info.title.toString());

    if (!info.comicVineID.isNull()) {
        comicVineLink->setHidden(false);
        comicVineLink->setText(QString(tr("Comic Vine link: <a style='color: #FFCB00; text-decoration:none; font-weight:bold;' href=\"http://www.comicvine.com/comic/4000-%1/\"> view </a>").arg(info.comicVineID.toString())));
    } else
        comicVineLink->setHidden(true);

    coverChanged = false;
    showPreviousCoverPageButton->setEnabled(comic != nullptr);
    showNextCoverPageButton->setEnabled(comic != nullptr);
    resetCoverButton->setEnabled(comic != nullptr);
    loadCustomCoverImageButton->setEnabled(true);

    if (!info.coverPage.isNull()) {
        coverPageEdit->setText(info.coverPage.toString());
        coverPageValidator.setRange(1, info.numPages.toInt());
        coverPageEdit->setValidator(&coverPageValidator);
        //----------
        int coverPage = info.coverPage.toInt();
        coverPageNumberLabel->setText(QString::number(coverPage));
        coverPageNumberLabel->adjustSize();

        updateCoverBoxForSingleComic();

        if (comic != nullptr && !QFileInfo(basePath + comic->file.path).exists()) {
            QMessageBox::warning(this, tr("Not found"), tr("Comic not found. You should update your library."));
            showPreviousCoverPageButton->setDisabled(true);
            showNextCoverPageButton->setDisabled(true);
        }
    } else {
        coverPageNumberLabel->setText("1");
    }

    if (const auto *deleted = std::get_if<ComicRecordCase::DeletedComic>(&record)) {
        auto note = tr("The file of this comic is not in the library.");
        if (deleted->lastKnownFile)
            note += "\n" + tr("Last known file: %1").arg(deleted->lastKnownFile->name);
        deletedComicNote->setText(note);
        deletedComicNote->show();
    } else {
        deletedComicNote->hide();
    }

    numberEdit->setText(info.number.isNull() ? QString() : info.number.toString());
    countEdit->setText(info.count.isNull() ? QString() : info.count.toString());
    volumeEdit->setText(info.volume.isNull() ? QString() : info.volume.toString());
    storyArcEdit->setText(info.storyArc.isNull() ? QString() : info.storyArc.toString());
    arcNumberEdit->setText(info.arcNumber.isNull() ? QString() : info.arcNumber.toString());
    arcCountEdit->setText(info.arcCount.isNull() ? QString() : info.arcCount.toString());
    alternateSeriesEdit->setText(info.alternateSeries.isNull() ? QString() : info.alternateSeries.toString());
    alternateNumberEdit->setText(info.alternateNumber.isNull() ? QString() : info.alternateNumber.toString());
    alternateCountEdit->setText(info.alternateCount.isNull() ? QString() : info.alternateCount.toString());
    seriesGroupEdit->setText(info.seriesGroup.isNull() ? QString() : info.seriesGroup.toString());
    genereEdit->setText(info.genere.isNull() ? QString() : info.genere.toString());

    writer->setPlainText(info.writer.isNull() ? QString() : info.writer.toString());
    penciller->setPlainText(info.penciller.isNull() ? QString() : info.penciller.toString());
    inker->setPlainText(info.inker.isNull() ? QString() : info.inker.toString());
    colorist->setPlainText(info.colorist.isNull() ? QString() : info.colorist.toString());
    letterer->setPlainText(info.letterer.isNull() ? QString() : info.letterer.toString());
    coverArtist->setPlainText(info.coverArtist.isNull() ? QString() : info.coverArtist.toString());
    editor->setPlainText(info.editor.isNull() ? QString() : info.editor.toString());
    imprint->setPlainText(info.imprint.isNull() ? QString() : info.imprint.toString());

    size->setText(QString::number(info.hash.right(info.hash.length() - 40).toInt() / 1024.0 / 1024.0, 'f', 2) + "Mb");

    if (!info.date.isNull()) {
        QStringList date = (info.date.toString()).split("/");

        if (date.length() == 3) {
            dayEdit->setText(date[0]);
            monthEdit->setText(date[1]);
            yearEdit->setText(date[2]);
        }
    } else {
        dayEdit->setText(QString());
        monthEdit->setText(QString());
        yearEdit->setText(QString());
    }

    publisherEdit->setText(info.publisher.isNull() ? QString() : info.publisher.toString());
    formatEdit->setText(info.format.isNull() ? QString() : info.format.toString());

    if (!info.color.isNull())
        colorCheck->setChecked(info.color.toBool());
    else
        colorCheck->setCheckState(Qt::PartiallyChecked);

    typeCombo->setCurrentIndex(info.type.toInt());

    languageEdit->setText(info.languageISO.isNull() ? QString() : info.languageISO.toString());
    ageRatingEdit->setText(info.ageRating.isNull() ? QString() : info.ageRating.toString());

    synopsis->setPlainText(info.synopsis.isNull() ? QString() : info.synopsis.toString());
    characters->setPlainText(info.characters.isNull() ? QString() : info.characters.toString());
    teams->setPlainText(info.teams.isNull() ? QString() : info.teams.toString());
    locations->setPlainText(info.locations.isNull() ? QString() : info.locations.toString());
    mainCharacterOrTeamEdit->setText(info.mainCharacterOrTeam.isNull() ? QString() : info.mainCharacterOrTeam.toString());

    review->setPlainText(info.review.isNull() ? QString() : info.review.toString());
    notes->setPlainText(info.notes.isNull() ? QString() : info.notes.toString());
    tags->setPlainText(info.tags.isNull() ? QString() : info.tags.toString());

    loadedColorState = colorCheck->checkState();
    loadedTypeIndex = typeCombo->currentIndex();

    this->setWindowTitle(tr("Edit comic information"));
    setCover(coverOf(record));
}

// a deleted comic shows its cover in grayscale, or the default image when the cover file is missing
QPixmap PropertiesDialog::coverOf(const ComicRecord &record) const
{
    const auto &info = *recordInfo(record);
    if (recordComic(record) != nullptr)
        return info.getCover(basePath);

    QImage image(LibraryPaths::coverPath(basePath, info.hash));
    if (image.isNull())
        image = QImage(":/images/unavailableCover.png");
    return QPixmap::fromImage(image.convertToFormat(QImage::Format_Grayscale8));
}

const ComicDB *PropertiesDialog::lastComicWithFile() const
{
    for (auto it = records.crbegin(); it != records.crend(); ++it) {
        if (const auto *comic = recordComic(*it))
            return comic;
    }
    return nullptr;
}

void PropertiesDialog::updateButtons()
{
    if (sequentialEditing) {
        previousButton->setDisabled(currentComicIndex == 0);
        nextButton->setDisabled(currentComicIndex == records.length() - 1);
        previousButton->setHidden(false);
        nextButton->setHidden(false);
    } else {
        previousButton->setHidden(true);
        nextButton->setHidden(true);
    }
}

void PropertiesDialog::setComics(QList<ComicRecord> records)
{
    sequentialEditing = false;

    updated = false;
    currentComicIndex = 0;
    coverChanged = false;
    customCover = QImage();

    this->records = records;

    loadComic(records[0]);

    updateButtons();

    if (records.length() > 1) {
        updateCoverBoxForMultipleComics();
        deletedComicNote->hide();

        // the reset needs the file, so it skips the deleted comics
        resetCoverButton->setEnabled(lastComicWithFile() != nullptr);
        loadCustomCoverImageButton->setEnabled(true);

        setDisableUniqueValues(true);
        this->setWindowTitle(tr("Edit selected comics information"));
        setMultipleCover();

        for (auto itr = ++records.cbegin(); itr != records.cend(); itr++) {
            const auto &info = *recordInfo(*itr);

            if (info.series.isNull() || info.series.toString() != series->text())
                series->clear();

            if (info.title.isNull() || info.title.toString() != title->text())
                title->clear();

            if (info.count.isNull() || info.count.toString() != countEdit->text())
                countEdit->clear();

            if (info.volume.isNull() || info.volume.toString() != volumeEdit->text())
                volumeEdit->clear();
            if (info.storyArc.isNull() || info.storyArc.toString() != storyArcEdit->text())
                storyArcEdit->clear();
            if (info.arcCount.isNull() || info.arcCount.toString() != arcCountEdit->text())
                arcCountEdit->clear();

            if (info.alternateSeries.isNull() || info.alternateSeries.toString() != alternateSeriesEdit->text())
                alternateSeriesEdit->clear();
            if (info.alternateCount.isNull() || info.alternateCount.toString() != alternateCountEdit->text())
                alternateCountEdit->clear();

            if (info.genere.isNull() || info.genere.toString() != genereEdit->text())
                genereEdit->clear();
            if (info.seriesGroup.isNull() || info.seriesGroup.toString() != seriesGroupEdit->text())
                seriesGroupEdit->clear();

            if (info.writer.isNull() || info.writer.toString() != writer->toPlainText())
                writer->clear();
            if (info.penciller.isNull() || info.penciller.toString() != penciller->toPlainText())
                penciller->clear();
            if (info.inker.isNull() || info.inker.toString() != inker->toPlainText())
                inker->clear();
            if (info.colorist.isNull() || info.colorist.toString() != colorist->toPlainText())
                colorist->clear();
            if (info.letterer.isNull() || info.letterer.toString() != letterer->toPlainText())
                letterer->clear();
            if (info.coverArtist.isNull() || info.coverArtist.toString() != coverArtist->toPlainText())
                coverArtist->clear();
            if (info.editor.isNull() || info.editor.toString() != editor->toPlainText())
                editor->clear();
            if (info.imprint.isNull() || info.imprint.toString() != imprint->toPlainText())
                imprint->clear();

            if (info.date.isNull()) {
                dayEdit->clear();
                monthEdit->clear();
                yearEdit->clear();
            } else {
                QStringList date = info.date.toString().split("/");
                if (dayEdit->text() != date[0])
                    dayEdit->clear();
                if (monthEdit->text() != date[1])
                    monthEdit->clear();
                if (yearEdit->text() != date[2])
                    yearEdit->clear();
            }

            if (info.publisher.isNull() || info.publisher.toString() != publisherEdit->text())
                publisherEdit->clear();
            if (info.format.isNull() || info.format.toString() != formatEdit->text())
                formatEdit->clear();
            if (info.color.isNull() || info.color.toBool() != colorCheck->isChecked())
                colorCheck->setCheckState(Qt::PartiallyChecked);

            if (info.type.toInt() != typeCombo->currentIndex()) {
                typeCombo->setCurrentIndex(-1);
            } else {
                typeCombo->setCurrentIndex(info.type.toInt());
            }

            if (info.languageISO.toString() != languageEdit->text())
                languageEdit->clear();

            if (info.ageRating.isNull() || info.ageRating.toString() != ageRatingEdit->text())
                ageRatingEdit->clear();

            if (info.synopsis.isNull() || info.synopsis.toString() != synopsis->toPlainText())
                synopsis->clear();
            if (info.characters.isNull() || info.characters.toString() != characters->toPlainText())
                characters->clear();
            if (info.teams.isNull() || info.teams.toString() != teams->toPlainText())
                teams->clear();
            if (info.locations.isNull() || info.locations.toString() != locations->toPlainText())
                locations->clear();
            if (info.mainCharacterOrTeam.isNull() || info.mainCharacterOrTeam.toString() != mainCharacterOrTeamEdit->text())
                mainCharacterOrTeamEdit->clear();

            if (info.review.isNull() || info.review.toString() != review->toPlainText())
                review->clear();
            if (info.notes.isNull() || info.notes.toString() != notes->toPlainText())
                notes->clear();
            if (info.tags.isNull() || info.tags.toString() != tags->toPlainText())
                tags->clear();
        }

        loadedColorState = colorCheck->checkState();
        loadedTypeIndex = typeCombo->currentIndex();
    }
}

bool PropertiesDialog::setComicsForSequentialEditing(int currentComicIndex, QList<ComicRecord> records)
{
    sequentialEditing = true;

    updated = false;
    coverChanged = false;
    customCover = QImage();

    this->records = records;
    this->currentComicIndex = currentComicIndex;

    return showRecord(currentComicIndex, 1);
}

// loads the record again, so a repeated comic shows the values saved at its first occurrence and changes made
// elsewhere are seen; a record that no longer exists is skipped in the direction of `step`
bool PropertiesDialog::showRecord(int index, int step)
{
    for (; index >= 0 && index < records.length(); index += step) {
        std::optional<ComicRecord> loaded;
        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
            loaded = DBHelper::loadRecord(records[index], db);
            connectionName = db.connectionName();
        }
        QSqlDatabase::removeDatabase(connectionName);

        if (loaded) {
            records[index] = *loaded;
            currentComicIndex = index;
            loadComic(records[index]);
            updateButtons();
            return true;
        }

        QMessageBox::information(this, tr("Comic skipped"), tr("A comic was removed from the library or now has a different file, so it is skipped."));
    }
    return false;
}

// a cleared field is NULL; the values that only one comic can have are only edited in sequential editing
DescriptiveMetadataPatch PropertiesDialog::editedMetadata() const
{
    using M = DescriptiveComicMetadata;
    DescriptiveMetadataPatch patch;

    const auto value = [](const QString &text) { return text.isEmpty() ? QVariant() : QVariant(text); };
    const auto line = [&](YACReaderFieldEdit *edit, QVariant M::*field) {
        if (edit->isModified())
            patch.set(field, value(edit->text()));
    };
    const auto plain = [&](YACReaderFieldPlainTextEdit *edit, QVariant M::*field) {
        if (edit->document()->isModified())
            patch.set(field, value(edit->toPlainText()));
    };

    line(series, &M::series);
    line(title, &M::title);
    if (sequentialEditing) {
        line(numberEdit, &M::number);
        line(arcNumberEdit, &M::arcNumber);
        line(alternateNumberEdit, &M::alternateNumber);
    }
    line(countEdit, &M::count);
    line(volumeEdit, &M::volume);
    line(storyArcEdit, &M::storyArc);
    line(arcCountEdit, &M::arcCount);
    line(alternateSeriesEdit, &M::alternateSeries);
    line(alternateCountEdit, &M::alternateCount);
    line(genereEdit, &M::genere);

    plain(writer, &M::writer);
    plain(penciller, &M::penciller);
    plain(inker, &M::inker);
    plain(colorist, &M::colorist);
    plain(letterer, &M::letterer);
    plain(coverArtist, &M::coverArtist);
    plain(editor, &M::editor);
    plain(imprint, &M::imprint);

    line(publisherEdit, &M::publisher);
    line(formatEdit, &M::format);
    if (colorCheck->checkState() != Qt::PartiallyChecked && colorCheck->checkState() != loadedColorState)
        patch.set(&M::color, colorCheck->isChecked());

    line(languageEdit, &M::languageISO);
    line(ageRatingEdit, &M::ageRating);

    plain(synopsis, &M::synopsis);
    plain(characters, &M::characters);
    plain(locations, &M::locations);
    plain(teams, &M::teams);
    line(mainCharacterOrTeamEdit, &M::mainCharacterOrTeam);
    line(seriesGroupEdit, &M::seriesGroup);

    plain(review, &M::review);
    plain(notes, &M::notes);
    plain(tags, &M::tags);

    return patch;
}

// the changed parts of the date, with the other parts of `info`
std::optional<QVariant> PropertiesDialog::editedDate(const ComicInfo &info) const
{
    if (!dayEdit->isModified() && !monthEdit->isModified() && !yearEdit->isModified())
        return std::nullopt;

    auto parts = info.date.toString().split("/");
    if (parts.length() != 3)
        parts = { QString(), QString(), QString() };
    if (dayEdit->isModified())
        parts[0] = dayEdit->text();
    if (monthEdit->isModified())
        parts[1] = monthEdit->text();
    if (yearEdit->isModified())
        parts[2] = yearEdit->text();

    const auto date = parts.join("/");
    return date == "//" ? QVariant() : QVariant(date);
}

std::optional<YACReader::FileType> PropertiesDialog::editedType() const
{
    const auto index = typeCombo->currentIndex();
    if (index == -1 || index == loadedTypeIndex)
        return std::nullopt;
    return static_cast<YACReader::FileType>(index);
}

// writes the new cover into `staged` and sets the cover columns of the record; false when the change does not apply
// to it (a cover page or a reset needs the file) or the file could not be written
bool PropertiesDialog::applyCoverChange(ComicRecord &record, StagedCover &staged)
{
    auto &info = *recordInfo(record);
    const auto *comic = recordComic(record);

    if (customCover.isNull()) {
        if (comic == nullptr)
            return false;

        // sequential editing keeps the chosen page; the reset of several comics goes back to the first page
        const int page = sequentialEditing ? coverPageNumberLabel->text().toInt() : 1;
        info.coverPage = sequentialEditing ? QVariant(page) : QVariant();

        InitialComicInfoExtractor ie(basePath + comic->file.path, staged.stagedPath(), page);
        ie.extract();
        if (!staged.isStaged())
            return false;

        if (ie.getOriginalCoverSize().second > 0) {
            info.originalCoverSize = QString("%1x%2").arg(ie.getOriginalCoverSize().first).arg(ie.getOriginalCoverSize().second);
            info.coverSizeRatio = static_cast<float>(ie.getOriginalCoverSize().first) / ie.getOriginalCoverSize().second;
        }
        info.lastTimeCoverSet = QDateTime::currentSecsSinceEpoch();
        info.usesExternalCover = false;
    } else {
        if (!staged.stage(customCover))
            return false;
        info.coverPage = QVariant();

        auto width = customCover.width();
        auto height = customCover.height();
        info.originalCoverSize = QString("%1x%2").arg(width).arg(height);
        info.coverSizeRatio = static_cast<float>(width) / height;
        info.lastTimeCoverSet = QDateTime::currentSecsSinceEpoch();
        info.usesExternalCover = true;
    }
    return true;
}

bool PropertiesDialog::updateComics()
{
    const auto metadata = editedMetadata();
    const auto type = editedType();
    const int begin = sequentialEditing ? currentComicIndex : 0;
    const int end = sequentialEditing ? currentComicIndex + 1 : records.length();

    bool refused = false;
    bool failed = false;
    bool wrote = false;
    // the records change only after the commit, so a failed save keeps them, the files, and the database as they were
    QList<std::pair<int, ComicRecord>> savedRecords;
    QList<StagedCover> stagedCovers;
    QList<ComicInfoId> changedCovers;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        db.transaction();

        for (int i = begin; i < end && !refused; i++) {
            refused = !DBHelper::loadRecord(records[i], db);
        }

        if (!refused) {
            bool written = true;
            // each comic_info is written once, also when several rows show it
            QSet<ComicInfoId> writtenIds;
            for (int i = begin; i < end; i++) {
                auto record = records[i];
                auto &info = *recordInfo(record);
                if (writtenIds.contains(info.id))
                    continue;
                writtenIds.insert(info.id);

                auto patch = metadata;
                if (const auto date = editedDate(info))
                    patch.set(&DescriptiveComicMetadata::date, *date);

                if (!patch.isEmpty()) {
                    written = DBHelper::updateComicInfoMetadata(info.id, patch, db) && written;
                    patch.applyTo(info);
                    info.edited = true;
                    wrote = true;
                }

                if (type && info.type.toInt() != static_cast<int>(*type)) {
                    written = DBHelper::updateComicInfoType(info.id, *type, db) && written;
                    info.type = QVariant::fromValue(*type);
                    info.edited = true;
                    wrote = true;
                }

                if (coverChanged) {
                    StagedCover staged(LibraryPaths::coverPath(basePath, info.hash));
                    if (applyCoverChange(record, staged)) {
                        written = DBHelper::updateComicInfoCover(info, db) && written;
                        info.edited = true;
                        stagedCovers.append(staged);
                        changedCovers.append(info.id);
                        wrote = true;
                    }
                }

                savedRecords.append({ i, std::move(record) });
            }

            written = written && std::all_of(stagedCovers.begin(), stagedCovers.end(), [](StagedCover &staged) { return staged.install(); });
            if (written && db.commit()) {
                for (auto &staged : stagedCovers)
                    staged.finish();
            } else {
                db.rollback();
                for (auto &staged : stagedCovers)
                    staged.restore();
                failed = true;
            }
        } else {
            db.rollback();
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (refused) {
        QMessageBox::warning(this, tr("Changes not saved"), tr("A comic that you are editing was removed from the library or now has a different file. Your changes were not saved."));
        return false;
    }
    if (failed) {
        QMessageBox::warning(this, tr("Changes not saved"), tr("The changes could not be saved."));
        return false;
    }

    for (auto &[index, record] : savedRecords)
        records[index] = std::move(record);
    updated = updated || wrote;

    for (const auto id : std::as_const(changedCovers))
        emit coverChangedSignal(id);
    return true;
}

void PropertiesDialog::setMultipleCover()
{
    QPixmap last = coverOf(records.last());
    last = last.scaledToHeight(575, Qt::SmoothTransformation);

    auto coverImage = QPixmap::fromImage(blurred(last.toImage(), QRect(0, 0, last.width(), last.height()), 15));

    cover->setPixmap(coverImage);
}

void PropertiesDialog::setCover(const QPixmap &coverI)
{
    auto coverImage = coverI.scaledToHeight(575, Qt::SmoothTransformation);

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

// a refused save shows its message, and the user can still move on
void PropertiesDialog::saveAndOpenPrevious()
{
    updateComics();

    if (!showRecord(currentComicIndex - 1, -1)) {
        close();
        return;
    }

    repaint();
}

void PropertiesDialog::saveAndOpenNext()
{
    updateComics();

    if (!showRecord(currentComicIndex + 1, 1)) {
        close();
        return;
    }

    repaint();
}

void PropertiesDialog::saveAndClose()
{
    if (!updateComics())
        return;

    close();
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

// every way of closing the dialog (Close, Save, Escape, the window button) comes here
void PropertiesDialog::done(int result)
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

    // the comics saved before (sequential editing) need a refresh of the views
    if (updated && result != QDialog::Accepted)
        emit accepted();
    updated = false;

    QDialog::done(result);
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
    const auto &info = *recordInfo(records[currentComicIndex]);
    int current = coverPageNumberLabel->text().toInt();
    int next;

    if (current == info.numPages.toInt())
        next = 1;
    else
        next = current + 1;

    setCoverPage(next);

    coverChanged = next != info.coverPage;
}

void PropertiesDialog::loadPreviousCover()
{
    const auto &info = *recordInfo(records[currentComicIndex]);
    int current = coverPageNumberLabel->text().toInt();
    int previous;

    if (current == 1)
        previous = info.numPages.toInt();
    else
        previous = current - 1;

    setCoverPage(previous);

    coverChanged = previous != info.coverPage.toInt();
}

void PropertiesDialog::resetCover()
{
    if (sequentialEditing) {
        setCoverPage(1);
        coverChanged = true; // it could be that the cover is a custom cover, so we need to always update it
    } else {
        const auto *comic = lastComicWithFile();
        if (comic == nullptr)
            return;

        InitialComicInfoExtractor ie(basePath + comic->file.path, "", 1);
        ie.extract();
        auto cover = ie.getCover();
        cover = cover.scaledToHeight(575, Qt::SmoothTransformation);

        auto coverImage = QPixmap::fromImage(blurred(cover.toImage(), QRect(0, 0, cover.width(), cover.height()), 15));

        this->cover->setPixmap(coverImage);

        customCover = QImage();
        coverChanged = true;
    }
}

void PropertiesDialog::loadCustomCoverImage()
{
    auto path = YACReader::imageFileLoader(this);
    loadCustomCoverImageFromPath(path);
}

void PropertiesDialog::loadCustomCoverImageFromPath(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    const QImage image(path);
    if (image.isNull()) {
        QMessageBox::warning(this, tr("Invalid cover"), tr("The image is invalid."));
        return;
    }

    customCover = image;
    coverChanged = true;

    const bool deleted = (sequentialEditing || records.length() == 1) && recordComic(records[currentComicIndex]) == nullptr;
    setCover(QPixmap::fromImage(deleted ? customCover.convertToFormat(QImage::Format_Grayscale8) : customCover));
}

void PropertiesDialog::setCoverPage(int pageNumber)
{
    const auto *comic = recordComic(records[currentComicIndex]);
    if (comic == nullptr)
        return;

    customCover = QImage();

    updateCoverPageNumberLabel(pageNumber);
    InitialComicInfoExtractor ie(basePath + comic->file.path, "", pageNumber);
    ie.extract();
    setCover(ie.getCover());
    repaint();
}

void PropertiesDialog::updateCoverBoxForMultipleComics()
{
    showPreviousCoverPageButton->hide();
    showNextCoverPageButton->hide();
    coverPageNumberLabel->hide();
}

void PropertiesDialog::updateCoverBoxForSingleComic()
{
    showPreviousCoverPageButton->show();
    showNextCoverPageButton->show();
    coverPageNumberLabel->show();
}
