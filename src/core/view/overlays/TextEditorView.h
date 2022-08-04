/*
 * Xournal++
 *
 * Text editor gui (for Text Tool)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>      // for string

#include <pango/pangocairo.h>  // for cairo_t, PangoAttrList, PangoLayout

#include "util/Color.h"  // for Color
#include "util/DispatchPool.h"
#include "util/raii/GObjectSPtr.h"
#include "util/raii/PangoSPtr.h"

#include "OverlayView.h"

class Text;

namespace xoj::view {
class TextEditorView: public OverlayView, public xoj::util::Listener<TextEditorView>{
public:
    TextEditorView(XojPageView* gui, GtkWidget* widget, Text* text, bool ownText);
    virtual ~TextEditorView();

    /**
     * @brief Draws the overlay to the given context
     */
    virtual void draw(cairo_t* cr) const override;
    
    virtual bool isViewOf(const OverlayBase* overlay) const override;

private:

    void repaintEditor(bool sizeChanged = true);
    void drawCursor(cairo_t* cr, double x, double y, double height, double zoom) const;
    void repaintCursor();

    static gint blinkCallback(TextEditor* te);

private:

    // cursor blinking timings. In millisecond.
    unsigned int cursorBlinkingTimeOn = 0;
    unsigned int cursorBlinkingTimeOff = 0;
    // unsigned int cursorBlinkTimeout = 0;  // Not used for now...
    unsigned int blinkCallbackId = 0;  // handler id
    bool cursorBlink = true;

    // Padding between the text logical box and the frame
    static constexpr int PADDING_IN_PIXELS = 5;
    // Width of the lines making the frame
    static constexpr int BORDER_WIDTH_IN_PIXELS = 1;
    // In a blinking period, how much time is the cursor visible vs not visible
    static constexpr unsigned int CURSOR_ON_MULTIPLIER = 2;
    static constexpr unsigned int CURSOR_OFF_MULTIPLIER = 1;
    static constexpr unsigned int CURSOR_DIVIDER = CURSOR_ON_MULTIPLIER + CURSOR_OFF_MULTIPLIER;
};
};
