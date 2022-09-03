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
#include "viewer.h"

class QMediaPlayer;

class YACReaderTranslator : public QWidget
{
    Q_OBJECT
public:
    YACReaderTranslator(Viewer *parent = nullptr);

public slots:
    void play();

protected slots:
    void translate();
    void setSpeak(const QUrl &url);
    void setTranslation(const QString &string);
    void error();
    void clear();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void hideResults();

    void populateCombos();
    bool drag;
    QPoint click;

private:
    QMediaPlayer *player;

    QTextEdit *text;
    QComboBox *from;
    QComboBox *to;
    QLabel *resultsTitle;
    QPushButton *speakButton;
    QLabel *resultText;
    YACReaderBusyWidget *busyIndicator;
    QUrl ttsSource;
    QPushButton *clearButton;
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
    void run() override;
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
    void run() override;
};
#endif
