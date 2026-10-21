#include "resize_image.h"

#include "lancir.h"

#include <algorithm>
#include <cmath>
#include <vector>

static QImage scaleImageLancir(const QImage &image, int width, int height)
{
    QImage src = (image.format() == QImage::Format_ARGB32)
            ? image
            : image.convertToFormat(QImage::Format_ARGB32);

    QImage dst(width, height, QImage::Format_ARGB32);

    // SrcSSize / NewSSize are in elements; for uint8_t that equals bytes,
    // so bytesPerLine() covers any Qt row-alignment padding correctly.
    avir::CLancIRParams params(src.bytesPerLine(), dst.bytesPerLine());
    params.la = 4.0; // Lanczos4

    avir::CLancIR lancir;
    lancir.resizeImage<uint8_t, uint8_t>(
            src.constBits(), src.width(), src.height(),
            dst.bits(), width, height, 4, &params);

    return dst;
}

// ---- Bicubic (Catmull-Rom, separable two-pass resampler) ---------------------

#include <thread>

struct BicubicTap {
    int index;
    int weight; // 16.16 fixed point, taps sum to 1<<16
};

// Catmull-Rom kernel (B=0, C=1/2): the classic "bicubic" of image editors.
static inline double bicubicKernel(double t)
{
    t = std::fabs(t);
    if (t <= 1.0)
        return 1.5 * t * t * t - 2.5 * t * t + 1.0;
    if (t < 2.0)
        return -0.5 * t * t * t + 2.5 * t * t - 4.0 * t + 2.0;
    return 0.0;
}

// Per-destination-tap weights. When downscaling (scale > 1) the kernel is
// widened by the scale factor so each output pixel integrates the source area
// it covers, which keeps aliasing/moiré under control.
static std::vector<std::vector<BicubicTap>> bicubicContributions(int srcSize, int dstSize)
{
    std::vector<std::vector<BicubicTap>> contributions(dstSize);
    const double scale = double(srcSize) / double(dstSize);
    const double filterScale = scale > 1.0 ? scale : 1.0;

    for (int i = 0; i < dstSize; ++i) {
        const double center = (i + 0.5) * scale - 0.5;
        const int left = std::max(0, int(std::ceil(center - 2.0 * filterScale)));
        const int right = std::min(srcSize - 1, int(std::floor(center + 2.0 * filterScale)));

        std::vector<double> raw;
        raw.reserve(8);
        double sum = 0.0;
        for (int j = left; j <= right; ++j) {
            const double w = bicubicKernel((j - center) / filterScale);
            if (w == 0.0)
                continue;
            raw.push_back(w);
            sum += w;
        }
        size_t k = 0;
        for (int j = left; j <= right && k < raw.size(); ++j) {
            const double w = bicubicKernel((j - center) / filterScale);
            if (w == 0.0)
                continue;
            contributions[i].push_back({ j, int(std::llround(raw[k] / sum * 65536.0)) });
            ++k;
        }
    }
    return contributions;
}

static inline uint8_t clampFinal(int v)
{
    if (v < 0)
        return 0;
    if (v > 255)
        return 255;
    return uint8_t(v);
}

static inline int16_t clampIntermediate(int v)
{
    // signed 8.6 intermediate for the alpha path: must carry Catmull-Rom
    // overshoot, NOT clip it — clamping between passes destroys the
    // negative-lobe energy that later cancels out and craters quality.
    if (v < -(511 << 6))
        return int16_t(-(511 << 6));
    if (v > 511 << 6)
        return int16_t(511 << 6);
    return int16_t(v);
}

// ---- gamma-correct (linear-light) resampling tables ----------------------------
//
// Weighted averaging must happen on linear luminance, not on gamma-encoded sRGB
// bytes; otherwise high-contrast edges darken systematically. Linear values live
// in [0,1] but the dark end collapses (sRGB 1/255 -> linear 0.0003), so they are
// quantized to 14 bits to keep shadow gradation. Alpha is not gamma-encoded and
// stays on the plain 8.6 path.
struct LinearLightLuts {
    int16_t decode[256]; // sRGB byte -> linear * 16384
    uint8_t encode[16385]; // linear * 16384 -> sRGB byte
    LinearLightLuts()
    {
        auto toLinear = [](double v) {
            return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
        };
        auto toSrgb = [](double v) {
            return v <= 0.0031308 ? v * 12.92 : 1.055 * std::pow(v, 1.0 / 2.4) - 0.055;
        };
        for (int c = 0; c < 256; ++c)
            decode[c] = int16_t(std::lround(toLinear(c / 255.0) * 16384.0));
        for (int i = 0; i <= 16384; ++i)
            encode[i] = uint8_t(std::lround(toSrgb(std::min(1.0, i / 16384.0)) * 255.0));
    }
};

static inline int16_t clampLinear14(int v)
{
    // signed 14-bit linear intermediate carrying Catmull-Rom overshoot
    if (v < -8192)
        return int16_t(-8192);
    if (v > 24576)
        return int16_t(24576);
    return int16_t(v);
}

