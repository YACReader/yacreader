/**
 * @file lancir.h
 *
 * @version 3.1
 *
 * @brief Self-contained header-only "LANCIR" image resizing algorithm.
 *
 * This is a self-contained inclusion file for the "LANCIR" image resizer,
 * a part of the AVIR library. Features scalar, AVX, SSE2, NEON, and WASM
 * SIMD128 optimizations as well as batched resizing technique which provides
 * a better CPU cache performance.
 *
 * AVIR Copyright (c) 2015-2025 Aleksey Vaneev
 *
 * @mainpage
 *
 * @section intro_sec Introduction
 *
 * Description is available at https://github.com/avaneev/avir
 *
 * @section license License
 *
 * LICENSE:
 *
 * Copyright (c) 2015-2025 Aleksey Vaneev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef AVIR_CLANCIR_INCLUDED
#define AVIR_CLANCIR_INCLUDED

#include <cmath>
#include <cstring>

#if __cplusplus >= 201103L

#include <cstdint>

#else // __cplusplus >= 201103L

#include <stdint.h>

#endif // __cplusplus >= 201103L

/**
 * @def LANCIR_ALIGN
 * @brief Address alignment (granularity) used by resizing functions,
 * in bytes.
 */

/**
 * @def LANCIR_NULLPTR
 * @brief Macro is defined, if `nullptr` workaround is in use, for pre-C++11
 * compilers. Undefined at the end of file.
 */

#if defined(__AVX__)

#include <immintrin.h>

#define LANCIR_AVX
#define LANCIR_SSE2 // Some functions use SSE2; AVX has a higher priority.
#define LANCIR_ALIGN 32

#elif defined(__aarch64__) || defined(__arm64__) || \
        defined(_M_ARM64) || defined(_M_ARM64EC)

#if defined(_MSC_VER)
#include <arm64_neon.h>

#if _MSC_VER < 1925
#define LANCIR_ARM32 // Do not use some newer NEON intrinsics.
#endif // _MSC_VER < 1925
#else // defined( _MSC_VER )
#include <arm_neon.h>
#endif // defined( _MSC_VER )

#define LANCIR_NEON
#define LANCIR_ALIGN 16

#elif defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(_M_ARM)

#include <arm_neon.h>

#define LANCIR_ARM32
#define LANCIR_NEON
#define LANCIR_ALIGN 16

#elif defined(__SSE2__) || defined(_M_AMD64) || \
        (defined(_M_IX86_FP) && _M_IX86_FP == 2)

#if defined(_MSC_VER)
#include <intrin.h>
#else // defined( _MSC_VER )
#include <emmintrin.h>
#endif // defined( _MSC_VER )

#define LANCIR_SSE2
#define LANCIR_ALIGN 16

#elif defined(__wasm_simd128__)

#include <wasm_simd128.h>

#define LANCIR_WASM
#define LANCIR_ALIGN 16

#else // WASM

#define LANCIR_ALIGN 4

#endif // WASM

#if defined(LANCIR_SSE2)

#define lancvec_t __m128
#define lancvec_const_splat(v) _mm_set1_ps(v)
#define lancvec_load(m) _mm_load_ps(m)
#define lancvec_loadu(m) _mm_loadu_ps(m)
#define lancvec_store(m, v) _mm_store_ps(m, v)
#define lancvec_storeu(m, v) _mm_storeu_ps(m, v)
#define lancvec_add(v1, v2) _mm_add_ps(v1, v2)
#define lancvec_mul(v1, v2) _mm_mul_ps(v1, v2)
#define lancvec_min(v1, v2) _mm_min_ps(v1, v2)
#define lancvec_max(v1, v2) _mm_max_ps(v1, v2)
#define lancvec_madd(va, v1, v2) _mm_add_ps(va, _mm_mul_ps(v1, v2))
#define lancvec_addhl(vl, vh) _mm_add_ps(vl, _mm_movehl_ps(vh, vh))
#define lancvec_store32_addhl(m, v) \
    _mm_store_ss(m, _mm_add_ss(v, _mm_shuffle_ps(v, v, 1)))

#define lancvec_store64_addhl(m, v) \
    _mm_storel_pi((__m64 *)(m), lancvec_addhl(v, v))

#elif defined(LANCIR_NEON)

#define lancvec_t float32x4_t
#define lancvec_const_splat(v) vdupq_n_f32(v)
#define lancvec_load(m) vld1q_f32(m)
#define lancvec_store(m, v) vst1q_f32(m, v)
#define lancvec_add(v1, v2) vaddq_f32(v1, v2)
#define lancvec_mul(v1, v2) vmulq_f32(v1, v2)
#define lancvec_min(v1, v2) vminq_f32(v1, v2)
#define lancvec_max(v1, v2) vmaxq_f32(v1, v2)
#define lancvec_madd(va, v1, v2) vmlaq_f32(va, v1, v2)

#if defined(LANCIR_ARM32)
#define lancvec_store32_hadd(m, v)                        \
    {                                                     \
        const float32x2_t v2 = vadd_f32(vget_high_f32(v), \
                                        vget_low_f32(v)); \
        *(m) = vget_lane_f32(v2, 0) +                     \
                vget_lane_f32(v2, 1);                     \
    }                                                     \
    (void)0
#else // defined( LANCIR_ARM32 )
#define lancvec_store32_hadd(m, v) *(m) = vaddvq_f32(v)
#endif // defined( LANCIR_ARM32 )

#define lancvec_store64_addhl(m, v) \
    vst1_f32(m, vadd_f32(vget_high_f32(v), vget_low_f32(v)));

#elif defined(LANCIR_WASM)

#define lancvec_t v128_t
#define lancvec_const_splat(v) wasm_f32x4_const_splat(v)
#define lancvec_load32_splat(m) wasm_v128_load32_splat(m)
#define lancvec_load(m) wasm_v128_load(m)
#define lancvec_store(m, v) wasm_v128_store(m, v)
#define lancvec_add(v1, v2) wasm_f32x4_add(v1, v2)
#define lancvec_mul(v1, v2) wasm_f32x4_mul(v1, v2)
#define lancvec_min(v1, v2) wasm_f32x4_min(v1, v2)
#define lancvec_max(v1, v2) wasm_f32x4_max(v1, v2)
#define lancvec_madd(va, v1, v2) wasm_f32x4_add(va, \
                                                wasm_f32x4_mul(v1, v2))

#define lancvec_addhl(vl, vh) wasm_f32x4_add(vl, \
                                             wasm_i32x4_shuffle(vh, vh, 6, 7, 2, 3))

#define lancvec_store32_addhl(m, v)         \
    *(m) = (wasm_f32x4_extract_lane(v, 0) + \
            wasm_f32x4_extract_lane(v, 1))

#define lancvec_store64_addhl(m, v) \
    wasm_v128_store64_lane(m, lancvec_addhl(v, v), 0)

#endif // defined( LANCIR_WASM )

#if LANCIR_ALIGN > 4

#if !defined(lancvec_load32_splat)
#define lancvec_load32_splat(m) lancvec_const_splat(*(m))
#endif // !defined( lancvec_load32_splat )

#if !defined(lancvec_loadu)
#define lancvec_loadu(m) lancvec_load(m)
#endif // !defined( lancvec_loadu )

#if !defined(lancvec_storeu)
#define lancvec_storeu(m, v) lancvec_store(m, v)
#endif // !defined( lancvec_storeu )

#if !defined(lancvec_store32_hadd)
#define lancvec_store32_hadd(m, v)                \
    {                                             \
        const lancvec_t v2 = lancvec_addhl(v, v); \
        lancvec_store32_addhl(m, v2);             \
    }                                             \
    (void)0
#endif // !defined( lancvec_store32_hadd )

#endif // LANCIR_ALIGN > 4

namespace avir {

using std ::ceil;
using std ::cos;
using std ::fabs;
using std ::floor;
using std ::memcpy;
using std ::memset;
using std ::sin;
using std ::size_t;

#if __cplusplus >= 201103L

using std ::intptr_t;
using std ::uintptr_t;

#else // __cplusplus >= 201103L

// Workaround for pre-C++11 compilers. `nullptr` is a keyword, and not a
// macro, but check if such workaround is already in place.

#if !defined(nullptr)
#define nullptr NULL
#define LANCIR_NULLPTR
#endif // !defined( nullptr )

#endif // __cplusplus >= 201103L

/**
 * @brief LANCIR resizing parameters class.
 *
 * An object of this class, which can be allocated on stack, can be used to
 * pass non-default parameters to the resizing algorithm. See the constructor
 * for the default values.
 */

class CLancIRParams
{
public:
    int SrcSSize; ///< Physical size of the source scanline, in elements (not
                  ///< bytes). If this value is below 1, `SrcWidth * ElCount` will be
                  ///< used.
    int NewSSize; ///< Physical size of the destination scanline, in elements
                  ///< (not bytes). If this value is below 1, `NewWidth * ElCount` will
                  ///< be used.
    double kx; ///< Resizing step - horizontal (one output pixel corresponds
               ///< to `k` input pixels). A downsizing factor if greater than 1.0;
               ///< upsizing factor if below or equal to 1.0. Multiply by -1 if you
               ///< would like to bypass `ox` and `oy` adjustment which is done by
               ///< default to produce a centered image. If this step value equals 0,
               ///< the step value will be chosen automatically.
    double ky; ///< Resizing step - vertical. Same as `kx`.
    double ox; ///< Start X pixel offset within the source image, can be
               ///< negative. A positive offset moves the image to the left.
    double oy; ///< Start Y pixel offset within the source image, can be
               ///< negative. A positive offset moves the image to the top.
    double la; ///< Lanczos window function's `a` parameter, greater or equal
               ///< to 2.0.

