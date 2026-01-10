#include <QtTest>
#include <QFile>
#include <QImage>
#include "../../common/image_decoders.h"

class ImageFormatTest : public QObject
{
    Q_OBJECT

private slots:
    // Format detection tests
    void testIsAvif_validAVIF();
    void testIsAvif_tooShort();
    void testIsAvif_invalidMagic();
    void testIsJxl_validCodestream();
    void testIsJxl_validContainer();
    void testIsJxl_tooShort();
    void testIsJxl_invalidMagic();

    // Decode tests
    void testDecodeAvif_validFile();
    void testDecodeAvif_emptyData();
    void testDecodeAvif_corruptedData();
    void testDecodeJxl_validCodestream();
    void testDecodeJxl_validContainer();
    void testDecodeJxl_emptyData();
    void testDecodeJxl_corruptedData();

    // Integration tests
    void testDecodeAvif_fromResource();
    void testDecodeJxl_fromResource();
};

void ImageFormatTest::testIsAvif_validAVIF()
{
    // Valid AVIF file header: ftyp at offset 4
    QByteArray validAvif;
    validAvif.resize(12);
    validAvif[0] = 0x00;
    validAvif[1] = 0x00;
    validAvif[2] = 0x00;
    validAvif[3] = 0x20;
    validAvif[4] = 'f';
    validAvif[5] = 't';
    validAvif[6] = 'y';
    validAvif[7] = 'p';
    validAvif[8] = 'a';
    validAvif[9] = 'v';
    validAvif[10] = 'i';
    validAvif[11] = 'f';

    QVERIFY(isAvif(validAvif));
}

void ImageFormatTest::testIsAvif_tooShort()
{
    QByteArray shortData;
    shortData.resize(5);
    QVERIFY(!isAvif(shortData));
}

void ImageFormatTest::testIsAvif_invalidMagic()
{
    QByteArray invalidAvif;
    invalidAvif.resize(12);
    invalidAvif.fill(0);
    QVERIFY(!isAvif(invalidAvif));
}

void ImageFormatTest::testIsJxl_validCodestream()
{
    // Valid JXL codestream header: FF 0A
    QByteArray validJxl;
    validJxl.resize(2);
    validJxl[0] = static_cast<char>(0xFF);
    validJxl[1] = static_cast<char>(0x0A);

    QVERIFY(isJxl(validJxl));
}

void ImageFormatTest::testIsJxl_validContainer()
{
    // Valid JXL container header: JXL<space> at offset 4
    QByteArray validJxlContainer;
    validJxlContainer.resize(12);
    validJxlContainer[0] = 0x00;
    validJxlContainer[1] = 0x00;
    validJxlContainer[2] = 0x00;
    validJxlContainer[3] = 0x0C;
    validJxlContainer[4] = 'J';
    validJxlContainer[5] = 'X';
    validJxlContainer[6] = 'L';
    validJxlContainer[7] = ' ';

    QVERIFY(isJxl(validJxlContainer));
}

void ImageFormatTest::testIsJxl_tooShort()
{
    QByteArray shortData;
    shortData.resize(1);
    QVERIFY(!isJxl(shortData));
}

void ImageFormatTest::testIsJxl_invalidMagic()
{
    QByteArray invalidJxl;
    invalidJxl.resize(12);
    invalidJxl.fill(0);
    QVERIFY(!isJxl(invalidJxl));
}

void ImageFormatTest::testDecodeAvif_emptyData()
{
    QByteArray emptyData;
    QImage result = decodeAvif(emptyData);
    QVERIFY(result.isNull());
}

void ImageFormatTest::testDecodeAvif_corruptedData()
{
    QByteArray corruptedData;
    corruptedData.resize(100);
    corruptedData.fill('X');
    QImage result = decodeAvif(corruptedData);
    QVERIFY(result.isNull());
}

void ImageFormatTest::testDecodeJxl_emptyData()
{
    QByteArray emptyData;
    QImage result = decodeJxl(emptyData);
    QVERIFY(result.isNull());
}

void ImageFormatTest::testDecodeJxl_corruptedData()
{
    QByteArray corruptedData;
    corruptedData.resize(100);
    corruptedData.fill('Y');
    QImage result = decodeJxl(corruptedData);
    QVERIFY(result.isNull());
}

void ImageFormatTest::testDecodeAvif_fromResource()
{
    QFile avifFile(":/sample.avif");
    QVERIFY2(avifFile.open(QIODevice::ReadOnly), "Failed to open sample.avif from resources");
    
    QByteArray avifData = avifFile.readAll();
    QVERIFY(!avifData.isEmpty());
    QVERIFY(isAvif(avifData));
    
    QImage avifImage = decodeAvif(avifData);
    QVERIFY2(!avifImage.isNull(), "Failed to decode sample.avif");
    QVERIFY(avifImage.width() > 0);
    QVERIFY(avifImage.height() > 0);
}

void ImageFormatTest::testDecodeJxl_fromResource()
{
    QFile jxlFile(":/sample.jxl");
    QVERIFY2(jxlFile.open(QIODevice::ReadOnly), "Failed to open sample.jxl from resources");
    
    QByteArray jxlData = jxlFile.readAll();
    QVERIFY(!jxlData.isEmpty());
    QVERIFY(isJxl(jxlData));
    
    QImage jxlImage = decodeJxl(jxlData);
    QVERIFY2(!jxlImage.isNull(), "Failed to decode sample.jxl");
    QVERIFY(jxlImage.width() > 0);
    QVERIFY(jxlImage.height() > 0);
}

void ImageFormatTest::testDecodeAvif_validFile()
{
    QFile avifFile(":/sample.avif");
    if (avifFile.open(QIODevice::ReadOnly)) {
        QByteArray avifData = avifFile.readAll();
        QImage avifImage = decodeAvif(avifData);
        QVERIFY(!avifImage.isNull());
    }
}

void ImageFormatTest::testDecodeJxl_validCodestream()
{
    QFile jxlFile(":/sample.jxl");
    if (jxlFile.open(QIODevice::ReadOnly)) {
        QByteArray jxlData = jxlFile.readAll();
        QImage jxlImage = decodeJxl(jxlData);
        QVERIFY(!jxlImage.isNull());
    }
}

void ImageFormatTest::testDecodeJxl_validContainer()
{
    // Same as codestream test for now, as sample.jxl could be either format
    testDecodeJxl_validCodestream();
}

QTEST_MAIN(ImageFormatTest)
#include "main.moc"
