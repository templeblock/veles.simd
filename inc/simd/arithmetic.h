/*! @file arithmetic.h
 *  @brief Inline arithmetic functions with SIMD acceleration.
 *  @author Markovtsev Vadim <v.markovtsev@samsung.com>
 *  @version 1.0
 *
 *  @section Notes
 *  This code partially conforms to <a href="http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml">Google C++ Style Guide</a>.
 *
 *  @section Copyright
 *  Copyright © 2013 Samsung R&D Institute Russia
 *
 *  @section License
 *  Licensed to the Apache Software Foundation (ASF) under one
 *  or more contributor license agreements.  See the NOTICE file
 *  distributed with this work for additional information
 *  regarding copyright ownership.  The ASF licenses this file
 *  to you under the Apache License, Version 2.0 (the
 *  "License"); you may not use this file except in compliance
 *  with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing,
 *  software distributed under the License is distributed on an
 *  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, either express or implied.  See the License for the
 *  specific language governing permissions and limitations
 *  under the License.
 */

#ifndef INC_SIMD_ARITHMETIC_H_
#define INC_SIMD_ARITHMETIC_H_

#include <assert.h>
#include <stdint.h>
#include <simd/attributes.h>
#include <simd/instruction_set.h>
#include <simd/memory.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

INLINE NOTNULL(1, 3) void int16_to_float_na(
    const int16_t *data, size_t length, float *__restrict res) {
  for (size_t i = 0; i < length; i++) {
    res[i] = (float)data[i];
  }
}

INLINE NOTNULL(1, 3) void float_to_int16_na(const float *data,
                                            size_t length, int16_t *res) {
  for (size_t i = 0; i < length; i++) {
    // Simple truncation here (fast).
    // If changed to roundf(), replace *cvttps* -> *cvtps*.
    res[i] = (int16_t)data[i];
  }
}

INLINE NOTNULL(1, 3) void int32_to_float_na(const int32_t *data,
                                            size_t length, float *res) {
  for (size_t i = 0; i < length; i++) {
    res[i] = (float)data[i];
  }
}

INLINE NOTNULL(1, 3) void float_to_int32_na(const float *data,
                                            size_t length, int32_t *res) {
  for (size_t i = 0; i < length; i++) {
    res[i] = (int32_t)data[i];
  }
}

INLINE NOTNULL(1, 3) void int32_to_int16_na(
    const int32_t *data, size_t length, int16_t *__restrict res) {
  for (size_t i = 0; i < length; i++) {
    res[i] = (int16_t)data[i];
  }
}

INLINE NOTNULL(1, 3) void int16_to_int32_na(
    const int16_t *data, size_t length, int32_t *__restrict res) {
  for (size_t i = 0; i < length; i++) {
    res[i] = (int32_t)data[i];
  }
}

typedef union {
  uint32_t i;
  float f;
} FloatUint32;

INLINE NOTNULL(1, 3) void float16_to_float_na(
    const uint16_t *data, size_t length, float *__restrict res) {
  for (size_t i = 0; i < length; i++) {
    uint16_t in = data[i];
    FloatUint32 bp;
    uint32_t exp = in & 0x7c00;    // Exponent
    switch (exp) {
      case 0: { // 0 or subnormal
        bp.i = in & 0x03ff;
        if (bp.i == 0) {  // Signed zero
            break;
        }
        // Subnormal
        int lz = __builtin_clz(bp.i) - 16 - 6;
        exp = (127u - 15u - lz) << 23;
        bp.i <<= lz + 1;
        bp.i = exp | ((bp.i & 0x03ff) << 13);
        break;
      }
      case 0x7c00:  // inf, nan
        bp.i = in & 0x03ff;
        bp.i <<= 13;
        bp.i |= 0x7f800000;
        break;
      default:
        bp.i = in & 0x7fff;  // Non-sign bits
        bp.i <<= 13;         // Align mantissa on MSB
        bp.i += 0x38000000;  // Adjust bias
        break;
    }
    uint32_t sign = in & 0x8000;   // Sign bit
    sign <<= 16;                   // Shift sign bit into position
    bp.i |= sign;                  // Re-insert sign bit
    res[i] = bp.f;
  }
}

INLINE NOTNULL(1, 2, 3) void real_multiply_na(
    const float *a, const float *b, float *res) {
  *res = *a * *b;
}

INLINE NOTNULL(1, 2, 4) void real_multiply_array_na(
    const float *a, const float *b, size_t length, float *res) {
  int ilength = length;
  for (int j = 0; j < ilength; j++) {
    res[j] = a[j] * b[j];
  }
}

INLINE NOTNULL(1, 2, 3) void complex_multiply_na(
    const float *a, const float *b, float *res) {
  float re1 = (a)[0];
  float im1 = a[1];
  float re2 = b[0];
  float im2 = b[1];
  res[0] = re1 * re2 - im1 * im2;
  res[1] = re1 * im2 + re2 * im1;
}

INLINE NOTNULL(1, 2, 3) void complex_multiply_conjugate_na(
    const float *a, const float *b, float *res) {
  float re1 = (a)[0];
  float im1 = a[1];
  float re2 = b[0];
  float im2 = -b[1];
  res[0] = re1 * re2 - im1 * im2;
  res[1] = re1 * im2 + re2 * im1;
}

INLINE NOTNULL(1, 3) void complex_conjugate_na(
    const float *array, size_t length, float *res) {
  for (size_t i = 1; i < length; i += 2) {
    res[i - 1] = array[i - 1];
    res[i] = -array[i];
  }
}

INLINE NOTNULL(1, 4) void real_multiply_scalar_na(const float *array,
                                                  size_t length,
                                                  float value, float *res) {
  for (size_t i = 0; i < length; i++) {
    res[i] = array[i] * value;
  }
}

INLINE NOTNULL(1) float sum_elements_na(const float *input, size_t length) {
  float res = 0.f;
  for (int j = 0; j < (int)length; j++) {
    res += input[j];
  }
  return res;
}

INLINE NOTNULL(1,4) void add_to_all_na(float *input, size_t length,
                                       float value, float *output) {
  for (int j = 0; j < (int)length; j++) {
    output[j] = input[j] + value;
  }
}

#ifdef __AVX__

#define SIMD
#define FLOAT_STEP 8
#define FLOAT_STEP_LOG2 3

#ifdef __AVX2__

#define INT16MUL_STEP 16
#define INT16MUL_STEP_LOG2 4

