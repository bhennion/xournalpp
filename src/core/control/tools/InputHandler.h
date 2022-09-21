/*
 * Xournal++
 *
 * Handles input for a tool
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gdk/gdk.h>  // for GdkEventKey

#include "model/OverlayBase.h"
#include "model/PageRef.h"  // for PageRef

class Control;
class PositionInputData;
class Element;

namespace xoj::view {
class OverlayView;
class Repaintable;
};  // namespace xoj::view

/**
 * @brief A base class to handle input
 *
 * The InputHandler receives various events from a XojPageView
 * and updates the XojPageView to display elements being created
 */
class InputHandler: public OverlayBase {
public:
    InputHandler(Control* control, const PageRef& page);
    virtual ~InputHandler();

public:
    /**
     * This method is called from the XojPageView as soon
     * as the pointer is moved while this InputHandler
     * is active. It is used to update internal data
     * structures and queue repaints of the XojPageView
     * if necessary
     */
    virtual void onMotionNotifyEvent(const PositionInputData& pos, double zoom) = 0;

    /**
     * This method is called from the XojPageView when a keypress is detected.
     * It is used to update internal data structures and queue
     * repaints of the XojPageView if necessary.
     */
    virtual bool onKeyPressEvent(GdkEventKey* event) = 0;

    /**
     * This method is called from the XojPageView when a keyrelease is detected.
     * It is used to update internal data structures and queue
     * repaints of the XojPageView if necessary.
     */
    virtual bool onKeyReleaseEvent(GdkEventKey* event) = 0;

    /**
     * The current input device for stroken, do not react on other devices (linke mices)
     * This method is called from the XojPageView as soon
     * as the pointer is released.
     */
    virtual void onButtonReleaseEvent(const PositionInputData& pos, double zoom) = 0;

    /**
     * This method is called from the XojPageView as soon
     * as the pointer is pressed.
     */
    virtual bool onButtonPressEvent(const PositionInputData& pos, double zoom) = 0;

    /**
     * This method is called from the XojPageView as soon
     * as the pointer is pressed a second time.
     */
    virtual bool onButtonDoublePressEvent(const PositionInputData& pos, double zoom) { return false; }

    virtual bool onButtonTriplePressEvent(const PositionInputData& pos, double zoom) { return false; }

    /**
     * This method is called when an action taken by the pointer is canceled.
     * It is used, for instance, to cancel a stroke drawn when a user starts
     * to zoom on a touchscreen device.
     */
    virtual void onSequenceCancelEvent() = 0;

    virtual std::unique_ptr<xoj::view::OverlayView> createView(xoj::view::Repaintable* parent) const = 0;

    virtual bool handlesElement(const Element* e) const = 0;

    bool isReadyForDeletion() const;

protected:
    Control* control;
    PageRef page;
    bool readyForDeletion = false;
};
