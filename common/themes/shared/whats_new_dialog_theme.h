#ifndef WHATS_NEW_DIALOG_THEME_H
#define WHATS_NEW_DIALOG_THEME_H

#include <QColor>
#include <QPixmap>

struct WhatsNewDialogTheme {
    QColor backgroundColor;
    QColor headerTextColor;
    QColor versionTextColor;
    QColor contentTextColor;
    QColor linkColor;
    QPixmap closeButtonIcon;
    QPixmap headerDecoration;
};

#endif // WHATS_NEW_DIALOG_THEME_H