/// @brief Multiplies the contents of two vectors, saving the result to the
/// third vector, using AVX2 SIMD (int16_t doubling version).
/// @details res[i] = a[i] * b[i], i = 0..15.
/// @param a First vector.
/// @param b Second vector.
/// @param res Result vector.
/// @pre a, b and res must be aligned to 32 bytes.
INLINE NOTNULL(1, 2, 3) void int16_multiply(
    const int16_t *a, const int16_t *b, int32_t *res) {
  __m256i aVec = _mm256_load_si256((const __m256i*)a);
  __m256i bVec = _mm256_load_si256((const __m256i*)b);
  __m256i resVecHiP = _mm256_mulhi_epi16(aVec, bVec);
  __m256i resVecLoP = _mm256_mullo_epi16(aVec, bVec);
  __m256i resVecHi = _mm256_unpackhi_epi16(resVecLoP, resVecHiP);
  __m256i resVecLo = _mm256_unpacklo_epi16(resVecLoP, resVecHiP);
  _mm256_store_si256((__m256i *)res, resVecLo);
  _mm256_store_si256((__m256i *)(res + 8), resVecHi);
}

/// @brief Converts an array of short integers to floating point numbers,
/// using AVX2 SIMD.
/// @param data The array of short integers.
/// @param length The length of the array (in int16_t-s, not in bytes).
/// @param res The floating point number array to write the results to.
/// @note align_complement_i16(data) % 8 must be equal to
/// align_complement_f32(res) % 8.
/// @note res must have at least the same length as data.
INLINE NOTNULL(1, 3) void int16_to_float(const int16_t *data,
                                         size_t length, float *res) {
  int ilength = (int)length;
  int startIndex = align_complement_i16(data);
  assert(startIndex % 8 == align_complement_f32(res) % 8);
  for (int i = 0; i < startIndex; i++) {
    res[i] = (float)data[i];
  }

  for (int i = startIndex; i < ilength - 15; i += 16) {
    __m256i intVec = _mm256_load_si256((const __m256i*)(data + i));
    __m256i intlo = _mm256_unpacklo_epi16(intVec, _mm256_set1_epi16(0));
    __m256i inthi = _mm256_unpackhi_epi16(intVec, _mm256_set1_epi16(0));
    __m256 flo = _mm256_cvtepi32_ps(intlo);
    __m256 fhi = _mm256_cvtepi32_ps(inthi);
    _mm256_store_ps(res + i, flo);
    _mm256_store_ps(res + i + 8, fhi);
  }

  for (int i = startIndex + (((ilength - startIndex) >> 4) << 4);
      i < ilength; i++) {
    res[i] = (float)data[i];
  }
}

INLINE NOTNULL(1, 3) void float_to_int16(const float *data,
                                         size_t length, int16_t *res) {
  int ilength = (int)length;
  int startIndex = align_complement_f32(data);
  assert(startIndex % 16 == align_complement_i16(res) % 16);
  for (int i = 0; i < startIndex; i++) {
    res[i] = (int16_t)data[i];
  }

  for (int i = startIndex; i < ilength - 15; i += 16) {
    __m256 fVecHi = _mm256_load_ps(data + i);
    __m256 fVecLo = _mm256_load_ps(data + i + 8);
    __m256i intVecHi = _mm256_cvttps_epi32(fVecHi);
    __m256i intVecLo = _mm256_cvttps_epi32(fVecLo);
    __m256i int16Vec = _mm256_packs_epi32(intVecHi, intVecLo);
    _mm256_store_si256((__m256i *)(res + i), int16Vec);
  }

  for (int i = startIndex + (((ilength - startIndex) >> 4) << 4);
       i < ilength; i++) {
    res[i] = (int16_t)data[i];
  }
}

INLINE NOTNULL(1, 3) void int32_to_float(const int32_t *data,
                                         size_t length, float *res) {
  int ilength = (int)length;
  int startIndex = align_complement_i32(data);
  assert(startIndex == align_complement_f32(res));
  for (int i = 0; i < startIndex; i++) {
    res[i] = (float)data[i];
  }

  for (int i = startIndex; i < ilength - 7; i += 8) {
    __m256i intVec = _mm256_load_si256((const __m256i*)(data + i));
    __m256 fVec = _mm256_cvtepi32_ps(intVec);
    _mm256_store_ps(res + i, fVec);
  }

  for (int i = startIndex + ((ilength - startIndex) & ~0x7);
       i < ilength; i++) {
    res[i] = (float)data[i];
  }
}

INLINE NOTNULL(1, 3) void float_to_int32(const float *data,
                                         size_t length, int32_t *res) {
  int ilength = (int)length;
  int startIndex = align_complement_f32(data);
  assert(startIndex == align_complement_i32(res));
  for (int i = 0; i < startIndex; i++) {
    res[i] = (int16_t)data[i];
  }

  for (int i = startIndex; i < ilength - 7; i += 8) {
    __m256 fVec = _mm256_load_ps(data + i);
    __m256i intVec = _mm256_cvttps_epi32(fVec);
    _mm256_store_si256((__m256i *)(res + i), intVec);
  }

  for (int i = startIndex + ((ilength - startIndex) & ~0x7);
       i < ilength; i++) {
    res[i] = (int32_t)data[i];
  }
}

INLINE NOTNULL(1, 3) void int16_to_int32(const int16_t *data,
                                         size_t length, int32_t *res) {
  int ilength = (int)length;
  int startIndex = align_complement_i16(data);
  assert(startIndex % 8 == align_complement_i32(res) % 8);
  for (int i = 0; i < startIndex; i++) {
    res[i] = (float)data[i];
  }

  for (int i = startIndex; i < ilength - 15; i += 16) {
    __m256i intVec = _mm256_load_si256((const __m256i*)(data + i));
    __m256i intlo = _mm256_unpacklo_epi16(intVec, _mm256_set1_epi16(0));
    __m256i inthi = _mm256_unpackhi_epi16(intVec, _mm256_set1_epi16(0));
    _mm256_store_si256((__m256i *)(res + i), intlo);
    _mm256_store_si256((__m256i *)(res + i + 8), inthi);
  }

  for (int i = startIndex + (((ilength - startIndex) >> 4) << 4);
      i < ilength; i++) {
    res[i] = (int32_t)data[i];
  }
}

INLINE NOTNULL(1, 3) void int32_to_int16(const int32_t *data,
                                         size_t length, int16_t *res) {
  int ilength = (int)length;
  int startIndex = align_complement_i32(data);
  assert(startIndex % 16 == align_complement_i16(res) % 16);
  for (int i = 0; i < startIndex; i++) {
    res[i] = (int16_t)data[i];
  }

  for (int i = startIndex; i < ilength - 15; i += 16) {
    __m256i intVecHi = _mm256_load_si256((const __m256i*)(data + i));
    __m256i intVecLo = _mm256_load_si256((const __m256i*)(data + i + 4));
    __m256i int16Vec = _mm256_packs_epi32(intVecHi, intVecLo);
    _mm256_store_si256((__m256i *)(res + i), int16Vec);
  }

  for (int i = startIndex + (((ilength - startIndex) >> 4) << 4);
        i < ilength; i++) {
      res[i] = (int16_t)data[i];
  }
}

