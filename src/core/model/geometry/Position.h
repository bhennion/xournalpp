/*
 * Xournal++
 *
 * Position of a rectangle
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include "util/Point.h"

struct Position {
    xoj::util::Point<double> center;  ///< Center of the snapping box
    double halfWidth;                 ///< Horizontal (before rotation)
    double halfHeight;                ///< Vertical (before rotation)
    double angle;                     ///< Rotation (around the center)

    struct FixedCorner {
        enum : bool { NORTH, SOUTH } ns;
        enum : bool { EAST, WEST } ew;
    };
    void rescale(FixedCorner corner, double halfW, double halfH);
    void setAngle(double angle);
    void moveTo(const xoj::util::Point<double>& c);
};
