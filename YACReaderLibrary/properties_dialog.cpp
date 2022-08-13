#include "properties_dialog.h"

#include "data_base_management.h"
#include "initial_comic_info_extractor.h"
#include "yacreader_field_edit.h"
#include "yacreader_field_plain_text_edit.h"
#include "db_helper.h"
//#include "yacreader_busy_widget.h"

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
    : QDialog(parent)
{

    createCoverBox();
    createGeneralInfoBox();
    createAuthorsBox();
    createPublishingBox();
    createButtonBox();
    createPlotBox();

    createTabBar();

    mainLayout = new QGridLayout;
    // mainLayout->addWidget(coverBox,0,0);
    mainLayout->addWidget(tabBar, 0, 1);
    mainLayout->setColumnStretch(1, 1);
    /*mainLayout->addWidget(authorsBox,1,1);
        mainLayout->addWidget(publishingBox,2,1);*/
    mainLayout->addWidget(buttonBox, 1, 1, Qt::AlignBottom);

    mainWidget = new QWidget(this);
    mainWidget->setAutoFillBackground(true);
    mainWidget->setFixedSize(470, 444);
    mainWidget->setLayout(mainLayout);
    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

    QScreen *screen = parent != nullptr ? parent->window()->screen() : nullptr;
    if (screen == nullptr) {
        screen = QApplication::screens().constFirst();
    }

    int heightDesktopResolution = screen->geometry().height();
    int widthDesktopResolution = screen->geometry().width();
    int sHeight, sWidth;
    sHeight = static_cast<int>(heightDesktopResolution * 0.65);
    sWidth = static_cast<int>(sHeight * 1.4);
    // setCover(QPixmap(":/images/notCover.png"));

    this->move(QPoint((widthDesktopResolution - sWidth) / 2, ((heightDesktopResolution - sHeight) - 40) / 2));
    setModal(true);

    setFixedSize(sizeHint());
    mainWidget->move(280, 0);
}

QSize PropertiesDialog::sizeHint() const
{
    return QSize(750, 444);
}

void PropertiesDialog::createTabBar()
{
    tabBar = new QTabWidget;
    tabBar->addTab(generalInfoBox, tr("General info"));
    tabBar->addTab(authorsBox, tr("Authors"));
    tabBar->addTab(publishingBox, tr("Publishing"));
    tabBar->addTab(plotBox, tr("Plot"));
}

void PropertiesDialog::createCoverBox()
{
    coverBox = new QWidget(this);

    auto layout = new QHBoxLayout;

    QLabel *label = new QLabel(tr("Cover page"));
    label->setStyleSheet("QLabel {color: white; font-weight:bold; font-size:14px;}");
    layout->addWidget(label);
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

    layout->addWidget(showPreviousCoverPageButton);
    layout->addSpacing(5);
    layout->addWidget(coverPageNumberLabel);
    layout->addSpacing(5);
    layout->addWidget(showNextCoverPageButton);

    coverPageEdit->setStyleSheet("QLineEdit {border:none;}");
    layout->setSpacing(0);

    coverBox->setLayout(layout);

    coverBox->setFixedWidth(280);
    coverBox->move(0, 444 - 28);
    layout->setContentsMargins(5, 4, 5, 0);

    // busyIndicator = new YACReaderBusyWidget(this);
    // busyIndicator->move((280-busyIndicator->width())/2,(444-busyIndicator->height()-28)/2);
    // busyIndicator->hide();

    connect(showPreviousCoverPageButton, &QAbstractButton::clicked, this, &PropertiesDialog::loadPreviousCover);
    connect(showNextCoverPageButton, &QAbstractButton::clicked, this, &PropertiesDialog::loadNextCover);
}

