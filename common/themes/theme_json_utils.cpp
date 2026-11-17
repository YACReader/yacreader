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

bool sameThemeValueType(const QJsonValue &reference, const QJsonValue &value)
{
    if (reference.isString() && isThemeHexColorString(reference.toString()))
        return value.isString() && isThemeHexColorString(value.toString());
    return reference.type() == value.type();
}

void collectMissingThemeKeys(const QJsonObject &reference, const QJsonObject &theme, const QString &prefix, QStringList &missing)
{
    for (auto it = reference.constBegin(); it != reference.constEnd(); ++it) {
        if (prefix.isEmpty() && it.key() == QLatin1String("meta"))
            continue;
        const QString path = prefix.isEmpty() ? it.key() : prefix + '.' + it.key();
        const QJsonValue value = theme.value(it.key());
        if (it.value().isObject())
            collectMissingThemeKeys(it.value().toObject(), value.toObject(), path, missing);
        else if (!sameThemeValueType(it.value(), value))
            missing.append(path);
    }
}

QJsonObject keepReferenceThemeKeys(const QJsonObject &reference, const QJsonObject &theme, bool topLevel)
{
    QJsonObject result;
    for (auto it = reference.constBegin(); it != reference.constEnd(); ++it) {
        if (topLevel && it.key() == QLatin1String("meta"))
            continue;
        const QJsonValue value = theme.value(it.key());
        if (it.value().isObject()) {
            result.insert(it.key(), keepReferenceThemeKeys(it.value().toObject(), value.toObject(), false));
        } else if (sameThemeValueType(it.value(), value)) {
            result.insert(it.key(), value);
        }
    }
    return result;
}

}

QStringList missingThemeKeys(const QJsonObject &reference, const QJsonObject &theme)
{
    QStringList missing;
    collectMissingThemeKeys(reference, theme, { }, missing);
    return missing;
}

QJsonObject referenceThemeKeys(const QJsonObject &reference, const QJsonObject &theme)
{
    QJsonObject result = keepReferenceThemeKeys(reference, theme, true);
    if (theme.contains(QLatin1String("meta")))
        result.insert(QLatin1String("meta"), theme.value(QLatin1String("meta")));
    return result;
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
