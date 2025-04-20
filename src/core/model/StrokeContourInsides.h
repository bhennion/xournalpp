/// Takes a double between -2*pi and 2*pi and returns the corresponding angle between -pi and pi
static constexpr double inMinusPiPiInterval(double t) {
    xoj_assert(t >= -2 * M_PI && t <= 2 * M_PI);
    return t > M_PI ? t - 2 * M_PI : t < -M_PI ? t + 2 * M_PI : t;
}

// Adds to cairo a segment corresponding to the cord of an arc obtained via cairo_arc() (with the same parameters)
static void drawChord(cairo_t* cr, double cx, double cy, double radius, double angle1, double angle2) {
    cairo_line_to(cr, cx + radius * std::cos(angle1), cy + radius * std::sin(angle1));
    cairo_line_to(cr, cx + radius * std::cos(angle2), cy + radius * std::sin(angle2));
}

static void lineTo(cairo_t* cr, double x, double y, double, double, double) { cairo_line_to(cr, x, y); }

static void hookAfter(cairo_t* cr, double x, double y, double r, double a, double b) {
    cairo_line_to(cr, x, y);
    cairo_arc(cr, x, y, r, a, b);
}
static void hookBefore(cairo_t* cr, double x, double y, double r, double a, double b) {
    cairo_arc(cr, x, y, r, a, b);
    cairo_line_to(cr, x, y);
}

struct ReturnOp {
    ReturnOp(void (*op)(cairo_t*, double, double, double, double, double), double x, double y, double r, double a,
             double b):
            op(op), x(x), y(y), r(r), a(a), b(b) {}
    void (*op)(cairo_t*, double, double, double, double, double);
    double x, y, r, a, b;

    void operator()(cairo_t* cr) { op(cr, x, y, r, a, b); }
};

// StrokeContour
xoj::view::StrokeContour::StrokeContour(const std::vector<Point>& path): path(path) {}
xoj::view::StrokeContour::~StrokeContour() = default;

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
                ops.emplace_back(lineTo, p2.x + .5 * p2.z * std::cos(a1 - angleIn),
                                 p2.y + .5 * p2.z * std::sin(a1 - angleIn), 0., 0., 0.);
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

