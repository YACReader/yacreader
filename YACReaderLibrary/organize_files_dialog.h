#ifndef ORGANIZE_FILES_DIALOG_H
#define ORGANIZE_FILES_DIALOG_H

#include <QDialog>

class QLineEdit;
class QLabel;
class QCheckBox;
class QSettings;

// Dialog that lets the user define the path/name format used to organize comic
// files on disk. The format is a path template where each path segment becomes a
// directory, except the last one which becomes the file name (the original
// extension is kept).
//
// Supported tokens: {publisher} {series} {number} {title} {volume} {year}
// {title} falls back to {series} when the comic has no title.
class OrganizeFilesDialog : public QDialog
{
    Q_OBJECT
public:
    // libraryRoot and selectedFolderPath are absolute paths used to render a
    // realistic preview and to reflect the "relative to library root" toggle.
    // settings persists that toggle across runs (may be null).
    explicit OrganizeFilesDialog(const QString &libraryRoot,
                                 const QString &selectedFolderPath,
                                 QSettings *settings = nullptr,
                                 QWidget *parent = nullptr);

    // Returns the format pattern entered by the user.
    QString formatPattern() const;

    // Whether the destination should be rooted at the library root (true) or at
    // the currently selected folder (false).
    bool relativeToRoot() const;

    // Default format used when none has been configured yet.
    static QString defaultPattern();

    // Builds the relative destination path (directories + file name, including
    // the given extension) for a comic, applying token substitution and
    // sanitizing every path segment. The extension should include the leading
    // dot (e.g. ".cbz"); pass an empty string for none.
    static QString buildRelativePath(const QString &pattern,
                                     const QString &publisher,
                                     const QString &series,
                                     const QString &number,
                                     const QString &title,
                                     const QString &volume,
                                     const QString &year,
                                     const QString &extension);

private slots:
    void updatePreview();

private:
    QLineEdit *patternEdit;
    QLabel *previewLabel;
    QCheckBox *relativeToRootCheck;

    QString libraryRoot;
    QString selectedFolderPath;
    QSettings *settings;

    void setupUI();
};

#endif // ORGANIZE_FILES_DIALOG_H
