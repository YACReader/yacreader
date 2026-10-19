#ifndef CBL_IMPORT_DIALOG_H
#define CBL_IMPORT_DIALOG_H

#include "cbl_import_model.h"
#include "library_ids.h"

#include <QDialog>

#include <memory>

class CblEntriesView;
class CblEntryDetail;
class CblFindComicPage;
class QButtonGroup;
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;

struct CblImportFile;

// imports the files one after the other; each Import writes the reading list of the current file
class CblImportDialog : public QDialog
{
    Q_OBJECT

public:
    CblImportDialog(const QStringList &filePaths, const QString &libraryPath, QWidget *parent = nullptr);
    ~CblImportDialog() override;

signals:
    void readingListImported(ReadingListId id, const QString &name);

private:
    QWidget *createMessagePage();
    QWidget *createEntriesPage();
    void loadCurrentFile();
    void showFile(const CblImportFile &file);
    void showMessage(const QString &message);
    void showEntries();
    void showFind(int row);
    // the next or previous Ambiguous or Missing entry; false when there is none
    bool showProblem(bool forward);
    void createDetail(const QModelIndex &index);
    void alignDetail(CblEntryDetail *detail) const;
    void importCurrentFile();
    void nextFile();
    void updateTitle();
    void updateCounts();
    void updateNameConflict();
    void showEntriesMenu(const QPoint &position);
    QList<int> selectedRows() const;
    QColor warningColor() const;

    QStringList filePaths;
    QString libraryPath;
    int currentFile = 0;
    // the entry of the Find comic page
    int findRow = -1;
    std::unique_ptr<CblImportFile> file;

    QStackedWidget *pages;
    QWidget *messagePage;
    QLabel *messageLabel;
    QWidget *entriesPage;
    QLineEdit *nameEdit;
    QLabel *nameConflictLabel;
    QButtonGroup *filterButtons;
    QList<QPushButton *> filterButtonList;
    QLineEdit *filterEdit;
    CblEntriesView *entriesView;
    CblFindComicPage *findPage;
    QWidget *bottomBar;
    CblImportModel *model;
    CblImportFilterModel *filterModel;
    QLabel *summaryLabel;
    QPushButton *closeButton;
    QPushButton *skipButton;
    QPushButton *importButton;
};

#endif
