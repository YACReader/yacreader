#ifndef __PROPERTIES_DIALOG_H
#define __PROPERTIES_DIALOG_H

#include <QDialog>
#include <QIntValidator>

class QGridLayout;
class QTabWidget;
class QGroupBox;
class QLabel;
class QScrollArea;
class QWidget;
class YACReaderFieldEdit;
class YACReaderFieldPlainTextEdit;
class QDialogButtonBox;
class QCheckBox;
class QComboBox;
// class YACReaderBusyWidget;
class QToolButton;

namespace YACReader {
class CoverLabel;
}

#include "comic_record.h"
#include "descriptive_comic_metadata.h"
#include "yacreader_global.h"

#include <optional>

class PropertiesDialog : public QDialog
{
    Q_OBJECT
private:
    QWidget *mainWidget;
    // YACReaderBusyWidget * busyIndicator;

    QGridLayout *mainLayout;

    QTabWidget *tabBar;

    QWidget *coverBox;
    YACReader::CoverLabel *cover;
    QScrollArea *sa;

    QWidget *generalInfoBox;
    YACReaderFieldEdit *series;
    YACReaderFieldEdit *title;
    YACReaderFieldEdit *numPagesEdit;
    QLabel *size;
    QLabel *comicVineLink;

    YACReaderFieldEdit *coverPageEdit;
    QIntValidator coverPageValidator;

    YACReaderFieldEdit *numberEdit;
    QIntValidator numberValidator;
    YACReaderFieldEdit *countEdit;
    QIntValidator countValidator;

    YACReaderFieldEdit *volumeEdit;
    YACReaderFieldEdit *storyArcEdit;
    YACReaderFieldEdit *arcNumberEdit;
    QIntValidator arcNumberValidator;
    YACReaderFieldEdit *arcCountEdit;
    QIntValidator arcCountValidator;

    YACReaderFieldEdit *alternateSeriesEdit;
    YACReaderFieldEdit *alternateNumberEdit;
    YACReaderFieldEdit *alternateCountEdit;

    YACReaderFieldEdit *seriesGroupEdit;

    YACReaderFieldEdit *genereEdit;

    YACReaderFieldPlainTextEdit *writer;
    YACReaderFieldPlainTextEdit *penciller;
    YACReaderFieldPlainTextEdit *inker;
    YACReaderFieldPlainTextEdit *colorist;
    YACReaderFieldPlainTextEdit *letterer;
    YACReaderFieldPlainTextEdit *coverArtist;
    YACReaderFieldPlainTextEdit *editor;
    YACReaderFieldPlainTextEdit *imprint;

    YACReaderFieldEdit *dayEdit;
    QIntValidator dayValidator;
    YACReaderFieldEdit *monthEdit;
    QIntValidator monthValidator;
    YACReaderFieldEdit *yearEdit;
    QIntValidator yearValidator;
    YACReaderFieldEdit *publisherEdit;
    YACReaderFieldEdit *formatEdit;
    QCheckBox *colorCheck;
    YACReaderFieldEdit *ageRatingEdit;
    QComboBox *typeCombo;
    YACReaderFieldEdit *languageEdit;

    YACReaderFieldPlainTextEdit *synopsis;
    YACReaderFieldPlainTextEdit *characters;
    YACReaderFieldPlainTextEdit *teams;
    YACReaderFieldPlainTextEdit *locations;
    YACReaderFieldEdit *mainCharacterOrTeamEdit;

    QWidget *authorsBox;

    QWidget *publishingBox;

    QWidget *plotBox;

    QWidget *notesBox;

    YACReaderFieldPlainTextEdit *review;
    YACReaderFieldPlainTextEdit *notes;
    YACReaderFieldPlainTextEdit *tags;

    QDialogButtonBox *buttonBox;
    QPushButton *closeButton;
    QPushButton *saveButton;
    QPushButton *nextButton;
    QPushButton *previousButton;
    QPushButton *restoreButton; //??

    QToolButton *showPreviousCoverPageButton;
    QToolButton *showNextCoverPageButton;
    QLabel *coverPageNumberLabel;

    QToolButton *resetCoverButton;
    QToolButton *loadCustomCoverImageButton;

    QLabel *deletedComicNote;

    void createTabBar();
    void createCoverBox();
    void createGeneralInfoBox();
    void createAuthorsBox();
    void createPublishingBox();
    void createPlotBox();
    void createNotesBox();

    void createButtonBox();

    void setDisableUniqueValues(bool disabled);

    QList<ComicRecord> records;
    int currentComicIndex;
    void closeEvent(QCloseEvent *e) override;
    void updateCoverPageNumberLabel(int n);
    void loadComic(const ComicRecord &record);
    QPixmap coverOf(const ComicRecord &record) const;
    const ComicDB *lastComicWithFile() const;
    void updateButtons();
    DescriptiveMetadataPatch editedMetadata() const;
    std::optional<YACReader::FileType> editedType() const;
    void applyCoverChange(ComicDB &comic);
    // false when the save was refused; the dialog then keeps the values
    bool updateComics();

    bool sequentialEditing;
    bool coverChanged;
    float coverSizeRatio;
    bool updated;
    Qt::CheckState loadedColorState;
    int loadedTypeIndex;
    QString originalCoverSize;

    QImage customCover;

public:
    PropertiesDialog(QWidget *parent = nullptr);
    QString databasePath;
    QString basePath;
    // TODO: this non-const member function hides rather than overrides
    // QWidget::sizeHint(). But the function cannot be simply removed as it is used
    // in our constructor. Will have to investigate and decide how to fix this.
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent *event) override;

public slots:
    void setComics(QList<ComicRecord> records);
    void setComicsForSequentialEditing(int currentComicIndex, QList<ComicRecord> records);
    void saveAndOpenPrevious();
    void saveAndOpenNext();
    void saveAndClose();
    // Deprecated
    void setCover(const QPixmap &cover);
    void setMultipleCover();
    void setFilename(const QString &name);
    void setNumpages(int pages);
    void setSize(float size);
    void loadNextCover();
    void loadPreviousCover();
    void resetCover();
    void loadCustomCoverImage();
    void loadCustomCoverImageFromPath(const QString &path);
    void setCoverPage(int pageNumber);
    bool close();
    void updateCoverBoxForMultipleComics();
    void updateCoverBoxForSingleComic();

signals:
    void coverChangedSignal(const ComicDB &comic);
};
#endif
