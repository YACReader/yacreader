#include <QTextToSpeech>
#include <QJsonDocument>
#include <QJsonObject>

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

YACReaderTranslator::YACReaderTranslator(Viewer *parent)
    : QWidget(parent), drag(false)
{
    this->setCursor(QCursor(Qt::ArrowCursor));
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Window);

    auto layout = new QVBoxLayout(this);

    // TITLE BAR
    auto titleBar = new QHBoxLayout();
    closeButton = new QPushButton(this);
    closeButton->setFlat(true);
    titleLabel = new QLabel(tr("YACReader translator"));
    titleBar->addWidget(titleLabel);
    titleBar->addStretch();
    closeButton->resize(14, 14);
    closeButton->setStyleSheet("QPushButton {margin:0;padding:0;border:none;}");
    titleBar->addWidget(closeButton);
    titleBar->setContentsMargins(0, 0, 0, 0);
    titleBar->setSpacing(0);
    connect(closeButton, &QAbstractButton::clicked, parent, &Viewer::animateHideTranslator);

    layout->addLayout(titleBar);

    // INPUT TEXT
    text = new QTextEdit(this);
    text->setMinimumHeight(110);
    text->setMaximumHeight(110);
    layout->addSpacing(12);
    layout->addWidget(text);

    // COMBOBOXES
    auto combos = new QHBoxLayout();
    from = new QComboBox(this);
    to = new QComboBox(this);
    from->setFixedHeight(22);
    to->setFixedHeight(22);
    arrowLabel = new QLabel(this);
    searchButton = new QPushButton(this);
    searchButton->setFixedSize(22, 22);
    combos->addWidget(from, 1);
    combos->addSpacing(9);
    combos->addWidget(arrowLabel, 0);
    combos->addSpacing(9);
    combos->addWidget(to, 1);
    combos->addSpacing(9);
    combos->addWidget(searchButton, 0);
    layout->addSpacing(12);
    layout->addLayout(combos);

    // RESULTS
    auto resultsTitleLayout = new QHBoxLayout();
    resultsTitle = new QLabel(tr("Translation"));
    speakButton = new QPushButton(this);
    speakButton->setStyleSheet("QPushButton {border:none;}");
    resultsTitleLayout->addWidget(resultsTitle, 0, Qt::AlignVCenter);
    resultsTitleLayout->addSpacing(10);
    resultsTitleLayout->addWidget(speakButton, 0, Qt::AlignVCenter);
    resultsTitleLayout->addStretch();

    layout->addSpacing(15);
    layout->addLayout(resultsTitleLayout);
    layout->addSpacing(12);

    resultText = new QLabel();
    resultText->setWordWrap(true);
    resultText->setText("");
    layout->addWidget(resultText);

    layout->addStretch();

    // CLEAR BUTTON
    clearButton = new QPushButton(tr("clear"));
    layout->addWidget(clearButton, 0, Qt::AlignRight);
    clearButton->setMinimumWidth(95);

    resize(400, 479);

    layout->setContentsMargins(18, 12, 18, 12);
    setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    hideResults();
    populateCombos();

    busyIndicator = new YACReaderBusyWidget(this);
    busyIndicator->move((this->width() - busyIndicator->width()) / 2, (this->height() - busyIndicator->height()) * 2 / 3);
    busyIndicator->hide();

    connect(searchButton, &QAbstractButton::pressed, this, &YACReaderTranslator::translate);
    connect(speakButton, &QAbstractButton::pressed, this, &YACReaderTranslator::play);
    connect(clearButton, &QAbstractButton::pressed, this, &YACReaderTranslator::clear);

    tts = new QTextToSpeech(this);

    initTheme(this);

    show();
}

void YACReaderTranslator::applyTheme(const Theme &theme)
{
    const auto &tr = theme.translator;

    QPalette p(this->palette());
    p.setColor(QPalette::Window, tr.backgroundColor);
    this->setPalette(p);

    text->setStyleSheet(tr.textEditQSS + tr.scrollBarQSS);
    from->setStyleSheet(tr.comboBoxQSS + tr.scrollBarQSS);
    to->setStyleSheet(tr.comboBoxQSS + tr.scrollBarQSS);

    titleLabel->setStyleSheet(tr.titleQSS);
    resultsTitle->setStyleSheet(tr.resultsTitleQSS);
    resultText->setStyleSheet(tr.resultTextQSS);
    clearButton->setStyleSheet(tr.clearButtonQSS);

    searchButton->setStyleSheet(
            QString("QPushButton {border:none; background:%1;}").arg(tr.inputBackgroundColor.name()));

    closeButton->setIcon(tr.closeIcon);
    speakButton->setIcon(tr.speakerIcon);
    searchButton->setIcon(tr.searchIcon);
    arrowLabel->setPixmap(tr.fromToPixmap);
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

    speakText = text;
    speakLocale = from;

    TranslationLoader *translationLoader = new TranslationLoader(text, from, to);
    connect(translationLoader, &TranslationLoader::requestFinished, this, &YACReaderTranslator::setTranslation);
    connect(translationLoader, &TranslationLoader::error, this, &YACReaderTranslator::error);
    connect(translationLoader, &TranslationLoader::timeOut, this, &YACReaderTranslator::error);
    connect(translationLoader, &QThread::finished, translationLoader, &QObject::deleteLater);

    translationLoader->start();

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

void YACReaderTranslator::setTranslation(const QString &string)
{
    resultText->setText(string);

    resultsTitle->setHidden(false);
    speakButton->setHidden(false);
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
        combo->addItem("Chinese Simplified", "zh-CN");
        combo->addItem("Chinese Traditional", "zh-TW");
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
    tts->setLocale(QLocale(speakLocale));
    tts->say(speakText);
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

    QString urlStr = QString("https://api.mymemory.translated.net/get?q=%1&langpair=%2|%3")
                             .arg(QString::fromUtf8(QUrl::toPercentEncoding(text)), from, to);

    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(urlStr)));

    tT.start(5000); // 5s timeout
    q.exec();

    if (tT.isActive()) {
        // download complete
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QString translated = doc["responseData"]["translatedText"].toString();
            if (!translated.isEmpty())
                emit requestFinished(translated);
            else
                emit error();
        } else
            emit error();
    } else {
        emit timeOut();
    }
}
