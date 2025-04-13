#include "StrokeContour.h"

#include <cmath>

#include "model/MathVect.h"
#include "util/Assert.h"

static_assert(std::numeric_limits<double>::is_iec559);  // Ensures atan2(0., 0.) does not error

/// Takes a double between -2*pi and 2*pi and returns the corresponding angle between -pi and pi
static constexpr double inMinusPiPiInterval(double t) {
    xoj_assert(t >= -2 * M_PI && t <= 2 * M_PI);
    return t > M_PI ? t - 2 * M_PI : t < -M_PI ? t + 2 * M_PI : t;
}

xoj::view::StrokeContour::StrokeContour(const std::vector<Point>& path): path(path) {}
xoj::view::StrokeContour::~StrokeContour() = default;

// Adds to cairo a segment corresponding to the cord of an arc obtained via cairo_arc() (with the same parameters)
static void drawChord(cairo_t* cr, double cx, double cy, double radius, double angle1, double angle2) {
    cairo_line_to(cr, cx + radius * std::cos(angle1), cy + radius * std::sin(angle1));
    cairo_line_to(cr, cx + radius * std::cos(angle2), cy + radius * std::sin(angle2));
}

static void lineTo(cairo_t* cr, double x, double y, double, double, double) {
    cairo_line_to(cr, x, y);
}

struct ReturnOp {
    ReturnOp(void (*op)(cairo_t*, double, double, double, double, double), double x, double y, double r, double a, double b): op(op), x(x), y(y), r(r), a(a), b(b) {}
    void (*op)(cairo_t*, double, double, double, double, double);
    double x, y, r, a, b;

    void operator()(cairo_t* cr) { op(cr, x, y, r, a, b); }
};

template <typename It>
static inline std::vector<ReturnOp> addSideToCairo(cairo_t* cr, It begin, It end) {
    std::vector<ReturnOp> ops;
    ops.reserve(static_cast<size_t>(std::distance(begin, end)));
    for (auto it1 = begin, it2 = it1 + 1, it3 = it2 + 1; it3 != end; it1++, it2++, it3++) {
        const auto& p1 = *it1;
        const auto& p2 = *it2;
        const auto& p3 = *it3;

        MathVect2 v1(p2, p1);
        MathVect2 v3(p2, p3);

        double a1 = v1.argument();
        double a3 = v3.argument();

        if (double norm1 = v1.norm(); .5 * std::abs(p1.z - p2.z) < norm1) [[likely]] {
            double angleIn = std::acos(.5 * (p2.z - p1.z) / norm1);
            if (double norm3 = v3.norm(); .5 * std::abs(p3.z - p2.z) < norm3) [[likely]] {
                double angleOut = std::acos(.5 * (p2.z - p3.z) / norm3);
                double a = inMinusPiPiInterval(a1 + angleIn);
                double b = inMinusPiPiInterval(a3 - angleOut);
                auto* arcFun = inMinusPiPiInterval(a3 - a1) <= 0.0 ?  // Turn right
                                       angleIn + angleOut - M_PI > 0 &&
                                                       inMinusPiPiInterval(b - a) <
                                                               0.0 ?  // The variation of pressure reverted the arc
                                               drawChord :
                                               cairo_arc :  // Turn left
                                       angleIn + angleOut - M_PI < 0 &&
                                                       inMinusPiPiInterval(b - a) >
                                                               0.0 ?  // The variation of pressure reverted the arc
                                               cairo_arc :
                                               drawChord;
                arcFun(cr, p2.x, p2.y, .5 * p2.z, a, b);

                double bp = inMinusPiPiInterval(a1 - angleIn);
                double ap = inMinusPiPiInterval(a3 + angleOut);
                auto* arcFunp = inMinusPiPiInterval(a3 - a1) >= 0.0 ?  // Turn right
                angleIn + angleOut - M_PI > 0 &&
                inMinusPiPiInterval(bp - ap) <
                0.0 ?  // The variation of pressure reverted the arc
                drawChord :
                cairo_arc :  // Turn left
                angleIn + angleOut - M_PI < 0 &&
                inMinusPiPiInterval(bp - ap) >
                0.0 ?  // The variation of pressure reverted the arc
                cairo_arc :
                drawChord;
                ops.emplace_back(arcFunp, p2.x, p2.y, .5 * p2.z, ap, bp);
            } else if (p2.z > p3.z) {
                // The next point is entirely inside the current point
                cairo_arc(cr, p2.x, p2.y, .5 * p2.z, a1 + angleIn, a3);
                ops.emplace_back(cairo_arc, p2.x, p2.y, .5 * p2.z, a3, a1 - angleIn);
            } else {
                // The current point is entirely inside the next point
                cairo_line_to(cr, p2.x + .5 * p2.z * std::cos(a1 + angleIn), p2.y + .5 * p2.z * std::sin(a1 + angleIn));
                ops.emplace_back(lineTo, p2.x + .5 * p2.z * std::cos(a1 - angleIn), p2.y + .5 * p2.z * std::sin(a1 - angleIn), 0., 0., 0.);
            }
        } else if (p2.z > p1.z) {
            // The previous point is entirely inside the current point
            if (double norm3 = v3.norm(); .5 * std::abs(p3.z - p2.z) < norm3) [[likely]] {
                double angleOut = std::acos(.5 * (p2.z - p3.z) / norm3);
                cairo_arc(cr, p2.x, p2.y, .5 * p2.z, a1, a3 - angleOut);
                ops.emplace_back(cairo_arc, p2.x, p2.y, .5 * p2.z, a3 + angleOut, a1);
            } else if (p2.z > p3.z) {
                // Both the next and previous points are entirely inside the current point
                cairo_arc(cr, p2.x, p2.y, .5 * p2.z, a1, a3);
                ops.emplace_back(cairo_arc, p2.x, p2.y, .5 * p2.z, a3, a1);
            } else {
                // Both the last and current points are entirely inside the next point
                // Do nothing...
            }
        } else {
            // The current point is entirely inside the previous point
            if (double norm3 = v3.norm(); std::abs(p3.z - p2.z) < norm3) [[likely]] {
                double angleOut = std::acos((p2.z - p3.z) / norm3);
                cairo_line_to(cr, p2.x + .5 * p2.z * std::cos(a3 - angleOut),
                              p2.y + .5 * p2.z * std::sin(a3 - angleOut));
                ops.emplace_back(lineTo, p2.x + .5 * p2.z * std::cos(a3 + angleOut),
                                 p2.y + .5 * p2.z * std::sin(a3 + angleOut), 0., 0., 0.);
            } else if (p2.z > p3.z) {
                // Both the next and current points are entirely inside the previous point
                // No need to do anything
            } else {
                // The current point is inside both the last and next points
                // Do nothing...
            }
        }
    }
    return ops;
}

