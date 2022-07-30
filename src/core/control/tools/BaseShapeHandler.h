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

#include <memory>
#include <vector>

#include <gdk/gdk.h>  // for GdkEventKey

#include "model/PageRef.h"  // for PageRef
#include "model/Point.h"    // for Point
#include "util/Range.h"

#include "InputHandler.h"            // for InputHandler
#include "SnapToGridInputHandler.h"  // for SnapToGridInputHandler

class PositionInputData;
class XournalView;

namespace xoj::util {
template <class T>
class DispatchPool;
}

namespace xoj::view {
class OverlayView;
class Repaintable;
class ShapeToolView;
};

enum DIRSET_MODIFIERS { NONE = 0, SET = 1, SHIFT = 1 << 1, CONTROL = 1 << 2 };


class BaseShapeHandler: public InputHandler {
public:
    BaseShapeHandler(XournalView* xournal, const PageRef& page, bool flipShift = false, bool flipControl = false);

    ~BaseShapeHandler() override;

    void onSequenceCancelEvent() override;
    bool onMotionNotifyEvent(const PositionInputData& pos) override;
    void onButtonReleaseEvent(const PositionInputData& pos) override;
    void onButtonPressEvent(const PositionInputData& pos) override;
    void onButtonDoublePressEvent(const PositionInputData& pos) override;
    bool onKeyEvent(GdkEventKey* event) override;

    std::unique_ptr<xoj::view::OverlayView> createView(const xoj::view::Repaintable* parent) const override;

    const std::shared_ptr<xoj::util::DispatchPool<xoj::view::ShapeToolView>>& getViewPool() const;

    /**
     * @brief Get the shape's points.
     */
    const std::vector<Point>& getShape() const;

private:
    /**
     * @brief Create the shape (to be drawn and added as a stroke), depending on the last event in
     * @return Pair [vector, range] where vector contains the points drawing the shape and range is the smallest range
     * containing all those points. WARNING: Stroke thickness is not taken into account.
     */
    virtual std::pair<std::vector<Point>, Range> createShape(const PositionInputData& pos) = 0;

    /**
     * @brief Update the current shape with the latest event info.
     *      Also warns the listeners about the change, usually triggering a redraw during the next screen update.
     */
    void updateShape(const PositionInputData& pos);

    /**
     * @brief Cancel the current shape creation: clears all data and wipes any drawing made
     */
    void cancelStroke();

protected:
    /**
     * modifyModifiersByDrawDir
     * @brief 	Toggle shift and control modifiers depending on initial drawing direction.
     */
    void modifyModifiersByDrawDir(double width, double height, bool changeCursor = true);

protected:
    std::vector<Point> shape;

    /**
     * @brief Bounding box of the stroke after its last update
     */
    Range lastDrawingRange;

    DIRSET_MODIFIERS drawModifierFixed = NONE;
    bool flipShift =
            false;  // use to reverse Shift key modifier action. i.e.  for separate Rectangle and Square Tool buttons.
    bool flipControl = false;  // use to reverse Control key modifier action.
    bool modShift = false;
    bool modControl = false;
    SnapToGridInputHandler snappingHandler;

    Point currPoint;
    Point buttonDownPoint;  // used for tapSelect and filtering - never snapped to grid.
    Point startPoint;       // May be snapped to grid

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::ShapeToolView>> viewPool;
};