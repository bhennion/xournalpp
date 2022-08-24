#include "TextEditor.h"

#include <cassert>
#include <cmath>
#include <cstring>  // for strcmp, size_t
#include <memory>   // for allocator, make_unique, __shared_p...
#include <utility>  // for move

#include <gdk/gdkkeysyms.h>  // for GDK_KEY_B, GDK_KEY_ISO_Enter, GDK_...
#include <glib-object.h>     // for g_object_get, g_object_unref, G_CA...

#include "control/Control.h"       // for Control
#include "model/Font.h"            // for XojFont
#include "model/PageRef.h"         // for PageRef
#include "model/Text.h"            // for Text
#include "model/XojPage.h"         // for XojPage
#include "undo/ColorUndoAction.h"  // for ColorUndoAction
#include "undo/TextUndoAction.h"   // for TextUndoAction
#include "undo/UndoRedoHandler.h"  // for UndoRedoHandler
#include "util/Range.h"
#include "util/Rectangle.h"

#include "PageView.h"          // for XojPageView
#include "TextEditorWidget.h"  // for gtk_xoj_int_txt_new
#include "XournalView.h"       // for XournalView
#include "XournalppCursor.h"   // for XournalppCursor

class UndoAction;

TextEditor::TextEditor(XojPageView* gui, GtkWidget* widget, Text* text, bool ownText):
        gui(gui),
        xournalWidget(widget),
        text(text),
        lastText(text->getText()),
        textWidget(gtk_xoj_int_txt_new(this)),
        imContext(gtk_im_multicontext_new()),
        buffer(*text),
        previousBoundingBox(text->boundingRect()),
        ownText(ownText) {
    this->text->setInEditing(true);

    this->replaceBufferContent(this->lastText);

    {  // Get cursor blinking settings
        GtkSettings* settings = gtk_widget_get_settings(this->xournalWidget);
        g_object_get(settings, "gtk-cursor-blink", &this->cursorBlink, nullptr);
        if (this->cursorBlink) {
            int tmp = 0;
            g_object_get(settings, "gtk-cursor-blink-time", &tmp, nullptr);
            assert(tmp >= 0);
            auto cursorBlinkingPeriod = static_cast<unsigned int>(tmp);
            this->cursorBlinkingTimeOn = cursorBlinkingPeriod * CURSOR_ON_MULTIPLIER / CURSOR_DIVIDER;
            this->cursorBlinkingTimeOff = cursorBlinkingPeriod - this->cursorBlinkingTimeOn;
            /*
             * The blink timeout is ignored. To remedy this, restore the following snippet and use cursorBlinkTimeout!
             *
             * g_object_get(settings, "gtk-cursor-blink-timeout", &tmp, nullptr);
             * assert(tmp >= 0);
             * this->cursorBlinkTimeout = static_cast<unsigned int>(tmp);
             */
        }
    }

    gtk_im_context_set_client_window(this->imContext.get(), gtk_widget_get_parent_window(this->xournalWidget));
    gtk_im_context_focus_in(this->imContext.get());

    g_signal_connect(this->imContext.get(), "commit", G_CALLBACK(iMCommitCallback), this);
    g_signal_connect(this->imContext.get(), "preedit-changed", G_CALLBACK(iMPreeditChangedCallback), this);
    g_signal_connect(this->imContext.get(), "retrieve-surrounding", G_CALLBACK(iMRetrieveSurroundingCallback), this);
    g_signal_connect(this->imContext.get(), "delete-surrounding", G_CALLBACK(imDeleteSurroundingCallback), this);

    if (this->cursorBlink) {
        blinkCallback(this);
    } else {
        this->cursorVisible = true;
    }
}

TextEditor::~TextEditor() {
    this->text->setInEditing(false);
    this->xournalWidget = nullptr;

    Control* control = gui->getXournal()->getControl();
    control->setCopyPasteEnabled(false);

    this->contentsChanged(true);

    if (this->ownText) {
        UndoRedoHandler* handler = gui->getXournal()->getControl()->getUndoRedoHandler();
        for (TextUndoAction& undo: this->undoActions) { handler->removeUndoAction(&undo); }
    } else {
        for (TextUndoAction& undo: this->undoActions) { undo.textEditFinished(); }
    }
    this->undoActions.clear();

    if (this->ownText) {
        delete this->text;
    }
    this->text = nullptr;

    if (this->blinkCallbackId) {
        g_source_remove(this->blinkCallbackId);
    }
}

auto TextEditor::getText() const -> Text* {
    this->text->setText(buffer.getContent());
    return this->text;
}

