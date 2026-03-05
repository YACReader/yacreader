#ifndef THEME_META_H
#define THEME_META_H

#include "theme_variant.h"

#include <QString>

struct ThemeMeta {
    QString id;
    QString displayName;
    ThemeVariant variant;
};

#endif // THEME_META_H
