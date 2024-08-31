#include "resize_image.h"

#include "qmath.h"
#include <QtConcurrent>
#include <QImage>
#include <QColor>

QPixmap scalePixmapBicubic(const QPixmap &pixmap, int width, int height);
QPixmap scalePixmapLanczos(const QPixmap &pixmap, int width, int height);
QPixmap scalePixmapArea(const QPixmap &pixmap, int width, int height);
QPixmap scalePixmapLanczosQt(const QPixmap &pixmap, int targetWidth, int targetHeight, int a = 3);

QPixmap smartScalePixmap(const QPixmap &pixmap, int width, int height)
{
    const int w = pixmap.width();
    const int h = pixmap.height();
    if ((w == width && h == height) || pixmap.isNull()) {
        return pixmap;
    }

    if (w <= width && h <= height) { // upscaling
        return scalePixmapLanczos(pixmap, width, height);
    }

    return pixmap;
}

QPixmap scalePixmap(const QPixmap &pixmap, int width, int height, ScaleMethod method)
{
    const int w = pixmap.width();
    const int h = pixmap.height();
    if (w == width && h == height) {
        return pixmap;
    }

    switch (method) {
    case ScaleMethod::QtFast:
        return pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::FastTransformation);
    case ScaleMethod::QtSmooth:
        return pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    case ScaleMethod::Bicubic:
        return scalePixmapBicubic(pixmap, width, height);
    case ScaleMethod::Lanczos:
        return scalePixmapLanczos(pixmap, width, height);
    case ScaleMethod::Area:
        return scalePixmapArea(pixmap, width, height);
    }
}