void TextEditor::replaceBufferContent(const std::string& text) {
    buffer.replaceContent(text);
}

auto TextEditor::setColor(Color color) -> UndoAction* {
    auto origColor = this->text->getColor();
    this->text->setColor(color);

    repaintEditor(false);

    if (this->ownText) {
        // This is a new text, so we don't need to create a undo action
        return nullptr;
    }

    auto* undo = new ColorUndoAction(gui->getPage(), gui->getPage()->getSelectedLayer());
    undo->addStroke(this->text, origColor, color);

    return undo;
}

void TextEditor::setFont(XojFont font) {
    this->text->setFont(font);
    this->buffer.updateFont(*this->text);
    this->repaintEditor();
}

void TextEditor::textCopyed() { this->ownText = false; }

void TextEditor::iMCommitCallback(GtkIMContext* context, const gchar* str, TextEditor* te) {

    bool had_selection = te->buffer.clearSelection();
    
    std::string_view strv(str);

    if (strv == "\n") {
        te->buffer.insert('\n');
        te->contentsChanged(true);
    } else {
        if (!had_selection && te->cursorOverwrite) {
            te->buffer.overwriteNextGraphemeWith(strv);
        } else {
            te->buffer.insert(strv);
        }
        te->contentsChanged();
    }
    te->repaintEditor();
}

void TextEditor::iMPreeditChangedCallback(GtkIMContext* context, TextEditor* te) {
    gchar* str = nullptr;
    gint cursorPos = 0;
    PangoAttrList* attrs = nullptr;
    gtk_im_context_get_preedit_string(context, &str, &attrs, &cursorPos);
    te->buffer.setPreeditData(str, attrs, static_cast<TextEditorBuffer::Mark::index_type>(cursorPos));

    te->contentsChanged();
    te->repaintEditor();

    // te->buffer took ownership of attrs but not of str
    g_free(str);
}

auto TextEditor::iMRetrieveSurroundingCallback(GtkIMContext* context, TextEditor* te) -> bool {
    auto sur = te->buffer.getCursorSurroundings();

    gtk_im_context_set_surrounding(context, sur.text.c_str(), static_cast<int>(sur.text.length()), sur.cursorByteIndex);

    // Todo(gtk4) For GTK >= 4.2
    // gtk_im_context_set_surrounding_with_selection(context, sur.paragraph.c_str(),
    //                                               static_cast<int>(sur.paragraph.length()),
    //                                               sur.cursorByteIndex, sur.selectionByteIndex);

    return true;
}

auto TextEditor::imDeleteSurroundingCallback(GtkIMContext* context, gint offset, gint n_chars, TextEditor* te) -> bool {
    te->buffer.deleteUTF8CharsAroundPreedit(offset, n_chars);

    te->contentsChanged();
    te->repaintEditor();

    return true;
}

auto TextEditor::onKeyPressEvent(GdkEventKey* event) -> bool {
    bool retval = false;
    bool obscure = false;

    GdkModifierType modifiers = gtk_accelerator_get_default_mod_mask();

    // IME needs to handle the input first so the candidate window works correctly
    if (gtk_im_context_filter_keypress(this->imContext.get(), event)) {
        this->needImReset = true;
        obscure = true;
        retval = true;
    } else if (gtk_bindings_activate_event(G_OBJECT(this->textWidget.get()), event)) {
        return true;
    } else if ((event->state & modifiers) == GDK_CONTROL_MASK) {
        if (event->keyval == GDK_KEY_b || event->keyval == GDK_KEY_B) {
            toggleBoldFace();
            return true;
        }
        if (event->keyval == GDK_KEY_plus) {
            increaseFontSize();
            return true;
        }
        if (event->keyval == GDK_KEY_minus) {
            decreaseFontSize();
            return true;
        }
    } else if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_ISO_Enter ||
               event->keyval == GDK_KEY_KP_Enter) {
        this->resetImContext();
        iMCommitCallback(nullptr, "\n", this);

        obscure = true;
        retval = true;
    }
    // Pass through Tab as literal tab, unless Control is held down
    else if ((event->keyval == GDK_KEY_Tab || event->keyval == GDK_KEY_KP_Tab ||
              event->keyval == GDK_KEY_ISO_Left_Tab) &&
             !(event->state & GDK_CONTROL_MASK)) {
        resetImContext();
        iMCommitCallback(nullptr, "\t", this);
        obscure = true;
        retval = true;
    } else {
        retval = false;
    }

    if (obscure) {
        XournalppCursor* cursor = gui->getXournal()->getCursor();
        cursor->setInvisible(true);
    }

    return retval;
}

