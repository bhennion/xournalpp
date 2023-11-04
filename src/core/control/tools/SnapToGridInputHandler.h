/*
 * Xournal++
 *
 * Snapping methods which take account of the settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include "model/Point.h"
#include "util/Point.h"

class Settings;

class SnapToGridInputHandler final {

public:
    SnapToGridInputHandler(const Settings* settings);

protected:
    const Settings* settings;

public:
    /**
     * @brief If a value is near enough to the y-coordinate of a grid point, it returns the nearest y-coordinate of the
     * grid point. Otherwise the original value itself.
     * @param y the value
     * @param alt indicates whether snapping mode is altered (via the Alt key)
     */
    [[nodiscard]] double snapVertically(double y, bool alt) const;

    /**
     * @brief If a value is near enough to the x-coordinate of a grid point, it returns the nearest x-coordinate of the
     * grid point. Otherwise the original value itself.
     * @param x the value
     * @param alt indicates whether snapping mode is altered (via the Alt key)
     */
    [[nodiscard]] double snapHorizontally(double x, bool alt) const;

    /**
     * @brief Snap a point to the nearest (and near enough) point on a grid line in the provided direction
     * @param direction The snapping direction. Assumption: the vector direction has length 1.
     * @param alt indicates whether snapping mode is altered (via the Alt key)
     * @return The distance in the provided direction between the original point and the snapped point.
     *          The return value could be 0.0 if no grid lines were close enough
     *          The return value could be negative if the snapped point is "behind" the original point
     *
     * If you know direction == {1.0, 0.0} or {0.0, 1.0}, use snapHorizontally or snapVertically for better perfs
     */
    [[nodiscard]] double snapAlong(Point const& pos, xoj::util::Point<double> direction, bool alt) const;

    /**
     * @brief If a points distance to the nearest grid point is under a certain tolerance, it returns the nearest
     * grid point. Otherwise the original Point itself.
     * @param pos the position
     * @param alt indicates whether snapping mode is altered (via the Alt key)
     */
    [[nodiscard]] Point snapToGrid(Point const& pos, bool alt) const;

    /**
     * @brief if the angles distance to a multiple quarter of PI is under a certain tolerance, it returns the latter.
     * Otherwise the original angle.
     * @param radian the angle (in radian)
     * @param alt indicates whether snapping mode is altered (via the Alt key)
     */
    [[nodiscard]] double snapAngle(double radian, bool alt) const;

    /**
     * @brief Snaps the angle between the horizontal axis and the line between the given point and center
     * and returns the point rotated accordingly
     * @param pos the coordinate of the point
     * @param center the center of rotation
     * @param alt indicates whether snapping mode is altered (via the Alt key)
     */
    [[nodiscard]] Point snapRotation(Point const& pos, Point const& center, bool alt) const;

    /**
     * @brief Does rotation snapping followed by snapping to grid
     * @param pos the coordinate of the point
     * @param center the center of rotation
     * @param alt indicates whether snapping mode is altered (via the Alt key)
     */
    [[nodiscard]] Point snap(Point const& pos, Point const& center, bool alt) const;
};
