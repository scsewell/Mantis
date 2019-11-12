#pragma once

#include "Mantis.h"

#include <nmmintrin.h>

namespace Mantis
{
    struct Vector2;
    struct Vector2Int;
    struct Color;

    //-------------------------------------------------------------------
    // Load/Store
    //-------------------------------------------------------------------
    MANTIS_FORCE_INLINE __m128 _load(const float& v)
    {
        return _mm_load_ss(&v);
    }
    MANTIS_FORCE_INLINE __m128 _broadcast(const float& v)
    {
        return _mm_load1_ps(&v);
    }

    MANTIS_FORCE_INLINE __m128 _load(const Vector2& v)
    {
        return _mm_castpd_ps(_mm_load_sd((double*)&v));
    }
    MANTIS_FORCE_INLINE __m128 _load(const Vector2* v)
    {
        return _mm_castpd_ps(_mm_load_sd((double*)v));
    }
    MANTIS_FORCE_INLINE __m128 _load(const Color& v)
    {
        return _mm_loadu_ps((float*)&v);
    }
    MANTIS_FORCE_INLINE __m128 _load(const Color* v)
    {
        return _mm_loadu_ps((float*)v);
    }

    MANTIS_FORCE_INLINE float _store_1(__m128 toStore)
    {
        float f;
        _mm_store_ss(&f, toStore);
        return f;
    }

    MANTIS_FORCE_INLINE void _store_2(Vector2* v, __m128 toStore)
    {
        return _mm_store_sd((double*)v, _mm_castps_pd(toStore));
    }
    MANTIS_FORCE_INLINE Vector2 _store_2(__m128 toStore)
    {
        Vector2 v;
        _store_2(&v, toStore);
        return v;
    }

    //-------------------------------------------------------------------
    // Compare
    //-------------------------------------------------------------------
    MANTIS_FORCE_INLINE bool _cmpeq(__m128 a, __m128 b)
    {
        int result = _mm_movemask_epi8(_mm_castps_si128(_mm_cmpeq_ps(a, b)));
        return result == 0xffff;
    }
    MANTIS_FORCE_INLINE bool _cmpneq(__m128 a, __m128 b)
    {
        int result = _mm_movemask_epi8(_mm_castps_si128(_mm_cmpneq_ps(a, b)));
        return static_cast<bool>(result);
    }

    MANTIS_FORCE_INLINE __m128 _min(__m128 a, __m128 b)
    {
        return _mm_min_ps(a, b);
    }
    MANTIS_FORCE_INLINE __m128 _max(__m128 a, __m128 b)
    {
        return _mm_max_ps(a, b);
    }
    MANTIS_FORCE_INLINE __m128 _clamp(__m128 v, __m128 min, __m128 max)
    {
        return _max(_min(v, max), min);
    }

    MANTIS_FORCE_INLINE __m128 _select(__m128 a, __m128 b, __m128 cmpResult)
    {
        __m128i cmp = _mm_shuffle_epi32(_mm_castps_si128(cmpResult), 0x0000);
        return _mm_blendv_ps(b, a, _mm_castsi128_ps(cmp));
    }
    MANTIS_FORCE_INLINE __m128 _min_mag(__m128 a, __m128 aLenSquared, __m128 b, __m128 bLenSquared)
    {
        return _select(a, b, _mm_cmplt_ss(aLenSquared, bLenSquared));
    }
    MANTIS_FORCE_INLINE __m128 _max_mag(__m128 a, __m128 aLenSquared, __m128 b, __m128 bLenSquared)
    {
        return _select(a, b, _mm_cmpgt_ss(aLenSquared, bLenSquared));
    }

    //-------------------------------------------------------------------
    // Arithmatic
    //-------------------------------------------------------------------
    MANTIS_FORCE_INLINE __m128 _add(__m128 a, __m128 b)
    {
        return _mm_add_ps(a, b);
    }
    MANTIS_FORCE_INLINE __m128 _sub(__m128 a, __m128 b)
    {
        return _mm_sub_ps(a, b);
    }
    MANTIS_FORCE_INLINE __m128 _mul(__m128 a, __m128 b)
    {
        return _mm_mul_ps(a, b);
    }
    MANTIS_FORCE_INLINE __m128 _div(__m128 a, __m128 b)
    {
        return _mm_div_ps(a, b);
    }

    //-------------------------------------------------------------------
    // Length
    //-------------------------------------------------------------------
    MANTIS_FORCE_INLINE __m128 _len_sqr_2(__m128 v0)
    {
        __m128 v1 = _mm_mul_ps(v0, v0);
        __m128 v2 = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(0, 0, 0, 1));
        return _mm_add_ps(v1, v2);;
    }
    MANTIS_FORCE_INLINE __m128 _len_2(__m128 v0)
    {
        return _mm_sqrt_ps(_len_sqr_2(v0));
    }
    MANTIS_FORCE_INLINE __m128 _len_fast_2(__m128 v0)
    {
        __m128 v1 = _len_sqr_2(v0);
        return _mm_mul_ps(v1, _mm_rsqrt_ps(v1));
    }

    //-------------------------------------------------------------------
    // Normalize
    //-------------------------------------------------------------------
    MANTIS_FORCE_INLINE __m128 _nrm(__m128 v, __m128 len)
    {
        return _mm_div_ps(v, len);
    }
    MANTIS_FORCE_INLINE __m128 _nrm_fast(__m128 v, __m128 lenSquared)
    {
        return _mm_mul_ps(v, _mm_rsqrt_ps(lenSquared));
    }
}