void xoj::view::StrokeContour::addToCairo(cairo_t* cr) const {
    xoj_assert(path.size() >= 2);
    contourStrokeEnd(cr, path.front(), path[1]);
    // left side of the stroke
    auto ops = addSideToCairo(cr, path.begin(), path.end());

    // Second end of the stroke
    contourStrokeEnd(cr, path.back(), path[path.size() - 2]);

    for (auto it = ops.rbegin(); it < ops.rend(); it++) {
        (*it)(cr);
    }

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
    for (auto it = ops.rbegin(); it < ops.rend(); it++) {
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

// StrokeContourPixelPrecise

xoj::view::StrokeContourPixelPrecise::StrokeContourPixelPrecise(const std::vector<Point>& path): path(path) {}
xoj::view::StrokeContourPixelPrecise::~StrokeContourPixelPrecise() = default;

static inline void drawCoupling(cairo_t* cr, std::vector<ReturnOp>& ops, const Point& p2, double n1, double n3,
                                double a1, double a3, double z1) {
    double normThinest = z1 > p2.z ? n3 : n1;

    if (.5 * std::abs(p2.z - z1) < normThinest) [[likely]] {
        if (z1 < p2.z) {
            double angleIn = std::asin(z1 / p2.z);
            double a = inMinusPiPiInterval(a1 + angleIn);
            double b = inMinusPiPiInterval(a3 - M_PI_2);
            auto* arcFun =
                    (inMinusPiPiInterval(a3 - a1) <= 0.0 || inMinusPiPiInterval(b - a) > 0.0) ? cairo_arc : drawChord;
            arcFun(cr, p2.x, p2.y, .5 * p2.z, a, b);

            double bp = inMinusPiPiInterval(a1 - angleIn);
            double ap = inMinusPiPiInterval(a3 + M_PI_2);
            auto* arcFunp =
                    (inMinusPiPiInterval(a3 - a1) >= 0.0 || inMinusPiPiInterval(bp - ap) > 0.0) ? cairo_arc : drawChord;
            ops.emplace_back(arcFunp, p2.x, p2.y, .5 * p2.z, ap, bp);
        } else {
            double angleOut = std::asin(p2.z / z1);
            double a = inMinusPiPiInterval(a1 + M_PI_2);
            double b = inMinusPiPiInterval(a3 - angleOut);
            auto* arcFun =
                    (inMinusPiPiInterval(a3 - a1) <= 0.0 || inMinusPiPiInterval(b - a) > 0.0) ? cairo_arc : drawChord;
            arcFun(cr, p2.x, p2.y, .5 * z1, a, b);

            double bp = inMinusPiPiInterval(a1 - M_PI_2);
            double ap = inMinusPiPiInterval(a3 + angleOut);
            auto* arcFunp =
                    (inMinusPiPiInterval(a3 - a1) >= 0.0 || inMinusPiPiInterval(bp - ap) > 0.0) ? cairo_arc : drawChord;
            ops.emplace_back(arcFunp, p2.x, p2.y, .5 * z1, ap, bp);
        }
    } else {
        if (z1 < p2.z) {
            cairo_line_to(cr, p2.x, p2.y);
            cairo_arc(cr, p2.x, p2.y, .5 * p2.z, a1, a3 - M_PI_2);
            ops.emplace_back(hookBefore, p2.x, p2.y, .5 * p2.z, a3 + M_PI_2, a1);
        } else {
            cairo_arc(cr, p2.x, p2.y, .5 * z1, a1 + M_PI_2, a3);
            cairo_line_to(cr, p2.x, p2.y);
            ops.emplace_back(hookAfter, p2.x, p2.y, .5 * z1, a3, a1 - M_PI_2);
        }
    }
}

template <typename It>
static inline std::vector<ReturnOp> addSideToCairoPixelPrecise(cairo_t* cr, It begin, It end) {
    std::vector<ReturnOp> ops;
    ops.reserve(static_cast<size_t>(std::distance(begin, end)));

    for (auto it1 = begin, it2 = it1 + 1, it3 = it2 + 1; it3 != end; it1++, it2++, it3++) {
        const auto& p1 = *it1;
        const auto& p2 = *it2;
        const auto& p3 = *it3;

        MathVect2 v1(p2, p1);
        MathVect2 v3(p2, p3);

        drawCoupling(cr, ops, p2, v1.norm(), v3.norm(), v1.argument(), v3.argument(), p1.z);
    }
    return ops;
}

template <bool forward>
static inline void contourStrokeEndPixelPrecise(cairo_t* cr, const Point& endPoint, const Point& adjacentPoint) {
    double a = MathVect2(endPoint, adjacentPoint).argument();
    cairo_arc(cr, endPoint.x, endPoint.y, .5 * (forward ? endPoint.z : adjacentPoint.z), a + M_PI_2, a - M_PI_2);
}

void xoj::view::StrokeContourPixelPrecise::addToCairo(cairo_t* cr) const {
    xoj_assert(path.size() >= 2);
    contourStrokeEndPixelPrecise<true>(cr, path.front(), path[1]);
    // left side of the stroke
    auto ops = addSideToCairoPixelPrecise(cr, path.begin(), path.end());

    // Second end of the stroke
    contourStrokeEndPixelPrecise<false>(cr, path.back(), path[path.size() - 2]);

    for (auto it = ops.rbegin(); it < ops.rend(); it++) {
        (*it)(cr);
    }

    cairo_close_path(cr);
}

void xoj::view::StrokeContourPixelPrecise::drawDebug(cairo_t* cr) const {
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
    contourStrokeEndPixelPrecise<true>(cr, path.front(), path[1]);
    // left side of the stroke
    auto ops = addSideToCairoPixelPrecise(cr, path.begin(), path.end());
    cairo_set_line_width(cr, .1);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_stroke(cr);

    // Second end of the stroke
    contourStrokeEndPixelPrecise<false>(cr, path.back(), path[path.size() - 2]);
    // right side of the stroke on the way back

    for (auto it = ops.rbegin(); it < ops.rend(); it++) {
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


// Dashes
xoj::view::StrokeContourPixelPreciseDashes::StrokeContourPixelPreciseDashes(const std::vector<Point>& path,
                                                                            const std::vector<double>& dashPattern):
        path(path), dashPattern(dashPattern) {}
xoj::view::StrokeContourPixelPreciseDashes::~StrokeContourPixelPreciseDashes() = default;

static void noop(cairo_t*) {};

template <auto xtraFun = noop>
static inline void doSeg(cairo_t* cr, std::vector<ReturnOp>& ops, double& dashoffset,
                         std::vector<double>::const_iterator& dashIt, const std::vector<double>& dashPattern,
                         const Point& p1, const Point& p2, double norm1, double a1, bool& on) {
    dashoffset += norm1;

    while (dashoffset >= *dashIt) {
        Point p = p1.lineTo(p2, *dashIt - dashoffset + norm1);
        if (on) {
            cairo_arc(cr, p.x, p.y, .5 * p1.z, a1 + M_PI_2, a1 - M_PI_2);
            for (auto it = ops.rbegin(); it < ops.rend(); it++) {
                (*it)(cr);
            }
            cairo_close_path(cr);
            xtraFun(cr);

            ops.clear();
        } else {
            cairo_new_sub_path(cr);
            cairo_arc(cr, p.x, p.y, .5 * p1.z, a1 - M_PI_2, a1 + M_PI_2);
        }

        dashoffset -= *dashIt;
        if (++dashIt == dashPattern.end()) {
            dashIt = dashPattern.begin();
        }
        on = !on;
    }
}

void xoj::view::StrokeContourPixelPreciseDashes::addToCairo(cairo_t* cr) const {
    double dashoffset = 0.;
    std::vector<ReturnOp> ops;
    auto dashIt = dashPattern.begin();
    bool on = true;

    contourStrokeEndPixelPrecise<true>(cr, path.front(), path[1]);

    for (auto it1 = path.begin(), it2 = it1 + 1, it3 = it2 + 1; it3 != path.end(); it1++, it2++, it3++) {
        const auto& p1 = *it1;
        const auto& p2 = *it2;
        const auto& p3 = *it3;

        MathVect2 v1(p2, p1);
        double norm1 = v1.norm();
        double a1 = v1.argument();

        doSeg(cr, ops, dashoffset, dashIt, dashPattern, p1, p2, norm1, a1, on);
        if (on) {
            MathVect2 v3(p2, p3);
            drawCoupling(cr, ops, p2, std::min(dashoffset, norm1), std::min(*dashIt - dashoffset, v3.norm()), a1,
                         v3.argument(), p1.z);
        }
    }

    const Point& p1 = path[path.size() - 2];
    const Point& p2 = path.back();
    MathVect2 v(p2, p1);
    double a = v.argument();
    doSeg(cr, ops, dashoffset, dashIt, dashPattern, p1, p2, v.norm(), a, on);
    if (on) {
        cairo_arc(cr, p2.x, p2.y, .5 * p1.z, a + M_PI_2, a - M_PI_2);
        for (auto it = ops.rbegin(); it < ops.rend(); it++) {
            (*it)(cr);
        }
        cairo_close_path(cr);
    }
}

static void xtraFun(cairo_t* cr) {
    static int i = 0;
    static constexpr struct {
        double r, g, b;
    } colors[] = {{1., 0., 0.}, {0., 0.2, 1.}};
    cairo_set_source_rgb(cr, colors[i].r, colors[i].g, colors[i].b);
    i = (i + 1) % 2;
    cairo_stroke(cr);
}

void xoj::view::StrokeContourPixelPreciseDashes::drawDebug(cairo_t* cr) const {
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

    cairo_set_line_width(cr, .1);
    double dashoffset = 0.;
    std::vector<ReturnOp> ops;
    auto dashIt = dashPattern.begin();
    bool on = true;

    contourStrokeEndPixelPrecise<true>(cr, path.front(), path[1]);

    for (auto it1 = path.begin(), it2 = it1 + 1, it3 = it2 + 1; it3 != path.end(); it1++, it2++, it3++) {
        const auto& p1 = *it1;
        const auto& p2 = *it2;
        const auto& p3 = *it3;

        MathVect2 v1(p2, p1);
        double norm1 = v1.norm();
        double a1 = v1.argument();
        doSeg<xtraFun>(cr, ops, dashoffset, dashIt, dashPattern, p1, p2, norm1, a1, on);

        if (on) {
            MathVect2 v3(p2, p3);
            drawCoupling(cr, ops, p2, std::min(dashoffset, norm1), std::min(*dashIt - dashoffset, v3.norm()), a1,
                         v3.argument(), p1.z);
        }
    }

    const Point& p1 = path[path.size() - 2];
    const Point& p2 = path.back();
    MathVect2 v(p2, p1);
    double a = v.argument();
    doSeg<xtraFun>(cr, ops, dashoffset, dashIt, dashPattern, p1, p2, v.norm(), a, on);
    if (on) {
        cairo_arc(cr, p2.x, p2.y, .5 * p1.z, a + M_PI_2, a - M_PI_2);
        for (auto it = ops.rbegin(); it < ops.rend(); it++) {
            (*it)(cr);
        }
        xtraFun(cr);
    }
}
