#pragma once

#include "Vector2.h"
#include "VectorMath.h"

namespace Mantis
{
    // Constructors
    MANTIS_FORCE_INLINE constexpr Vector2::Vector2(void) : x(0), y(0) {}
    MANTIS_FORCE_INLINE constexpr Vector2::Vector2(float v) : x(v), y(v) {}
    MANTIS_FORCE_INLINE constexpr Vector2::Vector2(float x, float y) : x(x), y(y) {}
    MANTIS_FORCE_INLINE Vector2::Vector2(const Vector2& v) : x(v.x), y(v.y) {}
    MANTIS_FORCE_INLINE Vector2::Vector2(const Vector2* v) : x(v->x), y(v->y) {}

    // Member functions
    MANTIS_FORCE_INLINE float Vector2::Length() const
    {
        return _store_1(_len_2(_load(this)));
    }
    MANTIS_FORCE_INLINE float Vector2::LengthFast() const
    {
        return _store_1(_len_fast_2(_load(this)));
    }
    MANTIS_FORCE_INLINE float Vector2::LengthSquared() const
    {
        return _store_1(_len_sqr_2(_load(this)));
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::Normalized() const
    {
        __m128 v0 = _load(this);
        return _store_2(_nrm(v0, _len_2(v0)));
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::NormalizedFast() const
    {
        __m128 v0 = _load(this);
        return _store_2(_nrm_fast(v0, _len_sqr_2(v0)));
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::PerpendicularRight() const
    {
        return Vector2(y, -x);
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::PerpendicularLeft() const
    {
        return Vector2(-y, x);
    }

    MANTIS_FORCE_INLINE String Vector2::ToString() const
    {
        String str;
        str.append_sprintf("(%f, %f)", x, y);
        return str;
    }

    // Static functions
    MANTIS_FORCE_INLINE Vector2 Vector2::ComponentMin(const Vector2 & a, const Vector2 & b)
    {
        return _store_2(_min(_load(a), _load(b)));
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::ComponentMax(const Vector2 & a, const Vector2 & b)
    {
        return _store_2(_max(_load(a), _load(b)));
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::ComponentClamp(const Vector2 & v, const Vector2 & min, const Vector2 & max)
    {
        return _store_2(_clamp(_load(v), _load(min), _load(max)));
    }

    MANTIS_FORCE_INLINE Vector2 Vector2::SelectMin(const Vector2 & a, const Vector2 & b)
    {
        __m128 a0 = _load(a);
        __m128 b0 = _load(b);
        return _store_2(_min_mag(a0, _len_sqr_2(a0), b0, _len_sqr_2(b0)));
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::SelectMax(const Vector2 & a, const Vector2 & b)
    {
        __m128 a0 = _load(a);
        __m128 b0 = _load(b);
        return _store_2(_max_mag(a0, _len_sqr_2(a0), b0, _len_sqr_2(b0)));
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::MagnitudeClamp(const Vector2 & v, const float& maxLength)
    {
        __m128 v0 = _load(v);
        __m128 m0 = _broadcast(maxLength);
        __m128 vSqrLen = _len_sqr_2(v0);

        __m128 clamped = _mul(_nrm_fast(v0, vSqrLen), m0);

        return _store_2(_min_mag(v0, vSqrLen, clamped, _mul(m0, m0)));
    }

    // Operators
    MANTIS_FORCE_INLINE float& Vector2::operator [] (const size_t& i)
    {
        return ((float*)this)[i];
    }
    MANTIS_FORCE_INLINE const float& Vector2::operator [] (const size_t& i) const
    {
        return ((float*)this)[i];
    }

    MANTIS_FORCE_INLINE Vector2& Vector2::operator = (const Vector2& v)
    {
        x = v.x;
        y = v.y;
        return *this;
    }

    MANTIS_FORCE_INLINE bool Vector2::operator == (const Vector2& v) const
    {
        return _cmpeq(_load(this), _load(v));
    }
    MANTIS_FORCE_INLINE bool Vector2::operator != (const Vector2 & v) const
    {
        return _cmpneq(_load(this), _load(v));
    }

    MANTIS_FORCE_INLINE Vector2& Vector2::operator - (void)
    {
        _store_2(this, _mul(_load(this), _broadcast(-1.f)));
        return *this;
    }

    MANTIS_FORCE_INLINE Vector2 Vector2::operator + (const Vector2 & v) const
    {
        return _store_2(_add(_load(this), _load(v)));
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::operator - (const Vector2 & v) const
    {
        return _store_2(_sub(_load(this), _load(v)));
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::operator * (const Vector2 & v) const
    {
        return _store_2(_mul(_load(this), _load(v)));
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::operator / (const Vector2 & v) const
    {
        return _store_2(_div(_load(this), _load(v)));
    }

    MANTIS_FORCE_INLINE Vector2& Vector2::operator += (const Vector2 & v)
    {
        _store_2(this, _add(_load(this), _load(v)));
        return *this;
    }
    MANTIS_FORCE_INLINE Vector2& Vector2::operator -= (const Vector2 & v)
    {
        _store_2(this, _sub(_load(this), _load(v)));
        return *this;
    }
    MANTIS_FORCE_INLINE Vector2& Vector2::operator *= (const Vector2 & v)
    {
        _store_2(this, _mul(_load(this), _load(v)));
        return *this;
    }
    MANTIS_FORCE_INLINE Vector2& Vector2::operator /= (const Vector2 & v)
    {
        _store_2(this, _div(_load(this), _load(v)));
        return *this;
    }

    MANTIS_FORCE_INLINE Vector2 Vector2::operator + (const float& v) const
    {
        return _store_2(_add(_load(this), _broadcast(v)));
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::operator - (const float& v) const
    {
        return _store_2(_sub(_load(this), _broadcast(v)));
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::operator * (const float& v) const
    {
        return _store_2(_mul(_load(this), _broadcast(v)));
    }
    MANTIS_FORCE_INLINE Vector2 Vector2::operator / (const float& v) const
    {
        return _store_2(_div(_load(this), _broadcast(v)));
    }

    MANTIS_FORCE_INLINE Vector2& Vector2::operator += (const float& v)
    {
        _store_2(this, _add(_load(this), _broadcast(v)));
        return *this;
    }
    MANTIS_FORCE_INLINE Vector2& Vector2::operator -= (const float& v)
    {
        _store_2(this, _sub(_load(this), _broadcast(v)));
        return *this;
    }
    MANTIS_FORCE_INLINE Vector2& Vector2::operator *= (const float& v)
    {
        _store_2(this, _mul(_load(this), _broadcast(v)));
        return *this;
    }
    MANTIS_FORCE_INLINE Vector2& Vector2::operator /= (const float& v)
    {
        _store_2(this, _div(_load(this), _broadcast(v)));
        return *this;
    }
}
