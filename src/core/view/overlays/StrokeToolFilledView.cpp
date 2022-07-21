#include "StrokeToolFilledView.h"

#include <algorithm>
#include <cassert>
#include <mutex>

#include "model/Stroke.h"
#include "util/Color.h"
#include "util/Range.h"
#include "view/Repaintable.h"
#include "view/StrokeViewHelper.h"

using namespace xoj::view;

static const Point& setupFirstPoint(const Stroke& s) {
    const auto& pts = s.getPointVector();
    assert(!pts.empty());
    return pts.front();
}

StrokeToolFilledView::StrokeToolFilledView(const StrokeHandler* strokeHandler, const Stroke& stroke,
                                           const Repaintable* parent):
        StrokeToolView(strokeHandler, stroke, parent), filling(stroke.getFill() / 255.0, setupFirstPoint(stroke)) {}

StrokeToolFilledView::~StrokeToolFilledView() noexcept = default;

void StrokeToolFilledView::drawFilling(cairo_t* cr, const std::vector<Point>& pts) const {
    this->filling.appendSegments(pts);
    /*
     * Draw the filling directly (not through the mask)
     *   Upon adding a segment, the filling can actually shrink, making it easier to redraw the filling every time.
     */
    StrokeViewHelper::pathToCairo(cr, this->filling.contour);
    Util::cairo_set_source_rgbi(cr, strokeColor, this->filling.alpha);
    cairo_fill(cr);
}

void StrokeToolFilledView::on(StrokeToolView::AddPointRequest, const Point& p) {
    Point lastPoint;
    {
        std::lock_guard lock(this->bufferMutex);
        assert(!this->pointBuffer.empty());
        lastPoint = this->pointBuffer.back();
        this->pointBuffer.emplace_back(p);
    }
    auto rg = this->getRepaintRange(lastPoint, p);
    // Add the first point, so that the range covers all the filling changes
    rg.addPoint(this->filling.firstPoint.x, this->filling.firstPoint.y);
    this->parent->flagDirtyRegion(rg);
}

void StrokeToolFilledView::FillingData::appendSegments(const std::vector<Point>& pts) {
    assert(!pts.empty());
    // Add new points to the contour
    // contour.back() == pts.front(), so we skip it.
    std::copy(std::next(pts.begin()), pts.end(), std::back_inserter(contour));
}
