/*
 * Xournal++
 *
 * Helper class to add the contour of a stroke to a cairo context
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>  // for vector

#include <cairo.h>

#include "model/Point.h"

namespace xoj::view {
class StrokeContour final {
public:
    explicit StrokeContour(const std::vector<Point>& path);
    ~StrokeContour();
    void addToCairo(cairo_t* cr) const;
    void drawDebug(cairo_t* cr) const;

private:
    const std::vector<Point>& path;
};
class StrokeContourPixelPrecise final {
public:
    explicit StrokeContourPixelPrecise(const std::vector<Point>& path);
    ~StrokeContourPixelPrecise();
    void addToCairo(cairo_t* cr) const;
    void drawDebug(cairo_t* cr) const;

private:
    const std::vector<Point>& path;
};

class StrokeContourPixelPreciseDashes final {
public:
    explicit StrokeContourPixelPreciseDashes(const std::vector<Point>& path, const std::vector<double>& dashPattern);
    ~StrokeContourPixelPreciseDashes();
    void addToCairo(cairo_t* cr) const;
    void drawDebug(cairo_t* cr) const;

private:
    const std::vector<Point>& path;
    const std::vector<double>& dashPattern;
};
};  // namespace xoj::view