    /**
     * @brief Default constructor, with optional arguments that correspond to
     * class variables.
     *
     * @param aSrcSSize Physical size of the source scanline.
     * @param aNewSSize Physical size of the destination scanline.
     * @param akx Resizing step - horizontal.
     * @param aky Resizing step - vertical.
     * @param aox Start X pixel offset.
     * @param aoy Start Y pixel offset.
     */

    CLancIRParams(const int aSrcSSize = 0, const int aNewSSize = 0,
                  const double akx = 0.0, const double aky = 0.0,
                  const double aox = 0.0, const double aoy = 0.0)
        : SrcSSize(aSrcSSize), NewSSize(aNewSSize), kx(akx), ky(aky), ox(aox), oy(aoy), la(3.0)
    {
    }
};

/**
 * @brief LANCIR image resizer class.
 *
 * The object of this class can be used to resize 1-4 channel images to any
 * required size. Resizing is performed by utilizing Lanczos filters, with
 * 8-bit precision. This class offers a kind of "optimal" Lanczos resampling
 * implementation.
 *
 * Object of this class can be allocated on stack.
 *
 * Note that object of this class does not free temporary buffers and
 * variables after the resizeImage() function call (until object's
 * destruction): these buffers are reused (or reallocated) on subsequent
 * calls, thus making batch resizing of images faster. This means resizing is
 * not thread-safe: a separate CLancIR object should be created for each
 * thread.
 */

class CLancIR
{
private:
    CLancIR(const CLancIR &)
    {
        // Unsupported.
    }

    CLancIR &operator=(const CLancIR &)
    {
        // Unsupported.
        return (*this);
    }

public:
    CLancIR()
        : FltBuf0(nullptr), FltBuf0Len(0), spv0(nullptr), spv0len(0), spv(nullptr)
    {
    }

    ~CLancIR()
    {
        delete[] FltBuf0;
        delete[] spv0;
    }

    /**
     * @brief Function resizes an image.
     *
     * Performs input-to-output type conversion, if necessary.
     *
     * @param[in] SrcBuf Source image buffer.
     * @param SrcWidth Source image width, in pixels.
     * @param SrcHeight Source image height, in pixels.
     * @param[out] NewBuf Buffer to accept the resized image. Cannot be equal
     * to `SrcBuf`.
     * @param NewWidth New image width, in pixels.
     * @param NewHeight New image height, in pixels.
     * @param ElCount The number of elements (channels) used to store each
     * source and destination pixel (1-4).
     * @param aParams Custom resizing parameters. Can be `nullptr`, for
     * default values.
     * @tparam Tin Input buffer's element type. Can be `uint8_t` (`0..255`
     * value range), `uint16_t` (`0..65535` value range), `float` (`0..1`
     * value range), `double` (`0..1` value range). `uint32_t` type is treated
     * as `uint16_t`. Signed integer types and larger integer types are not
     * supported.
     * @tparam Tout Output buffer's element type, treated like `Tin`. If `Tin`
     * and `Tout` types do not match, an output value scaling will be applied.
     * Floating-point output will not be clamped/clipped/saturated; integer
     * output is always rounded and clamped.
     * @return The number of available output scanlines. Equals to
     * `NewHeight`, or 0 on function parameters error.
     */

    template<typename Tin, typename Tout>
    int resizeImage(const Tin *const SrcBuf, const int SrcWidth,
                    const int SrcHeight, Tout *const NewBuf, const int NewWidth,
                    const int NewHeight, const int ElCount,
                    const CLancIRParams *const aParams = nullptr)
    {
        if ((SrcWidth < 0) | (SrcHeight < 0) |
            (NewWidth <= 0) | (NewHeight <= 0) |
            (SrcBuf == nullptr) | (NewBuf == nullptr) |
            ((const void *)SrcBuf == (const void *)NewBuf)) {
            return (0);
        }

        static const CLancIRParams DefParams;
        const CLancIRParams &Params = (aParams != nullptr ? *aParams : DefParams);

        if (Params.la < 2.0) {
            return (0);
        }

        const int OutSLen = NewWidth * ElCount;
        const size_t NewScanlineSize = (size_t)(Params.NewSSize < 1 ? OutSLen : Params.NewSSize);

        if ((SrcWidth == 0) | (SrcHeight == 0)) {
            Tout *op = NewBuf;
            int i;

            for (i = 0; i < NewHeight; i++) {
                memset(op, 0, (size_t)OutSLen * sizeof(Tout));
                op += NewScanlineSize;
            }

            return (NewHeight);
        }

        const size_t SrcScanlineSize = (size_t)(Params.SrcSSize < 1 ? SrcWidth * ElCount : Params.SrcSSize);

        double ox = Params.ox;
        double oy = Params.oy;
        double kx;
        double ky;

        if (Params.kx >= 0.0) {
            kx = (Params.kx == 0.0 ? (double)SrcWidth / NewWidth : Params.kx);

            ox += (kx - 1.0) * 0.5;
        } else {
            kx = -Params.kx;
        }

        if (Params.ky >= 0.0) {
            ky = (Params.ky == 0.0 ? (double)SrcHeight / NewHeight : Params.ky);

            oy += (ky - 1.0) * 0.5;
        } else {
            ky = -Params.ky;
        }

        if (rfv.update(Params.la, ky, ElCount)) {
            rsv.reset();
            rsh.reset();
        }

        CResizeFilters *rfh; // Pointer to resizing filters for horizontal
                             // resizing, may equal to `rfv` if the same stepping is in use.

        if (kx == ky) {
            rfh = &rfv;
        } else {
            rfh = &rfh0;

            if (rfh0.update(Params.la, kx, ElCount)) {
                rsh.reset();
            }
        }

        rsv.update(SrcHeight, NewHeight, oy, rfv, spv);
        rsh.update(SrcWidth, NewWidth, ox, *rfh);

        // Calculate vertical progressive resizing's batch size. Progressive
        // batching is used to try to keep addressing within the cache
        // capacity. This technique definitely works well for single-threaded
        // resizing on most CPUs, but may not provide an additional benefit
        // for multi-threaded resizing, or in a system-wide high-load
        // situations.

        const size_t FltWidthE = (size_t)((rsh.padl + SrcWidth +
                                           rsh.padr) *
                                          ElCount);

        const double CacheSize = 5500000.0; // Tuned for various CPUs.
        const double OpSize = (double)SrcScanlineSize * SrcHeight *
                        sizeof(Tin) +
                (double)FltWidthE * NewHeight * sizeof(float);

        int BatchSize = (int)(NewHeight * CacheSize / (OpSize + 1.0));

        if (BatchSize < 8) {
            BatchSize = 8;
        }

        if (BatchSize > NewHeight) {
            BatchSize = NewHeight;
        }

        // Allocate/resize intermediate buffers.

        const int svs = (rsv.padl + SrcHeight + rsv.padr) * ElCount;
        float *const pspv0 = spv0;
        reallocBuf(spv0, spv, spv0len, (svs > OutSLen ? svs : OutSLen));
        reallocBuf(FltBuf0, FltBuf, FltBuf0Len,
                   FltWidthE * (size_t)BatchSize);

        if (spv0 != pspv0) {
            rsv.updateSPO(rfv, spv);
        }

        // Prepare output-related constants.

        static const bool IsInFloat = ((Tin)0.25f != 0);
        static const bool IsOutFloat = ((Tout)0.25f != 0);
        static const bool IsUnityMul = (IsInFloat && IsOutFloat) ||
                (IsInFloat == IsOutFloat && sizeof(Tin) == sizeof(Tout));

        const float Clamp = (sizeof(Tout) == 1 ? 255.0f : 65535.0f);
        const float OutMul = (IsOutFloat ? 1.0f : Clamp) /
                (IsInFloat ? 1.0f : (sizeof(Tin) == 1 ? 255.0f : 65535.0f));

        // Perform batched resizing.

        const CResizePos *rpv = rsv.pos;
        Tout *opn = NewBuf;
        int bl = NewHeight;

        while (bl > 0) {
            const int bc = (bl > BatchSize ? BatchSize : bl);

            int kl = rfv.KernelLen;
            const Tin *ip = SrcBuf;
            float *op = FltBuf + rsh.padl * ElCount;

            const int so = (int)rpv[0].so;
            float *const sp = spv + so * ElCount;

            int cc = (int)rpv[bc - 1].so - so + kl; // Pixel copy count.
            int rl = 0; // Leftmost pixel's replication count.
            int rr = 0; // Rightmost pixel's replication count.

            const int socc = so + cc;
            const int spe = rsv.padl + SrcHeight;

            // Calculate scanline copying and padding parameters, depending on
            // the batch's size and its vertical offset.

            if (so < rsv.padl) {
                if (socc <= rsv.padl) {
                    rl = cc;
                    cc = 0;
                } else {
                    if (socc > spe) {
                        rr = socc - spe;
                        cc -= rr;
                    }

                    rl = rsv.padl - so;
                    cc -= rl;
                }
            } else {
                if (so >= spe) {
                    rr = cc;
                    cc = 0;
                    ip += (size_t)SrcHeight * SrcScanlineSize;
                } else {
                    if (socc > spe) {
                        rr = socc - spe;
                        cc -= rr;
                    }

                    ip += (size_t)(so - rsv.padl) * SrcScanlineSize;
                }
            }

            // Batched vertical resizing.

            int i;

            if (ElCount == 1) {
                for (i = 0; i < SrcWidth; i++) {
                    copyScanline1v(ip, SrcScanlineSize, sp, cc, rl, rr);
                    resize1<false>(nullptr, op, FltWidthE, rpv, kl, bc);
                    ip += 1;
                    op += 1;
                }
            } else if (ElCount == 2) {
                for (i = 0; i < SrcWidth; i++) {
                    copyScanline2v(ip, SrcScanlineSize, sp, cc, rl, rr);
                    resize2<false>(nullptr, op, FltWidthE, rpv, kl, bc);
                    ip += 2;
                    op += 2;
                }
            } else if (ElCount == 3) {
                for (i = 0; i < SrcWidth; i++) {
                    copyScanline3v(ip, SrcScanlineSize, sp, cc, rl, rr);
                    resize3<false>(nullptr, op, FltWidthE, rpv, kl, bc);
                    ip += 3;
                    op += 3;
                }
            } else // ElCount == 4
            {
                for (i = 0; i < SrcWidth; i++) {
                    copyScanline4v(ip, SrcScanlineSize, sp, cc, rl, rr);
                    resize4<false>(nullptr, op, FltWidthE, rpv, kl, bc);
                    ip += 4;
                    op += 4;
                }
            }

            // Perform horizontal resizing batch, and produce final output.

            float *ipf = FltBuf;
            kl = rfh->KernelLen;

            if (ElCount == 1) {
                for (i = 0; i < bc; i++) {
                    padScanline1h(ipf, rsh, SrcWidth);
                    resize1<true>(ipf, spv, 1, rsh.pos, kl, NewWidth);
                    outputScanline<IsOutFloat, IsUnityMul>(spv, opn,
                                                           OutSLen, Clamp, OutMul);

                    ipf += FltWidthE;
                    opn += NewScanlineSize;
                }
            } else if (ElCount == 2) {
                for (i = 0; i < bc; i++) {
                    padScanline2h(ipf, rsh, SrcWidth);
                    resize2<true>(ipf, spv, 2, rsh.pos, kl, NewWidth);
                    outputScanline<IsOutFloat, IsUnityMul>(spv, opn,
                                                           OutSLen, Clamp, OutMul);

                    ipf += FltWidthE;
                    opn += NewScanlineSize;
                }
            } else if (ElCount == 3) {
                for (i = 0; i < bc; i++) {
                    padScanline3h(ipf, rsh, SrcWidth);
                    resize3<true>(ipf, spv, 3, rsh.pos, kl, NewWidth);
                    outputScanline<IsOutFloat, IsUnityMul>(spv, opn,
                                                           OutSLen, Clamp, OutMul);

                    ipf += FltWidthE;
                    opn += NewScanlineSize;
                }
            } else // ElCount == 4
            {
                for (i = 0; i < bc; i++) {
                    padScanline4h(ipf, rsh, SrcWidth);
                    resize4<true>(ipf, spv, 4, rsh.pos, kl, NewWidth);
                    outputScanline<IsOutFloat, IsUnityMul>(spv, opn,
                                                           OutSLen, Clamp, OutMul);

                    ipf += FltWidthE;
                    opn += NewScanlineSize;
                }
            }

            rpv += bc;
            bl -= bc;
        }

        return (NewHeight);
    }

