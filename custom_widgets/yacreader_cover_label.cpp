#include "yacreader_cover_label.h"
#include "yacreader_global_gui.h"

YACReader::CoverLabel::CoverLabel(QWidget *parent)
    : QLabel(parent)
{
    setAcceptDrops(true);
}

void YACReader::CoverLabel::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls() && !YACReader::imagePathFromMimeData(event->mimeData()).isEmpty()) {
        event->acceptProposedAction();
    }
}

void YACReader::CoverLabel::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void YACReader::CoverLabel::dropEvent(QDropEvent *event)
{
    QString path = YACReader::imagePathFromMimeData(event->mimeData());
    if (!path.isEmpty()) {
        emit imageDropped(path);
        event->acceptProposedAction();
    }
}
