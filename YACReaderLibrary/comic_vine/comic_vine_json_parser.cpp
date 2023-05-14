
#include "comic_vine_json_parser.h"

#include "comic_vine_client.h"

#include <QJsonDocument>
#include <QJsonParseError>

QString getCharacters(const QVariant &json_characters);
QMultiMap<QString, QString> getAuthors(const QVariant &json_authors);
QPair<QString, QString> getFirstStoryArcIdAndName(const QVariant &json_story_arcs);
QPair<QString, QString> getArcNumberAndArcCount(const QString &storyArcId, const QString &comicId);
QList<QString> getNamesFromList(const QVariant &json_list);

ComicDB YACReader::parseCVJSONComicInfo(ComicDB &comic, const QString &json, int count, const QString &publisher)
{
    QJsonParseError Err;

    QVariantMap sc = QJsonDocument::fromJson(json.toUtf8(), &Err).toVariant().toMap();
    if (Err.error != QJsonParseError::NoError) {
        qDebug("Error detected");
        return comic;
    }

    int numResults = sc.value("number_of_total_results").toInt(); // fix to weird behaviour using hasNext

    if (numResults > 0) {
        QVariantMap result = sc.value("results").toMap();
        comic.info.title = result.value("name");
        comic.info.number = result.value("issue_number");

        // changed in 9.13, volume actually means series in ComicVine
        comic.info.series = result.value("volume").toMap().value("name");

        if (result.contains("person_credits") && !result.value("person_credits").isNull()) {
            auto authors = getAuthors(result.value("person_credits"));

            QString writer = authors.values("writer").join("\n");
            QString penciller = authors.values("penciller").join("\n");
            QString inker = authors.values("inker").join("\n");
            QString colorist = authors.values("colorist").join("\n");
            QString letterer = authors.values("letterer").join("\n");
            QString coverArtist = authors.values("cover").join("\n");

            comic.info.writer = writer;
            comic.info.penciller = penciller;
            comic.info.inker = inker;
            comic.info.colorist = colorist;
            comic.info.letterer = letterer;
            comic.info.coverArtist = coverArtist;
        }

        if (result.contains("cover_date") && !result.value("cover_date").isNull()) {
            QString date = result.value("cover_date").toString();

            QStringList tempList = date.split("-");

            if (tempList.length() == 3) {
                std::reverse(tempList.begin(), tempList.end());
                comic.info.date = tempList.join("/");
            }
        }

        if (result.contains("description") && !result.value("description").isNull()) {
            comic.info.synopsis = result.value("description");
        }

        if (result.contains("character_credits") && !result.value("character_credits").isNull()) {
            comic.info.characters = getCharacters(result.value("character_credits"));
        }

        if (result.contains("story_arc_credits") && !result.value("story_arc_credits").isNull()) {
            QPair<QString, QString> storyArcIdAndName = getFirstStoryArcIdAndName(result.value("story_arc_credits"));
            QString storyArcId = storyArcIdAndName.first;
            QString storyArcName = storyArcIdAndName.second;
            if (!storyArcId.isNull()) {

                QString comicId = result.value("id").toString();

                QPair<QString, QString> arcNumberAndArcCount = getArcNumberAndArcCount(storyArcId, comicId);
                if (!arcNumberAndArcCount.first.isNull()) {
                    QString arcNumber = arcNumberAndArcCount.first;
                    QString arcCount = arcNumberAndArcCount.second;

                    comic.info.storyArc = storyArcName;
                    comic.info.arcNumber = arcNumber;
                    comic.info.arcCount = arcCount;
                }
            }
        }

        if (result.contains("location_credits") && !result.value("location_credits").isNull()) {
            comic.info.locations = getNamesFromList(result.value("location_credits")).join("\n");
        }

        if (result.contains("team_credits") && !result.value("team_credits").isNull()) {
            comic.info.teams = getNamesFromList(result.value("team_credits")).join("\n");
        }

        if (result.contains("character_credits") && !result.value("character_credits").isNull()) {
            comic.info.characters = getNamesFromList(result.value("character_credits")).join("\n");
        }

        comic.info.count = count;

        comic.info.publisher = publisher;

        comic.info.edited = true;
    }

    return comic;
}

