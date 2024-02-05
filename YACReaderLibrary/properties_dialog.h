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

#include "comic_db.h"

class PropertiesDialog : public QDialog
{
    Q_OBJECT
private:
    QWidget *mainWidget;
    // YACReaderBusyWidget * busyIndicator;

    QGridLayout *mainLayout;

    QTabWidget *tabBar;

    QWidget *coverBox;
    QLabel *cover;
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

    QPixmap coverImage;

    QToolButton *showPreviousCoverPageButton;
    QToolButton *showNextCoverPageButton;
    QLabel *coverPageNumberLabel;

    void createTabBar();
    void createCoverBox();
    void createGeneralInfoBox();
    void createAuthorsBox();
    void createPublishingBox();
    void createPlotBox();
    void createNotesBox();

    void createButtonBox();

    void setDisableUniqueValues(bool disabled);

    QList<ComicDB> comics;
    int currentComicIndex;
    void closeEvent(QCloseEvent *e) override;
    void updateCoverPageNumberLabel(int n);
    void loadComic(ComicDB &comic);
    void updateButtons();

    bool sequentialEditing;
    bool coverChanged;
    float coverSizeRatio;
    bool updated;
    QString originalCoverSize;

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
    void setComics(QList<ComicDB> comics);
    void setComicsForSequentialEditing(int currentComicIndex, QList<ComicDB> comics);
    void updateComics();
    void save();
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
    bool close();

signals:
    void coverChangedSignal(const ComicDB &comic);
};
#endif
