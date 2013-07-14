#ifndef EXTRACT_DELEGATE_H
#define EXTRACT_DELEGATE_H

#include <QByteArray>

class ExtractDelegate
{
	public:
		virtual void fileExtracted(int index, const QByteArray & rawData) = 0;
};

#endif //EXTRACT_DELEGATE_H