QString getCharacters(const QVariant &json_characters)
{
    QStringList characters;

    QListIterator<QVariant> it(json_characters.toList());
    QVariantMap resultsValue;
    while (it.hasNext()) {
        resultsValue = it.next().toMap();

        characters << resultsValue.value("name").toString();
    }

    return (characters.isEmpty()) ? "" : (characters.join("\n") + "\n");
}

QMultiMap<QString, QString> getAuthors(const QVariant &json_authors)
{
    QMultiMap<QString, QString> authors;

    QListIterator<QVariant> it(json_authors.toList());
    QVariantMap resultsValue;
    while (it.hasNext()) {
        resultsValue = it.next().toMap();

        QString authorName = resultsValue.value("name").toString();

        QStringList roles = resultsValue.value("role").toString().split(",");
        foreach (QString role, roles) {
            if (role.trimmed() == "writer")
                authors.insert("writer", authorName);
            else if (role.trimmed() == "inker")
                authors.insert("inker", authorName);
            else if (role.trimmed() == "penciler" || role.trimmed() == "penciller")
                authors.insert("penciller", authorName);
            else if (role.trimmed() == "colorist")
                authors.insert("colorist", authorName);
            else if (role.trimmed() == "letterer")
                authors.insert("letterer", authorName);
            else if (role.trimmed() == "cover")
                authors.insert("cover", authorName);
        }
    }

    return authors;
}

QPair<QString, QString> getFirstStoryArcIdAndName(const QVariant &json_story_arcs)
{
    QString story_arc_id = QString();
    QString story_arc_name = QString();

    QListIterator<QVariant> it(json_story_arcs.toList());
    QVariantMap resultsValue;
    while (it.hasNext()) {
        resultsValue = it.next().toMap();
        story_arc_id = resultsValue.value("id").toString();
        story_arc_name = resultsValue.value("name").toString();
        break;
    }
    return qMakePair(story_arc_id, story_arc_name);
}

QPair<QString, QString> getArcNumberAndArcCount(const QString &storyArcId, const QString &comicId)
{
    auto comicVineClient = new ComicVineClient;
    bool error;
    bool timeout;
    QByteArray result = comicVineClient->getStoryArcDetail(storyArcId, error, timeout);
    if (error || timeout)
        return qMakePair(QString(), QString());
    QString json = result;

    QJsonParseError Err;
    QVariantMap sc = QJsonDocument::fromJson(json.toUtf8(), &Err).toVariant().toMap();

    if (Err.error != QJsonParseError::NoError) {
        qDebug("Error detected");
        return qMakePair(QString(), QString());
    }

    int numResults = sc.value("number_of_total_results").toInt(); // fix to weird behaviour using hasNext

    if (numResults > 0) {
        QVariantMap result = sc.value("results").toMap();

        if (result.contains("issues")) {
            QListIterator<QVariant> it(result.value("issues").toList());
            int arcNumber = 0;
            int arcCount = 0;

            QVariantMap resultsValue;
            while (it.hasNext()) {
                resultsValue = it.next().toMap();
                if (comicId == resultsValue.value("id").toString()) {
                    arcNumber = arcCount + 1;
                }
                arcCount++;
            }
            return qMakePair(QString::number(arcNumber), QString::number(arcCount));
        }
        return qMakePair(QString(), QString());
    }
    return qMakePair(QString(), QString());
}

QList<QString> getNamesFromList(const QVariant &json_list)
{
    QList<QString> names;

    QListIterator<QVariant> it(json_list.toList());
    QVariantMap resultsValue;
    while (it.hasNext()) {
        resultsValue = it.next().toMap();

        QString name = resultsValue.value("name").toString();
        names.append(name);
    }

    return names;
}