    /**
     * @brief Legacy image resizing function.
     *
     * Not recommended for new projects. See the prior resizeImage() function
     * and CLancIRParams class for details.
     *
     * @param[in] SrcBuf Source image buffer.
     * @param SrcWidth Source image width, in pixels.
     * @param SrcHeight Source image height, in pixels.
     * @param SrcSSize Physical size of the source scanline, in elements (not
     * bytes).
     * @param[out] NewBuf Buffer to accept the resized image. Cannot be equal
     * to SrcBuf.
     * @param NewWidth New image width, in pixels.
     * @param NewHeight New image height, in pixels.
     * @param NewSSize Physical size of the destination scanline, in elements
     * (not bytes).
     * @param ElCount The number of elements (channels) used to store each
     * source and destination pixel (1-4).
     * @param kx0 Resizing step - horizontal.
     * @param ky0 Resizing step - vertical. Same as `kx0`.
     * @param ox Start X pixel offset within the source image.
     * @param oy Start Y pixel offset within the source image.
     * @tparam Tin Input buffer's element type.
     * @tparam Tout Output buffer's element type.
     * @return The number of available output scanlines. Equals to
     * `NewHeight`, or 0 on function parameters error.
     */

    template<typename Tin, typename Tout>
    int resizeImage(const Tin *const SrcBuf, const int SrcWidth,
                    const int SrcHeight, const int SrcSSize, Tout *const NewBuf,
                    const int NewWidth, const int NewHeight, const int NewSSize,
                    const int ElCount, const double kx0 = 0.0, const double ky0 = 0.0,
                    double ox = 0.0, double oy = 0.0)
    {
        const CLancIRParams Params(SrcSSize, NewSSize, kx0, ky0, ox, oy);

        return (resizeImage(SrcBuf, SrcWidth, SrcHeight, NewBuf, NewWidth,
                            NewHeight, ElCount, &Params));
    }

protected:
    float *FltBuf0; ///< Intermediate resizing buffer.
    size_t FltBuf0Len; ///< Length of `FltBuf0`.
    float *FltBuf; ///< Address-aligned `FltBuf0`.
    float *spv0; ///< Scanline buffer for vertical resizing, also used at the
                 ///< output stage.
    int spv0len; ///< Length of `spv0`.
    float *spv; ///< Address-aligned `spv0`.

    /**
     * @brief Typed buffer reallocation function, with address alignment.
     *
     * Function reallocates a typed buffer if its current length is
     * smaller than the required length, applies `LANCIR_ALIGN` address
     * alignment to the buffer pointer.
     *
     * @param buf0 Reference to the pointer of the previously allocated
     * buffer.
     * @param buf Reference to address-aligned `buf0` pointer.
     * @param len The current length of the `buf0`.
     * @param newlen A new required length.
     * @tparam Tb Buffer element type.
     * @tparam Tl Length variable type.
     */

    template<typename Tb, typename Tl>
    static void reallocBuf(Tb *&buf0, Tb *&buf, Tl &len, Tl newlen)
    {
        newlen += LANCIR_ALIGN;

        if (newlen > len) {
            if (buf0 != nullptr) {
                delete[] buf0;
                buf0 = nullptr;
                len = 0;
            }

            buf0 = new Tb[newlen];
            len = newlen;
            buf = (Tb *)(((uintptr_t)buf0 + LANCIR_ALIGN - 1) &
                         ~(uintptr_t)(LANCIR_ALIGN - 1));
        }
    }

    /**
     * @brief Typed buffer reallocation function.
     *
     * Function reallocates a typed buffer if its current length is smaller
     * than the required length.
     *
     * @param buf Reference to the pointer of the previously allocated buffer;
     * address alignment will not be applied.
     * @param len The current length of the `buf0`.
     * @param newlen A new required length.
     * @tparam Tb Buffer element type.
     * @tparam Tl Length variable type.
     */

    template<typename Tb, typename Tl>
    static void reallocBuf(Tb *&buf, Tl &len, const Tl newlen)
    {
        if (newlen > len) {
            if (buf != nullptr) {
                delete[] buf;
                buf = nullptr;
                len = 0;
            }

            buf = new Tb[newlen];
            len = newlen;
        }
    }

    class CResizeScanline;

    /**
     * @brief Class for fractional delay filter bank storage and calculation.
     */

    class CResizeFilters
    {
        friend class CResizeScanline;

    public:
        int KernelLen; ///< Resampling filter kernel's length, taps. Available
                       ///< after the update() function call. Always an even value,
                       ///< should not be lesser than 4.

