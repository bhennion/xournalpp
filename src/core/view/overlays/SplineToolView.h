/*
 * Xournal++
 *
 * View active spline tool
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cairo.h>

#include "util/Color.h"
#include "util/DispatchPool.h"

#include "BaseShapeOrSplineToolView.h"

class SplineHandler;
class OverlayBase;
class Range;

namespace xoj::view {
class Repaintable;

class SplineToolView: public BaseShapeOrSplineToolView, public xoj::util::Listener<SplineToolView> {

public:
    SplineToolView(const SplineHandler* splineHandler, const Repaintable* parent);
    virtual ~SplineToolView() noexcept;

    /**
     * @brief Draws the overlay to the given context
     */
    void draw(cairo_t* cr) const override;

    bool isViewOf(const OverlayBase* overlay) const override;

    /**
     * Listener interface
     */
    static constexpr struct PaintRequest {
    } PAINT_REQUEST = {};
    void on(PaintRequest, const Range& rg);

private:
    const SplineHandler* splineHandler;

public:
    static constexpr double RADIUS_WITHOUT_ZOOM = 10.0;                   // for circling the spline's knots
    static constexpr double LINE_WIDTH_WITHOUT_ZOOM = 2.0;                // for tangent vectors & dynamic spline seg.
    static constexpr Color NODE_CIRCLE_COLOR = Color(0xff505050U);        // gray
    static constexpr Color FIRST_NODE_CIRCLE_COLOR = Color(0xff0000ffU);  // red
    static constexpr Color TANGENT_VECTOR_COLOR = Color(0xff00c000U);     // green
    static constexpr Color DYNAMIC_OBJECTS_COLOR = Color(0xff505050U);    // gray
};
};  // namespace xoj::view
