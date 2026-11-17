#ifndef THEME_JSON_UTILS_H
#define THEME_JSON_UTILS_H

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QStringList>

bool isThemeHexColorString(const QString &value);
// Key paths of the reference that are missing in the theme or have another type; "meta" is not checked.
QStringList missingThemeKeys(const QJsonObject &reference, const QJsonObject &theme);
// The theme with "meta" and only the reference keys that have the reference type.
QJsonObject referenceThemeKeys(const QJsonObject &reference, const QJsonObject &theme);
QJsonObject normalizeThemeJson(const QJsonObject &json);
QByteArray serializeNormalizedThemeJson(const QJsonObject &json,
                                        QJsonDocument::JsonFormat format = QJsonDocument::Indented);

#endif // THEME_JSON_UTILS_H
