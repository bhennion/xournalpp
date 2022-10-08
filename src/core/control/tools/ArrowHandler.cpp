#include "ArrowHandler.h"

#include <cmath>      // for cos, sin, atan2, M_PI

#include "control/Control.h"                       // for Control
#include "control/ToolHandler.h"                   // for ToolHandler
#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "model/Point.h"                           // for Point
#include "model/path/PiecewiseLinearPath.h"
#include "util/Range.h"                            // for Range

ArrowHandler::ArrowHandler(Control* control, const PageRef& page, bool doubleEnded):
        BaseShapeHandler(control, page), doubleEnded(doubleEnded) {}

ArrowHandler::~ArrowHandler() = default;

auto ArrowHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
        -> std::pair<std::shared_ptr<Path>, Range> {
    Point c = snappingHandler.snap(this->currPoint, this->startPoint, isAltDown);

    // We've now computed the line points for the arrow
    // so we just have to build the head

    // set up the size of the arrowhead to be 7x the thickness of the line
    double arrowDist = control->getToolHandler()->getThickness() * 7.0;

    // an appropriate delta is Pi/3 radians for an arrow shape
    double delta = M_PI / 6.0;
    double angle = atan2(c.y - this->startPoint.y, c.x - this->startPoint.x);

    auto shape = std::make_shared<PiecewiseLinearPath>(this->startPoint, doubleEnded ? 8 : 4);

    if (doubleEnded) {
        shape->addLineSegmentTo(startPoint.x + arrowDist * cos(angle + delta),
                                startPoint.y + arrowDist * sin(angle + delta));
        shape->addLineSegmentTo(startPoint);
        shape->addLineSegmentTo(startPoint.x + arrowDist * cos(angle - delta),
                                startPoint.y + arrowDist * sin(angle - delta));
        shape->addLineSegmentTo(startPoint);
    }

    shape->addLineSegmentTo(c);
    shape->addLineSegmentTo(c.x - arrowDist * cos(angle + delta), c.y - arrowDist * sin(angle + delta));
    shape->addLineSegmentTo(c);
    shape->addLineSegmentTo(c.x - arrowDist * cos(angle - delta), c.y - arrowDist * sin(angle - delta));

    auto res = std::make_pair<std::shared_ptr<Path>, Range>(std::move(shape), Range());

    // No fast trick to compute the bbox of the arrow, because of the head(s). Use the default algorithm
    res.second = Range(res.first->getThinBoundingBox());
    return res;
}
