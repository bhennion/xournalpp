#include "StrokeToolFilledHighlighterView.h"

#include <vector>

#include "model/Point.h"
#include "util/Color.h"
#include "util/raii/CairoWrappers.h"
#include "view/StrokeViewHelper.h"

using namespace xoj::view;

StrokeToolFilledHighlighterView::StrokeToolFilledHighlighterView(const StrokeHandler* strokeHandler,
                                                                 const Stroke& stroke, const Repaintable* parent):
        StrokeToolFilledView(strokeHandler, stroke, parent) {}

StrokeToolFilledHighlighterView::~StrokeToolFilledHighlighterView() noexcept = default;

void StrokeToolFilledHighlighterView::draw(cairo_t* cr) const {

    std::vector<Point> pts = this->flushBuffer();
    if (pts.empty()) {
        // The input sequence has probably been cancelled. This view should soon be deleted
        return;
    }

    this->filling.appendSegments(pts);

    if (!this->mask.isInitialized()) {
        // Initialize mask on first call
        this->mask = this->createMask(cr);
    }

    // Upon adding a segment, the filling can actually shrink, making it easier to redraw the filling every time.
    // First wipe the mask
    this->mask.wipe();

    /*
     * Draw both the filling and the stroke alike on the mask
     */
    cairo_set_line_width(this->mask.get(), this->strokeWidth);
    StrokeViewHelper::pathToCairo(this->mask.get(), this->filling.contour);
    cairo_fill_preserve(this->mask.get());
    cairo_stroke(this->mask.get());

    xoj::util::CairoSaveGuard saveGuard(cr);
    Util::cairo_set_source_argb(cr, this->strokeColor);
    cairo_set_operator(cr, this->cairoOp);

    cairo_mask_surface(cr, cairo_get_target(this->mask.get()), 0, 0);
}
