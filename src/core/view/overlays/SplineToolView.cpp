#include "SplineToolView.h"

#include <algorithm>  // for for_each
#include <cassert>
#include <cmath>
#include <cstdio>
#include <optional>
#include <vector>

#include "control/tools/SplineHandler.h"
#include "model/Point.h"
#include "model/SplineSegment.h"
#include "util/raii/CairoWrappers.h"
#include "view/Repaintable.h"

class OverlayBase;

using namespace xoj::view;

SplineToolView::SplineToolView(const SplineHandler* splineHandler, const Repaintable* parent):
        BaseShapeOrSplineToolView(splineHandler, parent), splineHandler(splineHandler) {
    this->registerToPool(splineHandler->getViewPool());
}

SplineToolView::~SplineToolView() noexcept = default;

void SplineToolView::draw(cairo_t* cr) const {
    auto data = this->splineHandler->getDataClone();
    if (!data) {
        return;
    }
    assert(!data->knots.empty() && data->knots.size() == data->tangents.size());

    const double zoom = this->parent->getZoom();
    const double radius = RADIUS_WITHOUT_ZOOM / zoom;
    const double lineWidth = LINE_WIDTH_WITHOUT_ZOOM / zoom;

    xoj::util::CairoSaveGuard saveGuard(cr);
    cairo_set_line_width(cr, lineWidth);

    const Point& firstKnot = data->knots.front();
    const Point& lastKnot = data->knots.back();
    const Point& firstTangent = data->tangents.front();
    const Point& lastTangent = data->tangents.back();

    auto drawCircle = [cr, radius](const Point& center) {
        cairo_new_sub_path(cr);
        cairo_arc(cr, center.x, center.y, radius, 0, 2 * M_PI);
    };

    // draw circles around knot points
    Util::cairo_set_source_argb(cr, NODE_CIRCLE_COLOR);
    std::for_each(std::next(data->knots.begin()), data->knots.end(), drawCircle);
    cairo_stroke(cr);

    Util::cairo_set_source_argb(cr, FIRST_NODE_CIRCLE_COLOR);
    drawCircle(firstKnot);
    if (data->closedSpline) {  // current point lies within the circle around the first knot
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }

    // draw dynamically changing segment and tangent vector
    Util::cairo_set_source_argb(cr, DYNAMIC_OBJECTS_COLOR);
    const Point& cp1 = Point(lastKnot.x + lastTangent.x, lastKnot.y + lastTangent.y);
    const Point& cp2 =
            data->closedSpline ? Point(firstKnot.x - firstTangent.x, firstKnot.y - firstTangent.y) : data->currPoint;
    const Point& otherKnot = data->closedSpline ? firstKnot : data->currPoint;
    SplineSegment(lastKnot, cp1, cp2, otherKnot).draw(cr);

    // draw dynamically changing tangent vector
    cairo_move_to(cr, lastKnot.x - lastTangent.x, lastKnot.y - lastTangent.y);
    cairo_line_to(cr, lastKnot.x + lastTangent.x, lastKnot.y + lastTangent.y);
    cairo_stroke(cr);


    // draw other tangent vectors
    Util::cairo_set_source_argb(cr, TANGENT_VECTOR_COLOR);
    for (size_t i = 0; i < data->knots.size() - 1; i++) {
        cairo_move_to(cr, data->knots[i].x - data->tangents[i].x, data->knots[i].y - data->tangents[i].y);
        cairo_line_to(cr, data->knots[i].x + data->tangents[i].x, data->knots[i].y + data->tangents[i].y);
    }
    cairo_stroke(cr);

    if (data->knots.size() > 1) {
        cairo_t* effCr = this->prepareContext(cr);

        cairo_move_to(effCr, firstKnot.x, firstKnot.y);
        auto itKnot1 = data->knots.begin();
        auto itKnot2 = std::next(itKnot1);
        auto itTgt1 = data->tangents.begin();
        auto itTgt2 = std::next(itTgt1);
        for (; itKnot2 != data->knots.end(); ++itKnot1, ++itKnot2, ++itTgt1, ++itTgt2) {
            cairo_curve_to(effCr, itKnot1->x + itTgt1->x, itKnot1->y + itTgt1->y, itKnot2->x - itTgt2->x,
                           itKnot2->y - itTgt2->y, itKnot2->x, itKnot2->y);
        }

        this->commitDrawing(cr);
    }
}

bool SplineToolView::isViewOf(const OverlayBase* overlay) const { return overlay == this->splineHandler; }

void SplineToolView::on(PaintRequest, const Range& rg) { this->parent->flagDirtyRegion(rg); }
