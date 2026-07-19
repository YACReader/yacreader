#include "pdf_comic.h"

#include "pdf_render_size.h"

#undef __OBJC_BOOL_IS_BOOL

#include "QsLog.h"
#include "QsLogDest.h"

#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#import <Foundation/Foundation.h>
#import <PDFKit/PDFKit.h>
#include <cmath>

namespace {

PDFDisplayBox preferredDisplayBox(PDFPage *page)
{
    if (page == nil) {
        return kPDFDisplayBoxMediaBox;
    }

    CGRect cropRect = CGRectStandardize([page boundsForBox:kPDFDisplayBoxCropBox]);
    if (cropRect.size.width > 0 && cropRect.size.height > 0) {
        return kPDFDisplayBoxCropBox;
    }

    return kPDFDisplayBoxMediaBox;
}

bool isValidRenderDimension(CGFloat value)
{
    return std::isfinite(static_cast<double>(value)) && value >= 1.0;
}

}

MacOSXPDFComic::MacOSXPDFComic()
    : document(nullptr), lastPageData(nullptr)
{
}

MacOSXPDFComic::~MacOSXPDFComic()
{
    closeComic();
}

bool MacOSXPDFComic::openComic(const QString &path)
{
    closeComic();

    QByteArray utf8Path = path.toUtf8();
    NSString *nsPath = [[NSString alloc] initWithUTF8String:utf8Path.constData()];
    NSURL *pdfFileUrl = [NSURL fileURLWithPath:nsPath isDirectory:NO];
    PDFDocument *pdf = [[PDFDocument alloc] initWithURL:pdfFileUrl];

    [nsPath release];

    if (pdf == nil) {
        return false;
    }

    document = pdf;
    return true;
}

void MacOSXPDFComic::closeComic()
{
    if (document != nullptr) {
        [(PDFDocument *)document release];
        document = nullptr;
    }
}

unsigned int MacOSXPDFComic::numPages()
{
    PDFDocument *pdf = (PDFDocument *)document;
    return pdf != nil ? static_cast<unsigned int>(pdf.pageCount) : 0;
}

QImage MacOSXPDFComic::getPage(const int pageNum)
{
    PDFDocument *pdf = (PDFDocument *)document;
    if (pdf == nil) {
        return QImage();
    }

    PDFPage *page = [pdf pageAtIndex:pageNum];
    if (page == nil) {
        return QImage();
    }

    PDFDisplayBox displayBox = preferredDisplayBox(page);
    CGRect sourceRect = CGRectStandardize([page boundsForBox:displayBox]);
    if (!isValidRenderDimension(sourceRect.size.width) || !isValidRenderDimension(sourceRect.size.height)) {
        return QImage();
    }

    const QSize renderSize = YACReaderPdfRender::renderSizeFromPagePoints(QSizeF(sourceRect.size.width,
                                                                                 sourceRect.size.height));
    if (!renderSize.isValid()) {
        return QImage();
    }

    const int imageWidth = renderSize.width();
    const int imageHeight = renderSize.height();
    const CGFloat pdfScale = static_cast<CGFloat>(imageWidth) / sourceRect.size.width;

    CGColorSpaceRef genericColorSpace = CGColorSpaceCreateDeviceRGB();

    QImage renderImage(imageWidth, imageHeight, QImage::Format_ARGB32_Premultiplied);
    if (renderImage.isNull()) {
        CGColorSpaceRelease(genericColorSpace);
        return QImage();
    }

    const uint32_t bitmapInfo = static_cast<uint32_t>(kCGImageAlphaPremultipliedFirst) |
            static_cast<uint32_t>(kCGBitmapByteOrder32Little);

    CGContextRef bitmapContext = CGBitmapContextCreate(renderImage.scanLine(0),
                                                       imageWidth,
                                                       imageHeight,
                                                       8, renderImage.bytesPerLine(),
                                                       genericColorSpace,
                                                       bitmapInfo // may need to be changed to kCGBitmapByteOrder32Big
    );
    if (bitmapContext == nullptr) {
        CGColorSpaceRelease(genericColorSpace);
        return QImage();
    }

    CGContextSetInterpolationQuality(bitmapContext, kCGInterpolationHigh);
    CGContextSetRenderingIntent(bitmapContext, kCGRenderingIntentDefault);
    CGContextSetRGBFillColor(bitmapContext, 1.0, 1.0, 1.0, 1.0);
    CGContextFillRect(bitmapContext, CGContextGetClipBoundingBox(bitmapContext));

    CGContextSaveGState(bitmapContext);
    CGContextScaleCTM(bitmapContext, pdfScale, pdfScale);
    [page drawWithBox:displayBox toContext:bitmapContext];
    CGContextRestoreGState(bitmapContext);

    CGContextRelease(bitmapContext);
    CGColorSpaceRelease(genericColorSpace);

    return renderImage;
}

/*void MacOSXPDFComic::releaseLastPageData()
{
    CFRelease((CFDataRef)lastPageData);
}*/