auto TextEditor::onKeyReleaseEvent(GdkEventKey* event) -> bool {
    if (gtk_im_context_filter_keypress(this->imContext.get(), event)) {
        this->needImReset = true;
        return true;
    }
    return false;
}

void TextEditor::toggleOverwrite() {
    this->cursorOverwrite = !this->cursorOverwrite;
    repaintCursor();
}

/**
 * I know it's a bit rough and duplicated
 * Improve that later on...
 */
void TextEditor::decreaseFontSize() {
    XojFont& font = text->getFont();
    double fontSize = font.getSize();
    fontSize--;
    font.setSize(fontSize);
    setFont(font);
}

void TextEditor::increaseFontSize() {
    XojFont& font = text->getFont();
    double fontSize = font.getSize();
    fontSize++;
    font.setSize(fontSize);
    setFont(font);
}

void TextEditor::toggleBoldFace() {
    // get the current/used font
    XojFont& font = text->getFont();
    std::string fontName = font.getName();

    std::size_t found = fontName.find(" Bold");

    // toggle bold
    if (found == std::string::npos) {
        fontName = fontName + " Bold";
    } else {
        fontName.erase(found, 5);
    }

    // commit changes
    font.setName(fontName);
    setFont(font);

    // this->repaintEditor();
}

void TextEditor::selectAtCursor(TextEditor::SelectType ty) {
    switch (ty) {
        case TextEditor::SelectType::WORD:
            buffer.selectWordAtCursor();
            break;
        case TextEditor::SelectType::LINE:
            buffer.selectLineOfCursor();
            break;
        case TextEditor::SelectType::ALL:
            buffer.selectAll();
            break;
    }
    this->repaintEditor(false);
}

void TextEditor::moveCursor(GtkMovementStep step, int count, bool extendSelection) {
    if (count == 0) {
        return;
    }
    resetImContext();

    if (extendSelection) {
        buffer.ensureSelection();
    } else {
        buffer.clearSelection();
    }

    switch (step) {
        case GTK_MOVEMENT_LOGICAL_POSITIONS:
        case GTK_MOVEMENT_VISUAL_POSITIONS:
            buffer.moveCursorByNGraphemes(count);
            break;
        case GTK_MOVEMENT_WORDS:
            buffer.moveCursorByNWords(count);
            break;
        case GTK_MOVEMENT_DISPLAY_LINES:
            buffer.moveCursorByNLines(count);
            break;
        case GTK_MOVEMENT_PARAGRAPHS:
            buffer.moveCursorByNParagraphs(count);
            break;
        case GTK_MOVEMENT_DISPLAY_LINE_ENDS:
            buffer.moveCursorToLineEnd(count > 0 ? TextEditorBuffer::FORWARDS : TextEditorBuffer::BACKWARDS);
            break;
        case GTK_MOVEMENT_PARAGRAPH_ENDS:
            buffer.moveCursorToParagraphEnd(count > 0 ? TextEditorBuffer::FORWARDS : TextEditorBuffer::BACKWARDS);
            break;
        case GTK_MOVEMENT_BUFFER_ENDS:
            buffer.moveCursorToBufferEnd(count > 0 ? TextEditorBuffer::FORWARDS : TextEditorBuffer::BACKWARDS);
            break;
        default:
            break;
    }

    if (this->cursorBlink) {
        this->cursorVisible = false;
        if (this->blinkCallbackId) {
            g_source_remove(this->blinkCallbackId);
        }
        blinkCallback(this);
    } else {
        repaintCursor();
    }
}

void TextEditor::contentsChanged(bool forceCreateUndoAction) {
    std::string currentText = getText()->getText();

    // I know it's a little bit bulky, but ABS on subtracted size_t is a little bit unsafe
    if (forceCreateUndoAction ||
        ((lastText.length() >= currentText.length()) ? (lastText.length() - currentText.length()) :
                                                       (currentText.length() - lastText.length())) > 100) {
        if (!lastText.empty() && !this->undoActions.empty() &&
            this->undoActions.front().get().getUndoText() != currentText) {
            auto undo = std::make_unique<TextUndoAction>(gui->getPage(), gui->getPage()->getSelectedLayer(), this->text,
                                                         lastText, this);
            UndoRedoHandler* handler = gui->getXournal()->getControl()->getUndoRedoHandler();
            this->undoActions.emplace_back(std::ref(*undo));
            handler->addUndoAction(std::move(undo));
        }
        lastText = std::move(currentText);
    }
}