        CResizeFilters()
            : Filters(nullptr), FiltersLen(0), la(0.0)
        {
            memset(Bufs0, 0, sizeof(Bufs0));
            memset(Bufs0Len, 0, sizeof(Bufs0Len));
        }

        ~CResizeFilters()
        {
            int i;

            for (i = 0; i < BufCount; i++) {
                delete[] Bufs0[i];
            }

            delete[] Filters;
        }

        /**
         * @brief Function updates the filter bank.
         *
         * @param la0 Lanczos `a` parameter value (greater or equal to 2.0),
         * can be fractional.
         * @param k0 Resizing step.
         * @param ElCount0 Image's element count, may be used for SIMD filter
         * tap replication.
         * @return `true`, if an update occured and scanline resizing
         * positions should be updated unconditionally.
         */

        bool update(const double la0, const double k0, const int ElCount0)
        {
            if (la0 == la && k0 == k && ElCount0 == ElCount) {
                return (false);
            }

            const double NormFreq = (k0 <= 1.0 ? 1.0 : 1.0 / k0);
            Freq = 3.1415926535897932 * NormFreq;
            FreqA = Freq / la0;

            Len2 = la0 / NormFreq;
            fl2 = (int)ceil(Len2);
            KernelLen = fl2 + fl2;

#if LANCIR_ALIGN > 4

            ElRepl = ElCount0;
            KernelLenA = KernelLen * ElRepl;

            const int elalign =
                    (int)(LANCIR_ALIGN / sizeof(float)) - 1;

            KernelLenA = (KernelLenA + elalign) & ~elalign;

#else // LANCIR_ALIGN > 4

            ElRepl = 1;
            KernelLenA = KernelLen;

#endif // LANCIR_ALIGN > 4

            FracCount = 1000; // Enough for Lanczos implicit 8-bit precision.

            la = 0.0;
            reallocBuf(Filters, FiltersLen, FracCount + 1);

            memset(Filters, 0, (size_t)FiltersLen * sizeof(Filters[0]));

            setBuf(0);

            la = la0;
            k = k0;
            ElCount = ElCount0;

            return (true);
        }

        /**
         * @brief Filter acquisition function.
         *
         * Function returns filter at the specified fractional offset. This
         * function can only be called after a prior update() function call.
         *
         * @param x Fractional offset, [0; 1].
         * @return Pointer to a previously-calculated or a new filter.
         */

        const float *getFilter(const double x)
        {
            const int Frac = (int)(x * FracCount + 0.5);
            float *flt = Filters[Frac];

            if (flt != nullptr) {
                return (flt);
            }

            flt = Bufs[CurBuf] + CurBufFill * KernelLenA;
            Filters[Frac] = flt;
            CurBufFill++;

            if (CurBufFill == BufLen) {
                setBuf(CurBuf + 1);
            }

            makeFilterNorm(flt, 1.0 - (double)Frac / FracCount);

            if (ElRepl > 1) {
                replicateFilter(flt, KernelLen, ElRepl);
            }

            return (flt);
        }

    protected:
        double Freq; ///< Circular frequency of the filter.
        double FreqA; ///< Circular frequency of the window function.
        double Len2; ///< Half resampling filter's length, unrounded.
        int fl2; ///< Half resampling filter's length, integer.
        int FracCount; ///< The number of fractional positions for which
                       ///< filters can be created.
        int KernelLenA; ///< SIMD-aligned and replicated filter kernel's
                        ///< length.
        int ElRepl; ///< The number of repetitions of each filter tap.
        static const int BufCount = 4; ///< The maximal number of buffers
                                       ///< (filter batches) that can be in use.
        static const int BufLen = 256; ///< The number of fractional filters
                                       ///< a single buffer (filter batch) may contain. Both the `BufLen`
                                       ///< and `BufCount` should correspond to the `FracCount` used.
        float *Bufs0[BufCount]; ///< Buffers that hold all filters,
                                ///< original.
        int Bufs0Len[BufCount]; ///< Allocated lengthes in `Bufs0`, in
                                ///< `float` elements.
        float *Bufs[BufCount]; ///< Address-aligned `Bufs0`.
        int CurBuf; ///< Filter buffer currently being filled.
        int CurBufFill; ///< The number of fractional positions filled in the
                        ///< current filter buffer.
        float **Filters; ///< Fractional delay filters for all positions.
                         ///< A particular pointer equals `nullptr`, if a filter for such
                         ///< position has not been created yet.
        int FiltersLen; ///< Allocated length of Filters, in elements.
        double la; ///< Current `la`.
        double k; ///< Current `k`.
        int ElCount; ///< Current `ElCount`.

        /**
         * @brief Current buffer (filter batch) repositioning function.
         *
         * Function changes the buffer currently being filled, checks its size
         * and reallocates it, if necessary, then resets its fill counter.
         *
         * @param bi A new current buffer index.
         */

        void setBuf(const int bi)
        {
            reallocBuf(Bufs0[bi], Bufs[bi], Bufs0Len[bi],
                       BufLen * KernelLenA);

            CurBuf = bi;
            CurBufFill = 0;
        }

        /**
         * @brief Sine-wave signal generator class.
         *
         * Class implements sine-wave signal generator without biasing, with
         * constructor-based initialization only. This generator uses an
         * oscillator instead of the `sin()` function.
         */

        class CSineGen
        {
        public:
            /**
             * @brief Constructor initializes *this* sine-wave signal
             * generator.
             *
             * @param si Sine function increment, in radians.
             * @param ph Starting phase, in radians. Add `0.5*pi` for a
             * cosine function.
             */

            CSineGen(const double si, const double ph)
                : svalue1(sin(ph)), svalue2(sin(ph - si)), sincr(2.0 * cos(si))
            {
            }

            /**
             * @brief Generates the next sine-wave sample, without biasing.
             */

            double generate()
            {
                const double res = svalue1;

                svalue1 = sincr * res - svalue2;
                svalue2 = res;

                return (res);
            }

        private:
            double svalue1; ///< Current sine value.
            double svalue2; ///< Previous sine value.
            double sincr; ///< Sine value increment.
        };

        /**
         * @brief Filter calculation function.
         *
         * Function creates a filter for the specified fractional delay. The
         * update() function should be called prior to calling this function.
         * The created filter is normalized (DC gain=1).
         *
         * @param[out] op Output filter buffer.
         * @param FracDelay Fractional delay, 0 to 1, inclusive.
         */

        void makeFilterNorm(float *op, const double FracDelay) const
        {
            CSineGen f(Freq, Freq * (FracDelay - fl2));
            CSineGen fw(FreqA, FreqA * (FracDelay - fl2));

            float *op0 = op;
            double s = 0.0;
            double ut;

            int t = -fl2;

            if (t + FracDelay < -Len2) {
                f.generate();
                fw.generate();
                *op = 0;
                op++;
                t++;
            }

            int IsZeroX = (fabs(FracDelay - 1.0) < 2.3e-13);
            int mt = 0 - IsZeroX;
            IsZeroX |= (fabs(FracDelay) < 2.3e-13);

            while (t < mt) {
                ut = t + FracDelay;
                *op = (float)(f.generate() * fw.generate() / (ut * ut));
                s += *op;
                op++;
                t++;
            }

            if (IsZeroX) // t+FracDelay==0
            {
                *op = (float)(Freq * FreqA);
                s += *op;
                f.generate();
                fw.generate();
            } else {
                ut = FracDelay; // t==0
                *op = (float)(f.generate() * fw.generate() / (ut * ut));
                s += *op;
            }

            mt = fl2 - 2;

            while (t < mt) {
                op++;
                t++;
                ut = t + FracDelay;
                *op = (float)(f.generate() * fw.generate() / (ut * ut));
                s += *op;
            }

            op++;
            ut = t + 1 + FracDelay;

            if (ut > Len2) {
                *op = 0;
            } else {
                *op = (float)(f.generate() * fw.generate() / (ut * ut));
                s += *op;
            }

            s = 1.0 / s;
            t = (int)(op - op0 + 1);

            while (t != 0) {
                *op0 = (float)(*op0 * s);
                op0++;
                t--;
            }
        }

        /**
         * @brief Filter tap replication function, for SIMD operations.
         *
         * Function replicates taps of the specified filter so that it can
         * be used with SIMD loading instructions. This function works
         * "in-place".
         *
         * @param[in,out] p Filter buffer pointer, should be sized to contain
         * `kl * erp` elements.
         * @param kl Filter kernel's length, in taps.
         * @param erp The number of repetitions to apply.
         */

        static void replicateFilter(float *const p, const int kl,
                                    const int erp)
        {
            const float *ip = p + kl - 1;
            float *op = p + (kl - 1) * erp;
            int c = kl;

            if (erp == 2) {
                while (c != 0) {
                    const float v = *ip;
                    op[0] = v;
                    op[1] = v;
                    ip--;
                    op -= 2;
                    c--;
                }
            } else if (erp == 3) {
                while (c != 0) {
                    const float v = *ip;
                    op[0] = v;
                    op[1] = v;
                    op[2] = v;
                    ip--;
                    op -= 3;
                    c--;
                }
            } else // erp == 4
            {
                while (c != 0) {
                    const float v = *ip;
                    op[0] = v;
                    op[1] = v;
                    op[2] = v;
                    op[3] = v;
                    ip--;
                    op -= 4;
                    c--;
                }
            }
        }
    };