static QImage scaleImageBicubic(const QImage &image, int width, int height)
{
    QImage src = (image.format() == QImage::Format_ARGB32)
            ? image
            : image.convertToFormat(QImage::Format_ARGB32);

    static const LinearLightLuts luts; // function-local static: thread-safe init

    const int sw = src.width();
    const int sh = src.height();

    // Horizontal pass. RGB into signed 14-bit linear-light; alpha into 8.6.
    std::vector<int16_t> tmp(size_t(width) * sh * 4);
    const auto hWeights = bicubicContributions(sw, width);
    const auto hPass = [&](int yBegin, int yEnd) {
        for (int y = yBegin; y < yEnd; ++y) {
            const QRgb *row = reinterpret_cast<const QRgb *>(src.constScanLine(y));
            int16_t *outRow = &tmp[size_t(y) * width * 4];
            for (int x = 0; x < width; ++x) {
                int32_t b = 0, g = 0, r = 0, a = 0;
                for (const auto &tap : hWeights[x]) {
                    const QRgb px = row[tap.index];
                    b += int32_t(tap.weight * luts.decode[qBlue(px)]);
                    g += int32_t(tap.weight * luts.decode[qGreen(px)]);
                    r += int32_t(tap.weight * luts.decode[qRed(px)]);
                    a += int32_t(tap.weight * qAlpha(px));
                }
                int16_t *out = &outRow[size_t(x) * 4];
                out[0] = clampLinear14((b + (1 << 15)) >> 16);
                out[1] = clampLinear14((g + (1 << 15)) >> 16);
                out[2] = clampLinear14((r + (1 << 15)) >> 16);
                out[3] = clampIntermediate((a + (1 << 15)) >> 16 << 6);
            }
        }
    };

    // Vertical pass into the destination image.
    QImage dst(width, height, QImage::Format_ARGB32);
    const auto vWeights = bicubicContributions(sh, height);
    const auto vPass = [&](int yBegin, int yEnd) {
        for (int y = yBegin; y < yEnd; ++y) {
            QRgb *row = reinterpret_cast<QRgb *>(dst.scanLine(y));
            for (int x = 0; x < width; ++x) {
                int64_t b = 0, g = 0, r = 0, a = 0;
                for (const auto &tap : vWeights[y]) {
                    const int16_t *in = &tmp[(size_t(tap.index) * width + x) * 4];
                    b += int64_t(tap.weight) * in[0];
                    g += int64_t(tap.weight) * in[1];
                    r += int64_t(tap.weight) * in[2];
                    a += int64_t(tap.weight) * in[3];
                }
                auto enc = [](int64_t acc) -> uint8_t {
                    // acc = sum(2^16 weights * 2^14 linear) -> linear14 = acc >> 16
                    int i = int((acc + (int64_t(1) << 15)) >> 16);
                    if (i < 0)
                        i = 0;
                    if (i > 16384)
                        i = 16384;
                    return luts.encode[i];
                };
                row[x] = qRgba(enc(r), enc(g), enc(b), clampFinal(int((a + (1 << 21)) >> 22)));
            }
        }
    };

    // Modest parallelism: scaling already runs off the GUI thread (dedicated
    // scale pool in continuous mode), so this just splits the heavy passes
    // across cores without touching any Qt thread pool.
    const unsigned hw = std::max(1u, std::min(4u, std::thread::hardware_concurrency()));
    if (hw == 1 || (size_t(width) * sh) < 262144) { // < 512x512 source: threading overhead not worth it
        hPass(0, sh);
        vPass(0, height);
    } else {
        std::vector<std::thread> threads;
        threads.reserve(hw - 1);
        for (unsigned t = 1; t < hw; ++t)
            threads.emplace_back(hPass, int(uint64_t(sh) * t / hw), int(uint64_t(sh) * (t + 1) / hw));
        hPass(0, int(uint64_t(sh) / hw));
        for (auto &t : threads)
            t.join();

        threads.clear();
        for (unsigned t = 1; t < hw; ++t)
            threads.emplace_back(vPass, int(uint64_t(height) * t / hw), int(uint64_t(height) * (t + 1) / hw));
        vPass(0, int(uint64_t(height) / hw));
        for (auto &t : threads)
            t.join();
    }
    return dst;
}

// ---- QPixmap API ------------------------------------------------------------

QPixmap scalePixmap(const QPixmap &pixmap, int width, int height, ScaleMethod method)
{
    if ((pixmap.width() == width && pixmap.height() == height) || pixmap.isNull())
        return pixmap;

    switch (method) {
    case ScaleMethod::Nearest:
        return pixmap.scaled(width, height, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    case ScaleMethod::Bilinear:
        return pixmap.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    case ScaleMethod::Lanczos:
        return QPixmap::fromImage(scaleImageLancir(pixmap.toImage(), width, height));
    case ScaleMethod::Bicubic:
        return QPixmap::fromImage(scaleImageBicubic(pixmap.toImage(), width, height));
    case ScaleMethod::Lanczos3:
        return QPixmap::fromImage(scaleImageLancir(pixmap.toImage(), width, height, 3.0));
    }
    return pixmap;
}

// ---- QImage API (avoids QPixmap round-trip in ContinuousPageWidget) ---------

QImage scaleImage(const QImage &image, int width, int height, ScaleMethod method)
{
    if ((image.width() == width && image.height() == height) || image.isNull())
        return image;

    switch (method) {
    case ScaleMethod::Nearest:
        return image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    case ScaleMethod::Bilinear:
        return image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    case ScaleMethod::Lanczos:
        return scaleImageLancir(image, width, height);
    case ScaleMethod::Bicubic:
        return scaleImageBicubic(image, width, height);
    }
    return image;
}