auto TextEditor::getFirstUndoAction() const -> UndoAction* {
    if (!this->undoActions.empty()) {
        return &this->undoActions.front().get();
    }
    return nullptr;
}

void TextEditor::mousePressed(double x, double y) {
    this->mouseDown = true;
    buffer.clearSelection();
    if (buffer.moveCursorAt(x, y)) {
        repaintCursor();
    }
    buffer.ensureSelection();
}

void TextEditor::mouseMoved(double x, double y) {
    if (this->mouseDown && buffer.moveCursorAt(x, y)) {
        gui->getXournal()->getControl()->setCopyPasteEnabled(buffer.hasSelection());
        repaintCursor();
    }
}

void TextEditor::mouseReleased() { this->mouseDown = false; }

void TextEditor::deleteFromCursor(GtkDeleteType type, int count) {
    bool deletedSomething = false;
    bool makeUndo = true;
    switch (type) {
        case GTK_DELETE_CHARS:
            if (buffer.hasSelection()) {
                deletedSomething = buffer.deleteSelection();
            } else {
                deletedSomething = buffer.deleteNGraphemesFromCursor(count);
                makeUndo = false;
            }
            break;
        case GTK_DELETE_WORD_ENDS:
            deletedSomething = buffer.deleteFromCursorUntilWordEnd(count > 0 ? TextEditorBuffer::FORWARDS :
                                                                               TextEditorBuffer::BACKWARDS);
            break;
        case GTK_DELETE_WORDS:
            deletedSomething = buffer.deleteNWordsFromCursor(count);
            break;
        case GTK_DELETE_DISPLAY_LINE_ENDS:
            deletedSomething = buffer.deleteFromCursorUntilLineEnd(count > 0 ? TextEditorBuffer::FORWARDS :
                                                                               TextEditorBuffer::BACKWARDS);
            break;
        case GTK_DELETE_DISPLAY_LINES:
            deletedSomething = buffer.deleteNLinesFromCursor(count);
            break;
        case GTK_DELETE_PARAGRAPH_ENDS:
            deletedSomething = buffer.deleteFromCursorUntilParagraphEnd(count > 0 ? TextEditorBuffer::FORWARDS :
                                                                                    TextEditorBuffer::BACKWARDS);
            break;
        case GTK_DELETE_PARAGRAPHS:
            deletedSomething = buffer.deleteNParagraphsFromCursor(count);
            break;
        case GTK_DELETE_WHITESPACE:
            deletedSomething = buffer.deleteWhiteSpacesAroundCursor();
            break;
        default:
            break;
    }

    if (deletedSomething) {
        this->contentsChanged(makeUndo);
        this->repaintEditor();
    }
}

void TextEditor::backspace() {
    resetImContext();

    if ((buffer.hasSelection() && buffer.deleteSelection()) || buffer.deleteUponBackspace()) {
        this->contentsChanged();
        this->repaintEditor();
    }
}

void TextEditor::copyToCliboard() const {
    //     GtkClipboard* clipboard = gtk_widget_get_clipboard(this->xournalWidget, GDK_SELECTION_CLIPBOARD);
    //     gtk_text_buffer_copy_clipboard(this->buffer.get(), clipboard);
}

void TextEditor::cutToClipboard() {
    //     GtkClipboard* clipboard = gtk_widget_get_clipboard(this->xournalWidget, GDK_SELECTION_CLIPBOARD);
    //     gtk_text_buffer_cut_clipboard(this->buffer.get(), clipboard, true);

    //     this->contentsChanged(true);
    //     this->repaintEditor();
}

void TextEditor::pasteFromClipboard() {
    //     GtkClipboard* clipboard = gtk_widget_get_clipboard(this->xournalWidget, GDK_SELECTION_CLIPBOARD);
    //     gtk_text_buffer_paste_clipboard(this->buffer.get(), clipboard, nullptr, true);
}

// void TextEditor::bufferPasteDoneCallback(GtkTextBuffer* buffer, GtkClipboard* clipboard, TextEditor* te) {
//     te->contentsChanged(true);
//     te->repaintEditor();
// }

void TextEditor::resetImContext() {
    if (this->needImReset) {
        this->needImReset = false;
        gtk_im_context_reset(this->imContext.get());
    }
}

void TextEditor::repaintCursor() { this->gui->repaintElement(this->text); }

/*
 * Blink!
 */