static void contourStrokeEnd(cairo_t* cr, const Point& endPoint, const Point& adjacentPoint) {
    auto v = MathVect2(endPoint, adjacentPoint);
    double a = v.argument();

    if (double norm = v.norm(); .5 * std::abs(adjacentPoint.z - endPoint.z) < norm) [[likely]] {
        double angleout = std::acos(.5 * (endPoint.z - adjacentPoint.z) / norm);
        cairo_arc(cr, endPoint.x, endPoint.y, .5 * endPoint.z, a + angleout, a - angleout);
    } else if (adjacentPoint.z > endPoint.z) {
        // The endpoint is entirely covered by the adjacent one
        // Do nothing
    } else {
        // The adjacent point is entirely covered by the endpoint -- Paint the full disk
        cairo_arc(cr, endPoint.x, endPoint.y, .5 * endPoint.z, 0, 2 * M_PI);
    }
}


template <bool forward, typename It>
static inline void addSideToCairoNew(cairo_t* cr, It begin, It end) {
    for (auto it1 = begin, it2 = it1 + 1, it3 = it2 + 1; it3 != end; it1++, it2++, it3++) {
        const auto& p1 = *it1;
        const auto& p2 = *it2;
        const auto& p3 = *it3;

        MathVect2 v1(p2, p1);
        MathVect2 v3(p2, p3);

        double a1 = v1.argument();
        double a3 = v3.argument();

        double widthIn = forward ? p1.z : p2.z;
        double widthOut = forward ? p2.z : p3.z;
        double normThinest = widthIn > widthOut ? v3.norm() : v1.norm();

        if (.5 * std::abs(widthOut - widthIn) < normThinest) [[likely]] {
            if (widthIn < widthOut) {
                double angleIn = std::asin(widthIn / widthOut);
                double a = inMinusPiPiInterval(a1 + angleIn);
                double b = inMinusPiPiInterval(a3 - M_PI_2);
                auto* arcFun = (inMinusPiPiInterval(a3 - a1) <= 0.0 || inMinusPiPiInterval(b - a) > 0.0) ? cairo_arc :
                                                                                                           drawChord;
                arcFun(cr, p2.x, p2.y, .5 * widthOut, a, b);
            } else {
                double angleOut = std::asin(widthOut / widthIn);
                double a = inMinusPiPiInterval(a1 + M_PI_2);
                double b = inMinusPiPiInterval(a3 - angleOut);
                auto* arcFun = (inMinusPiPiInterval(a3 - a1) <= 0.0 || inMinusPiPiInterval(b - a) > 0.0) ? cairo_arc :
                                                                                                           drawChord;
                arcFun(cr, p2.x, p2.y, .5 * widthIn, a, b);
            }
        } else {
            if (widthIn < widthOut) {
                cairo_line_to(cr, p2.x, p2.y);
                cairo_arc(cr, p2.x, p2.y, .5 * widthOut, a1, a3 - M_PI_2);
            } else {
                cairo_arc(cr, p2.x, p2.y, .5 * widthIn, a1 + M_PI_2, a3);
                cairo_line_to(cr, p2.x, p2.y);
            }
        }
    }
}
template <bool forward>
static void contourStrokeEndNew(cairo_t* cr, const Point& endPoint, const Point& adjacentPoint) {
    auto v = MathVect2(endPoint, adjacentPoint);
    double a = v.argument();
    cairo_arc(cr, endPoint.x, endPoint.y, .5 * (forward ? endPoint.z : adjacentPoint.z), a + M_PI_2, a - M_PI_2);
}

