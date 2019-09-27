#include "bookmarks_dialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>
#include <QImage>

#include "bookmarks.h"

BookmarksDialog::BookmarksDialog(QWidget *parent)
    : QDialog(parent)
{
    setModal(true);

    //animation = new QPropertyAnimation(this,"windowOpacity");
    //animation->setDuration(150);

    auto layout = new QHBoxLayout();

    //bookmarks
    auto bookmarksL = new QGridLayout();

    pages.push_back(new QLabel(tr("Lastest Page")));
    for (int i = 0; i < 3; i++)
        pages.push_back(new QLabel("-"));

    QString labelsStyle = "QLabel {color:white;}";

    foreach (QLabel *label, pages) {
        label->setStyleSheet(labelsStyle);
    }

    int heightDesktopResolution = QApplication::desktop()->screenGeometry().height();
    int height, width;
    height = heightDesktopResolution * 0.50;
    width = height * 0.65;

    coverSize = QSize(width, height);

    for (int i = 0; i < 4; i++) {
        QLabel *l = new QLabel();
        l->setFixedSize(coverSize);
        l->setScaledContents(false);
        //l->setPixmap(QPixmap(":/images/notCover.png"));
        l->installEventFilter(this);
        images.push_back(l);
    }

    for (int i = 0; i < 3; i++)
        bookmarksL->addWidget(pages.at(i + 1), 0, i, Qt::AlignCenter);

    for (int i = 0; i < 3; i++)
        bookmarksL->addWidget(images.at(i + 1), 1, i, Qt::AlignCenter);

    //last page
    auto lp = new QGridLayout();
    lp->addWidget(pages.at(0), 0, 0, Qt::AlignCenter);
    lp->addWidget(images.at(0), 1, 0, Qt::AlignCenter);

    layout->addLayout(bookmarksL);
    auto f = new QFrame(this);
    f->setFrameStyle(QFrame::VLine | QFrame::Sunken);
    layout->addWidget(f);
    layout->addLayout(lp);

    auto buttons = new QHBoxLayout();

    cancel = new QPushButton(tr("Close"));
    cancel->setFlat(true);
    connect(cancel, SIGNAL(clicked()), this, SLOT(hide()));
    buttons->addStretch();
    buttons->addWidget(cancel);

    cancel->setStyleSheet("QPushButton {border: 1px solid #242424; background: #2e2e2e; color:white; padding: 5px 26px 5px 26px; font-size:12px;font-family:Arial; font-weight:bold;}");

    auto l = new QVBoxLayout();

    l->addWidget(new QLabel("<font color=\"#FFFFFF\">" + tr("Click on any image to go to the bookmark") + "</font>"), 0, Qt::AlignCenter);
    l->addLayout(layout);
#ifdef Q_OS_MAC
    l->addLayout(buttons);
#endif

    QPalette Pal(palette());
    // set black background
    Pal.setColor(QPalette::Background, QColor("#454545"));
    this->setAutoFillBackground(true);
    this->setPalette(Pal);

    setLayout(l);
}

void BookmarksDialog::setBookmarks(const Bookmarks &bm)
{
    lastPage = bm.getLastPage();
    if (lastPage > 0) {
        QPixmap p = QPixmap::fromImage(bm.getLastPagePixmap());
        if (p.isNull()) {
            images.at(0)->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            images.at(0)->setText(tr("Loading..."));
        } else {
            images.at(0)->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            images.at(0)->setPixmap(p.scaled(coverSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    } else {
        images.at(0)->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        images.at(0)->setPixmap(QPixmap(":/images/notCover.png").scaled(coverSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    QList<int> l = bm.getBookmarkPages();
    int s = l.count();
    for (int i = 0; i < s; i++) {
        pages.at(i + 1)->setText(QString::number(l.at(i) + 1));
        QPixmap p = QPixmap::fromImage(bm.getBookmarkPixmap(l.at(i)));
        if (p.isNull()) {
            images.at(i + 1)->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            images.at(i + 1)->setText(tr("Loading..."));
        } else {
            images.at(i + 1)->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            images.at(i + 1)->setPixmap(p.scaled(coverSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
    for (int i = s; i < 3; i++) {
        pages.at(i + 1)->setText("-");
        images.at(i + 1)->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        images.at(i + 1)->setPixmap(QPixmap(":/images/notCover.png").scaled(coverSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

bool BookmarksDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (obj == images.at(0)) {
            emit(goToPage(lastPage));
            close();
            event->accept();
        }
        for (int i = 1; i <= 3; i++) {
            if (obj == images.at(i)) {
                bool b;
                int page = pages.at(i)->text().toInt(&b) - 1;
                if (b) {
                    emit(goToPage(page));
                    close();
                }
                event->accept();
            }
        }
    }
    // pass the event on to the parent class
    return QDialog::eventFilter(obj, event);
}

void BookmarksDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_M)
        hide();
}
/*
void BookmarksDialog::show()
{
		QDialog::show();
		disconnect(animation,SIGNAL(finished()),this,SLOT(close()));
		animation->setStartValue(0);
		animation->setEndValue(1);
		animation->start();
}

void BookmarksDialog::hide()
{
		connect(animation,SIGNAL(finished()),this,SLOT(close()));
		animation->setStartValue(1);
		animation->setEndValue(0);
		animation->start();
}*/