    /**
     * @brief Structure defines source scanline positions and filters for each
     * destination pixel.
     */

    struct CResizePos {
        const float *flt; ///< Fractional delay filter.
        intptr_t spo; ///< Source scanline's pixel offset, in bytes, or
                      ///< a direct pointer to scanline buffer.
        intptr_t so; ///< Offset within the source scanline, in pixels.
    };

    /**
     * @brief Scanline resizing positions class.
     *
     * Class contains resizing positions, and prepares source scanline
     * positions for resize filtering. The public variables become available
     * after the update() function call.
     */

    class CResizeScanline
    {
    public:
        int padl; ///< Left-padding (in pixels) required for source scanline.
        int padr; ///< Right-padding (in pixels) required for source scanline.
        CResizePos *pos; ///< Source scanline positions (offsets) and filters
                         ///< for each destination pixel position.

        CResizeScanline()
            : pos(nullptr), poslen(0), SrcLen(0)
        {
        }

        ~CResizeScanline()
        {
            delete[] pos;
        }

        /**
         * @brief Object's reset function.
         *
         * Function "resets" *this* object so that the next update() call
         * fully updates the position buffer. Reset is necessary if the
         * corresponding CResizeFilters object was updated.
         */

        void reset()
        {
            SrcLen = 0;
        }

        /**
         * @brief Scanline positions update function.
         *
         * Function updates resizing positions, updates `padl`, `padr`, and
         * `pos` buffer.
         *
         * @param SrcLen0 Source image scanline length, used to create a
         * scanline buffer without length pre-calculation.
         * @param DstLen0 Destination image scanline length.
         * @param o0 Initial source image offset.
         * @param rf Resizing filters object.
         * @param sp A pointer to scanline buffer, to use for absolute
         * scanline positioning, can be `nullptr`.
         */

        void update(const int SrcLen0, const int DstLen0, const double o0,
                    CResizeFilters &rf, float *const sp = nullptr)
        {
            if (SrcLen0 == SrcLen && DstLen0 == DstLen && o0 == o) {
                return;
            }

            const int fl2m1 = rf.fl2 - 1;
            padl = fl2m1 - (int)floor(o0);

            if (padl < 0) {
                padl = 0;
            }

            // Make sure `padr` and `pos` are in sync: calculate ending `pos`
            // offset in advance.

            const double k = rf.k;

            const int DstLen_m1 = DstLen0 - 1;
            const double oe = o0 + k * DstLen_m1;
            const int ie = (int)floor(oe);

            padr = ie + rf.fl2 + 1 - SrcLen0;

            if (padr < 0) {
                padr = 0;
            }

            SrcLen = 0;
            reallocBuf(pos, poslen, DstLen0);

            const intptr_t ElCountF = rf.ElCount * (intptr_t)sizeof(float);
            const int so = padl - fl2m1;
            CResizePos *rp = pos;
            intptr_t rpso;
            int i;

            for (i = 0; i < DstLen_m1; i++) {
                const double ox = o0 + k * i;
                const int ix = (int)floor(ox);

                rp->flt = rf.getFilter(ox - ix);
                rpso = so + ix;
                rp->spo = (intptr_t)sp + rpso * ElCountF;
                rp->so = rpso;
                rp++;
            }

            rp->flt = rf.getFilter(oe - ie);
            rpso = so + ie;
            rp->spo = (intptr_t)sp + rpso * ElCountF;
            rp->so = rpso;

            SrcLen = SrcLen0;
            DstLen = DstLen0;
            o = o0;
        }

        /**
         * @brief Scanline pixel offsets update function.
         *
         * Function updates `pos` buffer's `spo` (scanline pixel offset)
         * values.
         *
         * @param rf Resizing filters object.
         * @param sp A pointer to scanline buffer, to use for absolute
         * scanline positioning, can be `nullptr`.
         */

        void updateSPO(CResizeFilters &rf, float *const sp)
        {
            const intptr_t ElCountF = rf.ElCount * (intptr_t)sizeof(float);
            CResizePos *const rp = pos;
            int i;

            for (i = 0; i < DstLen; i++) {
                rp[i].spo = (intptr_t)sp + rp[i].so * ElCountF;
            }
        }

    protected:
        int poslen; ///< Allocated `pos` buffer's length.
        int SrcLen; ///< Current `SrcLen`.
        int DstLen; ///< Current `DstLen`.
        double o; ///< Current `o`.
    };

    CResizeFilters rfv; ///< Resizing filters for vertical resizing.
    CResizeFilters rfh0; ///< Resizing filters for horizontal resizing (may
                         ///< not be in use).
    CResizeScanline rsv; ///< Vertical resize scanline.
    CResizeScanline rsh; ///< Horizontal resize scanline.

    /**
     * @{
     * @brief Scanline copying function, for vertical resizing.
     *
     * Function copies scanline (fully or partially) from the source buffer,
     * in its native format, to the internal scanline buffer, in preparation
     * for vertical resizing. Variants for 1-4-channel images.
     *
     * @param ip Source scanline buffer pointer.
     * @param ipinc `ip` increment per pixel.
     * @param op Output scanline pointer.
     * @param cc Source pixel copy count.
     * @param repl Leftmost pixel's replication count.
     * @param repr Rightmost pixel's replication count.
     * @tparam T Source buffer's element type.
     */

    template<typename T>
    static void copyScanline1v(const T *ip, const size_t ipinc, float *op,
                               int cc, int repl, int repr)
    {
        float v0;

        if (repl > 0) {
            v0 = (float)ip[0];

            do {
                op[0] = v0;
                op += 1;

            } while (--repl != 0);
        }

        while (cc != 0) {
            op[0] = (float)ip[0];
            ip += ipinc;
            op += 1;
            cc--;
        }

        if (repr > 0) {
            const T *const ipe = ip - ipinc;
            v0 = (float)ipe[0];

            do {
                op[0] = v0;
                op += 1;

            } while (--repr != 0);
        }
    }

    template<typename T>
    static void copyScanline2v(const T *ip, const size_t ipinc, float *op,
                               int cc, int repl, int repr)
    {
        float v0, v1;

        if (repl > 0) {
            v0 = (float)ip[0];
            v1 = (float)ip[1];

            do {
                op[0] = v0;
                op[1] = v1;
                op += 2;

            } while (--repl != 0);
        }

        while (cc != 0) {
            op[0] = (float)ip[0];
            op[1] = (float)ip[1];
            ip += ipinc;
            op += 2;
            cc--;
        }

        if (repr > 0) {
            const T *const ipe = ip - ipinc;
            v0 = (float)ipe[0];
            v1 = (float)ipe[1];

            do {
                op[0] = v0;
                op[1] = v1;
                op += 2;

            } while (--repr != 0);
        }
    }

    template<typename T>
    static void copyScanline3v(const T *ip, const size_t ipinc, float *op,
                               int cc, int repl, int repr)
    {
        float v0, v1, v2;

        if (repl > 0) {
            v0 = (float)ip[0];
            v1 = (float)ip[1];
            v2 = (float)ip[2];

            do {
                op[0] = v0;
                op[1] = v1;
                op[2] = v2;
                op += 3;

            } while (--repl != 0);
        }

        while (cc != 0) {
            op[0] = (float)ip[0];
            op[1] = (float)ip[1];
            op[2] = (float)ip[2];
            ip += ipinc;
            op += 3;
            cc--;
        }

        if (repr > 0) {
            const T *const ipe = ip - ipinc;
            v0 = (float)ipe[0];
            v1 = (float)ipe[1];
            v2 = (float)ipe[2];

            do {
                op[0] = v0;
                op[1] = v1;
                op[2] = v2;
                op += 3;

            } while (--repr != 0);
        }
    }

    template<typename T>
    static void copyScanline4v(const T *ip, const size_t ipinc, float *op,
                               int cc, int repl, int repr)
    {
        float v0, v1, v2, v3;

        if (repl > 0) {
            v0 = (float)ip[0];
            v1 = (float)ip[1];
            v2 = (float)ip[2];
            v3 = (float)ip[3];

            do {
                op[0] = v0;
                op[1] = v1;
                op[2] = v2;
                op[3] = v3;
                op += 4;

            } while (--repl != 0);
        }

        while (cc != 0) {
            op[0] = (float)ip[0];
            op[1] = (float)ip[1];
            op[2] = (float)ip[2];
            op[3] = (float)ip[3];
            ip += ipinc;
            op += 4;
            cc--;
        }

        if (repr > 0) {
            const T *const ipe = ip - ipinc;
            v0 = (float)ipe[0];
            v1 = (float)ipe[1];
            v2 = (float)ipe[2];
            v3 = (float)ipe[3];

            do {
                op[0] = v0;
                op[1] = v1;
                op[2] = v2;
                op[3] = v3;
                op += 4;

            } while (--repr != 0);
        }
    }