#else

#define INT16MUL_STEP 8
#define INT16MUL_STEP_LOG2 3

/// @brief Multiplies the contents of two vectors, saving the result to the
/// third vector, using SSSE3 SIMD (int16_t doubling version).
/// @details res[i] = a[i] * b[i], i = 0..7.
/// @param a First vector.
/// @param b Second vector.
/// @param res Result vector.
/// @pre a, b and res must be aligned to 16 bytes.
INLINE NOTNULL(1, 2, 3) void int16_multiply(
    const int16_t *a, const int16_t *b, int32_t *res) {
  __m128i aVec = _mm_load_si128((const __m128i*)a);
  __m128i bVec = _mm_load_si128((const __m128i*)b);
  __m128i resVecHiP = _mm_mulhi_epi16(aVec, bVec);
  __m128i resVecLoP = _mm_mullo_epi16(aVec, bVec);
  __m128i resVecHi = _mm_unpackhi_epi16(resVecLoP, resVecHiP);
  __m128i resVecLo = _mm_unpacklo_epi16(resVecLoP, resVecHiP);
  _mm_store_si128((__m128i *)res, resVecLo);
  _mm_store_si128((__m128i *)(res + 4), resVecHi);
}

/// @brief Converts an array of short integers to floating point numbers,
/// using SSE2 SIMD.
/// @param data The array of short integers.
/// @param length The length of the array (in int16_t-s, not in bytes).
/// @param res The floating point number array to write the results to.
/// @note align_complement_i16(data) % 4 must be equal to
/// align_complement_f32(res) % 4.
/// @note res must have at least the same length as data.
INLINE NOTNULL(1, 3) void int16_to_float(const int16_t *data,
                                         size_t length, float *res) {
  int ilength = (int)length;
  int startIndex = align_complement_i16(data);
  assert(startIndex % 4 == align_complement_f32(res) % 4);
  for (int i = 0; i < startIndex; i++) {
    res[i] = (float)data[i];
  }

  for (int i = startIndex; i < ilength - 7; i += 8) {
    __m128i intVec = _mm_load_si128((const __m128i*)(data + i));
    // Be careful with the sign bit as it should remain on the leftmost place
    __m128i intlo = _mm_unpacklo_epi16(_mm_set1_epi16(0), intVec);
    __m128i inthi = _mm_unpackhi_epi16(_mm_set1_epi16(0), intVec);
   intlo = _mm_sra_epi32(intlo, _mm_set1_epi32(16));
   inthi = _mm_sra_epi32(inthi, _mm_set1_epi32(16));
    __m128 flo = _mm_cvtepi32_ps(intlo);
    __m128 fhi = _mm_cvtepi32_ps(inthi);
    _mm_store_ps(res + i, flo);
    _mm_store_ps(res + i + 4, fhi);
  }

  for (int i = startIndex + ((ilength - startIndex) & ~0x7);
      i < ilength; i++) {
    res[i] = (float)data[i];
  }
}

INLINE NOTNULL(1, 3) void float_to_int16(const float *data,
                                         size_t length, int16_t *res) {
  int ilength = (int)length;
  int startIndex = align_complement_f32(data);
  assert(startIndex % 8 == align_complement_i16(res) % 8);
  for (int i = 0; i < startIndex; i++) {
    res[i] = (int16_t)data[i];
  }

  for (int i = startIndex; i < ilength - 7; i += 8) {
    __m128 fVecHi = _mm_load_ps(data + i);
    __m128 fVecLo = _mm_load_ps(data + i + 4);
    __m128i intVecHi = _mm_cvttps_epi32(fVecHi);
    __m128i intVecLo = _mm_cvttps_epi32(fVecLo);
    __m128i int16Vec = _mm_packs_epi32(intVecHi, intVecLo);
    _mm_store_si128((__m128i *)(res + i), int16Vec);
  }

  for (int i = startIndex + ((ilength - startIndex) & ~0x7);
       i < ilength; i++) {
    res[i] = (int16_t)data[i];
  }
}

INLINE NOTNULL(1, 3) void int32_to_float(const int32_t *data,
                                         size_t length, float *res) {
  int ilength = (int)length;
  int startIndex = align_complement_i32(data);
  assert(startIndex == align_complement_f32(res));
  for (int i = 0; i < startIndex; i++) {
    res[i] = (float)data[i];
  }

  for (int i = startIndex; i < ilength - 3; i += 4) {
    __m128i intVec = _mm_load_si128((const __m128i*)(data + i));
    __m128 fVec = _mm_cvtepi32_ps(intVec);
    _mm_store_ps(res + i, fVec);
  }

  for (int i = startIndex + ((ilength - startIndex) & ~0x3);
       i < ilength; i++) {
    res[i] = (float)data[i];
  }
}

INLINE NOTNULL(1, 3) void float_to_int32(const float *data,
                                         size_t length, int32_t *res) {
  int ilength = (int)length;
  int startIndex = align_complement_f32(data);
  assert(startIndex == align_complement_i32(res));
  for (int i = 0; i < startIndex; i++) {
    res[i] = (int16_t)data[i];
  }

  for (int i = startIndex; i < ilength - 3; i += 4) {
    __m128 fVec = _mm_load_ps(data + i);
    __m128i intVec = _mm_cvttps_epi32(fVec);
    _mm_store_si128((__m128i *)(res + i), intVec);
  }

  for (int i = startIndex + ((ilength - startIndex) & ~0x3);
       i < ilength; i++) {
    res[i] = (int32_t)data[i];
  }
}

INLINE NOTNULL(1, 3) void int16_to_int32(const int16_t *data,
                                         size_t length, int32_t *res) {
  int ilength = (int)length;
  int startIndex = align_complement_i16(data);
  assert(startIndex % 4 == align_complement_i32(res) % 4);
  for (int i = 0; i < startIndex; i++) {
    res[i] = (float)data[i];
  }

  for (int i = startIndex; i < ilength - 7; i += 8) {
    __m128i intVec = _mm_load_si128((const __m128i*)(data + i));
    // Be careful with the sign bit as it should remain on the leftmost place
    __m128i intlo = _mm_unpacklo_epi16(_mm_set1_epi16(0), intVec);
    __m128i inthi = _mm_unpackhi_epi16(_mm_set1_epi16(0), intVec);
   intlo = _mm_sra_epi32(intlo, _mm_set1_epi32(16));
   inthi = _mm_sra_epi32(inthi, _mm_set1_epi32(16));
    _mm_store_si128((__m128i *)(res + i), intlo);
    _mm_store_si128((__m128i *)(res + i + 4), inthi);
  }

  for (int i = startIndex + ((ilength - startIndex) & ~0x7);
      i < ilength; i++) {
    res[i] = (int32_t)data[i];
  }
}

