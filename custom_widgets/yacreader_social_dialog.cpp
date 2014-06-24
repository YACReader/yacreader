#include "yacreader_social_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QToolButton>
#include <QPlainTextEdit>
#include <QGraphicsDropShadowEffect>

#include "comic_db.h"

YACReaderSocialDialog::YACReaderSocialDialog(QWidget *parent) :
    QWidget(parent)
{

	//setWindowFlags(Qt::Window | Qt::Dialog | Qt::FramelessWindowHint);
	//setModal(true);


	QToolButton * close = new QToolButton(this);
	close->setIcon(QIcon(":/images/social_dialog/close.png"));

	QToolButton * facebook = new QToolButton(this);
	facebook->setIcon(QIcon(":/images/social_dialog/facebook.png"));

	QToolButton * twitter = new QToolButton(this);
	twitter->setIcon(QIcon(":/images/social_dialog/twitter.png"));

	QToolButton * google = new QToolButton(this);
	google->setIcon(QIcon(":/images/social_dialog/google+.png"));

	QString styleSheet = "QToolButton {border:none; }";
	close->setStyleSheet(styleSheet);
	facebook->setStyleSheet(styleSheet);
	twitter->setStyleSheet(styleSheet);
	google->setStyleSheet(styleSheet);

	QLabel * icon = new QLabel(this);
	icon->setPixmap(QPixmap(":/images/social_dialog/icon.png"));

	plainText = new QTextEdit (this);
	plainText->setStyleSheet("QTextEdit  {border:none; padding:11px; font-size:12px; font-weight:bold; color:#525757;}");
	QTextCursor cursor(plainText->textCursor());
    QTextBlockFormat blockFormat = cursor.blockFormat();
	blockFormat.setLineHeight(12,QTextBlockFormat::SingleHeight);
	cursor.setBlockFormat(blockFormat);
	QLabel * sendTo = new QLabel(tr("send to:"),this);
	sendTo->setStyleSheet("QLabel{color:#ABABAB; font-size:12px; font-weight:bold;}");

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	resize( sizeHint() );

	close->move(437,5);

	QWidget * send = new QWidget(this);
	QHBoxLayout * sendLayout = new QHBoxLayout;

	QPushButton * follow = new QPushButton(tr("Follow YACReader!"),this);

	follow->setStyleSheet("QPushButton{border:none; color:#FFFFFF;background:#404040; padding: 9px 25px 9px 25px; font-weight:bold; font-size:12px;}"
		"QPushButton:hover{background:#E3B800;}");

	sendLayout->setMargin(0);
	sendLayout->setSpacing(0);

	sendLayout->addWidget(sendTo,1,Qt::AlignHCenter);
	sendLayout->addSpacing(11);
	sendLayout->addWidget(facebook,0,Qt::AlignHCenter);
	sendLayout->addSpacing(6);
	sendLayout->addWidget(twitter,0,Qt::AlignHCenter);
	sendLayout->addSpacing(6);
	sendLayout->addWidget(google,0,Qt::AlignHCenter);

	send->setLayout(sendLayout);
	send->move(317,259);

	icon->move(279,14);
	plainText->setFixedSize(291,155);
	plainText->move(169,96);

	follow->move(230,307);

	connect(close,SIGNAL(released()),this,SLOT(close()));

	

}

void YACReaderSocialDialog::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);

	//center
	painter.fillRect(169,0,291,369,QColor("#F0F0F0"));
	painter.fillRect(169,96,291,155,QColor("#FFFFFF"));


	//QPixmap cover = QPixmap("c:/temp/6.jpg").scaledToHeight(369,Qt::SmoothTransformation);
	painter.drawPixmap(0,0,169,369,cover,0,0, (169 * cover.height())/369 ,cover.height());


	QPixmap shadow(":/images/social_dialog/shadow.png");
	painter.drawPixmap(169-shadow.width(),0,shadow.width(),369,shadow);

	
	QPixmap separtor(":/images/social_dialog/separator.png");
	painter.drawPixmap(169,96-separtor.height(),separtor);

	QPen pen("#C3CAD6");
	painter.setPen(pen);
	painter.drawLine(169,251,460,251);

	QWidget::paintEvent(event);

}

QSize YACReaderSocialDialog::sizeHint() const
{
	return QSize(460,369);
}

void YACReaderSocialDialog::setComic(ComicDB & comic, QString & basePath)
{
	this->cover = comic.info.getCover(basePath).scaledToHeight(369,Qt::SmoothTransformation);
	plainText->setText(tr("I am reading %1 using YACReader.").arg(comic.path.split('/').last()));
}