    /** @} */

    /**
     * @{
     * @brief Scanline padding function, for horizontal resizing.
     *
     * Function pads the specified scanline buffer to the left and right by
     * replicating its first and last available pixels, in preparation for
     * horizontal resizing. Variants for 1-4-channel images.
     *
     * @param[in,out] op Scanline buffer to pad.
     * @param rs Scanline resizing positions object.
     * @param l Source scanline's length, in pixels.
     */

    static void padScanline1h(float *op, CResizeScanline &rs, const int l)
    {
        const float *ip = op + rs.padl;

        float v0 = ip[0];
        int i;

        for (i = 0; i < rs.padl; i++) {
            op[i] = v0;
        }

        ip += l;
        op += rs.padl + l;

        v0 = ip[-1];

        for (i = 0; i < rs.padr; i++) {
            op[i] = v0;
        }
    }

    static void padScanline2h(float *op, CResizeScanline &rs, const int l)
    {
        const float *ip = op + rs.padl * 2;

        float v0 = ip[0];
        float v1 = ip[1];
        int i;

        for (i = 0; i < rs.padl; i++) {
            op[0] = v0;
            op[1] = v1;
            op += 2;
        }

        const int lc = l * 2;
        ip += lc;
        op += lc;

        v0 = ip[-2];
        v1 = ip[-1];

        for (i = 0; i < rs.padr; i++) {
            op[0] = v0;
            op[1] = v1;
            op += 2;
        }
    }

    static void padScanline3h(float *op, CResizeScanline &rs, const int l)
    {
        const float *ip = op + rs.padl * 3;

        float v0 = ip[0];
        float v1 = ip[1];
        float v2 = ip[2];
        int i;

        for (i = 0; i < rs.padl; i++) {
            op[0] = v0;
            op[1] = v1;
            op[2] = v2;
            op += 3;
        }

        const int lc = l * 3;
        ip += lc;
        op += lc;

        v0 = ip[-3];
        v1 = ip[-2];
        v2 = ip[-1];

        for (i = 0; i < rs.padr; i++) {
            op[0] = v0;
            op[1] = v1;
            op[2] = v2;
            op += 3;
        }
    }

    static void padScanline4h(float *op, CResizeScanline &rs, const int l)
    {
        const float *ip = op + rs.padl * 4;

        float v0 = ip[0];
        float v1 = ip[1];
        float v2 = ip[2];
        float v3 = ip[3];
        int i;

        for (i = 0; i < rs.padl; i++) {
            op[0] = v0;
            op[1] = v1;
            op[2] = v2;
            op[3] = v3;
            op += 4;
        }

        const int lc = l * 4;
        ip += lc;
        op += lc;

        v0 = ip[-4];
        v1 = ip[-3];
        v2 = ip[-2];
        v3 = ip[-1];

        for (i = 0; i < rs.padr; i++) {
            op[0] = v0;
            op[1] = v1;
            op[2] = v2;
            op[3] = v3;
            op += 4;
        }
    }

    /** @} */

    /**
     * @brief Rounds a value, and applies clamping.
     *
     * @param v Value to round and clamp.
     * @param Clamp High clamp level; low level is 0.
     * @return Rounded and clamped value.
     */

    static inline int roundclamp(const float v, const float Clamp)
    {
        return ((int)((v > Clamp ? Clamp : (v < 0.0f ? 0.0f : v)) +
                      0.5f));
    }

    /**
     * @brief Scanline output function.
     *
     * Function performs output of the scanline pixels to the destination
     * image buffer, with type conversion, scaling, clamping, if necessary.
     *
     * @param[in] ip Input (resized) scanline. Pointer must be aligned to
     * LANCIR_ALIGN bytes.
     * @param[out] op Output image buffer. Must be different to `ip`.
     * @param l Output scanline's length, in elements (not pixel count).
     * @param Clamp Clamp high level, used if `IsOutFloat` is `false`.
     * @param OutMul Output multiplier, for value range conversion, applied
     * before clamping.
     * @tparam IsOutFloat `true`, if floating-point output, and no clamping is
     * necessary.
     * @tparam IsUnityMul `true`, if multiplication is optional. However, even
     * if this parameter was specified as `true`, `OutMul` must be 1.
     * @tparam T Output buffer's element type. Acquired implicitly.
     */

    template<bool IsOutFloat, bool IsUnityMul, typename T>
    static void outputScanline(const float *ip, T *op, int l,
                               const float Clamp, const float OutMul)
    {
        if (IsOutFloat) {
            if (IsUnityMul) {
                if (sizeof(op[0]) == sizeof(ip[0])) {
                    memcpy(op, ip, (size_t)l * sizeof(op[0]));
                } else {
                    int l4 = l >> 2;
                    l &= 3;

                    while (l4 != 0) {
                        op[0] = (T)ip[0];
                        op[1] = (T)ip[1];
                        op[2] = (T)ip[2];
                        op[3] = (T)ip[3];
                        ip += 4;
                        op += 4;
                        l4--;
                    }

                    while (l != 0) {
                        *op = (T)*ip;
                        ip++;
                        op++;
                        l--;
                    }
                }
            } else {
                int l4 = l >> 2;
                l &= 3;
                bool DoScalar = true;

                if (sizeof(op[0]) == sizeof(ip[0])) {
#if LANCIR_ALIGN > 4

                    DoScalar = false;
                    const lancvec_t om = lancvec_load32_splat(&OutMul);

                    while (l4 != 0) {
                        lancvec_storeu((float *)op,
                                       lancvec_mul(lancvec_load(ip), om));

                        ip += 4;
                        op += 4;
                        l4--;
                    }

#endif // LANCIR_ALIGN > 4
                }

                if (DoScalar) {
                    while (l4 != 0) {
                        op[0] = (T)(ip[0] * OutMul);
                        op[1] = (T)(ip[1] * OutMul);
                        op[2] = (T)(ip[2] * OutMul);
                        op[3] = (T)(ip[3] * OutMul);
                        ip += 4;
                        op += 4;
                        l4--;
                    }
                }

                while (l != 0) {
                    *op = (T)(*ip * OutMul);
                    ip++;
                    op++;
                    l--;
                }
            }
        } else {
            int l4 = l >> 2;
            l &= 3;

#if LANCIR_ALIGN > 4

            const lancvec_t minv = lancvec_const_splat(0.0f);
            const lancvec_t maxv = lancvec_load32_splat(&Clamp);
            const lancvec_t om = lancvec_load32_splat(&OutMul);

#if defined(LANCIR_SSE2)
            unsigned int prevrm = _MM_GET_ROUNDING_MODE();
            _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
#else // defined( LANCIR_SSE2 )
            const lancvec_t v05 = lancvec_const_splat(0.5f);
#endif // defined( LANCIR_SSE2 )

            if (sizeof(op[0]) == 4) {
                while (l4 != 0) {
                    const lancvec_t v = lancvec_load(ip);
                    const lancvec_t cv = lancvec_max(lancvec_min(
                                                             (IsUnityMul ? v : lancvec_mul(v, om)),
                                                             maxv),
                                                     minv);

#if defined(LANCIR_SSE2)

                    _mm_storeu_si128((__m128i *)op, _mm_cvtps_epi32(cv));

#elif defined(LANCIR_NEON)

                    vst1q_u32((unsigned int *)op, vcvtq_u32_f32(vaddq_f32(cv, v05)));

#elif defined(LANCIR_WASM)

                    wasm_v128_store(op, wasm_i32x4_trunc_sat_f32x4(wasm_f32x4_add(cv, v05)));

#endif // defined( LANCIR_WASM )

                    ip += 4;
                    op += 4;
                    l4--;
                }
            } else if (sizeof(op[0]) == 2) {
                while (l4 != 0) {
                    const lancvec_t v = lancvec_load(ip);
                    const lancvec_t cv = lancvec_max(lancvec_min(
                                                             (IsUnityMul ? v : lancvec_mul(v, om)),
                                                             maxv),
                                                     minv);

#if defined(LANCIR_SSE2)

                    const __m128i v32 = _mm_cvtps_epi32(cv);
                    const __m128i v16s = _mm_shufflehi_epi16(
                            _mm_shufflelo_epi16(v32, 0 | 2 << 2), 0 | 2 << 2);

                    const __m128i v16 = _mm_shuffle_epi32(v16s, 0 | 2 << 2);

                    __m128i tmp;
                    _mm_store_si128(&tmp, v16);
                    memcpy(op, &tmp, 8);

#elif defined(LANCIR_NEON)

                    const uint32x4_t v32 = vcvtq_u32_f32(
                            vaddq_f32(cv, v05));

                    const uint16x4_t v16 = vmovn_u32(v32);

                    vst1_u16((unsigned short *)op, v16);

#elif defined(LANCIR_WASM)

                    const v128_t v32 = wasm_i32x4_trunc_sat_f32x4(
                            wasm_f32x4_add(cv, v05));

                    wasm_v128_store64_lane(op,
                                           wasm_u16x8_narrow_i32x4(v32, v32), 0);

#endif // defined( LANCIR_WASM )

                    ip += 4;
                    op += 4;
                    l4--;
                }
            } else {
                while (l4 != 0) {
                    const lancvec_t v = lancvec_load(ip);
                    const lancvec_t cv = lancvec_max(lancvec_min(
                                                             (IsUnityMul ? v : lancvec_mul(v, om)),
                                                             maxv),
                                                     minv);

#if defined(LANCIR_SSE2)

                    const __m128i v32 = _mm_cvtps_epi32(cv);
                    const __m128i v16s = _mm_shufflehi_epi16(
                            _mm_shufflelo_epi16(v32, 0 | 2 << 2), 0 | 2 << 2);

                    const __m128i v16 = _mm_shuffle_epi32(v16s, 0 | 2 << 2);
                    const __m128i v8 = _mm_packus_epi16(v16, v16);

                    *(int *)op = _mm_cvtsi128_si32(v8);

#elif defined(LANCIR_NEON)

                    const uint32x4_t v32 = vcvtq_u32_f32(
                            vaddq_f32(cv, v05));

                    const uint16x4_t v16 = vmovn_u32(v32);
                    const uint8x8_t v8 = vmovn_u16(vcombine_u16(v16, v16));

                    *(unsigned int *)op = vget_lane_u32((uint32x2_t)v8, 0);

#elif defined(LANCIR_WASM)

                    const v128_t v32 = wasm_i32x4_trunc_sat_f32x4(
                            wasm_f32x4_add(cv, v05));

                    const v128_t v16 = wasm_u16x8_narrow_i32x4(v32, v32);

                    wasm_v128_store32_lane(op,
                                           wasm_u8x16_narrow_i16x8(v16, v16), 0);

#endif // defined( LANCIR_WASM )

                    ip += 4;
                    op += 4;
                    l4--;
                }
            }

#if defined(LANCIR_SSE2)
            _MM_SET_ROUNDING_MODE(prevrm);
#endif // defined( LANCIR_SSE2 )

#else // LANCIR_ALIGN > 4

            if (IsUnityMul) {
                while (l4 != 0) {
                    op[0] = (T)roundclamp(ip[0], Clamp);
                    op[1] = (T)roundclamp(ip[1], Clamp);
                    op[2] = (T)roundclamp(ip[2], Clamp);
                    op[3] = (T)roundclamp(ip[3], Clamp);
                    ip += 4;
                    op += 4;
                    l4--;
                }
            } else {
                while (l4 != 0) {
                    op[0] = (T)roundclamp(ip[0] * OutMul, Clamp);
                    op[1] = (T)roundclamp(ip[1] * OutMul, Clamp);
                    op[2] = (T)roundclamp(ip[2] * OutMul, Clamp);
                    op[3] = (T)roundclamp(ip[3] * OutMul, Clamp);
                    ip += 4;
                    op += 4;
                    l4--;
                }
            }

#endif // LANCIR_ALIGN > 4

            if (IsUnityMul) {
                while (l != 0) {
                    *op = (T)roundclamp(*ip, Clamp);
                    ip++;
                    op++;
                    l--;
                }
            } else {
                while (l != 0) {
                    *op = (T)roundclamp(*ip * OutMul, Clamp);
                    ip++;
                    op++;
                    l--;
                }
            }
        }
    }

