#ifndef YACREADER_LIBRARY_SEARCH_EDIT_H
#define YACREADER_LIBRARY_SEARCH_EDIT_H

#include <QObject>

#include "yacreader_global.h"

#include "yacreader_search_line_edit.h"

#ifdef Q_OS_MAC
#include "yacreader_macosx_toolbar.h"
#endif

class YACReaderLibrarySearchEdit: public QObject
{
        Q_OBJECT
public:
#ifdef Q_OS_MAC
    YACReaderLibrarySearchEdit(YACReaderMacOSXSearchLineEdit *searchEdit);
#endif
    YACReaderLibrarySearchEdit(YACReaderSearchLineEdit *searchEdit);

public slots:
    void clear();
    void clearText();
    void setDisabled(bool disabled);
    void setEnabled(bool enabled);
    QString text();

signals:
    void filterChanged(const YACReader::SearchModifiers, QString);

private:
#ifdef Q_OS_MAC
    YACReaderMacOSXSearchLineEdit *macosSearchEdit;
#endif
    YACReaderSearchLineEdit *defaultSearchEdit;
};

#endif // YACREADER_LIBRARY_SEARCH_EDIT_H
