#pragma once

#include "Vector2Int.h"

namespace Mantis
{
    // Constructors
    MANTIS_FORCE_INLINE constexpr Vector2Int::Vector2Int(void) : x(0), y(0) {}
    MANTIS_FORCE_INLINE constexpr Vector2Int::Vector2Int(int v) : x(v), y(v) {}
    MANTIS_FORCE_INLINE constexpr Vector2Int::Vector2Int(int x, int y) : x(x), y(y) {}
    MANTIS_FORCE_INLINE Vector2Int::Vector2Int(const Vector2Int& v) : x(v.x), y(v.y) {}
    MANTIS_FORCE_INLINE Vector2Int::Vector2Int(const Vector2Int* v) : x(v->x), y(v->y) {}
}
