#include "pdf_render_size.h"

#include <QObject>
#include <QSizeF>
#include <QTest>

class PdfRenderSizeTest : public QObject
{
    Q_OBJECT

private slots:
    void upscalesOrdinaryPagesToMinimumLongestSide();
    void keepsTallPagesWhenAreaIsReasonable();
    void keepsMaximumAreaTallPages();
    void capsLargeTallPagesByPixelArea();
    void capsPathologicalLongestSide();
    void rejectsInvalidInput();
};

void PdfRenderSizeTest::upscalesOrdinaryPagesToMinimumLongestSide()
{
    const QSize size = YACReaderPdfRender::renderSizeFromPagePoints(QSizeF(595.0, 842.0));

    QCOMPARE(size.height(), YACReaderPdfRender::MinimumLongestSide);
    QVERIFY(size.width() > 1700);
    QVERIFY(size.width() < 1900);
}

void PdfRenderSizeTest::keepsTallPagesWhenAreaIsReasonable()
{
    const QSize size = YACReaderPdfRender::renderSizeFromPixelSize(QSizeF(512.0, 4096.0));

    QCOMPARE(size, QSize(512, 4096));
}

void PdfRenderSizeTest::keepsMaximumAreaTallPages()
{
    const QSize size = YACReaderPdfRender::renderSizeFromPixelSize(QSizeF(512.0, 32768.0));

    QCOMPARE(size, QSize(512, YACReaderPdfRender::MaximumSide));
}

void PdfRenderSizeTest::capsLargeTallPagesByPixelArea()
{
    const QSize size = YACReaderPdfRender::renderSizeFromPixelSize(QSizeF(2000.0, 20000.0));

    QVERIFY(static_cast<qint64>(size.width()) * size.height() <= YACReaderPdfRender::MaximumPixelArea);
    QVERIFY(size.height() > 4096);
    QVERIFY(size.width() > 0);
}

void PdfRenderSizeTest::capsPathologicalLongestSide()
{
    const QSize size = YACReaderPdfRender::renderSizeFromPixelSize(QSizeF(100.0, 100000.0));

    QVERIFY(size.height() <= YACReaderPdfRender::MaximumSide);
    QVERIFY(size.width() > 0);
}

void PdfRenderSizeTest::rejectsInvalidInput()
{
    QVERIFY(YACReaderPdfRender::renderSizeFromPixelSize(QSizeF(0.0, 100.0)).isEmpty());
    QVERIFY(YACReaderPdfRender::renderSizeFromPagePoints(QSizeF(100.0, 100.0), 0.0).isEmpty());
}

QTEST_GUILESS_MAIN(PdfRenderSizeTest)

#include "main.moc"
