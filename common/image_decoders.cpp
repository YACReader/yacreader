#include "image_decoders.h"

#include <avif/avif.h>
#include <jxl/decode.h>
#include <jxl/decode_cxx.h>
#include <jxl/resizable_parallel_runner.h>
#include <jxl/types.h>
#include <vector>

bool isAvif(const QByteArray &data)
{
    if (data.size() < 12)
        return false;
    return (data.at(4) == 'f' && data.at(5) == 't' && data.at(6) == 'y' && data.at(7) == 'p' &&
            data.at(8) == 'a' && data.at(9) == 'v' && data.at(10) == 'i' && data.at(11) == 'f');
}

bool isJxl(const QByteArray &data)
{
    if (data.size() < 2)
        return false;
    
    // Check for raw JXL codestream (starts with FF0A)
    if (static_cast<quint8>(data.at(0)) == 0xFF && static_cast<quint8>(data.at(1)) == 0x0A)
        return true;
    
    // Check for JXL container format (JXL signature box at offset 4)
    if (data.size() >= 12 && data.at(4) == 'J' && data.at(5) == 'X' && data.at(6) == 'L' && data.at(7) == ' ')
        return true;
    
    return false;
}

QImage decodeAvif(const QByteArray &data)
{
    avifDecoder *decoder = avifDecoderCreate();
    avifResult result = avifDecoderSetIOMemory(decoder, (const uint8_t *)data.constData(), data.size());
    if (result != AVIF_RESULT_OK) {
        avifDecoderDestroy(decoder);
        return QImage();
    }

    result = avifDecoderParse(decoder);
    if (result != AVIF_RESULT_OK) {
        avifDecoderDestroy(decoder);
        return QImage();
    }

    QImage image;
    if (avifDecoderNextImage(decoder) == AVIF_RESULT_OK) {
        avifRGBImage rgb;
        avifRGBImageSetDefaults(&rgb, decoder->image);
        rgb.format = AVIF_RGB_FORMAT_RGBA;
        rgb.depth = 8;

        avifRGBImageAllocatePixels(&rgb);
        avifImageYUVToRGB(decoder->image, &rgb);
        image = QImage(rgb.pixels, decoder->image->width, decoder->image->height, QImage::Format_RGBA8888).copy();
        avifRGBImageFreePixels(&rgb);
    }

    avifDecoderDestroy(decoder);
    return image.convertToFormat(QImage::Format_ARGB32);
}

QImage decodeJxl(const QByteArray &data)
{
    auto dec = JxlDecoderMake(nullptr);
    if (JXL_DEC_SUCCESS != JxlDecoderSubscribeEvents(dec.get(), JXL_DEC_BASIC_INFO | JXL_DEC_FULL_IMAGE)) {
        return QImage();
    }

    void* runner = JxlResizableParallelRunnerCreate(nullptr);
    if (JXL_DEC_SUCCESS != JxlDecoderSetParallelRunner(dec.get(), JxlResizableParallelRunner, runner)) {
        JxlResizableParallelRunnerDestroy(runner);
        return QImage();
    }

    JxlBasicInfo info;
    JxlPixelFormat format = {4, JXL_TYPE_UINT8, JXL_LITTLE_ENDIAN, 0};
    std::vector<uint8_t> pixels;

    JxlDecoderSetInput(dec.get(), (const uint8_t *)data.constData(), data.size());
    JxlDecoderCloseInput(dec.get());

    for (;;) {
        JxlDecoderStatus status = JxlDecoderProcessInput(dec.get());
        if (status == JXL_DEC_ERROR) {
            JxlResizableParallelRunnerDestroy(runner);
            return QImage();
        } else if (status == JXL_DEC_NEED_MORE_INPUT) {
            JxlResizableParallelRunnerDestroy(runner);
            return QImage();
        } else if (status == JXL_DEC_BASIC_INFO) {
            if (JXL_DEC_SUCCESS != JxlDecoderGetBasicInfo(dec.get(), &info)) {
                JxlResizableParallelRunnerDestroy(runner);
                return QImage();
            }
            JxlResizableParallelRunnerSetThreads(runner,
                                                 JxlResizableParallelRunnerSuggestThreads(info.xsize, info.ysize));
        } else if (status == JXL_DEC_SUCCESS) {
            break;
        } else if (status == JXL_DEC_FULL_IMAGE) {
            // Nothing to do.
        } else if (status == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
            size_t buffer_size;
            if (JXL_DEC_SUCCESS !=
                JxlDecoderImageOutBufferSize(dec.get(), &format, &buffer_size)) {
                JxlResizableParallelRunnerDestroy(runner);
                return QImage();
            }
            if (buffer_size != info.xsize * info.ysize * 4) {
                JxlResizableParallelRunnerDestroy(runner);
                return QImage();
            }
            pixels.resize(buffer_size);
            if (JXL_DEC_SUCCESS != JxlDecoderSetImageOutBuffer(dec.get(), &format, pixels.data(), pixels.size())) {
                JxlResizableParallelRunnerDestroy(runner);
                return QImage();
            }
        } else {
            JxlResizableParallelRunnerDestroy(runner);
            return QImage();
        }
    }

    JxlResizableParallelRunnerDestroy(runner);

    if(pixels.empty())
        return QImage();

    return QImage(pixels.data(), info.xsize, info.ysize, QImage::Format_RGBA8888).copy().convertToFormat(QImage::Format_ARGB32);
}
