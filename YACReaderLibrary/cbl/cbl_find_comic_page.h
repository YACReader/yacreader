#ifndef CBL_FIND_COMIC_PAGE_H
#define CBL_FIND_COMIC_PAGE_H

#include "cbl_import_model.h"
#include "comic_db.h"

#include <QWidget>

class QCheckBox;
class QLabel;
class QLineEdit;
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
    CblFindComicPage(const QString &libraryPath, QWidget *parent = nullptr);

    void setFolders(const QList<CblLibraryFolder> &folders);
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
