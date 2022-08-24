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

#include <functional>  // for reference_wrapper
#include <string>      // for string
#include <vector>      // for vector

#include <gdk/gdk.h>           // for GdkEventKey
#include <glib.h>              // for gint, gboolean, gchar
#include <gtk/gtk.h>           // for GtkIMContext, GtkTextIter, GtkWidget
#include <pango/pangocairo.h>  // for cairo_t, PangoAttrList, PangoLayout

#include "control/tools/TextEditorBuffer.h"
#include "util/Color.h"  // for Color
#include "util/Range.h"
#include "util/raii/GObjectSPtr.h"
#include "util/raii/PangoSPtr.h"

class XojPageView;
class Text;
class TextUndoAction;
class UndoAction;
class XojFont;

namespace xoj::util {
template <class T>
class Rectangle;
};

class TextEditor {
public:
    TextEditor(XojPageView* gui, GtkWidget* widget, Text* text, bool ownText);
    virtual ~TextEditor();

    /** Represents the different kinds of text selection */
    enum class SelectType { WORD, LINE, ALL };

    void paint(cairo_t* cr, double zoom);

    bool onKeyPressEvent(GdkEventKey* event);
    bool onKeyReleaseEvent(GdkEventKey* event);

    void toggleOverwrite();
    void selectAtCursor(TextEditor::SelectType ty);
    void toggleBoldFace();
    void increaseFontSize();
    void decreaseFontSize();
    void moveCursor(GtkMovementStep step, int count, bool extendSelection);
    void deleteFromCursor(GtkDeleteType type, int count);
    void backspace();
    void copyToCliboard() const;
    void cutToClipboard();
    void pasteFromClipboard();

    Text* getText() const;
    void textCopyed();

    void mousePressed(double x, double y);
    void mouseMoved(double x, double y);
    void mouseReleased();

    UndoAction* getFirstUndoAction() const;

    void replaceBufferContent(const std::string& text);
    void setFont(XojFont font);
    UndoAction* setColor(Color color);

private:
    void repaintEditor(bool sizeChanged = true);

    /**
     * @brief Draws the cursor
     * @return The bounding box of the cursor, in TextBox coordinates (i.e relative to the text box upper left corner)
     *          The bounding box is returned even if the cursor is currently not visible (blinking...)
     */
    xoj::util::Rectangle<double> drawCursor(cairo_t* cr, double zoom) const;

    void repaintCursor();
    void resetImContext();

    static void bufferPasteDoneCallback(GtkTextBuffer* buffer, GtkClipboard* clipboard, TextEditor* te);

    static void iMCommitCallback(GtkIMContext* context, const gchar* str, TextEditor* te);
    static void iMPreeditChangedCallback(GtkIMContext* context, TextEditor* te);
    static bool iMRetrieveSurroundingCallback(GtkIMContext* context, TextEditor* te);
    static bool imDeleteSurroundingCallback(GtkIMContext* context, gint offset, gint n_chars, TextEditor* te);

    void moveCursor(const GtkTextIter* newLocation, gboolean extendSelection);

    static gint blinkCallback(TextEditor* te);

    void contentsChanged(bool forceCreateUndoAction = false);

private:
    XojPageView* gui;
    GtkWidget* xournalWidget;
    Text* text;
    std::string lastText;

    xoj::util::GSPtr<GtkWidget> textWidget;
    xoj::util::GSPtr<GtkIMContext> imContext;
    
    TextEditorBuffer buffer;

    std::vector<std::reference_wrapper<TextUndoAction>> undoActions;

    /**
     * @brief Tracks the bounding box of the editor from the last render.
     *
     * Because adding or deleting lines may cause the size of the bounding box to change,
     * we need to repaint the union of the current and previous bboxes.
     */
    Range previousBoundingBox;

    // cursor blinking timings. In millisecond.
    unsigned int cursorBlinkingTimeOn = 0;
    unsigned int cursorBlinkingTimeOff = 0;
    // unsigned int cursorBlinkTimeout = 0;  // Not used for now...
    unsigned int blinkCallbackId = 0;  // handler id
    bool cursorBlink = true;

    bool ownText = false;
    bool needImReset = false;
    bool mouseDown = false;
    bool cursorOverwrite = false;
    bool cursorVisible = false;

    // Padding between the text logical box and the frame
    static constexpr int PADDING_IN_PIXELS = 5;
    // Width of the lines making the frame
    static constexpr int BORDER_WIDTH_IN_PIXELS = 1;
    // In a blinking period, how much time is the cursor visible vs not visible
    static constexpr unsigned int CURSOR_ON_MULTIPLIER = 2;
    static constexpr unsigned int CURSOR_OFF_MULTIPLIER = 1;
    static constexpr unsigned int CURSOR_DIVIDER = CURSOR_ON_MULTIPLIER + CURSOR_OFF_MULTIPLIER;
};
