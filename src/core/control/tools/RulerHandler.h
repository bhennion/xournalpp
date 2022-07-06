/*
 * Xournal++
 *
 * Handles input of the ruler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>

#include "model/PageRef.h"  // for PageRef

#include "BaseShapeHandler.h"  // for BaseShapeHandler

class Point;
class PositionInputData;
class XournalView;

class RulerHandler: public BaseShapeHandler {
public:
    RulerHandler(XournalView* xournal, const PageRef& page);
    ~RulerHandler() override;

private:
    auto createShape(const PositionInputData& pos) -> std::pair<std::vector<Point>, Range> override;
};
