#ifndef __TRANSLATOR_H
#define __TRANSLATOR_H

class QMouseEvent;
class QPoint;
class QTextEdit;
class QComboBox;
class QLabel;
class QPushButton;
class YACReaderBusyWidget;
class QTextToSpeech;

#include "themable.h"
#include "viewer.h"

#include <QThread>
#include <QUrl>
#include <QWidget>

class YACReaderTranslator : public QWidget, protected Themable
{
    Q_OBJECT
public:
    YACReaderTranslator(Viewer *parent = nullptr);

public slots:
    void play();

protected slots:
    void translate();
    void setTranslation(const QString &string);
    void error();
    void clear();

protected:
    void applyTheme(const Theme &theme) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void hideResults();

    void populateCombos();
    bool drag;
    QPoint click;

private:
    QTextToSpeech *tts;
    QString speakText;
    QString speakLocale;

    QTextEdit *text;
    QComboBox *from;
    QComboBox *to;
    QLabel *titleLabel;
    QLabel *resultsTitle;
    QLabel *arrowLabel;
    QPushButton *speakButton;
    QPushButton *closeButton;
    QPushButton *searchButton;
    QLabel *resultText;
    YACReaderBusyWidget *busyIndicator;
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

#endif
