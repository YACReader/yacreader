#include "yacreader_global.h"


using namespace YACReader;

QString YACReader::getSettingsPath()
{
#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
	return QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif

}

QString YACReader::colorToName(LabelColors colors)
{
    switch(colors){
    case YRed:
        return "red";
    case YOrange:
        return "orange";
    case YYellow:
        return "yellow";
    case YGreen:
        return "green";
    case YCyan:
        return "cyan";
    case YBlue:
        return "blue";
    case YViolet:
        return "violet";
    case YPurple:
        return "purple";
    case YPink:
        return "pink";
    case YWhite:
        return "white";
    case YLight:
        return "light";
    case YDark:
        return "dark";
    }
}

QString YACReader::labelColorToRGBString(LabelColors color)
{
    switch (color) {
    case YRed:
        return "#FD777C";

    case YOrange:
        return "#FEBF34";

    case YYellow:
        return "#F5E934";

    case YGreen:
        return "#B6E525";

    case YCyan:
        return "#9FFFDD";

    case YBlue:
        return "#82C7FF";

    case YViolet:
        return "#8286FF";

    case YPurple:
        return "#E39FFF";

    case YPink:
        return "#FF9FDD";

#ifdef Q_OS_MAC
    case YWhite:
        return "#E3E3E3";
#else
    case YWhite:
        return "#FFFFFF";
#endif
    case YLight:
        return "#C8C8C8";
    case YDark:
        return "#ABABAB";


    }
}
