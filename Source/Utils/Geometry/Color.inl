#pragma once

#include "Color.h"
#include "VectorMath.h"

namespace Mantis
{
    // Constructors
    MANTIS_FORCE_INLINE constexpr Color::Color(void) :
        r(1.f),
        g(1.f),
        b(1.f),
        a(1.f)
    {}
    MANTIS_FORCE_INLINE constexpr Color::Color(const float& r, const float& g, const float& b, const float& a) :
        r(r),
        g(g),
        b(b),
        a(a)
    {}
    MANTIS_FORCE_INLINE constexpr Color::Color(const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) :
        r(r / (float)eastl::numeric_limits<uint8_t>::max()),
        g(g / (float)eastl::numeric_limits<uint8_t>::max()),
        b(b / (float)eastl::numeric_limits<uint8_t>::max()),
        a(a / (float)eastl::numeric_limits<uint8_t>::max())
    {}

    // Member functions
    MANTIS_FORCE_INLINE uint32_t Color::ToColor32() const
    {
        return
            ((uint32_t)(r * eastl::numeric_limits<uint8_t>::max()) << 24) |
            ((uint32_t)(g * eastl::numeric_limits<uint8_t>::max()) << 16) |
            ((uint32_t)(b * eastl::numeric_limits<uint8_t>::max()) <<  8) |
            ((uint32_t)(a * eastl::numeric_limits<uint8_t>::max()));
    }

    MANTIS_FORCE_INLINE Color Color::ToSrgb() const
    {

    }

    void Color::ToHSV(float& h, float& s, float& v, float a) const
    {

    }

    void Color::ToHSL(float& h, float& s, float& l, float a) const
    {

    }

    MANTIS_FORCE_INLINE String Color::ToString() const
    {
        String str;
        str.append_sprintf("(r:%.3f, g:%.3f, b:%.3f, a:%.3f)", r, g, b, a);
        return str;
    }

    // Static functions
    MANTIS_FORCE_INLINE Color Color::FromColor32(const uint32_t& color)
    {
    }
    MANTIS_FORCE_INLINE Color Color::FromSrgb(const Color& color)
    {
    }
    MANTIS_FORCE_INLINE Color Color::FromHSV(const float& h, const float& s, const float& v, const float& a)
    {
    }
    MANTIS_FORCE_INLINE Color Color::FromHSL(const float& h, const float& s, const float& l, const float& a)
    {
    }

    MANTIS_FORCE_INLINE Color Color::Lerp(const Color& a, const Color& b, const float& t)
    {
    }
    MANTIS_FORCE_INLINE Color Color::LerpClamped(const Color& a, const Color& b, const float& t)
    {
    }

    // Operators
    MANTIS_FORCE_INLINE float& Color::operator [] (const size_t& i)
    {
        return ((float*)this)[i];
    }
    MANTIS_FORCE_INLINE const float& Color::operator [] (const size_t& i) const
    {
        return ((float*)this)[i];
    }

    MANTIS_FORCE_INLINE bool Color::operator == (const Color& c) const
    {
        return _cmpeq(_load(this), _load(c));
    }
    MANTIS_FORCE_INLINE bool Color::operator != (const Color& c) const
    {
        return _cmpneq(_load(this), _load(c));
    }
}