void xoj::view::StrokeContour::addToCairo(cairo_t* cr) const {
    xoj_assert(path.size() >= 2);
    contourStrokeEnd(cr, path.front(), path[1]);
    // left side of the stroke
    auto ops = addSideToCairo(cr, path.begin(), path.end());

    // Second end of the stroke
    contourStrokeEnd(cr, path.back(), path[path.size() - 2]);

    for (auto it = ops.rbegin(); it < ops.rend() ; it++) {
        (*it)(cr);
    }

    // right side of the stroke on the way back
    // addSideToCairo(cr, path.rbegin(), path.rend());

    cairo_close_path(cr);
}

void xoj::view::StrokeContour::addToCairoPixelPrecise(cairo_t* cr) const {
    xoj_assert(path.size() >= 2);
    contourStrokeEndNew<true>(cr, path.front(), path[1]);
    // left side of the stroke
    addSideToCairoNew<true>(cr, path.begin(), path.end());

    // Second end of the stroke
    contourStrokeEndNew<false>(cr, path.back(), path[path.size() - 2]);

    // right side of the stroke on the way back
    addSideToCairoNew<false>(cr, path.rbegin(), path.rend());

    cairo_close_path(cr);
}

void xoj::view::StrokeContour::drawDebug(cairo_t* cr) const {
    {
        // Draw the points as dashed circles
        cairo_save(cr);
        for (auto&& p: path) {
            cairo_new_sub_path(cr);
            cairo_arc(cr, p.x, p.y, .5 * p.z, 0, 2 * M_PI);
        }
        cairo_set_line_width(cr, .05);
        double dashes[2] = {.2, .3};
        cairo_set_dash(cr, dashes, 2, 0.);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_stroke(cr);
        cairo_restore(cr);
    }

    // First end of the stroke
    contourStrokeEnd(cr, path.front(), path[1]);
    // left side of the stroke
    auto ops = addSideToCairo(cr, path.begin(), path.end());
    cairo_set_line_width(cr, .1);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_stroke(cr);

    // Second end of the stroke
    contourStrokeEnd(cr, path.back(), path[path.size() - 2]);
    // right side of the stroke on the way back

    for (auto it = ops.rbegin(); it < ops.rend() ; it++) {
        (*it)(cr);
    }
    cairo_set_line_width(cr, .1);
    cairo_set_source_rgb(cr, 0, .5, 1);
    cairo_stroke(cr);

    for (auto&& p: path) {
        cairo_move_to(cr, p.x, p.y);
        cairo_line_to(cr, p.x, p.y);
    }
    cairo_set_line_width(cr, .2);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_stroke(cr);

    for (auto&& p: {path.front(), path.back()}) {
        cairo_move_to(cr, p.x, p.y);
        cairo_line_to(cr, p.x, p.y);
    }
    cairo_set_line_width(cr, .4);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_stroke(cr);
}

void xoj::view::StrokeContour::drawDebugPixelPrecise(cairo_t* cr) const {
    {
        // Draw the points as dashed circles
        cairo_save(cr);
        for (auto&& p: path) {
            cairo_new_sub_path(cr);
            cairo_arc(cr, p.x, p.y, .5 * p.z, 0, 2 * M_PI);
        }
        cairo_set_line_width(cr, .05);
        double dashes[2] = {.2, .3};
        cairo_set_dash(cr, dashes, 2, 0.);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_stroke(cr);
        cairo_restore(cr);
    }

    // First end of the stroke
    contourStrokeEndNew<true>(cr, path.front(), path[1]);
    // left side of the stroke
    addSideToCairoNew<true>(cr, path.begin(), path.end());
    cairo_set_line_width(cr, .1);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_stroke(cr);

    // Second end of the stroke
    contourStrokeEndNew<false>(cr, path.back(), path[path.size() - 2]);
    // right side of the stroke on the way back
    addSideToCairoNew<false>(cr, path.rbegin(), path.rend());
    cairo_set_line_width(cr, .1);
    cairo_set_source_rgb(cr, 0, .5, 1);
    cairo_stroke(cr);

    for (auto&& p: path) {
        cairo_move_to(cr, p.x, p.y);
        cairo_line_to(cr, p.x, p.y);
    }
    cairo_set_line_width(cr, .2);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_stroke(cr);

    for (auto&& p: {path.front(), path.back()}) {
        cairo_move_to(cr, p.x, p.y);
        cairo_line_to(cr, p.x, p.y);
    }
    cairo_set_line_width(cr, .4);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_stroke(cr);
}
