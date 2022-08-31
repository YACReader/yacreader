#ifndef EXTRACT_DELEGATE_H
#define EXTRACT_DELEGATE_H

#include <QByteArray>

class ExtractDelegate
{
public:
    virtual void fileExtracted(int index, const QByteArray &rawData) = 0;
    virtual void crcError(int index) = 0;
    virtual void unknownError(int index) = 0;
    virtual bool isCancelled() = 0;
};

#endif // EXTRACT_DELEGATE_H