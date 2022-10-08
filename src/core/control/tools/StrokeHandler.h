/*
 * Xournal++
 *
 * Handles input of strokes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>
#include <memory>    // for unique_ptr
#include <optional>  // for optional

#include <cairo.h>    // for cairo_t, cairo_surface_t
#include <gdk/gdk.h>  // for GdkEventKey
#include <glib.h>     // for guint32

#include "model/PageRef.h"  // for PageRef
#include "model/Point.h"    // for Point
#include "view/StrokeView.h"

#include "InputHandler.h"            // for InputHandler
#include "SnapToGridInputHandler.h"  // for SnapToGridInputHandler

#define DEBUG_FPS
#ifdef DEBUG_FPS
#include <chrono>
#endif

class Layer;
class PositionInputData;
class Stroke;
class XojPageView;
class XournalView;

class Path;
class Spline;
class PiecewiseLinearPath;

namespace SplineApproximator {
class Live;
}

namespace StrokeStabilizer {
class Base;
class Active;
class Deadzone;
class Inertia;
}  // namespace StrokeStabilizer

/**
 * @brief The stroke handler draws a stroke on a XojPageView
 *
 * The stroke is drawn using a cairo_surface_t* as a mask:
 * As the pointer moves on the canvas single segments are
 * drawn opaquely on the initially transparent masking
 * surface. The surface is used to mask the stroke
 * when drawing it to the XojPageView
 */
class StrokeHandler: public InputHandler {
public:
    StrokeHandler(Control* control, XojPageView* pageView, const PageRef& page);
    ~StrokeHandler() override = default;

    void draw(cairo_t* cr) override;

    void onSequenceCancelEvent() override;
    bool onMotionNotifyEvent(const PositionInputData& pos, double zoom) override;
    void onButtonReleaseEvent(const PositionInputData& pos, double zoom) override;
    void onButtonPressEvent(const PositionInputData& pos, double zoom) override;
    void onButtonDoublePressEvent(const PositionInputData& pos, double zoom) override;
    bool onKeyEvent(GdkEventKey* event) override;

    /**
     * @brief Add a straight line to the stroke (if the movement is valid).
     * The line may be subdivided into smaller segments if the pressure variation is too big.
     * @param point The endpoint of the added line
     */
    void paintTo(Point point);

    /**
     * @brief paints a single dot
     */
    void paintDot(cairo_t* cr, const double x, const double y, const double width) const;

protected:
    /**
     * @brief Unconditionally add a segment to the stroke.
     * Warning: it does not set the width properly nor test if the motion is valid. Use paintTo instead.
     * @param point The endpoint of the added segment
     */
    inline void drawSegmentTo(const Point& point) {
#ifdef DEBUG_FPS
        // Debug info. Remove
        ptCnt++;
        if (secondPassed()) {
            g_message("Points: %4zu  Frames : %2zu", ptCnt, frameCnt);
            ptCnt = 0;
            frameCnt = 0;
        }  //
#endif
        this->drawEvent(point);
    };

    void strokeRecognizerDetected(std::shared_ptr<Path> result, Layer* layer);

protected:
    Point buttonDownPoint;  // used for tapSelect and filtering - never snapped to grid.
    SnapToGridInputHandler snappingHandler;

private:
    std::shared_ptr<PiecewiseLinearPath> path;

    // Helper class to safely handle cairo surface and context
    class Mask {
    public:
        Mask() = delete;
        Mask(const Mask&) = delete;
        Mask(Mask&&) = delete;
        Mask& operator=(const Mask&) = delete;
        Mask& operator=(Mask&&) = delete;

        Mask(int width, int height);
        ~Mask() noexcept;

        /**
         * The masking surface
         */
        cairo_surface_t* surf = nullptr;

        /**
         * And the corresponding cairo_t*
         */
        cairo_t* cr = nullptr;
    };

    /**
     * @brief Create and initialize the mask
     * The mask is used for strokes that do not require a full redraw at each input event.
     * For those strokes, whenever a new input event is received, the new segment is simply added to the mask.
     * The mask is then blitted upon a call to `draw`.
     *
     * A stroke requires a full redraw if
     *      * it has a filling (the filling can not be computed simply from just the last segment)
     *      * or it has dashes (to get the dash offset right)
     *
     * Nb: the dashed exception could be avoided if we recorded the dash offset (= the stroke's length so far)
     */
    void createMask(std::optional<Mask>& m);

    std::optional<Mask> mask;
    std::optional<Mask> maskForLiveSegment;


    std::optional<xoj::view::StrokeView> strokeView;

    // to filter out short strokes (usually the user tapping on the page to select it)
    guint32 startStrokeTime{};
    static guint32 lastStrokeTime;  // persist across strokes - allow us to not ignore persistent dotting.

    /**
     * @brief Pointer to the Stabilizer instance
     */
    std::unique_ptr<StrokeStabilizer::Base> stabilizer;

    bool hasPressure;

    bool splineLiveApproximation;

    friend class StrokeStabilizer::Active;
    friend class StrokeStabilizer::Deadzone;
    friend class StrokeStabilizer::Inertia;

    static constexpr double MAX_WIDTH_VARIATION = 0.3;

    XojPageView* pageView;

    std::function<void(const Point&)> drawEvent;

    void fullRedraw(const Point& p);
    void fullRedrawLiveApproximator(const Point& p);
    void normalDraw(const Point& p);
    void normalDrawLiveApproximator(const Point& p);

    std::shared_ptr<Spline> approximatedSpline;
    std::unique_ptr<SplineApproximator::Live> liveApprox;
    std::unique_ptr<Stroke> liveSegmentStroke;
    size_t liveSegmentPointCacheBegin = 0;

#ifdef DEBUG_FPS
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    bool secondPassed() {
        auto t2 = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(t2 - t1) >= std::chrono::seconds(1)) {
            t1 = t2;
            return true;
        }
        return false;
    }
    size_t frameCnt = 0;
    size_t ptCnt = 0;
#endif
};