QFrame *createLine()
{
    QFrame *line = new QFrame();
    line->setObjectName(QString::fromUtf8("line"));
    // line->setGeometry(QRect(320, 150, 118, 3));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

void PropertiesDialog::createGeneralInfoBox()
{
    generalInfoBox = new QWidget;

    auto generalInfoLayout = new QFormLayout;

    generalInfoLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    // generalInfoLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    generalInfoLayout->addRow(tr("Title:"), title = new YACReaderFieldEdit());

    auto number = new QHBoxLayout;
    number->addWidget(numberEdit = new YACReaderFieldEdit());
    numberValidator.setBottom(0);
    numberEdit->setValidator(&numberValidator);
    number->addWidget(new QLabel("Bis:"));
    number->addWidget(isBisCheck = new QCheckBox());
    number->addWidget(new QLabel(tr("of:")));
    number->addWidget(countEdit = new YACReaderFieldEdit());
    countValidator.setBottom(0);
    countEdit->setValidator(&countValidator);
    number->addStretch(1);
    /*generalInfoLayout->addRow(tr("&Issue number:"), );
        generalInfoLayout->addRow(tr("&Bis:"), );*/
    generalInfoLayout->addRow(tr("Issue number:"), number);

    generalInfoLayout->addRow(tr("Volume:"), volumeEdit = new YACReaderFieldEdit());

    auto arc = new QHBoxLayout;
    arc->addWidget(storyArcEdit = new YACReaderFieldEdit());
    arc->addWidget(new QLabel(tr("Arc number:")));
    arc->addWidget(arcNumberEdit = new YACReaderFieldEdit());
    arcNumberValidator.setBottom(0);
    arcNumberEdit->setValidator(&arcNumberValidator);
    arc->addWidget(new QLabel(tr("of:")));
    arc->addWidget(arcCountEdit = new YACReaderFieldEdit());
    arcCountValidator.setBottom(0);
    arcCountEdit->setValidator(&arcCountValidator);
    arc->addStretch(1);
    generalInfoLayout->addRow(tr("Story arc:"), arc);

    generalInfoLayout->addRow(tr("Genre:"), genereEdit = new YACReaderFieldEdit());

    generalInfoLayout->addRow(tr("Size:"), size = new QLabel("size"));

    // generalInfoLayout->addRow(tr("Comic Vine link:"), comicVineLink = new QLabel("..."));
    // generalInfoLayout->addRow(bottom);

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

    // authorsLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    auto h1 = new QHBoxLayout;
    auto vl1 = new QVBoxLayout;
    auto vr1 = new QVBoxLayout;
    vl1->addWidget(new QLabel(tr("Writer(s):")));
    vl1->addWidget(writer = new YACReaderFieldPlainTextEdit());
    h1->addLayout(vl1);
    vr1->addWidget(new QLabel(tr("Penciller(s):")));
    vr1->addWidget(penciller = new YACReaderFieldPlainTextEdit());
    h1->addLayout(vr1);
    // authorsLayout->addRow(tr("Writer(s):"), new YACReaderFieldPlainTextEdit());
    // authorsLayout->addRow(tr("Penciller(s):"), new YACReaderFieldPlainTextEdit());
    auto h2 = new QHBoxLayout;
    auto vl2 = new QVBoxLayout;
    auto vr2 = new QVBoxLayout;
    vl2->addWidget(new QLabel(tr("Inker(s):")));
    vl2->addWidget(inker = new YACReaderFieldPlainTextEdit());
    h2->addLayout(vl2);
    vr2->addWidget(new QLabel(tr("Colorist(s):")));
    vr2->addWidget(colorist = new YACReaderFieldPlainTextEdit());
    h2->addLayout(vr2);

    // authorsLayout->addRow(tr("Inker(s):"), new YACReaderFieldPlainTextEdit());
    // authorsLayout->addRow(tr("Colorist(s):"), new YACReaderFieldPlainTextEdit());

    auto h3 = new QHBoxLayout;
    auto vl3 = new QVBoxLayout;
    auto vr3 = new QVBoxLayout;
    vl3->addWidget(new QLabel(tr("Letterer(s):")));
    vl3->addWidget(letterer = new YACReaderFieldPlainTextEdit());
    h3->addLayout(vl3);
    vr3->addWidget(new QLabel(tr("Cover Artist(s):")));
    vr3->addWidget(coverArtist = new YACReaderFieldPlainTextEdit());
    h3->addLayout(vr3);
    // authorsLayout->addRow(tr("Letterer(es):"), new YACReaderFieldPlainTextEdit());
    // authorsLayout->addRow(tr("Cover Artist(s):"), new YACReaderFieldPlainTextEdit());

    authorsLayout->addLayout(h1);
    authorsLayout->addLayout(h2);
    authorsLayout->addLayout(h3);
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
    publishingLayout->addRow(tr("Manga:"), mangaCheck = new QCheckBox());

    publishingBox->setLayout(publishingLayout);
}

void PropertiesDialog::createPlotBox()
{
    plotBox = new QWidget;

    auto plotLayout = new QFormLayout;
    plotLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    plotLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    plotLayout->addRow(tr("Synopsis:"), synopsis = new YACReaderFieldPlainTextEdit());
    plotLayout->addRow(tr("Characters:"), characters = new YACReaderFieldPlainTextEdit());
    plotLayout->addRow(tr("Notes:"), notes = new YACReaderFieldPlainTextEdit());

    plotBox->setLayout(plotLayout);
}

void PropertiesDialog::createButtonBox()
{
    buttonBox = new QDialogButtonBox;

    closeButton = buttonBox->addButton(QDialogButtonBox::Close);
    saveButton = buttonBox->addButton(QDialogButtonBox::Save);
    // rotateWidgetsButton = buttonBox->addButton(tr("Rotate &Widgets"),QDialogButtonBox::ActionRole);

    // connect(rotateWidgetsButton, SIGNAL(clicked()), this, SLOT(rotateWidgets()));
    connect(closeButton, &QAbstractButton::clicked, this, &QWidget::close);
    connect(saveButton, &QAbstractButton::clicked, this, &PropertiesDialog::save);
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

void PropertiesDialog::setComics(QList<ComicDB> comics)
{
    this->comics = comics;

    ComicDB comic = comics.at(0);

    if (!comic.info.title.isNull())
        title->setText(comic.info.title.toString());
    if (!comic.info.comicVineID.isNull()) {
        comicVineLink->setHidden(false);
        comicVineLink->setText(QString(tr("Comic Vine link: <a style='color: #FFCB00; text-decoration:none; font-weight:bold;' href=\"http://www.comicvine.com/comic/4000-%1/\"> view </a>").arg(comic.info.comicVineID.toString())));
    } else
        comicVineLink->setHidden(true);

    if (comics.length() == 1 && !comic.info.coverPage.isNull()) {
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

        if (!QFileInfo(basePath + comics[0].path).exists()) {
            QMessageBox::warning(this, tr("Not found"), tr("Comic not found. You should update your library."));
            showPreviousCoverPageButton->setDisabled(true);
            showNextCoverPageButton->setDisabled(true);
        }
    }
    /*if(comic.info.numPages != NULL)
        numPagesEdit->setText(QString::number(*comic.info.numPages));*/

    if (!comic.info.number.isNull())
        numberEdit->setText(comic.info.number.toString());
    if (!comic.info.isBis.isNull())
        isBisCheck->setChecked(comic.info.isBis.toBool());
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

    mangaCheck->setChecked(comic.info.manga.toBool());

    if (!comic.info.ageRating.isNull())
        ageRatingEdit->setText(comic.info.ageRating.toString());

    if (!comic.info.synopsis.isNull())
        synopsis->setPlainText(comic.info.synopsis.toString());
    if (!comic.info.characters.isNull())
        characters->setPlainText(comic.info.characters.toString());
    if (!comic.info.notes.isNull())
        notes->setPlainText(comic.info.notes.toString());

    if (comics.length() > 1) {
        coverBox->hide();

        setDisableUniqueValues(true);
        this->setWindowTitle(tr("Edit selected comics information"));
        setMultipleCover();

        QList<ComicDB>::iterator itr;
        for (itr = ++comics.begin(); itr != comics.end(); itr++) {
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

            if (itr->info.genere.isNull() || itr->info.genere.toString() != genereEdit->text())
                genereEdit->clear();

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
            if (itr->info.manga.toBool() != colorCheck->isChecked())
                mangaCheck->setCheckState(Qt::PartiallyChecked);
            if (itr->info.ageRating.isNull() || itr->info.ageRating.toString() != ageRatingEdit->text())
                ageRatingEdit->clear();

            if (itr->info.synopsis.isNull() || itr->info.synopsis.toString() != synopsis->toPlainText())
                synopsis->clear();
            if (itr->info.characters.isNull() || itr->info.characters.toString() != characters->toPlainText())
                characters->clear();
            if (itr->info.notes.isNull() || itr->info.notes.toString() != notes->toPlainText())
                notes->clear();
        }
    } else {
        this->setWindowTitle(tr("Edit comic information"));
        setCover(comic.info.getCover(basePath));
    }
}

void PropertiesDialog::updateComics()
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        db.open();
        db.transaction();
        QList<ComicDB>::iterator itr;
        for (itr = comics.begin(); itr != comics.end(); itr++) {
            if (itr->info.edited)
                DBHelper::update(&(itr->info), db);
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
}

void PropertiesDialog::setCover(const QPixmap &coverI)
{
    coverImage = coverI.scaledToHeight(444, Qt::SmoothTransformation);
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
    for (itr = comics.begin(); itr != comics.end(); itr++) {
        // Comic & comic = comics[0];
        bool edited = false;

        if (title->isModified()) {
            auto titleString = title->text();
            itr->info.title = titleString.isEmpty() ? QVariant() : title->text();
            edited = true;
        }

        if (comics.size() == 1)
            if (coverChanged) {
                itr->info.coverPage = coverPageNumberLabel->text();
                edited = true;
            }

        /*if(comic.info.numPages != NULL)
                numPagesEdit->setText(QString::number(*comic.info.numPages));*/
        if (comics.size() == 1)
            if (numberEdit->isModified()) {
                if (numberEdit->text().isEmpty())
                    itr->info.number = QVariant();
                else
                    itr->info.number = numberEdit->text();
                edited = true;
            }
        if (comics.size() == 1)
            if (!itr->info.isBis.isNull() || isBisCheck->isChecked()) {
                itr->info.isBis = isBisCheck->isChecked();
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
        if (comics.size() == 1)
            if (arcNumberEdit->isModified() && !arcNumberEdit->text().isEmpty()) {
                itr->info.arcNumber = arcNumberEdit->text();
                edited = true;
            }
        if (arcCountEdit->isModified()) {
            itr->info.arcCount = arcCountEdit->text();
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

        if (mangaCheck->checkState() != Qt::PartiallyChecked) {
            itr->info.manga = mangaCheck->isChecked();
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
        if (notes->document()->isModified()) {
            itr->info.notes = notes->toPlainText();
            edited = true;
        }

        itr->info.edited = edited;
    }

    if (comics.count() == 1) {
        if (coverChanged) // && coverPageEdit->text().toInt() != *comics[0].info.coverPage)
        {
            InitialComicInfoExtractor ie(basePath + comics[0].path, basePath + "/.yacreaderlibrary/covers/" + comics[0].info.hash + ".jpg", comics[0].info.coverPage.toInt());
            ie.extract();

            if (ie.getOriginalCoverSize().second > 0) {
                comics[0].info.originalCoverSize = QString("%1x%2").arg(ie.getOriginalCoverSize().first).arg(ie.getOriginalCoverSize().second);
                comics[0].info.coverSizeRatio = static_cast<float>(ie.getOriginalCoverSize().first) / ie.getOriginalCoverSize().second;
            }
        }
    }

    updateComics();

    close();
    emit(accepted());
}

void PropertiesDialog::setDisableUniqueValues(bool disabled)
{
    coverPageEdit->setDisabled(disabled);
    coverPageEdit->clear();
    numberEdit->setDisabled(disabled);
    numberEdit->clear();
    isBisCheck->setDisabled(disabled);
    isBisCheck->setChecked(false);
    arcNumberEdit->setDisabled(disabled);
    arcNumberEdit->clear();
}

void PropertiesDialog::closeEvent(QCloseEvent *e)
{

    title->clear();
    title->setModified(false);
    coverPageEdit->clear();
    //			numPagesEdit->setText(QString::number(*comic.info.numPages));
    numberEdit->clear();
    isBisCheck->setChecked(false);
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
    mangaCheck->setChecked(false);
    ageRatingEdit->clear();
    synopsis->clear();
    characters->clear();
    notes->clear();

    setDisableUniqueValues(false);

    tabBar->setCurrentIndex(0);

    coverPageEdit->setFocus();

    QDialog::closeEvent(e);
}

void PropertiesDialog::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);

    QPainter p(this);

    p.drawPixmap(0, 0, coverImage);

    // QPixmap shadow(":/images/social_dialog/shadow.png");
    // p.drawPixmap(280-shadow.width(),0,shadow.width(),444,shadow);
    p.drawLine(279, 0, 279, 444);
    if (comics.length() == 1)
        p.fillRect(0, 444 - 28, 280, 28, QColor(0, 0, 0, 153));
}

void PropertiesDialog::updateCoverPageNumberLabel(int n)
{
    coverPageNumberLabel->setText(QString::number(n));
    coverPageNumberLabel->adjustSize();
}

void PropertiesDialog::loadNextCover()
{
    int current = coverPageNumberLabel->text().toInt();
    if (current < comics.at(0).info.numPages.toInt()) {
        updateCoverPageNumberLabel(current + 1);

        InitialComicInfoExtractor ie(basePath + comics[0].path, "", current + 1);
        ie.extract();
        setCover(ie.getCover());
        repaint();

        if ((current + 1) == comics.at(0).info.numPages.toInt()) {
            showNextCoverPageButton->setDisabled(true);
        }

        showPreviousCoverPageButton->setEnabled(true);
        // busyIndicator->show();
        if (current + 1 != comics.at(0).info.coverPage)
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
        InitialComicInfoExtractor ie(basePath + comics[0].path, "", current - 1);
        ie.extract();
        setCover(ie.getCover());
        repaint();

        if ((current - 1) == 1) {
            showPreviousCoverPageButton->setDisabled(true);
        }

        showNextCoverPageButton->setEnabled(true);
        // busyIndicator->show();
        if (current - 1 != comics.at(0).info.coverPage.toInt())
            coverChanged = true;
        else
            coverChanged = false;
    }
}
