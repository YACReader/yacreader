#include "pdf_comic.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <ApplicationServices/ApplicationServices.h>

#include "QsLog.h"
#include "QsLogDest.h"


MacOSXPDFComic::MacOSXPDFComic()
{

}

MacOSXPDFComic::~MacOSXPDFComic()
{
    CGPDFDocumentRelease((CGPDFDocumentRef)document);
}

bool MacOSXPDFComic::openComic(const QString &path)
{

    CFURLRef pdfFileUrl;
    CFStringRef str;
    str=CFStringCreateWithCString( kCFAllocatorDefault,path.toUtf8().data(),kCFStringEncodingUTF8);
    pdfFileUrl=CFURLCreateWithFileSystemPath( kCFAllocatorDefault,str,kCFURLPOSIXPathStyle,true );

    CGPDFDocumentRef pdf = CGPDFDocumentCreateWithURL((CFURLRef)pdfFileUrl);

    document = pdf;

    CFRelease(str);
    CFRelease(pdfFileUrl);

    return true;
}

void MacOSXPDFComic::closeComic()
{
    //CGPDFDocumentRelease((CGPDFDocumentRef)document);
}

unsigned int MacOSXPDFComic::numPages()
{
   return (int)CGPDFDocumentGetNumberOfPages((CGPDFDocumentRef)document);
}

QImage MacOSXPDFComic::getPage(const int pageNum)
{
    CGPDFPageRef page = CGPDFDocumentGetPage((CGPDFDocumentRef)document, pageNum+1);
    // Changed this line for the line above which is a generic line
    //CGPDFPageRef page = [self getPage:page_number];



    CGRect pageRect = CGPDFPageGetBoxRect(page, kCGPDFMediaBox);
    int width = 1200;

    //NSLog(@"-----%f",pageRect.size.width);
    CGFloat pdfScale = float(width)/pageRect.size.width;

    pageRect.size = CGSizeMake(pageRect.size.width*pdfScale, pageRect.size.height*pdfScale);
    pageRect.origin = CGPointZero;

    CGColorSpaceRef genericColorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef bitmapContext = CGBitmapContextCreate(NULL,
                                                       pageRect.size.width,
                                                       pageRect.size.height,
                                                       8, 0,
                                                       genericColorSpace,
                                                       kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little
                                                       );

    CGContextSetInterpolationQuality(bitmapContext, kCGInterpolationHigh);
    CGContextSetRenderingIntent(bitmapContext, kCGRenderingIntentDefault);
    CGContextSetRGBFillColor( bitmapContext, 1.0, 1.0, 1.0, 1.0 );
    CGContextFillRect( bitmapContext, CGContextGetClipBoundingBox( bitmapContext ));

    //CGContextTranslateCTM( bitmapContext, 0, pageRect.size.height );
    //CGContextScaleCTM( bitmapContext, 1.0, -1.0 );

    CGContextConcatCTM(bitmapContext, CGAffineTransformMakeScale(pdfScale, pdfScale));


    /*CGAffineTransform pdfXfm = CGPDFPageGetDrawingTransform( page, kCGPDFMediaBox, CGRectMake(pageRect.origin.x, pageRect.origin.y, pageRect.size.width, pageRect.size.height) , 0, true );
    */
    //CGContextConcatCTM( bitmapContext, pdfXfm );

    CGContextDrawPDFPage(bitmapContext, page);

    CGImageRef image = CGBitmapContextCreateImage(bitmapContext);

    QImage qtImage;

    CFDataRef dataRef = CGDataProviderCopyData(CGImageGetDataProvider(image));

    lastPageData = (void *)dataRef;

    const uchar *bytes = (const uchar *)CFDataGetBytePtr(dataRef);

    qtImage = QImage(bytes, pageRect.size.width, pageRect.size.height, QImage::Format_ARGB32);

    CGImageRelease(image);
    //CFRelease(dataRef);
    CGContextRelease(bitmapContext);
    //CGPDFPageRelease(page);
    CGColorSpaceRelease(genericColorSpace);

    return qtImage;
}

void MacOSXPDFComic::releaseLastPageData()
{
    CFRelease((CFDataRef)lastPageData);
}

