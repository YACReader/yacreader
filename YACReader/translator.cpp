#include <QUrl>

#if QT_VERSION >= 0x050000
#include <QMediaPlayer>
#else
#include <Phonon/MediaObject>
#include <Phonon/MediaSource>
#endif

#include <QPushButton>
#include <QPalette>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "translator.h"
#include "viewer.h"

#include "yacreader_busy_widget.h"

#include <QFile>
#include <QPoint>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QComboBox>
#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QMessageBox>

#define APPID "417CEAD93449502CC3C9B69FED26C54118E62BCC"

YACReaderTranslator::YACReaderTranslator(Viewer *parent)
    : QWidget(parent), drag(false)
{
    QString scrollBarStyle = "QScrollBar:vertical { border: none; background: #404040; width: 7px; margin: 0 3px 0 0; }"
                             "QScrollBar::handle:vertical { background: #DDDDDD; width: 7px; min-height: 20px; }"
                             "QScrollBar::add-line:vertical { border: none; background: #404040; height: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"

                             "QScrollBar::sub-line:vertical {  border: none; background: #404040; height: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                             "QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
                             "QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"

                             "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: none; }";

    this->setCursor(QCursor(Qt::ArrowCursor));
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Window);
    QPalette p(this->palette());
    p.setColor(QPalette::Window, QColor(0x404040));
    this->setPalette(p);

    auto layout = new QVBoxLayout(this);

    // TITLE BAR
    auto titleBar = new QHBoxLayout();
    auto close = new QPushButton(QIcon(QPixmap(":/images/close.png")), "");
    close->setFlat(true);
    auto title = new QLabel(tr("YACReader translator"));
    title->setStyleSheet("QLabel {font-size:18px; font-family:Arial; color:white;}");
    titleBar->addWidget(title);
    titleBar->addStretch();
    close->resize(14, 14);
    close->setStyleSheet("QPushButton {margin:0;padding:0;border:none;}");
    titleBar->addWidget(close);
    titleBar->setContentsMargins(0, 0, 0, 0);
    titleBar->setSpacing(0);
    connect(close, &QAbstractButton::clicked, parent, &Viewer::animateHideTranslator);

    layout->addLayout(titleBar);

    // INPUT TEXT
    text = new QTextEdit(this);
    text->setMinimumHeight(110);
    text->setMaximumHeight(110);
    layout->addSpacing(12);
    layout->addWidget(text);
    text->setStyleSheet("QTextEdit{border:none;background:#2a2a2a;color:white; font-size:12px; padding:6px;}" + scrollBarStyle);

    // COMBOBOXES
    auto combos = new QHBoxLayout();
    from = new QComboBox(this);
    to = new QComboBox(this);
    QString comboBoxStyle = "QComboBox {border:none;background:#2a2a2a;color:white;font-size:12px;font-family:Arial;padding-left:8px;}"
                            "QComboBox::down-arrow {image: url(:/images/dropDownArrow.png);}"
                            "QComboBox::drop-down {border:none; padding-right:10px;}"
                            "QComboBox QAbstractItemView {border: none; background:#272727; color:white; selection-background-color: #202020; outline:none;}"
                            "QComboBox QAbstractItemView::item {padding-left:8px;}" +
            scrollBarStyle;
    from->setStyleSheet(comboBoxStyle);
    to->setStyleSheet(comboBoxStyle);
    from->setFixedHeight(22);
    to->setFixedHeight(22);
    QLabel *arrow = new QLabel(this);
    QPixmap arrowPixmap(":/images/fromTo.png");
    arrow->setPixmap(arrowPixmap);
    auto searchButton = new QPushButton(this);
    searchButton->setIcon(QIcon(":/images/translatorSearch.png"));
    searchButton->setStyleSheet("QPushButton {border:none; background:#2a2a2a;}");
    searchButton->setFixedSize(22, 22);
    combos->addWidget(from, 1);
    combos->addSpacing(9);
    combos->addWidget(arrow, 0);
    combos->addSpacing(9);
    combos->addWidget(to, 1);
    combos->addSpacing(9);
    combos->addWidget(searchButton, 0);
    layout->addSpacing(12);
    layout->addLayout(combos);

    // RESULTS
    auto resultsTitleLayout = new QHBoxLayout();
    resultsTitle = new QLabel(tr("Translation"));
    resultsTitle->setStyleSheet("QLabel {font-family:Arial;font-size:14px;color:#e3e3e3;}");
    speakButton = new QPushButton(this);
    speakButton->setStyleSheet("QPushButton {border:none;}");
    speakButton->setIcon(QIcon(":/images/speaker.png"));
    resultsTitleLayout->addWidget(resultsTitle, 0, Qt::AlignVCenter);
    resultsTitleLayout->addSpacing(10);
    resultsTitleLayout->addWidget(speakButton, 0, Qt::AlignVCenter);
    resultsTitleLayout->addStretch();

    layout->addSpacing(15);
    layout->addLayout(resultsTitleLayout);
    layout->addSpacing(12);

    resultText = new QLabel();
    resultText->setWordWrap(true);
    resultText->setStyleSheet("QLabel {color:white;font-size:12px;}");
    resultText->setText("");
    layout->addWidget(resultText);

    layout->addStretch();

    // CLEAR BUTTON
    clearButton = new QPushButton(tr("clear"));
    layout->addWidget(clearButton, 0, Qt::AlignRight);
    clearButton->setMinimumWidth(95);
    clearButton->setStyleSheet("QPushButton {border:1px solid #212121; background:#2a2a2a; color:white; font-family:Arial; font-size:12px; padding-top:5px; padding-bottom:5px;}");

    resize(400, 479);

    layout->setContentsMargins(18, 12, 18, 12);
    setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    hideResults();
    populateCombos();

    busyIndicator = new YACReaderBusyWidget(this);
    busyIndicator->move((this->width() - busyIndicator->width()) / 2, (this->height() - busyIndicator->height()) * 2 / 3);
    busyIndicator->hide();

    show();

    connect(searchButton, &QAbstractButton::pressed, this, &YACReaderTranslator::translate);
    connect(speakButton, &QAbstractButton::pressed, this, &YACReaderTranslator::play);
    connect(clearButton, &QAbstractButton::pressed, this, &YACReaderTranslator::clear);

    // multimedia/phonon
