/**
 *  @brief Collection of Similarity Measures, SIMD-accelerated with SSE, AVX, NEON, SVE.
 *
 *  @author Ashot Vardanian
 *  @date March 14, 2023
 *
 *  x86 intrinsics: https://www.intel.com/content/www/us/en/docs/intrinsics-guide/
 *  Arm intrinsics: https://developer.arm.com/architectures/instruction-sets/intrinsics/
 *  Detecting target CPU features at compile time: https://stackoverflow.com/a/28939692/2766161
 */

#pragma once
#include <math.h> // `sqrt`

#if defined(__AVX2__)
#include <immintrin.h>
#endif

#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif

#if defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__ARM_FEATURE_SVE)
typedef float float32_t;
typedef double float64_t;
typedef int16_t float16_t;
#endif

union simsimd_f32i32_t {
    int32_t i;
    float32_t f;
};

#if defined(__ARM_FEATURE_SVE)

inline static float32_t simsimd_dot_f32sve(float32_t const* a, float32_t const* b, size_t d) {
    size_t i = 0;
    svfloat32_t ab_vec = svdupq_n_f32(0.f, 0.f, 0.f, 0.f);
    svbool_t pg_vec = svwhilelt_b32(i, d);
    do {
        svfloat32_t a_vec = svld1_f32(pg_vec, a + i);
        svfloat32_t b_vec = svld1_f32(pg_vec, b + i);
        ab_vec = svmla_f32_x(pg_vec, ab_vec, a_vec, b_vec);
        i += svcntw();
        pg_vec = svwhilelt_b32(i, d);
    } while (svptest_any(svptrue_b32(), pg_vec));
    return svaddv_f32(svptrue_b32(), ab_vec);
}

inline static float16_t simsimd_dot_f16sve(float16_t const* a_enum, float16_t const* b_enum, size_t d) {
    size_t i = 0;
    svfloat16_t ab_vec = svdupq_n_f16(0, 0, 0, 0, 0, 0, 0, 0);
    svbool_t pg_vec = svwhilelt_b16(i, d);
    float16_t const* a = (float16_t const*)(a_enum);
    float16_t const* b = (float16_t const*)(b_enum);
    do {
        svfloat16_t a_vec = svld1_f16(pg_vec, a + i);
        svfloat16_t b_vec = svld1_f16(pg_vec, b + i);
        ab_vec = svmla_f16_x(pg_vec, ab_vec, a_vec, b_vec);
        i += svcnth();
        pg_vec = svwhilelt_b16(i, d);
    } while (svptest_any(svptrue_b16(), pg_vec));
    return static_cast<float16_t>(svaddv_f16(svptrue_b16(), ab_vec));
}

inline static float32_t simsimd_cos_f32sve(float32_t const* a, float32_t const* b, size_t d) {
    size_t i = 0;
    svfloat32_t ab_vec = svdupq_n_f32(0.f, 0.f, 0.f, 0.f);
    svfloat32_t a2_vec = svdupq_n_f32(0.f, 0.f, 0.f, 0.f);
    svfloat32_t b2_vec = svdupq_n_f32(0.f, 0.f, 0.f, 0.f);
    svbool_t pg_vec = svwhilelt_b32(i, d);
    do {
        svfloat32_t a_vec = svld1_f32(pg_vec, a + i);
        svfloat32_t b_vec = svld1_f32(pg_vec, b + i);
        ab_vec = svmla_f32_x(pg_vec, ab_vec, a_vec, b_vec);
        a2_vec = svmla_f32_x(pg_vec, a2_vec, a_vec, a_vec);
        b2_vec = svmla_f32_x(pg_vec, b2_vec, b_vec, b_vec);
        i += svcntw();
        pg_vec = svwhilelt_b32(i, d);
    } while (svptest_any(svptrue_b32(), pg_vec));
    float32_t ab = svaddv_f32(svptrue_b32(), ab_vec);
    float32_t a2 = svaddv_f32(svptrue_b32(), a2_vec);
    float32_t b2 = svaddv_f32(svptrue_b32(), b2_vec);
    return ab / (sqrt(a2) * sqrt(b2));
}