INLINE NOTNULL(1, 3) void int32_to_int16(const int32_t *data,
                                         size_t length, int16_t *res) {
  int ilength = (int)length;
  int startIndex = align_complement_i32(data);
  assert(startIndex % 8 == align_complement_i16(res) % 8);
  for (int i = 0; i < startIndex; i++) {
    res[i] = (int16_t)data[i];
  }

  for (int i = startIndex; i < ilength - 7; i += 8) {
    __m128i intVecHi = _mm_load_si128((const __m128i*)(data + i));
    __m128i intVecLo = _mm_load_si128((const __m128i*)(data + i + 4));
    __m128i int16Vec = _mm_packs_epi32(intVecHi, intVecLo);
    _mm_store_si128((__m128i *)(res + i), int16Vec);
  }

  for (int i = startIndex + ((ilength - startIndex) & ~0x7);
        i < ilength; i++) {
      res[i] = (int16_t)data[i];
  }
}

/// @brief Converts an array of 16-bit floats to 32-bit floating point numbers,
/// using SSE2 SIMD.
/// @param data The array of float16 (unit16_t).
/// @param length The length of the array (in uint16_t-s, not in bytes).
/// @param res The floating point number array to write the results to.
/// @note align_complement_i16(data) % 4 must be equal to
/// align_complement_f32(res) % 4.
/// @note res must have at least the same length as data.
INLINE NOTNULL(1, 3) void float16_to_float(
    const uint16_t *data, size_t length, float *__restrict res) {
  int ilength = (int)length;
  int startIndex = align_complement_u16(data);
  assert(startIndex % 4 == align_complement_f32(res) % 4);
  float16_to_float_na(data, startIndex, res);
  int offset = startIndex + ((ilength - startIndex) & ~0x7);
  float16_to_float_na(data + offset, ilength - offset, res + offset);

  const __m128i expMask = _mm_set1_epi16(0x7c00);
  const __m128i zerosVec = _mm_set1_epi16(0);
  const __m128i addVecDef = _mm_set1_epi32(0x38000000);
  const __m128i addVecInfNan = _mm_set1_epi32(0x7f800000);
  for (int i = startIndex; i < ilength - 7; i += 8) {
    __m128i intVec = _mm_load_si128((const __m128i*)(data + i));
    __m128i expVec = _mm_and_si128(intVec, expMask);
    __m128i cmpVec = _mm_cmpeq_epi16(expVec, zerosVec);
    int zero_check = _mm_movemask_epi8(cmpVec);
    if (zero_check != 0) {
      // There are zeros or subnormals
      if (zero_check == 0xffff) {
        // there are only zeros or subnormals
        expVec = _mm_and_si128(intVec, _mm_set1_epi16(0x03ff));
        cmpVec = _mm_cmpeq_epi16(expVec, zerosVec);
        zero_check = _mm_movemask_epi8(cmpVec);
        if (zero_check == 0xffff) {
          // only zeros
          __m128i signVec = _mm_and_si128(intVec, _mm_set1_epi16(0x8000));
          __m128i signlo = _mm_unpacklo_epi16(zerosVec, signVec);
          __m128i signhi = _mm_unpackhi_epi16(zerosVec, signVec);
          _mm_store_si128((__m128i*)(res + i), signlo);
          _mm_store_si128((__m128i*)(res + i + 4), signhi);
          continue;
        } else {
          float16_to_float_na(data + i, 8, res + i);
          continue;
        }
      } else {
        float16_to_float_na(data + i, 8, res + i);
        continue;
      }
    }
    cmpVec = _mm_cmpeq_epi16(expVec, expMask);
    __m128i andVec = _mm_blendv_epi8(
        _mm_set1_epi16(0x7fff), _mm_set1_epi16(0x03ff), cmpVec);
    __m128i tmpVec = _mm_and_si128(intVec, andVec);
    __m128i intlo = _mm_unpacklo_epi16(tmpVec, zerosVec);
    __m128i inthi = _mm_unpackhi_epi16(tmpVec, zerosVec);
    intlo = _mm_slli_epi32(intlo, 13);
    inthi = _mm_slli_epi32(inthi, 13);
    __m128i cmplo = _mm_unpacklo_epi16(zerosVec, cmpVec);
    __m128i cmphi = _mm_unpackhi_epi16(zerosVec, cmpVec);
    __m128i addlo = _mm_blendv_epi8(addVecDef, addVecInfNan, cmplo);
    __m128i addhi = _mm_blendv_epi8(addVecDef, addVecInfNan, cmphi);
    intlo = _mm_add_epi32(intlo, addlo);
    inthi = _mm_add_epi32(inthi, addhi);
    __m128i signVec = _mm_and_si128(intVec, _mm_set1_epi16(0x8000));
    __m128i signlo = _mm_unpacklo_epi16(zerosVec, signVec);
    __m128i signhi = _mm_unpackhi_epi16(zerosVec, signVec);
    intlo = _mm_or_si128(intlo, signlo);
    inthi = _mm_or_si128(inthi, signhi);
    _mm_store_si128((__m128i*)(res + i), intlo);
    _mm_store_si128((__m128i*)(res + i + 4), inthi);
  }
}

#endif

/// @brief Multiplies the contents of two vectors, saving the result to the
/// third vector, using AVX SIMD (float version).
/// @details res[i] = a[i] * b[i], i = 0..7.
/// @param a First vector.
/// @param b Second vector.
/// @param res Resulting vector.
/// @pre a, b and res must be aligned to 32 bytes.
INLINE NOTNULL(1, 2, 3) void real_multiply(
    const float *a, const float *b, float *res) {
  __m256 aVec = _mm256_load_ps(a);
  __m256 bVec = _mm256_load_ps(b);
  __m256 resVec = _mm256_mul_ps(aVec, bVec);
  _mm256_store_ps(res, resVec);
}

/// @brief Multiplies the contents of two vectors, saving the result to the
/// third vector, using AVX SIMD (float array version).
/// @details res[i] = a[i] * b[i], i = 0..7.
/// @param a First vector.
/// @param b Second vector.
/// @param length The size of the vectors (in float-s, not in bytes).
/// @param res Resulting array.
INLINE NOTNULL(1, 2, 4) void real_multiply_array(
    const float *a, const float *b, size_t length, float *res) {
  int j, ilength = length;
  for (j = 0; j < ilength - FLOAT_STEP + 1; j += FLOAT_STEP) {
    __m256 aVec = _mm256_loadu_ps(a + j);
    __m256 bVec = _mm256_loadu_ps(b + j);
    __m256 resVec = _mm256_mul_ps(aVec, bVec);
    _mm256_storeu_ps(res + j, resVec);
  }
  for (; j < ilength; j++) {
    res[j] = a[j] * b[j];
  }
}

