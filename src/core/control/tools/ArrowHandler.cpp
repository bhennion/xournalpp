#include "ArrowHandler.h"

#include <cmath>  // for cos, sin, atan2, M_PI

#include "control/Control.h"                       // for Control
#include "control/ToolHandler.h"                   // for ToolHandler
#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "gui/XournalView.h"                       // for XournalView
#include "gui/inputdevices/PositionInputData.h"    // for PositionInputData
#include "model/Point.h"                           // for Point

ArrowHandler::ArrowHandler(XournalView* xournal, const PageRef& page, bool doubleEnded):
        BaseShapeHandler(xournal, page), doubleEnded(doubleEnded) {}

ArrowHandler::~ArrowHandler() = default;

auto ArrowHandler::createShape(const PositionInputData& pos) -> std::pair<std::vector<Point>, Range> {
    Point c = snappingHandler.snap(this->currPoint, this->startPoint, pos.isAltDown());

    // We've now computed the line points for the arrow
    // so we just have to build the head

    // set up the size of the arrowhead to be 7x the thickness of the line
    double arrowDist = xournal->getControl()->getToolHandler()->getThickness() * 7.0;

    // an appropriate delta is Pi/3 radians for an arrow shape
    double delta = M_PI / 6.0;
    double angle = atan2(c.y - this->startPoint.y, c.x - this->startPoint.x);

    std::pair<std::vector<Point>, Range> res;
    std::vector<Point>& shape = res.first;

    shape.reserve(doubleEnded ? 9 : 5);

    shape.emplace_back(this->startPoint);

    if (doubleEnded) {
        shape.emplace_back(startPoint.x + arrowDist * cos(angle + delta),
                           startPoint.y + arrowDist * sin(angle + delta));
        shape.emplace_back(startPoint);
        shape.emplace_back(startPoint.x + arrowDist * cos(angle - delta),
                           startPoint.y + arrowDist * sin(angle - delta));
        shape.emplace_back(startPoint);
    }

    shape.emplace_back(c);
    shape.emplace_back(c.x - arrowDist * cos(angle + delta), c.y - arrowDist * sin(angle + delta));
    shape.emplace_back(c);
    shape.emplace_back(c.x - arrowDist * cos(angle - delta), c.y - arrowDist * sin(angle - delta));

    auto [minX, maxX] = std::minmax_element(shape.begin(), shape.end(), [](auto& p, auto& q) { return p.x < q.x; });
    auto [minY, maxY] = std::minmax_element(shape.begin(), shape.end(), [](auto& p, auto& q) { return p.y < q.y; });
    res.second = Range(minX->x, minY->y, maxX->x, maxY->y);

    return res;
}
