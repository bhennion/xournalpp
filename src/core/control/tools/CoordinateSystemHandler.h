/*
 * Xournal++
 *
 * Handles input to draw an coordinate system
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef

#include "BaseShapeHandler.h"  // for BaseShapeHandler

class Point;
class PositionInputData;
class XournalView;

class CoordinateSystemHandler: public BaseShapeHandler {
public:
    CoordinateSystemHandler(XournalView* xournal, const PageRef& page, bool flipShift = false,
                            bool flipControl = false);
    ~CoordinateSystemHandler() override;

private:
    auto createShape(const PositionInputData& pos) -> std::pair<std::vector<Point>, Range> override;
};