    /**
     * @def LANCIR_LF_PRE
     * @brief Scanline resize function prologue.
     */

#define LANCIR_LF_PRE                                     \
    const CResizePos *const rpe = rp + DstLen;            \
    while (rp != rpe) {                                   \
        const float *flt = rp->flt;                       \
        const float *ip;                                  \
        if (UseSP) {                                      \
            ip = (const float *)((intptr_t)sp + rp->spo); \
        } else {                                          \
            ip = (const float *)rp->spo;                  \
        }

    /**
     * @def LANCIR_LF_POST
     * @brief Scanline resize function epilogue.
     */

#define LANCIR_LF_POST \
    op += opinc;       \
    rp++;              \
    }

    /**
     * @{
     * @brief Function performs scanline resizing. Variants for 1-4-channel
     * images.
     *
     * @param[in] sp Source scanline buffer.
     * @param[out] op Destination buffer.
     * @param opinc `op` increment.
     * @param rp Source scanline offsets and resizing filters.
     * @param kl Filter kernel's length, in taps (always an even value).
     * @param DstLen Destination length, in pixels.
     * @tparam UseSP `true`, if `sp` pointer should be added to `spo`.
     */

    template<bool UseSP>
    static void resize1(const float *const sp, float *op, const size_t opinc,
                        const CResizePos *rp, const int kl, const int DstLen)
    {
        const int ci = kl >> 2;

        if ((kl & 3) == 0) {
            LANCIR_LF_PRE

            int c = ci;

#if LANCIR_ALIGN > 4

            lancvec_t sum = lancvec_mul(
                    lancvec_load(flt), lancvec_loadu(ip));

            while (--c != 0) {
                flt += 4;
                ip += 4;
                sum = lancvec_madd(sum, lancvec_load(flt),
                                   lancvec_loadu(ip));
            }

            lancvec_store32_hadd(op, sum);

#else // LANCIR_ALIGN > 4

            float sum0 = flt[0] * ip[0];
            float sum1 = flt[1] * ip[1];
            float sum2 = flt[2] * ip[2];
            float sum3 = flt[3] * ip[3];

            while (--c != 0) {
                flt += 4;
                ip += 4;
                sum0 += flt[0] * ip[0];
                sum1 += flt[1] * ip[1];
                sum2 += flt[2] * ip[2];
                sum3 += flt[3] * ip[3];
            }

            op[0] = (sum0 + sum1) + (sum2 + sum3);

#endif // LANCIR_ALIGN > 4

            LANCIR_LF_POST
        } else {
            LANCIR_LF_PRE

            int c = ci;

#if LANCIR_ALIGN > 4

            lancvec_t sum = lancvec_mul(lancvec_load(flt),
                                        lancvec_loadu(ip));

            while (--c != 0) {
                flt += 4;
                ip += 4;
                sum = lancvec_madd(sum, lancvec_load(flt),
                                   lancvec_loadu(ip));
            }

#if defined(LANCIR_NEON)

            float32x2_t sum2 = vadd_f32(vget_high_f32(sum),
                                        vget_low_f32(sum));

            sum2 = vmla_f32(sum2, vld1_f32(flt + 4),
                            vld1_f32(ip + 4));

#if defined(LANCIR_ARM32)
            op[0] = vget_lane_f32(sum2, 0) +
                    vget_lane_f32(sum2, 1);
#else // defined( LANCIR_ARM32 )
            op[0] = vaddv_f32(sum2);
#endif // defined( LANCIR_ARM32 )

#else // defined( LANCIR_NEON )

            const lancvec_t sum2 = lancvec_mul(lancvec_loadu(flt + 2),
                                               lancvec_loadu(ip + 2));

            sum = lancvec_addhl(sum, sum);
            sum = lancvec_addhl(sum, sum2);

            lancvec_store32_addhl(op, sum);

#endif // defined( LANCIR_NEON )

#else // LANCIR_ALIGN > 4

            float sum0 = flt[0] * ip[0];
            float sum1 = flt[1] * ip[1];
            float sum2 = flt[2] * ip[2];
            float sum3 = flt[3] * ip[3];

            while (--c != 0) {
                flt += 4;
                ip += 4;
                sum0 += flt[0] * ip[0];
                sum1 += flt[1] * ip[1];
                sum2 += flt[2] * ip[2];
                sum3 += flt[3] * ip[3];
            }

            op[0] = (sum0 + sum1) + (sum2 + sum3) +
                    flt[4] * ip[4] + flt[5] * ip[5];

#endif // LANCIR_ALIGN > 4

            LANCIR_LF_POST
        }
    }

