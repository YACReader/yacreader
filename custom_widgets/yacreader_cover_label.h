#ifndef DROP_LABEL_H
#define DROP_LABEL_H

#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

namespace YACReader {

class CoverLabel : public QLabel
{
    Q_OBJECT

public:
    explicit CoverLabel(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void imageDropped(const QString &path);
};

}

#endif // DROP_LABEL_H
