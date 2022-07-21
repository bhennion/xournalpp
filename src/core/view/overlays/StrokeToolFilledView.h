/*
 * Xournal++
 *
 * View active stroke tool -- filled strokes, but not filled highlighter.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <vector>

#include <cairo.h>

#include "model/Point.h"
#include "util/Point.h"

#include "StrokeToolView.h"

namespace xoj::view {
class StrokeToolFilledHighlighterView;

/**
 * @brief View active stroke tool -- with a filling
 *
 * In draw, the filling is painted directly, while the stroke itself is blitted from the mask
 */
class StrokeToolFilledView: public StrokeToolView {
public:
    StrokeToolFilledView(const StrokeHandler* strokeHandler, const Stroke& stroke, const Repaintable* parent);
    virtual ~StrokeToolFilledView() noexcept;

    void drawFilling(cairo_t* cr, const std::vector<Point>& pts) const override;

    void on(xoj::view::StrokeToolView::AddPointRequest, const Point& p) override;

protected:
    class FillingData {
    public:
        FillingData(double alpha, const Point& p): alpha(alpha), firstPoint(p.x, p.y), contour{p} {}

        const double alpha;
        const utl::Point<double> firstPoint;  // Store a copy for safe concurrent access
    private:
        void appendSegments(const std::vector<Point>& pts);

        /**
         * @brief Filling contour (i.e. the stroke's entire path)
         */
        std::vector<Point> contour;
        /*
         * The data is not mutex protected. Only use it in the drawing thread.
         */
        friend void StrokeToolFilledView::drawFilling(cairo_t*, const std::vector<Point>& pts) const;
        friend StrokeToolFilledHighlighterView;  //::drawFilling(cairo_t*) const;
    };

    mutable FillingData filling;
};
};  // namespace xoj::view