/// @brief Performs complex multiplication of the contents of two complex
/// vectors, saving the result to the third vector, using AVX SIMD.
/// @details res[i] = a[i] * b[i] - a[i + 1] * b[i + 1], i = 0, 2, 4, 6;
/// res[i + 1] = a[i] * b[i + 1] + a[i + 1] * b[i], i = 1, 3, 5, 7.
/// @param a First vector.
/// @param b Second vector.
/// @param res Result vector.
/// @pre a, b and res must be aligned to 32 bytes.
INLINE NOTNULL(1, 2, 3) void complex_multiply(
    const float *a, const float *b, float *res) {
  __m256 Xvec = _mm256_load_ps(a);
  __m256 Hvec = _mm256_load_ps(b);
  __m256 Xim = _mm256_movehdup_ps(Xvec);
  __m256 Xre = _mm256_moveldup_ps(Xvec);
  __m256 HvecExch = _mm256_shuffle_ps(Hvec, Hvec, 0xB1);
  __m256 resHalf1 = _mm256_mul_ps(Xre, Hvec);
  __m256 resHalf2 = _mm256_mul_ps(Xim, HvecExch);
  __m256 resVec = _mm256_addsub_ps(resHalf1, resHalf2);
  _mm256_store_ps(res, resVec);
}

/// @brief Performs complex multiplication of the contents of two complex
/// vectors, saving the result to the third vector, using AVX SIMD (
/// conjugate version).
/// @details res[i] = a[i] * b[i] + a[i + 1] * b[i + 1], i = 0, 2, 4, 6;
/// res[i + 1] = - a[i] * b[i + 1] + a[i + 1] * b[i], i = 1, 3, 5, 7.
/// @param a First vector.
/// @param b Second vector.
/// @param res Result vector.
/// @pre a, b and res must be aligned to 32 bytes.
INLINE NOTNULL(1, 2, 3) void complex_multiply_conjugate(
    const float *a, const float *b, float *res) {
  __m256 Xvec = _mm256_load_ps(a);
  __m256 Hvec = _mm256_load_ps(b);
  Hvec = _mm256_mul_ps(Hvec, _mm256_set_ps(-1, 1, -1, 1, -1, 1, -1, 1));
  __m256 Xim = _mm256_movehdup_ps(Xvec);
  __m256 Xre = _mm256_moveldup_ps(Xvec);
  __m256 HvecExch = _mm256_shuffle_ps(Hvec, Hvec, 0xB1);
  __m256 resHalf1 = _mm256_mul_ps(Xre, Hvec);
  __m256 resHalf2 = _mm256_mul_ps(Xim, HvecExch);
  __m256 resVec = _mm256_addsub_ps(resHalf1, resHalf2);
  _mm256_store_ps(res, resVec);
}

/// @brief Calculates complex conjugates to array.
/// @param array The array of complex numbers (interleaved).
/// @param length The length of the array (in float-s, not in bytes).
/// @param res The result.
INLINE NOTNULL(1, 3) void complex_conjugate(
    const float *array, size_t length, float *res) {
  int ilength = (int)length;
  int startIndex = align_complement_f32(array);
  if (startIndex == align_complement_f32(res)) {
    for (int i = 1; i < startIndex; i += 2) {
      res[i - 1] = array[i - 1];
      res[i] = -array[i];
    }

    const __m256 mulVec = (startIndex % 2 == 0)?
      _mm256_set_ps(-1, 1, -1, 1, -1, 1, -1, 1) :
      _mm256_set_ps(1, -1, 1, -1, 1, -1, 1, -1);
    for (int i = startIndex; i < ilength - 7; i += 8) {
      __m256 vec = _mm256_load_ps(array + i);
      vec = _mm256_mul_ps(vec, mulVec);
      _mm256_store_ps(res + i, vec);
    }

    for (int i = startIndex + ((ilength - startIndex) & ~0x7) +
             1 - (startIndex % 2);
         i < ilength; i++) {
      res[i - 1] = array[i - 1];
      res[i] = -array[i];
    }
  } else {
    const __m256 mulVec = _mm256_set_ps(-1, 1, -1, 1, -1, 1, -1, 1);
    for (int i = 0; i < ilength - 7; i += 8) {
      __m256 vec = _mm256_loadu_ps(array + i);
      vec = _mm256_mul_ps(vec, mulVec);
      _mm256_storeu_ps(res + i, vec);
    }

    for (int i = ((ilength - startIndex) & ~0x7) + 1;
         i < ilength; i++) {
      res[i - 1] = array[i - 1];
      res[i] = -array[i];
    }
  }
}

/// @brief Multiplies each floating point number in the specified array
/// by the specified value, using AVX SIMD.
/// @details This functions does the same thing as real_multiply_scalar_na, but
/// likely much faster.
/// @param array The array of floating point numbers.
/// @param length The length of the array (in float-s, not in bytes).
/// @param value The value to multiply each number in array.
/// @param res The array to write the results to.
/// @note array and res must have the same alignment.
/// @note res must have at least the same length as array.
INLINE NOTNULL(1, 4) void real_multiply_scalar(const float *array,
                                               size_t length,
                                               float value, float *res) {
  int ilength = (int)length;
  int startIndex = align_complement_f32(array);
  const __m256 mulVec = _mm256_set_ps(value, value, value, value,
                                      value, value, value, value);
  if (startIndex == align_complement_f32(res)) {
    for (int i = 0; i < startIndex; i++) {
      res[i] = array[i] * value;
    }
    for (int i = startIndex; i < ilength - 7; i += 8) {
      __m256 vec = _mm256_load_ps(array + i);
      vec = _mm256_mul_ps(vec, mulVec);
      _mm256_store_ps(res + i, vec);
    }

    for (int i = startIndex + ((ilength - startIndex) & ~0x7);
         i < ilength; i++) {
      res[i] = array[i] * value;
    }
  } else {
    for (int i = 0; i < ilength - 7; i += 8) {
      __m256 vec = _mm256_loadu_ps(array + i);
      vec = _mm256_mul_ps(vec, mulVec);
      _mm256_storeu_ps(res + i, vec);
    }

    for (int i = ((ilength - startIndex) & ~0x7);
         i < ilength; i++) {
      res[i] = array[i] * value;
    }
  }
}

/// @brief Sums all the elements of the array.
/// @param input The array which will be summed.
/// @param length The size of the array (in float-s, not in bytes).
/// @return The sum of all the elements in the array.
INLINE NOTNULL(1) float sum_elements(const float *input, size_t length) {
  assert(align_complement_f32(input) == 0);
  int ilength = (int)length;
  __m256 accum = _mm256_setzero_ps();
  for (int j = 0; j < ilength - 15; j += 16) {
    __m256 vec1 = _mm256_load_ps(input + j);
    __m256 vec2 = _mm256_load_ps(input + j + 8);
    accum = _mm256_add_ps(accum, vec1);
    accum = _mm256_add_ps(accum, vec2);
  }
  accum = _mm256_hadd_ps(accum, accum);
  accum = _mm256_hadd_ps(accum, accum);
  float res = _mm256_get_ps(accum, 0) + _mm256_get_ps(accum, 4);
  for (int j = (ilength & ~0xF); j < ilength; j++) {
    res += input[j];
  }
  return res;
}