#if QT_VERSION >= 0x050000
    player = new QMediaPlayer;
#else
    music = createPlayer(MusicCategory);
#endif
}

void YACReaderTranslator::hideResults()
{
    resultsTitle->setHidden(true);
    speakButton->setHidden(true);
    resultText->setHidden(true);
}

void YACReaderTranslator::clear()
{
    hideResults();
    text->clear();
}

void YACReaderTranslator::translate()
{
    QString text = this->text->toPlainText();
    if (text.isEmpty())
        return;
    QString from = this->from->itemData(this->from->currentIndex()).toString();
    QString to = this->to->itemData(this->to->currentIndex()).toString();

    TranslationLoader *translationLoader = new TranslationLoader(text, from, to);
    connect(translationLoader, &TranslationLoader::requestFinished, this, &YACReaderTranslator::setTranslation);
    connect(translationLoader, &TranslationLoader::error, this, &YACReaderTranslator::error);
    connect(translationLoader, &TranslationLoader::timeOut, this, &YACReaderTranslator::error);
    connect(translationLoader, &QThread::finished, translationLoader, &QObject::deleteLater);

    TextToSpeachLoader *tts = new TextToSpeachLoader(text, from);
    connect(tts, &TextToSpeachLoader::requestFinished, this, &YACReaderTranslator::setSpeak);
    connect(tts, &TextToSpeachLoader::error, this, &YACReaderTranslator::error);
    connect(tts, &TextToSpeachLoader::timeOut, this, &YACReaderTranslator::error);
    connect(tts, &QThread::finished, tts, &QObject::deleteLater);

    translationLoader->start();
    tts->start();

    resultsTitle->setText(tr("Translation"));

    hideResults();

    busyIndicator->show();
}

void YACReaderTranslator::error()
{
    resultsTitle->setText(tr("Service not available"));
    resultsTitle->setHidden(false);
    busyIndicator->hide();
}

void YACReaderTranslator::setSpeak(const QUrl &url)
{
    resultsTitle->setHidden(false);
    speakButton->setHidden(false);

    ttsSource = url;
}

void YACReaderTranslator::setTranslation(const QString &string)
{
    resultText->setText(string);

    resultsTitle->setHidden(false);
    resultText->setHidden(false);
    busyIndicator->hide();
}

