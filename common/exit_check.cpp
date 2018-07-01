#include "exit_check.h"

#include "yacreader_global.h"

#include <QMessageBox>

using namespace YACReader;

void YACReader::exitCheck(int ret)
{
	switch(ret)
	{
	case YACReader::SevenZNotFound:
		QMessageBox::critical(0,QObject::tr("7z lib not found"),QObject::tr("unable to load 7z lib from ./utils"));
		break;
	default:
		break;
	}

}