QPixmap scalePixmapBicubic(const QPixmap &pixmap, int width, int height)
{
    // TODO: implement
    return pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QPixmap scalePixmapLanczos(const QPixmap &pixmap, int width, int height)
{
    return scalePixmapLanczosQt(pixmap, width, height);
}

QPixmap scalePixmapArea(const QPixmap &pixmap, int width, int height)
{
    // TODO: implement
    return pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

// Platform-specific SIMD includes and checks
#if defined(__AVX__) || defined(__AVX2__)
#include <immintrin.h> // For x86 SSE/AVX
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h> // For ARM NEON
#else
#warning "No SIMD instructions detected, falling back to scalar implementation."
#endif

// Define SIMD intrinsics for different platforms
#if defined(__AVX__) || defined(__AVX2__)

// Function to normalize angles in radians to the range [-PI, PI]
__m256d normalize_angle(__m256d x)
{
    const __m256d pi = _mm256_set1_pd(M_PI);
    const __m256d two_pi = _mm256_set1_pd(2 * M_PI);
    // Calculate the quotient of x / (2*PI)
    __m256d quotient = _mm256_div_pd(x, two_pi);
    // Use floor to get the nearest lower integer
    quotient = _mm256_floor_pd(quotient);
    // Calculate the remainder
    __m256d remainder = _mm256_sub_pd(x, _mm256_mul_pd(quotient, two_pi));
    // Adjust the range to [-PI, PI]
    __m256d adjust = _mm256_cmp_pd(remainder, pi, _CMP_GT_OS);
    remainder = _mm256_sub_pd(remainder, _mm256_and_pd(adjust, two_pi));
    return remainder;
}

// Improved sine approximation function for __m256d using the normalized angle
__m256d sin_pd_approx(__m256d x)
{
    x = normalize_angle(x); // Normalize x to the range [-PI, PI]

    // Sine approximation coefficients
    const __m256d a0 = _mm256_set1_pd(-0.16666666666666666);
    const __m256d a1 = _mm256_set1_pd(0.008333333333333333);
    const __m256d a2 = _mm256_set1_pd(-0.0001984126984126984);

    __m256d x2 = _mm256_mul_pd(x, x);
    __m256d x3 = _mm256_mul_pd(x2, x);
    __m256d x5 = _mm256_mul_pd(x3, x2);
    __m256d x7 = _mm256_mul_pd(x5, x2);

    // Compute the polynomial approximation
    __m256d result = _mm256_add_pd(x, _mm256_mul_pd(a0, x3));
    result = _mm256_add_pd(result, _mm256_mul_pd(a1, x5));
    result = _mm256_add_pd(result, _mm256_mul_pd(a2, x7));

    return result;
}

inline __m256d lanczosKernelAVX(const __m256d &x, const __m256d &a_val)
{
    __m256d zero = _mm256_setzero_pd();
    __m256d one = _mm256_set1_pd(1.0);
    __m256d pix = _mm256_mul_pd(_mm256_set1_pd(M_PI), x);
    __m256d sin_pix = sin_pd_approx(pix);
    __m256d sin_pix_a = sin_pd_approx(_mm256_div_pd(pix, a_val));
    __m256d numerator = _mm256_mul_pd(_mm256_mul_pd(a_val, sin_pix), sin_pix_a);
    __m256d denominator = _mm256_mul_pd(pix, pix);
    __m256d result = _mm256_div_pd(numerator, denominator);
    result = _mm256_blendv_pd(result, one, _mm256_cmp_pd(x, zero, _CMP_EQ_OQ));
    return result;
}

QVector<QRgb> processRow(int y, int targetWidth, int targetHeight, const QImage &sourceImage, int a)
{
    QVector<QRgb> resultRow(targetWidth);
    int sourceWidth = sourceImage.width();
    int sourceHeight = sourceImage.height();
    __m256d a_vec = _mm256_set1_pd(a);

    for (int x = 0; x < targetWidth; ++x) {
        double gx = ((double)x / targetWidth) * (sourceWidth - 1);
        double gy = ((double)y / targetHeight) * (sourceHeight - 1);

        __m256d red_vec = _mm256_setzero_pd();
        __m256d green_vec = _mm256_setzero_pd();
        __m256d blue_vec = _mm256_setzero_pd();
        __m256d alpha_vec = _mm256_setzero_pd();
        __m256d weight_vec = _mm256_setzero_pd();

        for (int ix = (int)gx - a + 1; ix <= (int)gx + a; ++ix) {
            for (int iy = (int)gy - a + 1; iy <= (int)gy + a; ++iy) {
                if (ix >= 0 && ix < sourceWidth && iy >= 0 && iy < sourceHeight) {
                    __m256d gx_vec = _mm256_set1_pd(gx - ix);
                    __m256d gy_vec = _mm256_set1_pd(gy - iy);
                    __m256d weight_x = lanczosKernelAVX(gx_vec, a_vec);
                    __m256d weight_y = lanczosKernelAVX(gy_vec, a_vec);
                    __m256d weight = _mm256_mul_pd(weight_x, weight_y);

                    QColor color(sourceImage.pixel(ix, iy));
                    __m256d color_red = _mm256_set1_pd(color.red());
                    __m256d color_green = _mm256_set1_pd(color.green());
                    __m256d color_blue = _mm256_set1_pd(color.blue());
                    __m256d color_alpha = _mm256_set1_pd(color.alpha());

                    red_vec = _mm256_add_pd(red_vec, _mm256_mul_pd(weight, color_red));
                    green_vec = _mm256_add_pd(green_vec, _mm256_mul_pd(weight, color_green));
                    blue_vec = _mm256_add_pd(blue_vec, _mm256_mul_pd(weight, color_blue));
                    alpha_vec = _mm256_add_pd(alpha_vec, _mm256_mul_pd(weight, color_alpha));

                    weight_vec = _mm256_add_pd(weight_vec, weight);
                }
            }
        }

        double red = _mm256_cvtsd_f64(_mm256_hadd_pd(red_vec, red_vec));
        double green = _mm256_cvtsd_f64(_mm256_hadd_pd(green_vec, green_vec));
        double blue = _mm256_cvtsd_f64(_mm256_hadd_pd(blue_vec, blue_vec));
        double alpha = _mm256_cvtsd_f64(_mm256_hadd_pd(alpha_vec, alpha_vec));
        double sumWeights = _mm256_cvtsd_f64(_mm256_hadd_pd(weight_vec, weight_vec));

        if (sumWeights > 0.0) {
            red = std::clamp(red / sumWeights, 0.0, 255.0);
            green = std::clamp(green / sumWeights, 0.0, 255.0);
            blue = std::clamp(blue / sumWeights, 0.0, 255.0);
            alpha = std::clamp(alpha / sumWeights, 0.0, 255.0);
        }

        resultRow[x] = qRgba(static_cast<int>(red), static_cast<int>(green), static_cast<int>(blue), static_cast<int>(alpha));
    }

    return resultRow;
}

#elif defined(__ARM_NEON) || defined(__ARM_NEON__)

inline float64x2_t lanczosKernelNEON(const float64x2_t &x, int a)
{
    float64x2_t zero = vdupq_n_f64(0.0);
    float64x2_t one = vdupq_n_f64(1.0);
    float64x2_t a_val = vdupq_n_f64(a);
    float64x2_t pix = vmulq_f64(vdupq_n_f64(M_PI), x);
    float64x2_t sin_pix = vsin_f64(pix);
    float64x2_t sin_pix_a = vsin_f64(vdivq_f64(pix, a_val));
    float64x2_t numerator = vmulq_f64(vmulq_f64(a_val, sin_pix), sin_pix_a);
    float64x2_t denominator = vmulq_f64(pix, pix);
    float64x2_t result = vdivq_f64(numerator, denominator);
    uint64x2_t mask = vceqq_f64(x, zero);
    result = vbslq_f64(mask, one, result);
    return result;
}

QVector<QRgb> processRow(int y, int targetWidth, int targetHeight, const QImage &sourceImage, int a)
{
    QVector<QRgb> resultRow(targetWidth);
    int sourceWidth = sourceImage.width();
    int sourceHeight = sourceImage.height();

    for (int x = 0; x < targetWidth; ++x) {
        double gx = ((double)x / targetWidth) * (sourceWidth - 1);
        double gy = ((double)y / targetHeight) * (sourceHeight - 1);

        float64x2_t red_vec = vdupq_n_f64(0.0);
        float64x2_t green_vec = vdupq_n_f64(0.0);
        float64x2_t blue_vec = vdupq_n_f64(0.0);
        float64x2_t alpha_vec = vdupq_n_f64(0.0);
        float64x2_t weight_vec = vdupq_n_f64(0.0);

        for (int ix = (int)gx - a + 1; ix <= (int)gx + a; ++ix) {
            for (int iy = (int)gy - a + 1; iy <= (int)gy + a; ++iy) {
                if (ix >= 0 && ix < sourceWidth && iy >= 0 && iy < sourceHeight) {
                    float64x2_t gx_vec = vdupq_n_f64(gx - ix);
                    float64x2_t gy_vec = vdupq_n_f64(gy - iy);
                    float64x2_t weight_x = lanczosKernelNEON(gx_vec, a);
                    float64x2_t weight_y = lanczosKernelNEON(gy_vec, a);
                    float64x2_t weight = vmulq_f64(weight_x, weight_y);

                    QColor color(sourceImage.pixel(ix, iy));
                    float64x2_t color_red = vdupq_n_f64(color.red());
                    float64x2_t color_green = vdupq_n_f64(color.green());
                    float64x2_t color_blue = vdupq_n_f64(color.blue());
                    float64x2_t color_alpha = vdupq_n_f64(color.alpha());

                    red_vec = vmlaq_f64(red_vec, weight, color_red);
                    green_vec = vmlaq_f64(green_vec, weight, color_green);
                    blue_vec = vmlaq_f64(blue_vec, weight, color_blue);
                    alpha_vec = vmlaq_f64(alpha_vec, weight, color_alpha);
                    weight_vec = vaddq_f64(weight_vec, weight);
                }
            }
        }

        double red = vgetq_lane_f64(red_vec, 0) + vgetq_lane_f64(red_vec, 1);
        double green = vgetq_lane_f64(green_vec, 0) + vgetq_lane_f64(green_vec, 1);
        double blue = vgetq_lane_f64(blue_vec, 0) + vgetq_lane_f64(blue_vec, 1);
        double alpha = vgetq_lane_f64(alpha_vec, 0) + vgetq_lane_f64(alpha_vec, 1);
        double sumWeights = vgetq_lane_f64(weight_vec, 0) + vgetq_lane_f64(weight_vec, 1);

        if (sumWeights > 0.0) {
            red = std::clamp(red / sumWeights, 0.0, 255.0);
            green = std::clamp(green / sumWeights, 0.0, 255.0);
            blue = std::clamp(blue / sumWeights, 0.0, 255.0);
            alpha = std::clamp(alpha / sumWeights, 0.0, 255.0);
        }

        resultRow[x] = qRgba(static_cast<int>(red), static_cast<int>(green), static_cast<int>(blue), static_cast<int>(alpha));
    }

    return resultRow;
}

#else

// Scalar fallback for unsupported platforms
double lanczosKernel(double x, int a)
{
    if (x == 0.0)
        return 1.0;
    if (x < -a || x > a)
        return 0.0;
    double pix = M_PI * x;
    return a * std::sin(pix) * std::sin(pix / a) / (pix * pix);
}

QVector<QRgb> processRow(int y, int targetWidth, int targetHeight, const QImage &sourceImage, int a)
{
    QVector<QRgb> resultRow(targetWidth);
    int sourceWidth = sourceImage.width();
    int sourceHeight = sourceImage.height();

    for (int x = 0; x < targetWidth; ++x) {
        double gx = ((double)x / targetWidth) * (sourceWidth - 1);
        double gy = ((double)y / targetHeight) * (sourceHeight - 1);

        double red = 0.0, green = 0.0, blue = 0.0, alpha = 0.0, sumWeights = 0.0;

        for (int ix = (int)gx - a + 1; ix <= (int)gx + a; ++ix) {
            for (int iy = (int)gy - a + 1; iy <= (int)gy + a; ++iy) {
                if (ix >= 0 && ix < sourceWidth && iy >= 0 && iy < sourceHeight) {
                    double weight = lanczosKernel(gx - ix, a) * lanczosKernel(gy - iy, a);

                    QColor color(sourceImage.pixel(ix, iy));

                    red += weight * color.red();
                    green += weight * color.green();
                    blue += weight * color.blue();
                    alpha += weight * color.alpha();
                    sumWeights += weight;
                }
            }
        }

        if (sumWeights > 0.0) {
            red = std::clamp(red / sumWeights, 0.0, 255.0);
            green = std::clamp(green / sumWeights, 0.0, 255.0);
            blue = std::clamp(blue / sumWeights, 0.0, 255.0);
            alpha = std::clamp(alpha / sumWeights, 0.0, 255.0);
        }

        resultRow[x] = qRgba(static_cast<int>(red), static_cast<int>(green), static_cast<int>(blue), static_cast<int>(alpha));
    }

    return resultRow;
}

#endif

// Main function to scale the image
QImage scaleImageLanczos(const QImage &sourceImage, int targetWidth, int targetHeight, int a = 3)
{
    QImage targetImage(targetWidth, targetHeight, QImage::Format_ARGB32);

    QVector<int> rows(targetHeight);
    for (int i = 0; i < targetHeight; ++i) {
        rows[i] = i;
    }

    // Use a QVector to store the results of the processed rows
    QVector<QVector<QRgb>> results(targetHeight);

    // Launch concurrent tasks using QtConcurrent::map, which modifies the container in place
    QtConcurrent::blockingMap(rows, [&results, targetWidth, targetHeight, &sourceImage, a](int y) {
        results[y] = processRow(y, targetWidth, targetHeight, sourceImage, a);
    });

    // Set the pixels in the target image
    for (int y = 0; y < targetHeight; ++y) {
        const QVector<QRgb> &row = results[y];
        for (int x = 0; x < targetWidth; ++x) {
            targetImage.setPixel(x, y, row[x]);
        }
    }

    return targetImage;
}
QPixmap scalePixmapLanczosQt(const QPixmap &pixmap, int targetWidth, int targetHeight, int a)
{
    QImage sourceImage = pixmap.toImage();
    QImage scaledImage = scaleImageLanczos(sourceImage, targetWidth, targetHeight, a);
    return QPixmap::fromImage(scaledImage);
}
