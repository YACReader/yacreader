#ifndef YACREADER_LIBRARIES_H
#define YACREADER_LIBRARIES_H

#include <QtCore>

class YACReaderLibrary;

class YACReaderLibraries : public QObject
{
    Q_OBJECT

public:
    YACReaderLibraries();
    YACReaderLibraries(const YACReaderLibraries &source);
    QList<QString> getNames();
    QString getPath(const QString &name);
    QString getPath(int id);
    QString getPath(const QUuid &id);
    QString getDBPath(int id);
    QString getName(int id);
    bool isEmpty();
    bool contains(const QString &name);
    bool contains(int id);
    void remove(const QString &name);
    void rename(const QString &oldName, const QString &newName);
    int getId(const QString &name);
    QUuid getUuid(const QString &name);
    int getIdFromUuid(const QUuid &uuid);
    YACReaderLibraries &operator=(const YACReaderLibraries &source);
    QList<YACReaderLibrary> getLibraries() const;
    QList<YACReaderLibrary> sortedLibraries() const;
    QUuid getLibraryIdFromLegacyId(int legacyId) const;
public slots:
    void addLibrary(const QString &name, const QString &path);
    void load();
    bool save();

private:
    QList<YACReaderLibrary> libraries;
};

class YACReaderLibrary
{
public:
    YACReaderLibrary();
    YACReaderLibrary(const QString &name, const QString &path, int legacyId, const QUuid &id);
    QString getName() const;
    QString getPath() const;
    QString getDBPath() const;
    int getLegacyId() const;
    QUuid getId() const;
    bool operator==(const YACReaderLibrary &other) const;
    bool operator!=(const YACReaderLibrary &other) const;
    friend QDataStream &operator<<(QDataStream &out, const YACReaderLibrary &library);
    friend QDataStream &operator>>(QDataStream &in, YACReaderLibrary &library);
    operator QString() const { return QString("%1 [%2, %3, %4]").arg(name, QString::number(legacyId), id.toString(QUuid::WithoutBraces), path); }

private:
    QString name;
    QString path;
    int legacyId;
    QUuid id; // stored in `id` file in .yacreaderlibrary
};

#endif // YACREADER_LIBRARIES_H
