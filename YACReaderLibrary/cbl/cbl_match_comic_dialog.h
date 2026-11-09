#ifndef CBL_MATCH_COMIC_DIALOG_H
#define CBL_MATCH_COMIC_DIALOG_H

#include "cbl_matcher.h"

#include <QDialog>

#include <optional>

class CblFindComicPage;
class QLabel;
class QStackedWidget;
struct DescriptiveComicMetadata;

// chooses the library comic of one missing entry or deleted comic, with the Find comic page of the import; with a
// folder, only among the comics of that folder
class CblMatchComicDialog : public QDialog
{
    Q_OBJECT

public:
    CblMatchComicDialog(const DescriptiveComicMetadata &metadata, const QString &libraryPath, std::optional<qulonglong> folderId = std::nullopt, QWidget *parent = nullptr);

    // set when the dialog is accepted
    std::optional<CblLibraryComic> chosenComic() const;

private:
    QStackedWidget *pages;
    QLabel *messageLabel;
    CblFindComicPage *findPage;
    std::optional<CblLibraryComic> chosen;
};

#endif
