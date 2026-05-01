#ifndef THEME_JSON_UTILS_H
#define THEME_JSON_UTILS_H

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

bool isThemeHexColorString(const QString &value);
QJsonObject normalizeThemeJson(const QJsonObject &json);
QByteArray serializeNormalizedThemeJson(const QJsonObject &json,
                                        QJsonDocument::JsonFormat format = QJsonDocument::Indented);

#endif // THEME_JSON_UTILS_H
