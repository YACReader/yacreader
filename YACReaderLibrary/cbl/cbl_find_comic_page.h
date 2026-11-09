#ifndef CBL_FIND_COMIC_PAGE_H
#define CBL_FIND_COMIC_PAGE_H

#include "cbl_import_model.h"
#include "comic_db.h"

#include <QWidget>

#include <optional>

class CblMatcher;
class QCheckBox;
class QLabel;
class QLineEdit;
class QSqlDatabase;
class QPushButton;
class QStandardItem;
class QStandardItemModel;
class QTimer;
class QTreeView;

struct CblLibraryFolder {
    qulonglong id = 0;
    qulonglong parentId = 0;
    QString name;
    QString path;
};

// browse the folders or search the library to choose the comic of one entry
class CblFindComicPage : public QWidget
{
    Q_OBJECT

public:
    // OneEntry has no entry list to go back to and no other problems; backRequested is its Cancel
    enum class Mode {
        ImportEntries,
        OneEntry,
    };

    CblFindComicPage(const QString &libraryPath, Mode mode = Mode::ImportEntries, QWidget *parent = nullptr);

    // every folder but the root, sorted by name
    static std::optional<QList<CblLibraryFolder>> loadFolders(QSqlDatabase &db);
    // the folder of the comic the entry refers to, else of its first possible comic, else of the closest series
    static QString startFolderPath(const CblImportEntry &entry, const CblMatcher &matcher);

    void setFolders(const QList<CblLibraryFolder> &folders);
    // only the comics of the folder are shown and searched, without the folder tree; after setFolders
    void limitToFolder(qulonglong folderId);
    void setWarningColor(const QColor &color);
    // `problem` is the position of the entry among the problems, or -1 when it is not one
    void showEntry(const CblImportEntry &entry, int row, int entryCount, CblImportStatus status, const QString &folderPath, int problem, int problemCount, bool hasPrevious, bool hasNext);

signals:
    void backRequested();
    void previousProblemRequested();
    void nextProblemRequested();
    void comicChosen(const CblLibraryComic &comic, bool goToNextProblem);

private:
    void showFolder(qulonglong folderId);
    void search();
    void showComics(const QList<ComicDB> &comics);
    void chooseSelectedComic();
    QString breadcrumb(QStandardItem *folder) const;

    QString libraryPath;
    Mode mode;
    std::optional<qulonglong> onlyFolderId;
    CblImportEntry entry;
    QColor warningColor;
    QList<ComicDB> comics;

    QLabel *entryLabel;
    QLabel *problemLabel;
    QLabel *problemCountLabel;
    QPushButton *previousButton;
    QPushButton *nextButton;
    QTreeView *foldersView;
    QStandardItemModel *foldersModel;
    QHash<qulonglong, QStandardItem *> folderItems;
    QHash<QString, qulonglong> folderIdsByPath;
    QLabel *breadcrumbLabel;
    QLineEdit *searchEdit;
    QTimer *searchTimer;
    QTreeView *comicsView;
    QStandardItemModel *comicsModel;
    QCheckBox *nextProblemCheck;
    QPushButton *useButton;
};

#endif
