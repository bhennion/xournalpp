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

// static auto getIteratorAtCursor(GtkTextBuffer* buffer) -> GtkTextIter {
//     GtkTextIter cursorIter = {nullptr};
//     gtk_text_buffer_get_iter_at_mark(buffer, &cursorIter, gtk_text_buffer_get_insert(buffer));
//     return cursorIter;
// }
// 
// static auto getCharacterOffsetOfCursor(GtkTextBuffer* buffer) -> int {
//     auto it = getIteratorAtCursor(buffer);
//     return gtk_text_iter_get_offset(&it);
// }

auto TextEditor::getText() const -> Text* {
    this->text->setText(this->newBuf);
    return this->text;
}

void TextEditor::replaceBufferContent(const std::string& text) {
    buffer.replaceContent(text);
}

auto TextEditor::setColor(Color color) -> UndoAction* {
    auto origColor = this->text->getColor();
    this->text->setColor(color);

    repaintEditor(false);

    // This is a new text, so we don't need to create a undo action
    if (this->ownText) {
        return nullptr;
    }

    auto* undo = new ColorUndoAction(gui->getPage(), gui->getPage()->getSelectedLayer());
    undo->addStroke(this->text, origColor, color);

    return undo;
}

void TextEditor::setFont(XojFont font) {
    this->text->setFont(font);
    this->buffer.updateFont(*this->text);
    this->computeVirtualCursorPosition();
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
            te->buffer.overwrite(strv);
        } else {
            te->buffer.insert(strv);
        }
        te->contentsChanged();
    }
    te->repaintEditor();
}

void TextEditor::iMPreeditChangedCallback(GtkIMContext* context, TextEditor* te) {
    gchar* str = nullptr;
    gint cursor_pos = 0;

    {
        PangoAttrList* attrs = nullptr;
        gtk_im_context_get_preedit_string(context, &str, &attrs, &cursor_pos);
        if (attrs == nullptr) {
            attrs = pango_attr_list_new();
        }
        te->preeditAttrList.reset(attrs);
    }
    
    if (str != nullptr) {
        te->preeditString = str;
    } else {
        te->preeditString = "";
    }
    te->preeditCursor = cursor_pos;
    te->contentsChanged();
    te->repaintEditor();

    g_free(str);
}

auto TextEditor::iMRetrieveSurroundingCallback(GtkIMContext* context, TextEditor* te) -> bool {
    size_t beginLine = te->insertMark ? te->newBuf.rfind('\n', te->insertMark - 1) + 1 : 0U;
    size_t endLine = te->newBuf.find('\n', te->insertMark);
    
    int length = endLine == std::string::npos ? -1 : static_cast<int>(endLine - beginLine);
    int position = static_cast<int>(te->insertMark - beginLine);
    
    const char* begin = te->newBuf.c_str() + beginLine;

    gtk_im_context_set_surrounding(context, begin, length, position);

    return true;
}

auto TextEditor::imDeleteSurroundingCallback(GtkIMContext* context, gint offset, gint n_chars, TextEditor* te) -> bool {
    
    auto start = utf8_next(te->newBuf, getIteratorAt(te->newBuf, te->insertMark), offset);
    auto end  = utf8_next(te->newBuf, start, n_chars);
    
    auto it = te->newBuf.erase(start, end);
    te->insertMark = static_cast<size_t>(&*it - te->newBuf.data());

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

    std::size_t found = fontName.find("Bold");

    // toggle bold
    if (found == std::string::npos) {
        fontName = fontName + " Bold";
    } else {
        fontName = fontName.substr(0, found - 1);
    }

    // commit changes
    font.setName(fontName);
    setFont(font);

    // this->repaintEditor();
}

