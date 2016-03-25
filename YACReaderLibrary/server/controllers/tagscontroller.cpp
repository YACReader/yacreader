#include "tagscontroller.h"

#include "db_helper.h"
#include "yacreader_libraries.h"

#include "reading_list_item.h"
#include "../static.h"
#include "yacreader_global.h"

#include "QsLog.h"

TagsController::TagsController() {}

void TagsController::service(HttpRequest& request, HttpResponse& response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(2).toInt();

    QList<LabelItem *> tags = DBHelper::getLabelItems(libraryId);

    foreach(LabelItem * tag, tags)
    {
        response.writeText(QString("%1\t%2\t%3\r\n").arg(tag->getId()).arg(tag->name()).arg(labelColorToRGBString(tag->colorid())));
    }

    response.writeText("",true);
}
