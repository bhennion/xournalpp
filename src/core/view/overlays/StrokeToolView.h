/*
 * Xournal++
 *
 * View active stroke tool - abstract base class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <mutex>
#include <vector>

#include <cairo.h>

#include "util/DispatchPool.h"
#include "view/Mask.h"

#include "BaseStrokeToolView.h"

class StrokeHandler;
class Point;
class Range;
class Stroke;
class OverlayBase;

namespace xoj::view {
class Repaintable;

class StrokeToolView: public BaseStrokeToolView, public xoj::util::Listener<StrokeToolView> {
public:
    StrokeToolView(const StrokeHandler* strokeHandler, const Stroke& stroke, const Repaintable* parent);
    virtual ~StrokeToolView() noexcept;

    bool isViewOf(const OverlayBase* overlay) const override;

    void draw(cairo_t* cr) const override;

    /**
     * Listener interface
     */
    static constexpr struct AddPointRequest {
    } ADD_POINT_REQUEST = {};
    virtual void on(AddPointRequest, const Point& p);

    static constexpr struct ThickenFirstPointRequest {
    } THICKEN_FIRST_POINT_REQUEST = {};
    void on(ThickenFirstPointRequest, double newPressure);

    static constexpr struct CancellationRequest {
    } CANCELLATION_REQUEST = {};
    void on(CancellationRequest, const Range& rg);

protected:
    /**
     * @brief Compute the bounding box of the given segment, taking stroke width into account.
     */
    auto getRepaintRange(const Point& lastPoint, const Point& addedPoint) const -> Range;

    void drawDot(cairo_t* cr, const Point& p) const;

    /**
     * @brief (Thread-safe) Flush the communication buffer and returns its content.
     */
    std::vector<Point> flushBuffer() const;

    // Nothing in the base class
    virtual void drawFilling(cairo_t*, const std::vector<Point>&) const {}

protected:
    const StrokeHandler* strokeHandler;

protected:
    bool singleDot = true;

    /**
     * @brief offset for drawing dashes (if any)
     *      In effect, this stores the length of the path already drawn to the mask.
     */
    mutable double dashOffset = 0;

    /**
     * @brief Thread communication buffer and its mutex
     */
    mutable std::vector<Point> pointBuffer;  // Todo: implement a lock-free fifo?
    mutable std::mutex bufferMutex;          //

    /**
     * @brief Drawing mask.
     *
     * The stroke is drawn to the mask and the mask is then blitted wherever needed.
     * Upon calls to draw(), the buffer is flushed and the corresponding part of stroke is added to the mask.
     *
     * WARNING: The mask is not mutex-protected. It can only be used in the drawing thread (i.e. in draw())
     *      (or in constructors...)
     */
    mutable Mask mask;
};
};  // namespace xoj::view
