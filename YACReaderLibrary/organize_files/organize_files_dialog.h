#ifndef ORGANIZE_FILES_DIALOG_H
#define ORGANIZE_FILES_DIALOG_H

#include "organize_files_plan.h"
#include "organize_files_worker.h"

#include <QDialog>
#include <QHash>
#include <QList>

#include <functional>

class QCheckBox;
class QCloseEvent;
class QLabel;
class QLineEdit;
class QListWidget;
class QMenu;
class QProgressBar;
class QPushButton;
class QResizeEvent;
class QSettings;
class QStackedWidget;
class QToolButton;
class QThread;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;

class OrganizeFilesDialog : public QDialog
{
    Q_OBJECT
public:
    struct Context {
        OrganizeFiles::Mode mode = OrganizeFiles::Mode::Organize;
        QString libraryPath;
        QString folderPath;
        QList<OrganizeFiles::ComicEntry> entries;
    };

    // Both run on a worker thread and must not touch the GUI.
    using Applier = std::function<bool(const QList<OrganizeFiles::FileMove> &moves, const QStringList &removedDirectories, const QString &journalPath, QString *error)>;
    using Undoer = std::function<bool(const QString &journalPath,
                                      QList<OrganizeFiles::FileFailure> *failures,
                                      QString *error,
                                      const std::function<void(int done, int total, const QString &currentFile)> &fileProgress,
                                      const std::function<void()> &databasePhase)>;

    OrganizeFilesDialog(const Context &context, QSettings *settings, QWidget *parent = nullptr);
    ~OrganizeFilesDialog() override;

    void setApplier(Applier applier);
    void setUndoer(Undoer undoer);

    bool libraryChanged() const;

private slots:
    void patternEdited();
    void scheduleBuild();
    void startBuild();
    void planBuilt(const QList<OrganizeFiles::PlannedMove> &moves, quint64 buildGeneration);
    void itemChanged(QTreeWidgetItem *item, int column);
    void removeSelectedItems();
    void resetOverrides();
    void updateSelectionState();
    void startMove();
    void moveProgress(int done, int total, const QString &currentFile);
    void showUpdatingLibrary();
    void moveFinished();
    void undo();
    void undoProgress(int done, int total, const QString &currentFile);
    void undoFinished();
    void copyFailures();
    void wrapSelectionInOptionalGroup();
    void showFormatHelp();
    void saveCurrentPatternAsPreset();

public slots:
    void reject() override;
    void done(int result) override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void buildRequested(const QString &pattern, const QString &base, const OrganizeFiles::Overrides &overrides, quint64 generation);

private:
    void setupPages();
    QWidget *createLoadingPage();
    QWidget *createPlanPage();
    QWidget *createWorkingPage();
    QWidget *createResultPage();
    void setupPlanWorker();

    // Every input that changes what the run would do goes through this: it arms
    // the rebuild and keeps Move files refused until the rebuild lands.
    void markPlanStale(int delayMs);

    bool renaming() const;
    QString currentBase() const;
    void updateBasePathLabel();
    void rebuildTree();
    void updateStatusLine();
    void captureOverrides(QTreeWidgetItem *item);
    QString relativePathForItem(QTreeWidgetItem *item) const;
    bool isFileItem(QTreeWidgetItem *item) const;
    void collectFileItems(QTreeWidgetItem *item, QList<QTreeWidgetItem *> &out) const;
    QList<OrganizeFiles::FileMove> movesToExecute() const;
    void showCompletedMoves(const QList<OrganizeFiles::FileMove> &moves, const QList<OrganizeFiles::FileFailure> &failures = { }, bool restored = false);
    void showFailures(const QList<OrganizeFiles::FileFailure> &failures);
    void saveSettings();
    QString presetsKey() const;
    QList<QPair<QString, QString>> userPresets() const;
    void saveUserPresets(const QList<QPair<QString, QString>> &presets);
    void rebuildPresetsMenu();

    Context context;
    QSettings *settings;

    Applier applier;
    Undoer undoer;

    QStackedWidget *pages;

    QLineEdit *patternEdit;
    QLabel *patternError;
    QMenu *presetsMenu;
    QPushButton *folderBaseButton;
    QPushButton *rootBaseButton;
    QWidget *baseSelector;
    QLabel *basePathLabel;
    QLabel *overridesBanner;
    QPushButton *resetButton;
    QPushButton *removeButton;
    QCheckBox *showUnchangedCheck;
    QTreeWidget *tree;
    QLabel *statusLabel;
    QLabel *warningLabel;
    QPushButton *moveButton;
    QPushButton *cancelButton;

    QLabel *loadingLabel;

    QProgressBar *progressBar;
    QLabel *progressLabel;

    QLabel *resultLabel;
    QTreeWidget *resultTree;
    QListWidget *failureList;
    QPushButton *copyFailuresButton;
    QPushButton *undoButton;
    QPushButton *finishButton;

    QTimer *buildTimer;
    QThread *planThread;
    OrganizeFiles::PlanWorker *planWorker;
    QThread *moveThread;
    OrganizeFiles::MoveWorker *moveWorker;
    QThread *undoThread;
    OrganizeFiles::UndoWorker *undoWorker;

    OrganizeFiles::Overrides overrides;
    QList<OrganizeFiles::PlannedMove> plan;
    QHash<QString, QString> planDestinations;
    // Valid for the dialog's whole life: nothing on disk moves until commit.
    QHash<QString, bool> folderExistsCache;

    QString lastJournalPath;
    QList<OrganizeFiles::FileMove> lastRequestedMoves;
    QList<OrganizeFiles::FileMove> lastCompletedMoves;

    int newFolderCount = 0;

    quint64 generation;
    bool updatingTree;
    bool changedLibrary;
    bool patternIsValid;
    bool moveRunning;
    bool undoRunning;
    bool planIsStale;
};

#endif // ORGANIZE_FILES_DIALOG_H
