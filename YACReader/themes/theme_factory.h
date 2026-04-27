#ifndef THEME_FACTORY_H
#define THEME_FACTORY_H

#include "theme.h"

#include <QJsonObject>

Theme makeTheme(const QJsonObject &json);

#endif // THEME_FACTORY_H
