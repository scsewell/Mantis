#pragma once

#include "Mantis.h"

namespace Mantis
{
    /// <summary>
    /// Represents a 2d rectangle.
    /// </summary>
    class Rect
    {
    public:
        float x;
        float y;
        float width;
        float height;

        /// <summary>
        /// Gest the position of the rect.
        /// </summary>
        Vector2 Position() const;

        /// <summary>
        /// Gets the size of the rect.
        /// </summary>
        Vector2 Size() const;

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
