/*
 * Xournal++
 *
 * Handles input to draw a spline consisting of linear and cubic spline segments
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>    // for unique_ptr
#include <mutex>     // for mutex
#include <optional>  // for optional
#include <vector>    // for vector

#include <gdk/gdk.h>  // for GdkEventKey

#include "model/PageRef.h"   // for PageRef
#include "model/Point.h"     // for Point
#include "util/Range.h"      // for Range

#include "InputHandler.h"            // for InputHandler
#include "SnapToGridInputHandler.h"  // for SnapToGridInputHandler

class PositionInputData;
class XournalView;

namespace xoj::util {
template <class T>
class DispatchPool;
};

namespace xoj::view {
class OverlayView;
class Repaintable;
class SplineToolView;
};  // namespace xoj::view

/**
 * @brief A class to handle splines
 *
 * Drawing of a spline is started by a ButtonPressEvent. Every ButtonPressEvent gives a new knot.
 * Click-dragging will set the tangents. After a ButtonReleaseEvent the spline segment is dynamically
 * drawn and finished after the next ButtonPressEvent.
 * The spline is completed through a ButtonDoublePressEvent, the escape key or a
 * ButtonPressEvent near the first knot. The latter event closes the spline.
 *
 * Splines segments can be linear or cubic (as in Inkscape). Where there is a nontrivial tangent,
 * the join is smooth.
 * The last knot and tangent can be modified using the keyboard.
 */

class SplineHandler: public InputHandler {
public:
    SplineHandler(XournalView* xournal, const PageRef& page);
    ~SplineHandler() override;

    std::unique_ptr<xoj::view::OverlayView> createView(const xoj::view::Repaintable* parent) const override;

    void onSequenceCancelEvent() override;
    bool onMotionNotifyEvent(const PositionInputData& pos) override;
    void onButtonReleaseEvent(const PositionInputData& pos) override;
    void onButtonPressEvent(const PositionInputData& pos) override;
    void onButtonDoublePressEvent(const PositionInputData& pos) override;
    bool onKeyEvent(GdkEventKey* event) override;

    void finalizeSpline();

public:
    const std::shared_ptr<xoj::util::DispatchPool<xoj::view::SplineToolView>>& getViewPool() const;

    struct Data {
        std::vector<Point> knots;
        std::vector<Point> tangents;
        Point currPoint;
        bool closedSpline;
    };
    std::optional<Data> getDataClone() const;

    static auto linearizeSpline(const Data& data) -> std::vector<Point>;

    Range computeTotalRepaintRange(const Data& data, double strokeWidth) const;

    Range computeLastSegmentRepaintRange(std::unique_lock<std::mutex>& lock) const;

private:
    void addKnot(const Point& p);
    void addKnotWithTangent(const Point& p, const Point& t);
    void modifyLastTangent(const Point& t);
    void deleteLastKnotWithTangent();
    void movePoint(double dx, double dy);

private:
    std::vector<Point> knots{};
    std::vector<Point> tangents{};
    Point currPoint;
    Point buttonDownPoint;  // used for tapSelect and filtering - never snapped to grid. See startPoint defined in
                            // derived classes such as EllipseHandler.
    mutable std::mutex dataMutex;  // Protects knots, tangents, currPoint and buttonDownPoint

    bool isButtonPressed = false;
    SnapToGridInputHandler snappingHandler;

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::SplineToolView>> viewPool;
};
