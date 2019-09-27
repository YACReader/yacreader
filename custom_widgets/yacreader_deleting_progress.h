#ifndef YACREADER_DELETING_PROGRESS_H
#define YACREADER_DELETING_PROGRESS_H

#include <QWidget>

class QLabel;

class YACReaderDeletingProgress : public QWidget
{
    Q_OBJECT
public:
    explicit YACReaderDeletingProgress(QWidget *parent = 0);
    QSize sizeHint() const;
signals:

public slots:

protected:
    void paintEvent(QPaintEvent *);

private:
    QLabel *textMessage;
};

#endif // YACREADER_DELETING_PROGRESS_H
