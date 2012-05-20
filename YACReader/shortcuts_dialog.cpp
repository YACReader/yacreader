#include "shortcuts_dialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QPixmap>
#include <QFile>
#include <QTextStream>

ShortcutsDialog::ShortcutsDialog(QWidget * parent)
    :QDialog(parent)//,Qt::FramelessWindowHint)
{
    setModal(true);
    setWindowIcon(QIcon(":/images/shortcuts.png"));
    setWindowTitle(tr("YACReader keyboard shortcuts"));

    QVBoxLayout * mainLayout = new QVBoxLayout;

    close = new QPushButton(tr("Close"));
    connect(close,SIGNAL(clicked()),this,SLOT(close()));

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(close);
	
	QHBoxLayout * shortcutsLayout = new QHBoxLayout;

    shortcuts = new QTextEdit();
	QTextEdit * shortcuts2 = new QTextEdit();
	shortcuts->setFrameStyle(QFrame::NoFrame);
	shortcuts2->setFrameStyle(QFrame::NoFrame);
    //"<p><b>General functions:</b><hr/><b>O</b> : Open comic<br/><b>Esc</b> : Exit</p>"
    shortcuts->setReadOnly(true);
	shortcuts2->setReadOnly(true);
    shortcutsLayout->addWidget(shortcuts);
	shortcutsLayout->addWidget(shortcuts2);
	shortcutsLayout->setSpacing(0);
	mainLayout->addLayout(shortcutsLayout);
    mainLayout->addLayout(bottomLayout);

    QHBoxLayout *imgMainLayout = new QHBoxLayout;
    QLabel * imgLabel = new QLabel();
    QPixmap p(":/images/shortcuts.png");
    imgLabel->setPixmap(p);

    QVBoxLayout * imgLayout = new QVBoxLayout;
	imgLayout->addWidget(imgLabel);
    imgLayout->addStretch();

    imgMainLayout->addLayout(imgLayout);
    imgMainLayout->addLayout(mainLayout);

    setLayout(imgMainLayout);

    setFixedSize(QSize(700,500));

    QFile f(":/files/shortcuts.html");
    f.open(QIODevice::ReadOnly);
    QTextStream txtS(&f);
    QString content = txtS.readAll();
    f.close();

    shortcuts->setHtml(content);
	
    QFile f2(":/files/shortcuts2.html");
    f2.open(QIODevice::ReadOnly);
    QTextStream txtS2(&f2);
    content = txtS2.readAll();
    f2.close();

    shortcuts2->setHtml(content);

	setWindowTitle(tr("Keyboard Shortcuts"));
}
