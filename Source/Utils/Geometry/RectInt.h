#pragma once

#include "Mantis.h"

namespace Mantis
{
    /// <summary>
    /// Represents a 2d rectangle.
    /// </summary>
    class RectInt
    {
    public:
        int x;
        int y;
        int width;
        int height;

        /// <summary>
        /// Gest the position of the rect.
        /// </summary>
        Vector2Int Position() const;

        /// <summary>
        /// Gets the size of the rect.
        /// </summary>
        Vector2Int Size() const;

        /// <summary>
        /// Gets the center of the rect.
        /// </summary>
        Vector2 Center() const;

        /// <summary>
        /// Gets the aspect ratio of the rect.
        /// </summary>
        float GetAspectRatio() const;
    };
}