void YACReaderTranslator::populateCombos()
{
    QList<QComboBox *> combos;
    combos.append(from);
    combos.append(to);

    for (int i = 0; i < combos.count(); i++) {
        QComboBox *combo = combos.at(i);
        combo->addItem("Arabic", "ar");
        combo->addItem("Bulgarian", "bg");
        combo->addItem("Catalan", "ca");
        combo->addItem("Chinese Simplified", "zh-CHS");
        combo->addItem("Chinese Traditional", "zh-CHT");
        combo->addItem("Czech", "cs");
        combo->addItem("Danish", "da");
        combo->addItem("Dutch", "nl");
        combo->addItem("English", "en");
        combo->addItem("Estonian", "et");
        combo->addItem("Finnish", "fi");
        combo->addItem("French", "fr");
        combo->addItem("German", "de");
        combo->addItem("Greek", "el");
        combo->addItem("Haitian Creole", "ht");
        combo->addItem("Hebrew", "he");
        combo->addItem("Hindi", "hi");
        combo->addItem("Hungarian", "hu");
        combo->addItem("Indonesian", "id");
        combo->addItem("Italian", "it");
        combo->addItem("Japanese", "ja");
        combo->addItem("Korean", "ko");
        combo->addItem("Latvian", "lv");
        combo->addItem("Lithuanian", "lt");
        combo->addItem("Norwegian", "no");
        combo->addItem("Polish", "pl");
        combo->addItem("Portuguese", "pt");
        combo->addItem("Romanian", "ro");
        combo->addItem("Russian", "ru");
        combo->addItem("Slovak", "sk");
        combo->addItem("Slovenian", "sl");
        combo->addItem("Spanish", "es");
        combo->addItem("Swedish", "sv");
        combo->addItem("Thai", "th");
        combo->addItem("Turkish", "tr");
        combo->addItem("Ukrainian", "uk");
        combo->addItem("Vietnamese", "vi");
    }
    from->setCurrentIndex(from->findText("English"));
    to->setCurrentIndex(from->findText("Spanish"));
}

void YACReaderTranslator::play()
{

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    player->setSource(ttsSource);
#else
    player->setMedia(ttsSource);
#endif

    player->play();
}

void YACReaderTranslator::mousePressEvent(QMouseEvent *event)
{
    QPoint p = mapTo(this, event->pos());
    if (p.y() < 40) {
        drag = true;
        click = event->pos();
    }
}

void YACReaderTranslator::mouseReleaseEvent(QMouseEvent *event)
{
    drag = false;
    event->accept();
}

void YACReaderTranslator::mouseMoveEvent(QMouseEvent *event)
{
    if (drag)
        this->move(QPoint(mapToParent(event->pos()) - click));
    event->accept();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

TranslationLoader::TranslationLoader(QString text, QString from, QString to)
    : QThread(), text(text), from(from), to(to)
{
}

void TranslationLoader::run()
{
    QNetworkAccessManager manager;
    QEventLoop q;
    QTimer tT;

    tT.setSingleShot(true);
    connect(&tT, &QTimer::timeout, &q, &QEventLoop::quit);
    connect(&manager, &QNetworkAccessManager::finished, &q, &QEventLoop::quit);

    QString url = "http://api.microsofttranslator.com/V2/Ajax.svc/Translate?appid=%1&from=%2&to=%3&text=%4&contentType=text/plain";
    url = url.arg(APPID, from, to, text);

    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));

    tT.start(5000); // 5s timeout
    q.exec();

    if (tT.isActive()) {
        // download complete
        if (reply->error() == QNetworkReply::NoError) {
            QString utf8 = QString::fromUtf8(reply->readAll());
            utf8 = utf8.remove(0, 1);
            utf8 = utf8.remove(utf8.count() - 1, 1);

            QString translated(utf8);
            emit requestFinished(translated);
        } else
            emit error();
    } else {
        emit timeOut();
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

TextToSpeachLoader::TextToSpeachLoader(QString text, QString language)
    : QThread(), text(text), language(language)
{
}

void TextToSpeachLoader::run()
{
    QNetworkAccessManager manager;
    QEventLoop q;
    QTimer tT;

    tT.setSingleShot(true);
    connect(&tT, &QTimer::timeout, &q, &QEventLoop::quit);
    connect(&manager, &QNetworkAccessManager::finished, &q, &QEventLoop::quit);

    QString url = "http://api.microsofttranslator.com/V2/Ajax.svc/Speak?appid=%1&language=%2&text=%3&contentType=text/plain";
    url = url.arg(APPID, language, text);

    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));

    tT.start(5000); // 5s timeout
    q.exec();

    if (tT.isActive()) {
        // download complete
        if (reply->error() == QNetworkReply::NoError) {
            QString utf8 = QString::fromUtf8(reply->readAll());
            utf8 = utf8.remove(0, 1);
            utf8 = utf8.remove(utf8.count() - 1, 1);
            utf8 = utf8.replace("\\", "");

            emit requestFinished(QUrl(utf8));
        } else
            emit error();
    } else {
        emit timeOut();
    }
}
