#include "tagscontroller_v2.h"

#include "db_helper.h"
#include "yacreader_libraries.h"

#include "reading_list.h"
#include "../static.h"
#include "yacreader_global.h"

#include "QsLog.h"

TagsControllerV2::TagsControllerV2() {}

void TagsControllerV2::service(HttpRequest& request, HttpResponse& response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();

    QList<Label> tags = DBHelper::getLabels(libraryId);

    foreach(const Label &tag, tags)
    {
        response.write(QString("%1\t%2\t%3\t%4\r\n").arg(libraryId).arg(tag.getId()).arg(tag.getName()).arg(labelColorToRGBString(tag.getColorID())).toUtf8());
    }

    response.write("",true);
}
