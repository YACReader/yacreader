#ifndef YACREADER_LIBRARIES_H
#define YACREADER_LIBRARIES_H

#include <QtCore>

class YACReaderLibraries : public QObject
{
    Q_OBJECT

public:
    YACReaderLibraries();
    YACReaderLibraries(const YACReaderLibraries &source);
    QList<QString> getNames();
    QString getPath(const QString &name);
    QString getPath(int id);
    QString getName(int id);
    bool isEmpty();
    bool contains(const QString &name);
    bool contains(int id);
    void remove(const QString &name);
    void rename(const QString &oldName, const QString &newName);
    int getId(const QString &name);
    YACReaderLibraries &operator=(const YACReaderLibraries &source);
    QMap<QString, QPair<int, QString>> getLibraries();
public slots:
    void addLibrary(const QString &name, const QString &path);
    void load();
    bool save();

private:
    //name <id,path>
    QMap<QString, QPair<int, QString>> libraries;
};

#endif // YACREADER_LIBRARIES_H
