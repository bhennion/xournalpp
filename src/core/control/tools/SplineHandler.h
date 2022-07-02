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

#include <vector>  // for vector

#include <cairo.h>    // for cairo_t
#include <gdk/gdk.h>  // for GdkEventKey
#include <glib.h>     // for guint32

#include "model/PageRef.h"   // for PageRef
#include "model/Point.h"     // for Point
#include "util/Rectangle.h"  // for Rectangle

#include "InputHandler.h"            // for InputHandler
#include "SnapToGridInputHandler.h"  // for SnapToGridInputHandler

class PositionInputData;
class XojPageView;
class XournalView;

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
    SplineHandler(XournalView* xournal, const std::shared_ptr<xoj::view::PageViewPool>& pool, const PageRef& page);
    ~SplineHandler() override;

    void draw(cairo_t* cr) override;

    void onMotionCancelEvent() override;
    bool onMotionNotifyEvent(const PositionInputData& pos) override;
    void onButtonReleaseEvent(const PositionInputData& pos) override;
    void onButtonPressEvent(const PositionInputData& pos) override;
    void onButtonDoublePressEvent(const PositionInputData& pos) override;
    bool onKeyEvent(GdkEventKey* event) override;

private:
    void finalizeSpline();
    void movePoint(double dx, double dy);
    void updateStroke();
    xoj::util::Rectangle<double> computeRepaintRectangle() const;

    // to filter out short strokes (usually the user tapping on the page to select it)
    guint32 startStrokeTime{};
    static guint32 lastStrokeTime;  // persist across strokes - allow us to not ignore persistent dotting.


private:
    std::vector<Point> knots{};
    std::vector<Point> tangents{};
    bool isButtonPressed = false;
    SnapToGridInputHandler snappingHandler;

public:
    void addKnot(const Point& p);
    void addKnotWithTangent(const Point& p, const Point& t);
    void modifyLastTangent(const Point& t);
    void deleteLastKnotWithTangent();
    int getKnotCount() const;

protected:
    Point currPoint;
    Point buttonDownPoint;  // used for tapSelect and filtering - never snapped to grid. See startPoint defined in
                            // derived classes such as EllipseHandler.
};
