#pragma once

#include "Mantis.h"

namespace Mantis
{
    class Vector2Int
    {
    public:
        int x;
        int y;

        /// <summary>
        /// A vector with components (0, 0).
        /// </summary>
        static constexpr Vector2Int Zero();

        /// <summary>
        /// A vector with components (1, 1).
        /// </summary>
        static constexpr Vector2Int One();

        /// <summary>
        /// A vector with components (1, 0).
        /// </summary>
        static constexpr Vector2Int UnitX();

        /// <summary>
        /// A vector with components (0, 1).
        /// </summary>
        static constexpr Vector2Int UnitY();

        /// <summary>
        /// Constructs a zero vector.
        /// </summary>
        constexpr Vector2Int(void);

        /// <summary>
        /// Constructs a vector with all components set to the same value.
        /// </summary>
        /// <param name="v">The x and y coordinates.</param>
        constexpr Vector2Int(const int v);

        /// <summary>
        /// Constructs a vector.
        /// </summary>
        /// <param name="x">The x coordinate.</param>
        /// <param name="y">The y coordinate.</param>
        constexpr Vector2Int(const int x, const int y);

        /// <summary>
        /// Copies a vector.
        /// </summary>
        /// <param name="v">The vector to copy.</param>
        Vector2Int(const Vector2Int& v);

        /// <summary>
        /// Copies a vector.
        /// </summary>
        /// <param name="v">The vector to copy.</param>
        Vector2Int(const Vector2Int* v);
    };

    constexpr Vector2Int Vector2Int::Zero()
    {
        return Vector2Int(0);
    }
    constexpr Vector2Int Vector2Int::One()
    {
        return Vector2Int(1);
    }
    constexpr Vector2Int Vector2Int::UnitX()
    {
        return Vector2Int(1, 0);
    }
    constexpr Vector2Int Vector2Int::UnitY()
    {
        return Vector2Int(0, 1);
    }
}

#include "Vector2Int.inl"
