#ifndef READING_LIST_H
#define READING_LIST_H

#include "yacreader_global.h"

class ReadingList
{
public:
    ReadingList(const QString &name, qulonglong id, int ordering);

    qulonglong getId() const;
    QString getName() const;
    int getOrdering() const;
private:
    QString name;
    qulonglong id;
    int ordering;
};

class Label
{
public:
    Label(const QString &name, qulonglong id, YACReader::LabelColors colorid);

    YACReader::LabelColors getColorID() const;
    QString getName() const;
    qulonglong getId() const;

private:
    QString name;
    qulonglong id;
    YACReader::LabelColors colorid;

};

#endif // READING_LIST_H
