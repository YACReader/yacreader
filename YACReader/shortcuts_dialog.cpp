#include "shortcuts_dialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QPixmap>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>

ShortcutsDialog::ShortcutsDialog(QWidget *parent)
    : QDialog(parent) //,Qt::FramelessWindowHint)
{
    setModal(true);
    setWindowIcon(QIcon(":/images/shortcuts.png"));
    setWindowTitle(tr("YACReader keyboard shortcuts"));

    auto mainLayout = new QVBoxLayout;

    close = new QPushButton(tr("Close"));
    connect(close, &QAbstractButton::clicked, this, &QWidget::close);

    auto bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(close);

    auto shortcutsLayout = new QHBoxLayout;

    shortcuts = new QTextEdit();
    shortcuts->setFrameStyle(QFrame::NoFrame);

    //"<p><b>General functions:</b><hr/><b>O</b> : Open comic<br/><b>Esc</b> : Exit</p>"
    shortcuts->setReadOnly(true);
    shortcutsLayout->addWidget(shortcuts);
    //shortcutsLayout->addWidget(shortcuts2);
    shortcutsLayout->setSpacing(0);
    mainLayout->addLayout(shortcutsLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);

    setFixedSize(QSize(700, 500));

    QFile f(":/files/shortcuts.html");
    f.open(QIODevice::ReadOnly);
    QTextStream txtS(&f);
    txtS.setCodec(QTextCodec::codecForName("UTF-8"));
    QString content = txtS.readAll();

    f.close();

    shortcuts->setHtml(content);

    setWindowTitle(tr("Keyboard Shortcuts"));
}