/// @brief Adds the same value to all elements in the array.
/// @param input The array to sum with the specified value.
/// @param length The size of the arrays (in float-s, not in bytes).
/// @param value The real number to add to all the elements in input.
/// @param output The array to write the result to.
INLINE NOTNULL(1,4) void add_to_all(float *input, size_t length,
                                    float value, float *output) {
  int ilength = (int)length;
  const __m256 add_vec = _mm256_set1_ps(value);
  for (int j = 0; j < ilength - 15; j += 16) {
    __m256 vec1 = _mm256_load_ps(input + j);
    __m256 vec2 = _mm256_load_ps(input + j + 8);
    vec1 = _mm256_add_ps(add_vec, vec1);
    vec2 = _mm256_add_ps(add_vec, vec2);
    _mm256_store_ps(output + j, vec1);
    _mm256_store_ps(output + j + 8, vec2);
  }
  for (int j = (ilength & ~0xF); j < ilength; j++) {
    output[j] = input[j] + value;
  }
}

#elif defined(__ARM_NEON__)

#define SIMD
#define FLOAT_STEP 4
#define FLOAT_STEP_LOG2 2
#define INT16MUL_STEP 4
#define INT16MUL_STEP_LOG2 2

/// @brief Multiplies the contents of two vectors, saving the result to the
/// third vector, using NEON SIMD (int16_t doubling version).
/// @details res[i] = a[i] * b[i], i = 0..3.
/// @param a First vector.
/// @param b Second vector.
/// @param res Result vector.
INLINE NOTNULL(1, 2, 3) void int16_multiply(
    const int16_t *a, const int16_t *b, int32_t *res) {
  int16x4_t aVec = vld1_s16(a);
  int16x4_t bVec = vld1_s16(b);
  int32x4_t resVec = vmull_s16(aVec, bVec);
  vst1q_s32(res, resVec);
}

INLINE NOTNULL(1, 3) void int16_to_float(const int16_t *data,
                                         size_t length, float *res) {
  int ilength = (int)length;
  for (int i = 0; i < ilength - 3; i += 4) {
    int16x4_t intVec = vld1_s16(data + i);
    int32x4_t extIntVec = vmovl_s16(intVec);
    float32x4_t floatVec = vcvtq_f32_s32(extIntVec);
    vst1q_f32(res + i, floatVec);
  }
  for (int i = (ilength & ~0x3); i < (int)length; i++) {
    res[i] = (float)data[i];
  }
}

INLINE NOTNULL(1, 3) void float_to_int16(const float *data,
                                         size_t length, int16_t *res) {
  int ilength = (int)length;
  for (int i = 0; i < ilength - 3; i += 4) {
    float32x4_t floatVec = vld1q_f32(data + i);
    int32x4_t extIntVec = vcvtq_s32_f32(floatVec);
    int16x4_t intVec = vqmovn_s32(extIntVec);
    vst1_s16(res + i, intVec);
  }
  for (int i = (ilength & ~0x3); i < ilength; i++) {
    res[i] = (int16_t)data[i];
  }
}

INLINE NOTNULL(1, 3) void int32_to_float(const int32_t *data,
                                         size_t length, float *res) {
  int ilength = (int)length;
  for (int i = 0; i < ilength - 3; i += 4) {
    int32x4_t intVec = vld1q_s32(data + i);
    float32x4_t floatVec = vcvtq_f32_s32(intVec);
    vst1q_f32(res + i, floatVec);
  }
  for (int i = (ilength & ~0x3); i < ilength; i++) {
    res[i] = (float)data[i];
  }
}

INLINE NOTNULL(1, 3) void float_to_int32(const float *data,
                                         size_t length, int32_t *res) {
  int ilength = (int)length;
  for (int i = 0; i < ilength - 3; i += 4) {
    float32x4_t floatVec = vld1q_f32(data + i);
    int32x4_t intVec = vcvtq_s32_f32(floatVec);
    vst1q_s32(res + i, intVec);
  }
  for (int i = (ilength & ~0x3); i < ilength; i++) {
    res[i] = (int32_t)data[i];
  }
}

INLINE NOTNULL(1, 3) void int16_to_int32(const int16_t *data,
                                         size_t length, int32_t *res) {
  int ilength = (int)length;
  for (int i = 0; i < ilength - 3; i += 4) {
    int16x4_t intVec = vld1_s16(data + i);
    int32x4_t extIntVec = vmovl_s16(intVec);
    vst1q_s32(res + i, extIntVec);
  }
  for (int i = (ilength & ~0x3); i < ilength; i++) {
    res[i] = (float)data[i];
  }
}

INLINE NOTNULL(1, 3) void int32_to_int16(const int32_t *data,
                                           size_t length, int16_t *res) {
  int ilength = (int)length;
  for (int i = 0; i < ilength - 3; i += 4) {
    int32x4_t extIntVec = vld1q_s32(data + i);
    int16x4_t intVec = vqmovn_s32(extIntVec);
    vst1_s16(res + i, intVec);
  }
  for (int i = (ilength & ~0x3); i < ilength; i++) {
    res[i] = (int16_t)data[i];
  }
}