auto TextEditor::blinkCallback(TextEditor* te) -> gint {
    if (te->cursorVisible) {
        te->blinkCallbackId =
                gdk_threads_add_timeout(te->cursorBlinkingTimeOff, reinterpret_cast<GSourceFunc>(blinkCallback), te);
    } else {
        te->blinkCallbackId =
                gdk_threads_add_timeout(te->cursorBlinkingTimeOn, reinterpret_cast<GSourceFunc>(blinkCallback), te);
    }

    te->cursorVisible = !te->cursorVisible;

    te->repaintCursor();

    // Remove ourselves
    return false;
}

void TextEditor::repaintEditor(bool sizeChanged) {
    Range dirtyRange(this->previousBoundingBox);
    if (sizeChanged) {
        this->previousBoundingBox = buffer.getTextBoundingBox();
        const double zoom = this->gui->getXournal()->getZoom();
        const double padding = (BORDER_WIDTH_IN_PIXELS + PADDING_IN_PIXELS) / zoom;
        this->previousBoundingBox.addPadding(padding);
        dirtyRange = dirtyRange.unite(this->previousBoundingBox);
    }
    this->gui->repaintArea(dirtyRange.minX, dirtyRange.minY, dirtyRange.maxX, dirtyRange.maxY);
}

auto TextEditor::drawCursor(cairo_t* cr, double zoom) const -> xoj::util::Rectangle<double> {

    xoj::util::Rectangle<double> cursorBox(buffer.getCursorBoundingBox(this->cursorOverwrite));

    if (!this->cursorOverwrite || cursorBox.width == 0.0) {
        cursorBox.x -= 1.0 / zoom;
        cursorBox.width = 2.0 / zoom;
    }

    if (this->cursorVisible) {
        xoj::util::CairoSaveGuard saveGuard(cr);

        cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_rectangle(cr, cursorBox.x, cursorBox.y, cursorBox.width, cursorBox.height);
        cairo_fill(cr);
    }

    return cursorBox;
}

void TextEditor::paint(cairo_t* cr, double zoom) {
    GdkRGBA selectionColor = this->gui->getSelectionColor();

    cairo_save(cr);

    Util::cairo_set_source_rgbi(cr, this->text->getColor());

    double x0 = this->text->getX();
    double y0 = this->text->getY();
    cairo_translate(cr, x0, y0);

    PangoLayout* pl = buffer.getPangoLayout();

    // The cairo context might have changed. Update the pango layout
    pango_cairo_update_layout(cr, pl);
    // Workaround https://gitlab.gnome.org/GNOME/pango/-/issues/691
    pango_context_set_matrix(pango_layout_get_context(pl), nullptr);

    if (auto sel = buffer.getSelectionExtents(); sel) {
        auto selectionColorU16 = Util::GdkRGBA_to_ColorU16(selectionColor);
        PangoAttribute* attrib =
                pango_attr_background_new(selectionColorU16.red, selectionColorU16.green, selectionColorU16.blue);
        attrib->start_index = sel->min;
        attrib->end_index = sel->max;

        xoj::util::PangoAttrListSPtr attrlist = buffer.cloneAttributeList();
        pango_attr_list_insert(attrlist.get(), attrib);  // attrlist takes ownership of attrib
        pango_layout_set_attributes(pl, attrlist.get());
    }

    pango_cairo_show_layout(cr, pl);

    {
        auto cursorBox = drawCursor(cr, zoom);

        // Notify the IM of the app's window and cursor position.
        double x1 = this->gui->getX();
        double y1 = this->gui->getY();
        GdkRectangle cursorRect;  // cursor position in window coordinates
        cursorRect.x = static_cast<int>(zoom * x0 + x1 + zoom * cursorBox.x);
        cursorRect.y = static_cast<int>(zoom * y0 + y1 + zoom * cursorBox.y);
        cursorRect.height = static_cast<int>(zoom * cursorBox.height);
        cursorRect.width = static_cast<int>(zoom * cursorBox.width);
        gtk_im_context_set_cursor_location(this->imContext.get(), &cursorRect);
    }

    /*
     * Draw the box around the text
     *
     * Set the line width independently of the zoom
     */
    cairo_set_line_width(cr, BORDER_WIDTH_IN_PIXELS / zoom);
    gdk_cairo_set_source_rgba(cr, &selectionColor);

    auto bb = buffer.getTextBoundingBox();
    bb.addPadding(PADDING_IN_PIXELS / zoom);

    cairo_rectangle(cr, bb.minX, bb.minY, bb.getWidth(), bb.getHeight());
    cairo_stroke(cr);

    cairo_restore(cr);

    this->text->setWidth(bb.getWidth());
    this->text->setHeight(bb.getHeight());
}