inline static float32_t simsimd_euclidean_f32sve(float32_t const* a, float32_t const* b, size_t d) {
    size_t i = 0;
    svfloat32_t d2_vec = svdupq_n_f32(0.f, 0.f, 0.f, 0.f);
    svbool_t pg_vec = svwhilelt_b32(i, d);
    do {
        svfloat32_t a_vec = svld1_f32(pg_vec, a + i);
        svfloat32_t b_vec = svld1_f32(pg_vec, b + i);
        svfloat32_t a_minus_b_vec = svsub_f32_x(pg_vec, a_vec, b_vec);
        d2_vec = svmla_f32_x(pg_vec, d2_vec, a_minus_b_vec, a_minus_b_vec);
        i += svcntw();
        pg_vec = svwhilelt_b32(i, d);
    } while (svptest_any(svptrue_b32(), pg_vec));
    float32_t d2 = svaddv_f32(svptrue_b32(), d2_vec);
    return sqrt(d2);
}

inline static float16_t simsimd_euclidean_f16sve(float16_t const* a_enum, float16_t const* b_enum, size_t d) {
    size_t i = 0;
    svfloat16_t d2_vec = svdupq_n_f16(0, 0, 0, 0, 0, 0, 0, 0);
    svbool_t pg_vec = svwhilelt_b16(i, d);
    float16_t const* a = (float16_t const*)(a_enum);
    float16_t const* b = (float16_t const*)(b_enum);
    do {
        svfloat16_t a_vec = svld1_f16(pg_vec, a + i);
        svfloat16_t b_vec = svld1_f16(pg_vec, b + i);
        svfloat16_t a_minus_b_vec = svsub_f16_x(pg_vec, a_vec, b_vec);
        d2_vec = svmla_f16_x(pg_vec, d2_vec, a_minus_b_vec, a_minus_b_vec);
        i += svcnth();
        pg_vec = svwhilelt_b16(i, d);
    } while (svptest_any(svptrue_b16(), pg_vec));
    float16_t d2 = svaddv_f16(svptrue_b16(), d2_vec);
    return static_cast<float16_t>(sqrt(d2));
}

inline static size_t simsimd_hamming_u1x8sve(uint8_t const* a, uint8_t const* b, size_t d) {
    size_t i = 0;
    svuint8_t d_vec = svdupq_n_u8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    svbool_t pg_vec = svwhilelt_b8(i, d);
    do {
        svuint8_t a_vec = svld1_u8(pg_vec, a + i);
        svuint8_t b_vec = svld1_u8(pg_vec, b + i);
        svuint8_t a_xor_b_vec = sveor_u8_m(pg_vec, a_vec, b_vec);
        d_vec = svadd_u8_m(pg_vec, d_vec, svcnt_u8_x(pg_vec, a_xor_b_vec));
        i += svcntb() * __CHAR_BIT__;
        pg_vec = svwhilelt_b32(i, d);
    } while (svptest_any(svptrue_b32(), pg_vec));
    return svaddv_u8(svptrue_b32(), d_vec);
}

#endif

#if defined(__ARM_NEON)

inline static float32_t simsimd_dot_f32x4neon(float32_t const* a, float32_t const* b, size_t d) {
    float32x4_t ab_vec = vdupq_n_f32(0);
    for (size_t i = 0; i != d; i += 4)
        ab_vec = vmlaq_f32(ab_vec, vld1q_f32(a + i), vld1q_f32(b + i));
    return vaddvq_f32(ab_vec);
}

inline static float32_t simsimd_cos_f32x4neon(float32_t const* a, float32_t const* b, size_t d) {
    float32x4_t ab_vec = vdupq_n_f32(0);
    float32x4_t a2_vec = vdupq_n_f32(0);
    float32x4_t b2_vec = vdupq_n_f32(0);
    for (size_t i = 0; i != d; i += 4) {
        float32x4_t a_vec = vld1q_f32(a + i);
        float32x4_t b_vec = vld1q_f32(b + i);
        ab_vec = vmlaq_f32(ab_vec, a_vec, b_vec);
        a2_vec = vmlaq_f32(a2_vec, a_vec, a_vec);
        b2_vec = vmlaq_f32(b2_vec, b_vec, b_vec);
    }
    float32_t ab = vaddvq_f32(ab_vec);
    float32_t a2 = vaddvq_f32(a2_vec);
    float32_t b2 = vaddvq_f32(b2_vec);
    return ab / (sqrt(a2) * sqrt(b2));
}

inline static size_t simsimd_hamming_u1x128sve(uint8_t const* a, uint8_t const* b, size_t d) {
    /// Contains 16x 8-bit integers with running population count sums.
    uint8x16_t d_vec = vdupq_n_u8(0);
    for (size_t i = 0; i != d; i += 16) {
        uint8x16_t a_vec = vld1q_u8(a + i);
        uint8x16_t b_vec = vld1q_u8(b + i);
        uint8x16_t a_xor_b_vec = veorq_u8(a_vec, b_vec);
        d_vec = vaddq_u8(d_vec, vcntq_u8(a_xor_b_vec));
    }
    return vaddvq_u8(d_vec);
}