/// @brief Converts an array of 16-bit floats to 32-bit floating point numbers,
/// using ARM NEON SIMD.
/// @param data The array of float16 (unit16_t).
/// @param length The length of the array (in uint16_t-s, not in bytes).
/// @param res The floating point number array to write the results to.
/// @note align_complement_i16(data) % 4 must be equal to
/// align_complement_f32(res) % 4.
/// @note res must have at least the same length as data.
INLINE NOTNULL(1, 3) void float16_to_float(
    const uint16_t *data, size_t length, float *__restrict res) {
  int ilength = (int)length;
  const uint16x8_t expMask = vdupq_n_u16(0x7c00);
  const uint16x8_t zerosVec = vdupq_n_u16(0);
  for (int i = 0; i < ilength - 7; i += 8) {
    uint16x8_t intVec = vld1q_u16(data + i);
    uint16x8_t expVec = vandq_u16(intVec, expMask);
    uint16x8_t cmpVec = vceqq_u16(expVec, zerosVec);
    uint64x2_t zeroAdd = vpaddlq_u32(vpaddlq_u16(cmpVec));
    uint64_t zero_check = vgetq_lane_u64(zeroAdd, 0) + vgetq_lane_u64(zeroAdd, 1);
    if (zero_check != 0) {
      // There are zeros or subnormals
      if (zero_check == (0xffff << 3)) {
        // there are only zeros or subnormals
        uint16x8_t tmpVec = vandq_u16(intVec, vdupq_n_u16(0x03ff));
        cmpVec = vceqq_u16(tmpVec, zerosVec);
        zeroAdd = vpaddlq_u32(vpaddlq_u16(cmpVec));
        zero_check = vgetq_lane_u64(zeroAdd, 0) + vgetq_lane_u64(zeroAdd, 1);
        if (zero_check == (0xffff << 3)) {
          // only zeros
          uint16x8_t signVec = vandq_u16(intVec, vdupq_n_u16(0x8000));
          uint32x4_t signlo = vmovl_u16(vget_low_u16(signVec));
          uint32x4_t signhi = vmovl_u16(vget_high_u16(signVec));
          signlo = vshlq_n_u32(signlo, 16);
          signhi = vshlq_n_u32(signhi, 16);
          vst1q_u32((uint32_t*)(res + i), signlo);
          vst1q_u32((uint32_t*)(res + i + 4), signhi);
          continue;
        } else if (zero_check == 0) {
          // only subnormals
          uint16x8_t lz = vclzq_u16(tmpVec);
          lz = vsubq_u16(lz, vdupq_n_u16(5));
          expVec = vsubq_u16(vdupq_n_u16(127 - 15 + 1), lz);
          tmpVec = vshlq_u16(tmpVec, vreinterpretq_s16_u16(lz));
          tmpVec = vandq_u16(tmpVec, vdupq_n_u16(0x03ff));
          uint32x4_t tmplo = vmovl_u16(vget_low_u16(tmpVec));
          uint32x4_t tmphi = vmovl_u16(vget_high_u16(tmpVec));
          tmplo = vshlq_n_u32(tmplo, 13);
          tmphi = vshlq_n_u32(tmphi, 13);
          uint32x4_t explo = vmovl_u16(vget_low_u16(expVec));
          uint32x4_t exphi = vmovl_u16(vget_high_u16(expVec));
          explo = vshlq_n_u32(explo, 23);
          exphi = vshlq_n_u32(exphi, 23);
          tmplo = vorrq_u32(tmplo, explo);
          tmphi = vorrq_u32(tmphi, exphi);
          uint16x8_t signVec = vandq_u16(intVec, vdupq_n_u16(0x8000));
          uint32x4_t signlo = vmovl_u16(vget_low_u16(signVec));
          uint32x4_t signhi = vmovl_u16(vget_high_u16(signVec));
          signlo = vshlq_n_u32(signlo, 16);
          signhi = vshlq_n_u32(signhi, 16);
          tmplo = vorrq_u32(signlo, tmplo);
          tmphi = vorrq_u32(signhi, tmplo);
          vst1q_u32((uint32_t*)(res + i), tmplo);
          vst1q_u32((uint32_t*)(res + i + 4), tmphi);
          continue;
        } else {
          float16_to_float_na(data + i, 8, res + i);
          continue;
        }
      } else {
        float16_to_float_na(data + i, 8, res + i);
        continue;
      }
    }
    cmpVec = vceqq_u16(expVec, expMask);
    uint16x8_t masked1 = vandq_u16(vdupq_n_u16(0x03ff), cmpVec);
    uint16x8_t masked2 = vbicq_u16(vdupq_n_u16(0x7fff), cmpVec);
    uint16x8_t andVec = vorrq_u16(masked1, masked2);
    uint16x8_t tmpVec = vandq_u16(intVec, andVec);
    uint32x4_t intlo = vmovl_u16(vget_low_u16(tmpVec));
    uint32x4_t inthi = vmovl_u16(vget_high_u16(tmpVec));
    intlo = vshlq_n_u32(intlo, 13);
    inthi = vshlq_n_u32(inthi, 13);
    masked1 = vandq_u16(vdupq_n_u16(0x7f80), cmpVec);
    masked2 = vbicq_u16(vdupq_n_u16(0x3800), cmpVec);
    uint16x8_t addVec = vorrq_u16(masked1, masked2);
    uint32x4_t addlo = vmovl_u16(vget_low_u16(addVec));
    uint32x4_t addhi = vmovl_u16(vget_high_u16(addVec));
    addlo = vshlq_n_u32(addlo, 16);
    addhi = vshlq_n_u32(addhi, 16);
    intlo = vaddq_u32(intlo, addlo);
    inthi = vaddq_u32(inthi, addhi);
    uint16x8_t signVec = vandq_u16(intVec, vdupq_n_u16(0x8000));
    uint32x4_t signlo = vmovl_u16(vget_low_u16(signVec));
    uint32x4_t signhi = vmovl_u16(vget_high_u16(signVec));
    signlo = vshlq_n_u32(signlo, 16);
    signhi = vshlq_n_u32(signhi, 16);
    intlo = vorrq_u32(intlo, signlo);
    inthi = vorrq_u32(inthi, signhi);
    vst1q_u32((uint32_t*)(res + i), intlo);
    vst1q_u32((uint32_t*)(res + i + 4), inthi);
  }

  int offset = ilength & ~0x7;
  float16_to_float_na(data + offset, ilength - offset, res + offset);
}

/// @brief Multiplies the contents of two vectors, saving the result to the
/// third vector, using NEON SIMD (float version).
/// @details res[i] = a[i] * b[i], i = 0..3.
/// @param a First vector.
/// @param b Second vector.
/// @param res Result vector.
INLINE NOTNULL(1, 2, 3) void real_multiply(
    const float *a, const float *b, float *res) {
  float32x4_t aVec = vld1q_f32(a);
  float32x4_t bVec = vld1q_f32(b);
  float32x4_t resVec = vmulq_f32(aVec, bVec);
  vst1q_f32(res, resVec);
}

/// @brief Multiplies the contents of two vectors, saving the result to the
/// third vector, using NEON SIMD (float array version).
/// @details res[i] = a[i] * b[i], i = 0..3.
/// @param a First vector.
/// @param b Second vector.
/// @param length The size of the vectors (in float-s, not in bytes).
/// @param res Resulting array.
INLINE NOTNULL(1, 2, 4) void real_multiply_array(
    const float *a, const float *b, size_t length, float *res) {
  int j, ilength = length;
  for (j = 0; j < ilength - FLOAT_STEP + 1; j += FLOAT_STEP) {
    real_multiply(a + j, b + j, res + j);
  }
  for (; j < ilength; j++) {
    res[j] = a[j] * b[j];
  }
}