    template<bool UseSP>
    static void resize2(const float *const sp, float *op, const size_t opinc,
                        const CResizePos *rp, const int kl, const int DstLen)
    {
#if LANCIR_ALIGN > 4
        const int ci = kl >> 2;
        const int cir = kl & 3;
#else // LANCIR_ALIGN > 4
        const int ci = kl >> 1;
#endif // LANCIR_ALIGN > 4

        LANCIR_LF_PRE

        int c = ci;

#if defined(LANCIR_AVX)

        __m256 sum = _mm256_mul_ps(_mm256_load_ps(flt),
                                   _mm256_loadu_ps(ip));

        while (--c != 0) {
            flt += 8;
            ip += 8;
            sum = _mm256_add_ps(sum, _mm256_mul_ps(_mm256_load_ps(flt), _mm256_loadu_ps(ip)));
        }

        __m128 res = _mm_add_ps(_mm256_extractf128_ps(sum, 0),
                                _mm256_extractf128_ps(sum, 1));

        if (cir == 2) {
            res = _mm_add_ps(res, _mm_mul_ps(_mm_load_ps(flt + 8), _mm_loadu_ps(ip + 8)));
        }

        _mm_storel_pi((__m64 *)op,
                      _mm_add_ps(res, _mm_movehl_ps(res, res)));

#elif LANCIR_ALIGN > 4

        lancvec_t sumA = lancvec_mul(
                lancvec_load(flt), lancvec_loadu(ip));

        lancvec_t sumB = lancvec_mul(
                lancvec_load(flt + 4), lancvec_loadu(ip + 4));

        while (--c != 0) {
            flt += 8;
            ip += 8;
            sumA = lancvec_madd(sumA, lancvec_load(flt),
                                lancvec_loadu(ip));

            sumB = lancvec_madd(sumB, lancvec_load(flt + 4),
                                lancvec_loadu(ip + 4));
        }

        sumA = lancvec_add(sumA, sumB);

        if (cir == 2) {
            sumA = lancvec_madd(sumA, lancvec_load(flt + 8),
                                lancvec_loadu(ip + 8));
        }

        lancvec_store64_addhl(op, sumA);

#else // LANCIR_ALIGN > 4

        const float xx = flt[0];
        const float xx2 = flt[1];
        float sum0 = xx * ip[0];
        float sum1 = xx * ip[1];
        float sum2 = xx2 * ip[2];
        float sum3 = xx2 * ip[3];

        while (--c != 0) {
            flt += 2;
            ip += 4;
            const float xx = flt[0];
            const float xx2 = flt[1];
            sum0 += xx * ip[0];
            sum1 += xx * ip[1];
            sum2 += xx2 * ip[2];
            sum3 += xx2 * ip[3];
        }

        op[0] = sum0 + sum2;
        op[1] = sum1 + sum3;

#endif // LANCIR_ALIGN > 4

        LANCIR_LF_POST
    }

    template<bool UseSP>
    static void resize3(const float *const sp, float *op, const size_t opinc,
                        const CResizePos *rp, const int kl, const int DstLen)
    {
#if LANCIR_ALIGN > 4

        const int ci = kl >> 2;
        const int cir = kl & 3;

        LANCIR_LF_PRE

        float res[12];
        int c = ci;

#if defined(LANCIR_AVX)

        __m128 sumA = _mm_mul_ps(_mm_load_ps(flt), _mm_loadu_ps(ip));
        __m256 sumB = _mm256_mul_ps(_mm256_loadu_ps(flt + 4),
                                    _mm256_loadu_ps(ip + 4));

        while (--c != 0) {
            flt += 12;
            ip += 12;
            sumA = _mm_add_ps(sumA, _mm_mul_ps(_mm_load_ps(flt), _mm_loadu_ps(ip)));

            sumB = _mm256_add_ps(sumB, _mm256_mul_ps(_mm256_loadu_ps(flt + 4), _mm256_loadu_ps(ip + 4)));
        }

        if (cir == 2) {
            sumA = _mm_add_ps(sumA, _mm_mul_ps(_mm_load_ps(flt + 12), _mm_loadu_ps(ip + 12)));
        }

        _mm_storeu_ps(res, sumA);

        float o0 = res[0] + res[3];
        float o1 = res[1];
        float o2 = res[2];

        _mm256_storeu_ps(res + 4, sumB);

        o1 += res[4];
        o2 += res[5];

#else // defined( LANCIR_AVX )

        lancvec_t sumA = lancvec_mul(lancvec_load(flt),
                                     lancvec_loadu(ip));

        lancvec_t sumB = lancvec_mul(lancvec_load(flt + 4),
                                     lancvec_loadu(ip + 4));

        lancvec_t sumC = lancvec_mul(lancvec_load(flt + 8),
                                     lancvec_loadu(ip + 8));

        while (--c != 0) {
            flt += 12;
            ip += 12;
            sumA = lancvec_madd(sumA, lancvec_load(flt),
                                lancvec_loadu(ip));

            sumB = lancvec_madd(sumB, lancvec_load(flt + 4),
                                lancvec_loadu(ip + 4));

            sumC = lancvec_madd(sumC, lancvec_load(flt + 8),
                                lancvec_loadu(ip + 8));
        }

        if (cir == 2) {
            sumA = lancvec_madd(sumA, lancvec_load(flt + 12),
                                lancvec_loadu(ip + 12));
        }

        lancvec_storeu(res, sumA);
        lancvec_storeu(res + 4, sumB);

        float o0 = res[0] + res[3];
        float o1 = res[1] + res[4];
        float o2 = res[2] + res[5];

        lancvec_storeu(res + 8, sumC);

#endif // defined( LANCIR_AVX )

        o0 += res[6] + res[9];
        o1 += res[7] + res[10];
        o2 += res[8] + res[11];

        if (cir == 2) {
            o1 += flt[16] * ip[16];
            o2 += flt[17] * ip[17];
        }

        op[0] = o0;
        op[1] = o1;
        op[2] = o2;

#else // LANCIR_ALIGN > 4

        const int ci = kl >> 1;

        LANCIR_LF_PRE

        int c = ci;

        const float xx = flt[0];
        float sum0 = xx * ip[0];
        float sum1 = xx * ip[1];
        float sum2 = xx * ip[2];
        const float xx2 = flt[1];
        float sum3 = xx2 * ip[3];
        float sum4 = xx2 * ip[4];
        float sum5 = xx2 * ip[5];

        while (--c != 0) {
            flt += 2;
            ip += 6;
            const float xx = flt[0];
            sum0 += xx * ip[0];
            sum1 += xx * ip[1];
            sum2 += xx * ip[2];
            const float xx2 = flt[1];
            sum3 += xx2 * ip[3];
            sum4 += xx2 * ip[4];
            sum5 += xx2 * ip[5];
        }

        op[0] = sum0 + sum3;
        op[1] = sum1 + sum4;
        op[2] = sum2 + sum5;

#endif // LANCIR_ALIGN > 4

        LANCIR_LF_POST
    }

    template<bool UseSP>
    static void resize4(const float *const sp, float *op, const size_t opinc,
                        const CResizePos *rp, const int kl, const int DstLen)
    {
#if LANCIR_ALIGN > 4
        const int ci = kl >> 1;
#else // LANCIR_ALIGN > 4
        const int ci = kl;
#endif // LANCIR_ALIGN > 4

        LANCIR_LF_PRE

        int c = ci;

#if defined(LANCIR_AVX)

        __m256 sum = _mm256_mul_ps(_mm256_load_ps(flt),
                                   _mm256_loadu_ps(ip));

        while (--c != 0) {
            flt += 8;
            ip += 8;
            sum = _mm256_add_ps(sum, _mm256_mul_ps(_mm256_load_ps(flt), _mm256_loadu_ps(ip)));
        }

        _mm_store_ps(op, _mm_add_ps(_mm256_extractf128_ps(sum, 0), _mm256_extractf128_ps(sum, 1)));

#elif LANCIR_ALIGN > 4

        lancvec_t sumA = lancvec_mul(lancvec_load(flt),
                                     lancvec_load(ip));

        lancvec_t sumB = lancvec_mul(lancvec_load(flt + 4),
                                     lancvec_load(ip + 4));

        while (--c != 0) {
            flt += 8;
            ip += 8;
            sumA = lancvec_madd(sumA, lancvec_load(flt),
                                lancvec_load(ip));

            sumB = lancvec_madd(sumB, lancvec_load(flt + 4),
                                lancvec_load(ip + 4));
        }

        lancvec_store(op, lancvec_add(sumA, sumB));

#else // LANCIR_ALIGN > 4

        const float xx = flt[0];
        float sum0 = xx * ip[0];
        float sum1 = xx * ip[1];
        float sum2 = xx * ip[2];
        float sum3 = xx * ip[3];

        while (--c != 0) {
            flt++;
            ip += 4;
            const float xx = flt[0];
            sum0 += xx * ip[0];
            sum1 += xx * ip[1];
            sum2 += xx * ip[2];
            sum3 += xx * ip[3];
        }

        op[0] = sum0;
        op[1] = sum1;
        op[2] = sum2;
        op[3] = sum3;

#endif // LANCIR_ALIGN > 4

        LANCIR_LF_POST
    }

    /** @} */

#undef LANCIR_LF_PRE
#undef LANCIR_LF_POST
};

#undef lancvec_t
#undef lancvec_const_splat
#undef lancvec_load32_splat
#undef lancvec_load
#undef lancvec_loadu
#undef lancvec_store
#undef lancvec_storeu
#undef lancvec_add
#undef lancvec_mul
#undef lancvec_min
#undef lancvec_max
#undef lancvec_madd
#undef lancvec_addhl
#undef lancvec_store32_addhl
#undef lancvec_store32_hadd
#undef lancvec_store64_addhl

#if defined(LANCIR_NULLPTR)
#undef nullptr
#undef LANCIR_NULLPTR
#endif // defined( LANCIR_NULLPTR )

} // namespace avir

#endif // AVIR_CLANCIR_INCLUDED
