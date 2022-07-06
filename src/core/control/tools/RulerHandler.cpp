#include "RulerHandler.h"

#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "gui/inputdevices/PositionInputData.h"    // for PositionInputData
#include "model/Point.h"                           // for Point

class XournalView;

RulerHandler::RulerHandler(XournalView* xournal, const PageRef& page): BaseShapeHandler(xournal, page) {}

RulerHandler::~RulerHandler() = default;

auto RulerHandler::createShape(const PositionInputData& pos) -> std::pair<std::vector<Point>, Range> {
    Point secondPoint = snappingHandler.snap(this->currPoint, this->startPoint, pos.isAltDown());
    Range rg(this->startPoint.x, this->startPoint.y);
    rg.addPoint(secondPoint.x, secondPoint.y);
    return {{this->startPoint, secondPoint}, rg};
}
