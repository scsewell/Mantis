#pragma once

#include "Mantis.h"

namespace Mantis
{
    /// <summary>
    /// Represents a RGBA color.
    /// </summary>
    class Color
    {
    public:
        static constexpr Color Clear();
        static constexpr Color Black();
        static constexpr Color Grey();
        static constexpr Color White();
        static constexpr Color Red();
        static constexpr Color Yellow();
        static constexpr Color Green();
        static constexpr Color Cyan();
        static constexpr Color Blue();
        static constexpr Color Magenta();

        float r;
        float g;
        float b;
        float a;

        /// <summary>
        /// Constructs a Color.
        /// </summary>
        constexpr Color(void);

        /// <summary>
        /// Constructs a Color.
        /// </summary>
        /// <param name="r">The red component value.</param>
        /// <param name="g">The green component value.</param>
        /// <param name="b">The blue component value.</param>
        /// <param name="a">The alpha component value.</param>
        constexpr Color(const float& r, const float& g, const float& b, const float& a = 1.0f);

        /// <summary>
        /// Constructs a Color.
        /// </summary>
        /// <param name="r">The red component value.</param>
        /// <param name="g">The green component value.</param>
        /// <param name="b">The blue component value.</param>
        /// <param name="a">The alpha component value.</param>
        constexpr Color(const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a = eastl::numeric_limits<uint8_t>::max());

        /// <summary>
        /// Converts the color to a 32-bit color.
        /// </summary>
        uint32_t ToColor32() const;

        /// <summary>
        /// Transforms a linear color to a sRGB color.
        /// </summary>
        Color ToSrgb() const;

        /// <summary>
        /// Converts RGB color values to HSL color values.
        /// </summary>
        void ToHSV(float& h, float& s, float& v, float a) const;

        /// <summary>
        /// Converts RGB color values to HSL color values.
        /// </summary>
        void ToHSL(float& h, float& s, float& l, float a) const;

        /// <summary>
        /// Converts from an RGBA 8-bit packed color.
        /// </summary>
        static Color FromColor32(const uint32_t& color);
        
        /// <summary>
        /// Transforms a sRGB to a linear color color.
        /// </summary>
        static Color FromSrgb(const Color& color);

        /// <summary>
        /// Converts HSV color values to RGB color values.
        /// </summary>
        static Color FromHSV(const float& h, const float& s, const float& v, const float& a);

        /// <summary>
        /// Converts HSL color values to RGB color values.
        /// </summary>
        static Color FromHSL(const float& h, const float& s, const float& l, const float& a);

        /// <summary>
        /// Linearly interpolates from a to b by factor t.
        /// </summary>
        /// <param name="a">The color to interpolate from.</param>
        /// <param name="b">The color to interpolate to.</param>
        /// <param name="t">The interpolation factor.</param>
        static Color Lerp(const Color& a, const Color& b, const float& t);

        /// <summary>
        /// Linearly interpolates from a to b by factor t, where t is clamped to 0 and 1.
        /// </summary>
        /// <param name="a">The color to interpolate from.</param>
        /// <param name="b">The color to interpolate to.</param>
        /// <param name="t">The interpolation factor.</param>
        static Color LerpClamped(const Color& a, const Color& b, const float& t);

        /// <summary>
        /// Gets a string representation of this instance.
        /// </summary>
        String ToString() const;

        float& operator [] (const size_t& index);
        const float& operator [] (const size_t& index) const;

        bool operator == (const Color& c) const;
        bool operator != (const Color& c) const;
    };

    constexpr Color Color::Clear()      { return Color(0.f, 0.f, 0.f, 0.f); }
    constexpr Color Color::Black()      { return Color(0.f, 0.f, 0.f, 1.f); }
    constexpr Color Color::Grey()       { return Color(0.5f, 0.5f, 0.5f, 1.f); }
    constexpr Color Color::White()      { return Color(1.f, 1.f, 1.f, 1.f); }
    constexpr Color Color::Red()        { return Color(1.f, 0.f, 0.f, 1.f); }
    constexpr Color Color::Yellow()     { return Color(1.f, 1.f, 0.f, 1.f); }
    constexpr Color Color::Green()      { return Color(0.f, 1.f, 0.f, 1.f); }
    constexpr Color Color::Cyan()       { return Color(0.f, 1.f, 1.f, 1.f); }
    constexpr Color Color::Blue()       { return Color(0.f, 0.f, 1.f, 1.f); }
    constexpr Color Color::Magenta()    { return Color(1.f, 0.f, 1.f, 1.f); }
}

namespace std
{
    template<>
    struct hash<Mantis::Color>
    {
        size_t operator () (const Mantis::Color& color) const
        {
            return color.ToColor32();
        }
    };
}

#include "Color.inl"
