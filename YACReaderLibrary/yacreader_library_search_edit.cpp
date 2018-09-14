#include "yacreader_library_search_edit.h"

#ifdef Q_OS_MAC
YACReaderLibrarySearchEdit::YACReaderLibrarySearchEdit(YACReaderMacOSXSearchLineEdit *searchEdit)
    :macosSearchEdit(searchEdit), defaultSearchEdit(nullptr)
{
    connect(searchEdit,
            &YACReaderMacOSXSearchLineEdit::filterChanged,
            this,
            &YACReaderLibrarySearchEdit::filterChanged);
}
#endif

YACReaderLibrarySearchEdit::YACReaderLibrarySearchEdit(YACReaderSearchLineEdit *searchEdit)
    :defaultSearchEdit(searchEdit)
{
    connect(searchEdit,
            &YACReaderSearchLineEdit::filterChanged,
            this,
            &YACReaderLibrarySearchEdit::filterChanged);
}

void YACReaderLibrarySearchEdit::clear()
{
    if (defaultSearchEdit != nullptr) {
        defaultSearchEdit->clear();
    } else {
        #ifdef Q_OS_MAC
        macosSearchEdit->clear();
        #endif
    }
}

void YACReaderLibrarySearchEdit::clearText()
{
    if (defaultSearchEdit != nullptr) {
        defaultSearchEdit->clearText();
    } else {
        #ifdef Q_OS_MAC
        macosSearchEdit->clearText();
        #endif
    }
}

void YACReaderLibrarySearchEdit::setDisabled(bool disabled)
{
    if (defaultSearchEdit != nullptr) {
        defaultSearchEdit->setDisabled(disabled);
    } else {
        #ifdef Q_OS_MAC
        macosSearchEdit->setDisabled(disabled);
        #endif
    }
}

void YACReaderLibrarySearchEdit::setEnabled(bool enabled)
{
    if (defaultSearchEdit != nullptr) {
        defaultSearchEdit->setEnabled(enabled);
    } else {
        #ifdef Q_OS_MAC
        macosSearchEdit->setEnabled(enabled);
        #endif
    }
}

QString YACReaderLibrarySearchEdit::text()
{
    if (defaultSearchEdit != nullptr) {
        return defaultSearchEdit->text();
    } else {
        #ifdef Q_OS_MAC
        return macosSearchEdit->text();
        #endif
    }

    return "";
}