#endif

#if defined(__AVX2__)

inline static float32_t simsimd_dot_f32x4avx2(float32_t const* a, float32_t const* b, size_t d) {
    __m128 ab_vec = _mm_set1_ps(0);
    for (size_t i = 0; i != d; i += 4)
        ab_vec = _mm_fmadd_ps(_mm_loadu_ps(a + i), _mm_loadu_ps(b + i), ab_vec);
    ab_vec = _mm_hadd_ps(ab_vec, ab_vec);
    ab_vec = _mm_hadd_ps(ab_vec, ab_vec);
    simsimd_f32i32_t ab_union = {_mm_cvtsi128_si32(_mm_castps_si128(ab_vec))};
    return ab_union.f;
}

inline static int32_t simsimd_dot_i8x16avx2(int8_t const* a, int8_t const* b, size_t d) {
    __m256i ab_vec = _mm256_set1_epi16(0);
    for (size_t i = 0; i != d; i += 4)
        ab_vec = _mm256_add_epi16(                                              //
            ab_vec,                                                             //
            _mm256_mullo_epi16(                                                 //
                _mm256_cvtepi8_epi16(_mm_loadu_si128((__m128i const*)(a + i))), //
                _mm256_cvtepi8_epi16(_mm_loadu_si128((__m128i const*)(b + i)))));
    ab_vec = _mm256_hadd_epi16(ab_vec, ab_vec);
    ab_vec = _mm256_hadd_epi16(ab_vec, ab_vec);
    ab_vec = _mm256_hadd_epi16(ab_vec, ab_vec);
    return (_mm256_cvtsi256_si32(ab_vec) & 0xFF);
}

inline static float32_t simsimd_cos_f32x4avx2(float32_t const* a, float32_t const* b, size_t d) {
    __m128 ab_vec = _mm_set1_ps(0);
    __m128 a2_vec = _mm_set1_ps(0);
    __m128 b2_vec = _mm_set1_ps(0);
    for (size_t i = 0; i != d; i += 4) {
        __m128 a_vec = _mm_loadu_ps(a + i);
        __m128 b_vec = _mm_loadu_ps(b + i);
        ab_vec = _mm_fmadd_ps(a_vec, b_vec, ab_vec);
        a2_vec = _mm_fmadd_ps(a_vec, a_vec, a2_vec);
        b2_vec = _mm_fmadd_ps(b_vec, b_vec, b2_vec);
    }
    ab_vec = _mm_hadd_ps(ab_vec, ab_vec);
    ab_vec = _mm_hadd_ps(ab_vec, ab_vec);
    a2_vec = _mm_hadd_ps(a2_vec, a2_vec);
    a2_vec = _mm_hadd_ps(a2_vec, a2_vec);
    b2_vec = _mm_hadd_ps(b2_vec, b2_vec);
    b2_vec = _mm_hadd_ps(b2_vec, b2_vec);
    simsimd_f32i32_t ab_union = {_mm_cvtsi128_si32(_mm_castps_si128(ab_vec))};
    simsimd_f32i32_t a2_union = {_mm_cvtsi128_si32(_mm_castps_si128(a2_vec))};
    simsimd_f32i32_t b2_union = {_mm_cvtsi128_si32(_mm_castps_si128(b2_vec))};
    return ab_union.f / (sqrt(a2_union.f) * sqrt(b2_union.f));
}

#endif

#if defined(__AVX512VPOPCNTDQ__)

inline static size_t simsimd_hamming_u1x128avx512(uint8_t const* a, uint8_t const* b, size_t d) {
    size_t words = d / 128;
    uint64_t const* a64 = (uint64_t const*)(a);
    uint64_t const* b64 = (uint64_t const*)(b);
    /// Contains 2x 64-bit integers with running population count sums.
    __m128i d_vec = _mm_set_epi64x(0, 0);
    for (size_t i = 0; i != words; i += 2)
        d_vec = _mm_add_epi64( //
            d_vec,             //
            _mm_popcnt_epi64(  //
                _mm_xor_si128( //
                    _mm_load_si128((__m128i const*)(a64 + i)), _mm_load_si128((__m128i const*)(b64 + i)))));
    return _mm_cvtm64_si64(_mm_movepi64_pi64(d_vec)) + _mm_extract_epi64(d_vec, 1);
}

#endif

#ifdef __cplusplus
}
#endif