/// @brief Performs complex multiplication of the contents of two complex
/// vectors, saving the result to the third vector, using NEON SIMD.
/// @details res[i] = a[i] * b[i] - a[i + 1] * b[i + 1], i = 0, 2;
/// res[i + 1] = a[i] * b[i + 1] + a[i + 1] * b[i], i = 1, 3.
/// @param a First vector.
/// @param b Second vector.
/// @param res Result vector.
INLINE NOTNULL(1, 2, 3) void complex_multiply(
    const float *a, const float *b, float *res) {
  const float32x4_t negVec = { 1.0f, -1.0f, 1.0f, -1.0f };
  float32x4_t Xvec = vld1q_f32(a);
  float32x4_t Hvec = vld1q_f32(b);
  float32x4_t Xrev = vrev64q_f32(Xvec);
  float32x4_t fwdMul = vmulq_f32(Xvec, Hvec);
  float32x4_t resIm = vmulq_f32(Hvec, Xrev);
  float32x4_t negMul = vmulq_f32(fwdMul, negVec);
  float32x4x2_t resPair = vtrnq_f32(negMul, resIm);
  float32x4_t resVec = vaddq_f32(resPair.val[0], resPair.val[1]);
  vst1q_f32(res, resVec);
}

/// @brief Performs complex multiplication of the contents of two complex
/// vectors, saving the result to the third vector, using NEON SIMD (
/// conjugate version).
/// @details res[i] = a[i] * b[i] + a[i + 1] * b[i + 1], i = 0, 2;
/// res[i + 1] = - a[i] * b[i + 1] + a[i + 1] * b[i], i = 1, 3.
/// @param a First vector.
/// @param b Second vector.
/// @param res Result vector.
INLINE NOTNULL(1, 2, 3) void complex_multiply_conjugate(
    const float *a, const float *b, float *res) {
  const float32x4_t negVec = { 1.0f, -1.0f, 1.0f, -1.0f };
  float32x4_t Xvec = vld1q_f32(a);
  float32x4_t Hvec = vld1q_f32(b);
  Hvec = vmulq_f32(Hvec, negVec);
  float32x4_t Xrev = vrev64q_f32(Xvec);
  float32x4_t fwdMul = vmulq_f32(Xvec, Hvec);
  float32x4_t resIm = vmulq_f32(Hvec, Xrev);
  float32x4_t negMul = vmulq_f32(fwdMul, negVec);
  float32x4x2_t resPair = vtrnq_f32(negMul, resIm);
  float32x4_t resVec = vaddq_f32(resPair.val[0], resPair.val[1]);
  vst1q_f32(res, resVec);
}

/// @brief Calculates complex conjugates to array.
/// @param array The array of complex numbers (interleaved).
/// @param length The length of the array (in float-s, not in bytes).
/// @param res The result.
INLINE NOTNULL(1, 3) void complex_conjugate(const float *array,
                                            size_t length,
                                            float *res) {
  int ilength = (int)length;
  const float32x4_t negVec = { 1.0f, -1.0f, 1.0f, -1.0f };
  for (int i = 0; i < ilength - 3; i += 4) {
    float32x4_t vec = vld1q_f32(array + i);
    vec = vmulq_f32(vec, negVec);
    vst1q_f32(res + i, vec);
  }
  for (int i = (ilength & ~0x3) + 1; i < ilength; i++) {
    res[i - 1] = array[i - 1];
    res[i] = -array[i];
  }
}

/// @brief Multiplies each floating point number in the specified array
/// by the specified value, using NEON SIMD.
/// @details This functions does the same thing as real_multiply_scalar_na, but
/// likely much faster.
/// @param array The array of floating point numbers.
/// @param length The length of the array (in float-s, not in bytes).
/// @param value The value to multiply each number in array.
/// @param res The array to write the results to.
/// @note res must have at least the same length as array.
INLINE NOTNULL(1, 4) void real_multiply_scalar(const float *array,
                                               size_t length,
                                               float value, float *res) {
  int ilength = (int)length;
  for (int i = 0; i < ilength - 3; i += 4) {
    float32x4_t vec = vld1q_f32(array + i);
    vec = vmulq_n_f32(vec, value);
    vst1q_f32(res + i, vec);
  }
  for (int i = (ilength & ~0x3); i < ilength; i++) {
    res[i] = array[i] * value;
  }
}

/// @brief Sums all the elements of the array.
/// @param input The array which will be summed.
/// @param length The size of the array (in float-s, not in bytes).
/// @return The sum of all the elements in the array.
INLINE NOTNULL(1) float sum_elements(const float *input, size_t length) {
  int ilength = (int)length;
  float32x4_t accum = vdupq_n_f32(0.f);
  for (int j = 0; j < ilength - 7; j += 8) {
    float32x4_t vec1 = vld1q_f32(input + j);
    float32x4_t vec2 = vld1q_f32(input + j + 4);
    accum = vaddq_f32(accum, vec1);
    accum = vaddq_f32(accum, vec2);
  }
  float32x2_t accum2 = vpadd_f32(vget_high_f32(accum),
                                 vget_low_f32(accum));
  float res = vget_lane_f32(accum2, 0) + vget_lane_f32(accum2, 1);
  for (int j = (ilength & ~0x7); j < ilength; j++) {
    res += input[j];
  }
  return res;
}

/// @brief Adds the same value to all elements in the array.
/// @param input The array to sum with the specified value.
/// @param length The size of the arrays (in float-s, not in bytes).
/// @param value The real number to add to all the elements in input.
/// @param output The array to write the result to.
INLINE NOTNULL(1,4) void add_to_all(float *input, size_t length,
                                    float value, float *output) {
  int ilength = (int)length;
  const float32x4_t add_vec = vdupq_n_f32(value);
  for (int j = 0; j < ilength - 7; j += 8) {
    float32x4_t vec1 = vld1q_f32(input + j);
    float32x4_t vec2 = vld1q_f32(input + j + 4);
    vec1 = vaddq_f32(add_vec, vec1);
    vec2 = vaddq_f32(add_vec, vec2);
    vst1q_f32(output + j, vec1);
    vst1q_f32(output + j + 8, vec2);
  }
  for (int j = (ilength & ~0x7); j < ilength; j++) {
    output[j] = input[j] + value;
  }
}

#else

#define int16_to_float int16_to_float_na
#define int32_to_float int32_to_float_na
#define float_to_int16 float_to_int16_na
#define float_to_int32 float_to_int32_na
#define int32_to_int16 int32_to_int16_na
#define int16_to_int32 int16_to_int32_na
#define float16_to_float float16_to_float_na
#define real_multiply real_multiply_na
#define real_multiply_array real_multiply_array_na
#define complex_multiply complex_multiply_na
#define complex_multiply_conjugate complex_multiply_conjugate_na
#define complex_conjugate complex_conjugate_na
#define real_multiply_scalar real_multiply_scalar_na
#define sum_elements sum_elements_na
#define add_to_all add_to_all_na

#endif

/*
 * Below are the functions without SIMD instructions.
 */

INLINE int next_highest_power_of_2(int value) {
  value--;
  value |= value >> 1;
  value |= value >> 2;
  value |= value >> 4;
  value |= value >> 8;
  value |= value >> 16;
  return ++value;
}

#pragma GCC diagnostic pop

#endif  // INC_SIMD_ARITHMETIC_H_
