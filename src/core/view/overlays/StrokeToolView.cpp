#include "StrokeToolView.h"

#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <numeric>

#include "control/tools/StrokeHandler.h"
#include "model/LineStyle.h"
#include "model/Stroke.h"
#include "util/Color.h"
#include "util/PairView.h"
#include "util/Range.h"
#include "util/raii/CairoWrappers.h"  // for CairoSaveGuard
#include "view/Repaintable.h"
#include "view/StrokeViewHelper.h"

using namespace xoj::view;

StrokeToolView::StrokeToolView(const StrokeHandler* strokeHandler, const Stroke& stroke, const Repaintable* parent):
        BaseStrokeToolView(parent, stroke), strokeHandler(strokeHandler), pointBuffer(stroke.getPointVector()) {
    this->registerToPool(strokeHandler->getViewPool());
    parent->flagDirtyRegion(Range(stroke.boundingRect()));
}

StrokeToolView::~StrokeToolView() noexcept = default;

bool StrokeToolView::isViewOf(const OverlayBase* overlay) const { return overlay == this->strokeHandler; }

void StrokeToolView::draw(cairo_t* cr) const {

    std::vector<Point> pts = this->flushBuffer();
    if (pts.empty()) {
        // The input sequence has probably been cancelled. This view should soon be deleted
        return;
    }
    // pts.front() is the last point we painted on the mask during the last iteration (see flushBuffer())

    if (!mask.isInitialized()) {
        // Initialize the mask on first call
        mask = createMask(cr);
    }

    xoj::util::CairoSaveGuard saveGuard(cr);
    cairo_set_operator(cr, this->cairoOp);

    this->drawFilling(cr, pts);  // Noop in the base class.

    Util::cairo_set_source_argb(cr, strokeColor);

    if (pts.size() > 1) {
        // Draw the new segments on the mask
        if (pts.front().z == Point::NO_PRESSURE) {
            StrokeViewHelper::drawNoPressure(this->mask.get(), pts, this->strokeWidth, this->lineStyle,
                                             this->dashOffset);
            if (this->lineStyle.hasDashes()) {
                // Keep the offset up to date, so we do not have to redraw the entire stroke every time.
                PairView segments(pts);
                this->dashOffset =
                        std::transform_reduce(segments.begin(), segments.end(), this->dashOffset, std::plus<>(),
                                              [](auto& seg) { return seg.front().lineLengthTo(seg.back()); });
            }
        } else {
            this->dashOffset =
                    StrokeViewHelper::drawWithPressure(this->mask.get(), pts, this->lineStyle, this->dashOffset);
        }
    } else if (this->singleDot) {
        this->drawDot(this->mask.get(), pts.back());
    }

    cairo_mask_surface(cr, cairo_get_target(this->mask.get()), 0, 0);
}

void StrokeToolView::on(StrokeToolView::AddPointRequest, const Point& p) {
    this->singleDot = false;
    Point lastPoint;
    {
        std::lock_guard lock(this->bufferMutex);
        assert(!this->pointBuffer.empty());  // front() is the last point we painted on the mask (see flushBuffer())
        lastPoint = this->pointBuffer.back();
        this->pointBuffer.emplace_back(p);
    }
    this->parent->flagDirtyRegion(this->getRepaintRange(lastPoint, p));
}

void StrokeToolView::on(StrokeToolView::ThickenFirstPointRequest, double newWidth) {
    assert(newWidth > 0.0);
    Range rg;
    {
        std::lock_guard lock(this->bufferMutex);
        assert(this->pointBuffer.size() == 1);
        Point& p = this->pointBuffer.back();
        assert(p.z <= newWidth);  // Thicken means thicken
        p.z = newWidth;
        rg = Range(p.x, p.y);
    }
    rg.addPadding(0.5 * newWidth);
    this->parent->flagDirtyRegion(rg);
}

void StrokeToolView::on(StrokeToolView::CancellationRequest, const Range& rg) {
    {
        std::lock_guard lock(this->bufferMutex);
        this->pointBuffer.clear();
    }
    this->parent->flagDirtyRegion(rg);
}

auto StrokeToolView::getRepaintRange(const Point& lastPoint, const Point& addedPoint) const -> Range {
    const double width = lastPoint.z == Point::NO_PRESSURE ? this->strokeWidth : lastPoint.z;
    Range rg(lastPoint.x, lastPoint.y);
    rg.addPoint(addedPoint.x, addedPoint.y);
    rg.addPadding(0.5 * width);
    return rg;
}

void StrokeToolView::drawDot(cairo_t* cr, const Point& p) const {
    cairo_set_line_width(cr, p.z == Point::NO_PRESSURE ? this->strokeWidth : p.z);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, p.x, p.y);
    cairo_line_to(cr, p.x, p.y);
    cairo_stroke(cr);
}

std::vector<Point> StrokeToolView::flushBuffer() const {
    std::vector<Point> pts;
    std::lock_guard lock(this->bufferMutex);
    std::swap(this->pointBuffer, pts);
    if (!pts.empty()) {
        // Keep the last point in the buffer - to be used in the next iteration
        this->pointBuffer.emplace_back(pts.back());
    }
    return pts;
}