void TextEditor::selectAtCursor(TextEditor::SelectType ty) {
    if (this->newBuf.empty()) {
        return;
    }

    switch (ty) {
        case TextEditor::SelectType::WORD: {
            auto it = getIteratorAt(this->newBuf, this->insertMark);
            auto isSpace = [](const char* c){return g_unichar_isspace(g_utf8_get_char(c));};
            
            if (isSpace(&*it)) {
                // Do nothing if cursor is over whitespace
                this->selectionMark = std::string::npos;
                return;
            }
            this->selectionMark = utf8_rfind_if(this->newBuf, this->insertMark, isSpace);
            this->insertMark = utf8_find_if(this->newBuf, this->insertMark, isSpace);
            break;
        }
        case TextEditor::SelectType::PARAGRAPH: {
            // We define a paragraph as text separated by double newlines.
            size_t start = this->newBuf.rfind("\n\n", this->insertMark);
            size_t end = this->newBuf.find("\n\n", this->insertMark);
            assert(start == std::string::npos || end == std::string::npos || start < end);
            this->selectionMark = start == std::string::npos ? 0U : start;
            this->insertMark = end == std::string::npos ? this->newBuf.length() : end;
            break;
        }
        case TextEditor::SelectType::ALL:
            this->selectionMark = 0U;
            this->insertMark = this->newBuf.length();
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
    this->computeVirtualCursorPosition();
}

auto TextEditor::getFirstUndoAction() const -> UndoAction* {
    if (!this->undoActions.empty()) {
        return &this->undoActions.front().get();
    }
    return nullptr;
}

void TextEditor::markPos(double x, double y, bool extendSelection) {
    GtkTextIter iter = getIteratorAtCursor(this->buffer.get());
    GtkTextIter newplace = iter;

    findPos(&newplace, x, y);

    // Noting changed
    if (gtk_text_iter_equal(&newplace, &iter)) {
        return;
    }
    moveCursor(&newplace, extendSelection);
    computeVirtualCursorPosition();
    repaintCursor();
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

    this->resetImContext();

    if (type == GTK_DELETE_CHARS) {
        // Char delete deletes the selection, if one exists
        if (gtk_text_buffer_delete_selection(this->buffer.get(), true, true)) {
            this->contentsChanged(true);
            this->repaintEditor();
            return;
        }
    }

    GtkTextIter insert = getIteratorAtCursor(this->buffer.get());

    GtkTextIter start = insert;
    GtkTextIter end = insert;

    switch (type) {
        case GTK_DELETE_CHARS:
            gtk_text_iter_forward_cursor_positions(&end, count);
            break;

        case GTK_DELETE_WORD_ENDS:
            if (count > 0) {
                gtk_text_iter_forward_word_ends(&end, count);
            } else if (count < 0) {
                gtk_text_iter_backward_word_starts(&start, 0 - count);
            }
            break;

        case GTK_DELETE_WORDS:
            break;

        case GTK_DELETE_DISPLAY_LINE_ENDS:
            break;

        case GTK_DELETE_DISPLAY_LINES:
            break;

        case GTK_DELETE_PARAGRAPH_ENDS:
            if (count > 0) {
                /* If we're already at a newline, we need to
                 * simply delete that newline, instead of
                 * moving to the next one.
                 */
                if (gtk_text_iter_ends_line(&end)) {
                    gtk_text_iter_forward_line(&end);
                    --count;
                }

                while (count > 0) {
                    if (!gtk_text_iter_forward_to_line_end(&end)) {
                        break;
                    }

                    --count;
                }
            } else if (count < 0) {
                if (gtk_text_iter_starts_line(&start)) {
                    gtk_text_iter_backward_line(&start);
                    if (!gtk_text_iter_ends_line(&end)) {
                        gtk_text_iter_forward_to_line_end(&start);
                    }
                } else {
                    gtk_text_iter_set_line_offset(&start, 0);
                }
                ++count;

                gtk_text_iter_backward_lines(&start, -count);
            }
            break;

        case GTK_DELETE_PARAGRAPHS:
            if (count > 0) {
                gtk_text_iter_set_line_offset(&start, 0);
                gtk_text_iter_forward_to_line_end(&end);

                /* Do the lines beyond the first. */
                while (count > 1) {
                    gtk_text_iter_forward_to_line_end(&end);
                    --count;
                }
            }

            break;

        case GTK_DELETE_WHITESPACE: {
            find_whitepace_region(&insert, &start, &end);
        } break;

        default:
            break;
    }

    if (!gtk_text_iter_equal(&start, &end)) {
        gtk_text_buffer_begin_user_action(this->buffer.get());

        if (!gtk_text_buffer_delete_interactive(this->buffer.get(), &start, &end, true)) {
            gtk_widget_error_bell(this->xournalWidget);
        }

        gtk_text_buffer_end_user_action(this->buffer.get());
    } else {
        gtk_widget_error_bell(this->xournalWidget);
    }

    this->contentsChanged();
    this->repaintEditor();
}

void TextEditor::backspace() {

    resetImContext();

    // Backspace deletes the selection, if one exists
    if (gtk_text_buffer_delete_selection(this->buffer.get(), true, true)) {
        this->contentsChanged();
        this->repaintEditor();
        return;
    }

    GtkTextIter insert = getIteratorAtCursor(this->buffer.get());

    if (gtk_text_buffer_backspace(this->buffer.get(), &insert, true, true)) {
        this->contentsChanged();
        this->repaintEditor();
    } else {
        gtk_widget_error_bell(this->xournalWidget);
    }
}

auto TextEditor::getSelection() const -> std::string {
    std::string s;

    if (GtkTextIter start, end; gtk_text_buffer_get_selection_bounds(this->buffer.get(), &start, &end)) {
        char* text = gtk_text_iter_get_text(&start, &end);
        s = text;
        g_free(text);
    }
    return s;
}

void TextEditor::copyToCliboard() const {
    GtkClipboard* clipboard = gtk_widget_get_clipboard(this->xournalWidget, GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_copy_clipboard(this->buffer.get(), clipboard);
}

void TextEditor::cutToClipboard() {
    GtkClipboard* clipboard = gtk_widget_get_clipboard(this->xournalWidget, GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_cut_clipboard(this->buffer.get(), clipboard, true);

    this->contentsChanged(true);
    this->repaintEditor();
}

void TextEditor::pasteFromClipboard() {
    GtkClipboard* clipboard = gtk_widget_get_clipboard(this->xournalWidget, GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_paste_clipboard(this->buffer.get(), clipboard, nullptr, true);
}

void TextEditor::bufferPasteDoneCallback(GtkTextBuffer* buffer, GtkClipboard* clipboard, TextEditor* te) {
    te->contentsChanged(true);
    te->repaintEditor();
}

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

void TextEditor::setTextToPangoLayout(PangoLayout* pl) const {
    std::string txt = this->text->getText();

    if (!this->preeditString.empty()) {
        // When using an Input Method, we need to insert the preeditString into the text at the cursor location

        // Get the byte position of the cursor in the string, so we can insert at the right place
        int pos = 0;
        {
            GtkTextIter it = getIteratorAtCursor(this->buffer.get());
            // Bytes from beginning of line to iterator
            pos = gtk_text_iter_get_line_index(&it);
            gtk_text_iter_set_line_index(&it, 0);
            // Count bytes of previous lines
            while (gtk_text_iter_backward_line(&it)) {
                pos += gtk_text_iter_get_bytes_in_line(&it);
            }
        }
        txt.insert(static_cast<size_t>(pos), this->preeditString);

        xoj::util::PangoAttrListSPtr attrlist(pango_attr_list_new());
        pango_attr_list_splice(attrlist.get(), this->preeditAttrList.get(), pos,
                               static_cast<int>(preeditString.length()));

        pango_layout_set_attributes(pl, attrlist.get());
    }
    pango_layout_set_text(pl, txt.c_str(), static_cast<int>(txt.length()));
}

auto TextEditor::computeBoundingBox() const -> Range {
    /*
     * NB: we cannot rely on TextView::calcSize directly, since it would not take the size changes due to the IM
     * preeditString into account.
     */
    auto* textElement = this->getText();

    setTextToPangoLayout(this->layout.get());

    int w = 0;
    int h = 0;
    pango_layout_get_size(this->layout.get(), &w, &h);
    double width = (static_cast<double>(w)) / PANGO_SCALE;
    double height = (static_cast<double>(h)) / PANGO_SCALE;
    double x = textElement->getX();
    double y = textElement->getY();

    return Range(x, y, x + width, y + height);
}

void TextEditor::repaintEditor(bool sizeChanged) {
    Range dirtyRange(this->previousBoundingBox);
    if (sizeChanged) {
        this->previousBoundingBox = this->computeBoundingBox();
        dirtyRange = dirtyRange.unite(this->previousBoundingBox);
    }
    const double zoom = this->gui->getXournal()->getZoom();
    const double padding = (BORDER_WIDTH_IN_PIXELS + PADDING_IN_PIXELS) / zoom;
    dirtyRange.addPadding(padding);
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

    // The cairo context might have changed. Update the pango layout
    pango_cairo_update_layout(cr, this->layout.get());
    // Workaround https://gitlab.gnome.org/GNOME/pango/-/issues/691
    pango_context_set_matrix(pango_layout_get_context(this->layout.get()), nullptr);

    this->setTextToPangoLayout(this->layout.get());

    if (this->preeditString.empty()) {
        GtkTextIter start;
        GtkTextIter end;
        bool hasSelection = gtk_text_buffer_get_selection_bounds(this->buffer.get(), &start, &end);

        if (hasSelection) {
            auto selectionColorU16 = Util::GdkRGBA_to_ColorU16(selectionColor);
            PangoAttribute* attrib =
                    pango_attr_background_new(selectionColorU16.red, selectionColorU16.green, selectionColorU16.blue);
            attrib->start_index = static_cast<unsigned int>(getByteOffset(gtk_text_iter_get_offset(&start)));
            attrib->end_index = static_cast<unsigned int>(getByteOffset(gtk_text_iter_get_offset(&end)));

            xoj::util::PangoAttrListSPtr attrlist(pango_attr_list_new());
            pango_attr_list_insert(attrlist.get(), attrib);  // attrlist takes ownership of attrib
            pango_layout_set_attributes(this->layout.get(), attrlist.get());
        } else {
            // remove all attributes
            PangoAttrList* attrlist = pango_attr_list_new();
            pango_layout_set_attributes(this->layout.get(), attrlist);
            pango_attr_list_unref(attrlist);
            attrlist = nullptr;
        }
    }

    pango_cairo_show_layout(cr, this->layout.get());

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

    cairo_restore(cr);

    /*
     * Draw the box around the text
     *
     * Set the line width independently of the zoom
     */
    cairo_set_line_width(cr, BORDER_WIDTH_IN_PIXELS / zoom);
    gdk_cairo_set_source_rgba(cr, &selectionColor);

    int w = 0;
    int h = 0;
    pango_layout_get_size(this->layout.get(), &w, &h);
    double width = (static_cast<double>(w)) / PANGO_SCALE;
    double height = (static_cast<double>(h)) / PANGO_SCALE;

    cairo_rectangle(cr, x0 - PADDING_IN_PIXELS / zoom, y0 - PADDING_IN_PIXELS / zoom,
                    width + 2 * PADDING_IN_PIXELS / zoom, height + 2 * PADDING_IN_PIXELS / zoom);
    cairo_stroke(cr);

    this->text->setWidth(width);
    this->text->setHeight(height);
}
