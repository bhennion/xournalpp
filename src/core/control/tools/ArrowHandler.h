/*
 * Xournal++
 *
 * Handles input to draw an arrow
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

class ArrowHandler: public BaseShapeHandler {
public:
    ArrowHandler(XournalView* xournal, const PageRef& page, bool doubleEnded);
    ~ArrowHandler() override;

private:
    auto createShape(const PositionInputData& pos) -> std::pair<std::vector<Point>, Range> override;
    bool doubleEnded = false;
};
