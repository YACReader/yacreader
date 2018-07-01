#ifndef __TRANSLATOR_H
#define __TRANSLATOR_H
 
class QUrl; 
class QMouseEvent;
class QPoint;
class QTextEdit;
class QComboBox;
class QLabel;
class QPushButton;
class YACReaderBusyWidget;

#include <QWidget>
#include <QThread>
#include <QUrl>

#if QT_VERSION >= 0x050000
	class QMediaPlayer;
#else
	#include<Phonon/MediaObject>
	using namespace Phonon;
#endif



class YACReaderTranslator : public QWidget
{
  Q_OBJECT
  public:
	YACReaderTranslator(QWidget * parent = 0);
	~YACReaderTranslator();

	public slots:
		void play();

	protected slots:
		void translate();
		void setSpeak(const QUrl & url);
		void setTranslation(const QString & string);
		void error();
		void clear();

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent ( QMouseEvent * event );
	void hideResults();

	void populateCombos();
	bool drag;
	QPoint click; 
private:

#if QT_VERSION >= 0x050000
	QMediaPlayer *player;
#else
	MediaObject * music;
#endif

	QTextEdit * text;
	QComboBox * from;
	QComboBox * to;
	QLabel * resultsTitle;
	QPushButton * speakButton;
	QLabel * resultText;
	YACReaderBusyWidget * busyIndicator;
	QUrl ttsSource;
	QPushButton * clearButton;

};

class TranslationLoader : public QThread
{
	Q_OBJECT
public:
	TranslationLoader(QString text, QString from, QString to);
signals:
	void requestFinished(QString);
	void timeOut();
	void error();
private:
	QString text;
	QString from;
	QString to;
	void run();
};

class TextToSpeachLoader : public QThread
{
	Q_OBJECT
public:
	TextToSpeachLoader(QString text, QString language);
signals:
	void requestFinished(QUrl);
	void timeOut();
	void error();
private:
	QString text;
	QString language;
	void run();
};
#endif
