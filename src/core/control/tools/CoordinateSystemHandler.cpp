#include "CoordinateSystemHandler.h"

#include <algorithm>  // for max

#include "control/Control.h"                       // for Control
#include "control/settings/Settings.h"             // for Settings
#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "gui/XournalView.h"                       // for XournalView
#include "model/Point.h"                           // for Point

CoordinateSystemHandler::CoordinateSystemHandler(Control* control, const PageRef& page, bool flipShift,
                                                 bool flipControl):
        OnlyGridSnappingShapeHandler(control, page, flipShift, flipControl) {}

CoordinateSystemHandler::~CoordinateSystemHandler() = default;

/**
 * Draw a Cartesian coordinate system.
 *
 * @param currentPoint The current point the mouse cursor is pointing to.
 * @param shiftDown Boolean to indicate if "shift" is currently pressed.
 *                  It is currently not used.
 */
auto CoordinateSystemHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
        -> std::pair<std::vector<Point>, Range> {
    /**
     * Snap point to grid (if enabled)
     */
    const Point& c = this->currPoint;

    double width = c.x - this->startPoint.x;
    double height = c.y - this->startPoint.y;

    this->modShift = isShiftDown;
    this->modControl = isControlDown;

    Settings* settings = control->getSettings();
    if (settings->getDrawDirModsEnabled()) {
        // change modifiers based on draw dir
        this->modifyModifiersByDrawDir(width, height, true);
    }

    if (this->modShift) {
        // make square
        int signW = width > 0 ? 1 : -1;
        int signH = height > 0 ? 1 : -1;
        width = std::max(width * signW, height * signH) * signW;
        height = (width * signW) * signH;
    }

    const Point& p1 = this->startPoint;

    Range rg(p1.x, p1.y);
    rg.addPoint(p1.x + width, p1.y + height);

    if (!this->modControl) {
        // draw out from starting point
        return {{p1, Point(p1.x, p1.y + height), Point(p1.x + width, p1.y + height)}, rg};
    } else {
        // Control is down
        return {{Point(p1.x, p1.y + height), p1, Point(p1.x + width, p1.y)}, rg};
    }
}
