#pragma once

#include "Mantis.h"

namespace Mantis
{
    /// <summary>
    /// Represents a 2d vector.
    /// </summary>
    class Vector2
    {
    public:
        float x;
        float y;

        /// <summary>
        /// A vector with components (0, 0).
        /// </summary>
        static constexpr Vector2 Zero();

        /// <summary>
        /// A vector with components (1, 1).
        /// </summary>
        static constexpr Vector2 One();

        /// <summary>
        /// A vector with components (1, 0).
        /// </summary>
        static constexpr Vector2 UnitX();

        /// <summary>
        /// A vector with components (0, 1).
        /// </summary>
        static constexpr Vector2 UnitY();

        /// <summary>
        /// Constructs a zero vector.
        /// </summary>
        constexpr Vector2(void);

        /// <summary>
        /// Constructs a vector with all components set to the same value.
        /// </summary>
        /// <param name="v">The x and y coordinates.</param>
        constexpr Vector2(const float v);

        /// <summary>
        /// Constructs a vector.
        /// </summary>
        /// <param name="x">The x coordinate.</param>
        /// <param name="y">The y coordinate.</param>
        constexpr Vector2(const float x, const float y);

        /// <summary>
        /// Copies a vector.
        /// </summary>
        /// <param name="v">The vector to copy.</param>
        Vector2(const Vector2& v);

        /// <summary>
        /// Copies a vector.
        /// </summary>
        /// <param name="v">The vector to copy.</param>
        Vector2(const Vector2* v);

        /// <summary>
        /// Returns the length of this vector.
        /// </summary>
        float Length() const;

        /// <summary>
        /// Returns an approximation for the length of this vector, with 1.5*2^-12 relative error.
        /// </summary>
        float LengthFast() const;

        /// <summary>
        /// Returns the squared length of this vector.
        /// </summary>
        float LengthSquared() const;

        /// <summary>
        /// Returns a copy of the vector scaled to unit length.
        /// </summary>
        Vector2 Normalized() const;

        /// <summary>
        /// Returns a copy of the vector scaled to approximately unit length, with 1.5*2^-12 relative error.
        /// </summary>
        Vector2 NormalizedFast() const;

        /// <summary>
        /// Gets the perpendicular vector on the right side of this vector.
        /// </summary>
        Vector2 PerpendicularRight() const;

        /// <summary>
        /// Gets the perpendicular vector on the left side of this vector.
        /// </summary>
        Vector2 PerpendicularLeft() const;

        /// <summary>
        /// Gets a string representation of this instance.
        /// </summary>
        String ToString() const;

        /// <summary>
        /// Returns the minimum per component of a set of vectors.
        /// </summary>
        /// <param name="a">First operand.</param>
        /// <param name="b">Second operand.</param>
        static Vector2 ComponentMin(const Vector2& a, const Vector2& b);

        /// <summary>
        /// Returns the maximum per component of a set of vectors.
        /// </summary>
        /// <param name="a">First operand.</param>
        /// <param name="b">Second operand.</param>
        static Vector2 ComponentMax(const Vector2& a, const Vector2& b);

        /// <summary>
        /// Returns the maximum per component of a set of vectors.
        /// </summary>
        /// <param name="a">First operand.</param>
        /// <param name="b">Second operand.</param>
        static Vector2 ComponentClamp(const Vector2& v, const Vector2& min, const Vector2& max);

        /// <summary>
        /// Returns the vector with the lesser magnitude.
        /// </summary>
        /// <param name="a">First operand.</param>
        /// <param name="b">Second operand.</param>
        static Vector2 SelectMin(const Vector2& a, const Vector2& b);

        /// <summary>
        /// Returns the vector with the greater magnitude.
        /// </summary>
        /// <param name="a">First operand.</param>
        /// <param name="b">Second operand.</param>
        static Vector2 SelectMax(const Vector2& a, const Vector2& b);

        /// <summary>
        /// Returns the vector with a magnitude less than or equal to the specified length.
        /// </summary>
        /// <param name="vector">The vector to clamp.</param>
        /// <param name="maxLength">The maxiumum maginude of the returned vector.</param>
        static Vector2 MagnitudeClamp(const Vector2& v, const float& maxLength);

        float& operator [] (const size_t& i);
        const float& operator [] (const size_t& i) const;

        Vector2& Vector2::operator = (const Vector2& v);
        
        bool Vector2::operator == (const Vector2& v) const;
        bool Vector2::operator != (const Vector2& v) const;

        Vector2& Vector2::operator - (void);

        Vector2 operator + (const Vector2& v) const;
        Vector2 operator - (const Vector2& v) const;
        Vector2 operator * (const Vector2& v) const;
        Vector2 operator / (const Vector2& v) const;

        Vector2& operator += (const Vector2& v);
        Vector2& operator -= (const Vector2& v);
        Vector2& operator *= (const Vector2& v);
        Vector2& operator /= (const Vector2& v);

        Vector2 operator + (const float& v) const;
        Vector2 operator - (const float& v) const;
        Vector2 operator * (const float& v) const;
        Vector2 operator / (const float& v) const;

        Vector2& operator += (const float& v);
        Vector2& operator -= (const float& v);
        Vector2& operator *= (const float& v);
        Vector2& operator /= (const float& v);
    };

    constexpr Vector2 Vector2::Zero()
    {
        return Vector2(0.f);
    }
    constexpr Vector2 Vector2::One()
    {
        return Vector2(1.f);
    }
    constexpr Vector2 Vector2::UnitX()
    {
        return Vector2(1.f, 0.f);
    }
    constexpr Vector2 Vector2::UnitY()
    {
        return Vector2(0.f, 1.f);
    }
}

#include "Vector2.inl"
