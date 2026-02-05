/*
 * Xournal++
 *
 * Controls the zoom level
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <vector>   // for vector

#include <gdk/gdk.h>  // for GdkEvent, GdkEventScroll, GdkEve...
#include <gtk/gtk.h>  // for GtkWidget

#include "model/DocumentListener.h"  // for DocumentListener
#include "util/Point.h"              // for Point
#include "util/Rectangle.h"          // for Rectangle
#include "model/PageRef.h"                // for PageRef
#include "pdf/base/XojPdfPage.h"          // for XojPdfRectangle

class LinkDestination;


constexpr auto DEFAULT_ZOOM_MAX{7};
constexpr auto DEFAULT_ZOOM_MIN{0.3};
constexpr auto DEFAULT_ZOOM_STEP{0.1};
constexpr auto DEFAULT_ZOOM_STEP_SCROLL{0.01};

enum class ZoomDirection { IN, OUT };
enum class NavMode { FREE, FIT_PAGE_WIDTH, PRESENTATION };
enum class ZoomStep { TINY, SMALL, NORMAL };

class XournalView;
class Control;
class ZoomListener;

class NavControl: public DocumentListener {
public:
    NavControl() = default;
    ~NavControl() override = default;

    void startTouchScreenTwoPointsSequence(xoj::util::Point<double> finger1, xoj::util::Point<double> finger2);
    void updateTouchScreenTwoPointsSequence(xoj::util::Point<double> finger1, xoj::util::Point<double> finger2);
    void cancelTouchScreenTwoPointsSequence();
    void finishTouchScreenTwoPointsSequence();

    void startTouchScreenSinglePointSequence(xoj::util::Point<double> finger);
    void updateTouchScreenSinglePointSequence(xoj::util::Point<double> finger);
    void cancelTouchScreenSinglePointSequence();
    void finishTouchScreenSinglePointSequence();

    void zoomOneStep(ZoomDirection direction, ZoomStep step, xoj::util::Point<double> zoomCenter);
    void zoomOneStep(ZoomDirection direction, ZoomStep step);  ///< Centered at the center of the visible area

    void setNavigationMode(NavMode mode);
    NavMode getNavigationMode() const;


    void goToPreviousPage();
    void goToNextPage();
    void goToLastPage();
    void goToFirstPage();
    void goToAnnotatedPage(bool next);

    void goToPage(const PageRef& page);
    void goToPage(size_t page);

    void goToPDFDestination(size_t page, XojPdfRectangle rect);


    /**
     * Scroll to a given link's destination, provided the
     * destination is a local destination and not a URI.
     *
     *  If the destination is a non-existent PDF page,
     * we ask the user whether to add the missing page or not.
     *
     * @param dest is to shown
     */
    void scrollToLinkDest(const LinkDestination& dest);

    bool isPageVisible(size_t page, int* visibleHeight = nullptr);

    /**
     * Zoom so that the displayed page on the screen has the same size as the real size
     * The dpi has to be set correctly
     */
    void zoom100();

    /**
     * @return zoom value depending zoom100Value
     */
    double getZoom() const;

    /**
     * @return real zoom value in percent
     */
    double getZoomReal() const;

    /**
     * Set the current zoom, does not preserve the current page position.
     * Use startZoomSequence() / zoomSequnceChange() / endZoomSequence() to preserve position
     * e.g. use zoomOneStep function
     *
     * @param zoomI zoom value depending zoom100Value
     */
    void setZoom(double zoomI);

    /**
     * Updates the when dpi is changed.
     * updates zoomMax, zoomMin, zoomStepBig, zoomStepScroll
     *
     * @param zoom100Val zoom value depending zoom100Value
     */
    void setZoom100Value(double zoom100Val);

    /**
     * @return zoom value for zoom 100% depending zoom100Value
     */
    double getZoom100Value() const;

    /**
     * Updates when, the window size changes
     * @param zoom zoom value depending zoom100Value
     */
    bool updateZoomFitValue(size_t pageNo = 0);

    /**
     * @return zoom value for zoom fit depending zoom100Value
     */
    double getZoomFitValue() const;

    bool updateZoomPresentationValue(size_t pageNo = 0);

    void addZoomListener(ZoomListener* listener);
    void removeZoomListener(ZoomListener* listener);

    void initZoomHandler(GtkWidget* window, GtkWidget* widget, XournalView* v, Control* c);

    /**
     * Call this before any zoom is done, it saves the current page and position
     *
     * @param zoomCenter position of zoom focus in widget pixel coordinates. That
     * is, absolute coordinates (not scaling with the document) translated to the
     * top left corner of the drawing area.
     */
    void startZoomSequence(xoj::util::Point<double> zoomCenter);

    /**
     * Call this before any zoom is done, it saves the current page and position
     * zooms to the center of the visible rect
     */
    void startZoomSequence();

    /**
     * Change the zoom within a Zoom sequence (startZoomSequence() / endZoomSequence())
     *
     * @param zoom Current zoom value
     * @param relative If the zoom is relative to the start value (for Gesture)
     */
    void zoomSequenceChange(double zoom, bool relative);

    /**
     * Change the zoom and zoomCenter within a Zoom sequence (startZoomSequence() / endZoomSequence())
     * Used while touch pinch event
     *
     * @param zoom Current zoom value
     * @param relative If the zoom is relative to the start value (for Gesture)
     * @param scrollVector relative zoom center movement in pixels since last call
     */
    void zoomSequenceChange(double zoom, bool relative, xoj::util::Point<double> scrollVector);

    /// Clear all stored data from startZoomSequence()
    void endZoomSequence();

    /// Revert and end the current zoom sequence
    void cancelZoomSequence();

    /// Check if we are between calls to `startZoomSequence()` and `endZoomSequence()`
    bool isZoomSequenceActive() const;

    /**
     * Zoom to correct position on zooming.
     * This function should only be called during a zoom sequence.
     */
    xoj::util::Point<double> getScrollPositionAfterZoom() const;

    /// Get visible rect on xournal view, for Zoom Gesture
    xoj::util::Rectangle<double> getVisibleRect();

    void setZoomStep(double zoomStep);

    void setZoomStepScroll(double zoomStep);

