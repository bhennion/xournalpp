#include "RulerHandler.h"

#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "model/Point.h"                           // for Point

class XournalView;

RulerHandler::RulerHandler(Control* control, const PageRef& page): AllSnappingShapeHandler(control, page) {}

RulerHandler::~RulerHandler() = default;

auto RulerHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
        -> std::pair<std::vector<Point>, Range> {
    Range rg(this->startPoint.x, this->startPoint.y);
    rg.addPoint(this->currPoint.x, this->currPoint.y);
    return {{this->startPoint, this->currPoint}, rg};
}
