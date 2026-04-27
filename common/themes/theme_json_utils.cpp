#include "theme_json_utils.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QRegularExpression>

namespace {

QJsonValue normalizeThemeJsonValue(const QJsonValue &value)
{
    if (value.isObject())
        return normalizeThemeJson(value.toObject());

    if (value.isArray()) {
        QJsonArray normalizedArray = value.toArray();
        for (int i = 0; i < normalizedArray.size(); ++i)
            normalizedArray[i] = normalizeThemeJsonValue(normalizedArray.at(i));
        return normalizedArray;
    }

    if (value.isString()) {
        const QString stringValue = value.toString();
        if (isThemeHexColorString(stringValue))
            return stringValue.toUpper();
    }

    return value;
}

}

bool isThemeHexColorString(const QString &value)
{
    static const QRegularExpression colorRegex(
            "^#(?:[0-9A-Fa-f]{3}|[0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})$");
    return colorRegex.match(value).hasMatch();
}

QJsonObject normalizeThemeJson(const QJsonObject &json)
{
    QJsonObject normalized;
    for (auto it = json.constBegin(); it != json.constEnd(); ++it)
        normalized.insert(it.key(), normalizeThemeJsonValue(it.value()));
    return normalized;
}

QByteArray serializeNormalizedThemeJson(const QJsonObject &json, QJsonDocument::JsonFormat format)
{
    return QJsonDocument(normalizeThemeJson(json)).toJson(format);
}