protected:
    void fireZoomChanged();
    void fireZoomRangeValueChanged();

    void pageSizeChanged(size_t page) override;
    void pageSelected(size_t page) override;

private:
    void zoomFit();
    void zoomPresentation();

    /**
     * Get this->zoom changed by a step in the given direction.
     *
     * @param direction Direction to change the zoom (step direction).
     * @param stepSize Size of the step to take
     * @return The new zoom if a step is taken in the given direction.
     */
    double withZoomStep(ZoomDirection direction, double stepSize) const;

    friend bool onWindowSizeChangedEvent(GtkWidget* widget, GdkEvent* event, NavControl* zoom);
    friend bool onScrolledwindowMainScrollEvent(GtkWidget* widget, GdkEventScroll* event, NavControl* zoom);
    friend bool onTouchpadPinchEvent(GtkWidget* widget, GdkEventTouchpadPinch* event, NavControl* zoom);

private:
    xoj::util::Rectangle<int> visibleArea;  ///< Area of main widget currently visible - in widget coordinates
    xoj::util::Rectangle<int> visibleAreaBeforeInput;  ///< Area of main widget visible when touch input started

    XournalView* view = nullptr;
    Control* control = nullptr;
    std::vector<ZoomListener*> listener;

    /**
     * current Zoom value
     * depends dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
     */
    double zoom = 1.0;
    bool zoomFitMode = false;
    bool zoomPresentationMode = false;

    /// Zoom value for 100% depends on the dpi
    double zoom100Value = 1.0;
    double zoomFitValue = 1.0;
    double zoomPresentationValue = 1.0;

    /// Base zoom on start, for relative zoom (Gesture)
    double zoomSequenceStart = -1;

    /// Zoom center position in widget coordinate space, will not be zoomed!
    xoj::util::Point<double> zoomWidgetPos;

    /// Scroll position (top left corner of view) to scale
    xoj::util::Point<double> scrollPosition;

    /// Size {x, y} of the pixels before the current page that
    /// do not scale.
    xoj::util::Point<double> unscaledPixels;

    /**
     * Zoomstep value for Ctrl - and Zoom In and Out Button
     * depends on dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
     */
    double zoomStep = DEFAULT_ZOOM_STEP;

    /**
     * Zoomstep value for Ctrl-Scroll zooming
     * depends on dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
     */
    double zoomStepScroll = DEFAULT_ZOOM_STEP_SCROLL;

    /**
     * Zoom maximal value
     * depends on dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
     */
    double zoomMax = DEFAULT_ZOOM_MAX;

    /**
     * Zoom mininmal value
     * depends on dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
     */
    double zoomMin = DEFAULT_ZOOM_MIN;

    size_t current_page = static_cast<size_t>(-1);
    size_t last_page = static_cast<size_t>(-1);
    bool isZoomFittingNow = false;
};
