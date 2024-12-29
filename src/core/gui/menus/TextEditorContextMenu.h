/*
 * Xournal++
 *
 * Context Menu complementing the text editor
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <vector>

#include <gdk/gdk.h>  // for g_signal_connect
#include <gtk/gtk.h>  // for GtkPopover, etc.

#include "util/Color.h"
#include "util/raii/GObjectSPtr.h"  // for xoj::util::GObjectSPtr
#include "util/raii/PangoSPtr.h"

class Control;
class TextEditor;
class XojPageView;
enum class TextAlignment;

class TextEditorContextMenu {
public:
    TextEditorContextMenu(Control* control, TextEditor* editor, XojPageView* pageView, GtkWidget* xournalWidget);
    ~TextEditorContextMenu();

    void show();
    void hide();

    void setAttributes(std::vector<xoj::util::PangoAttributeUPtr> attributes);

    void reposition();

    void changeFont();
    void changeColor();
    void changeBgColor();
    void changeAlignment(TextAlignment align);

    void toggleItalic();
    void toggleBold();
    void toggleUnderline();
    void toggleStrikethrough();

    struct Button {
        GtkToggleButton* btn = nullptr;
        gulong signalId = 0;
    };

private:
    void resetContextMenuState();

    void switchAlignmentButtons(TextAlignment alignment);

private:
    Control* control;
    TextEditor* editor;
    XojPageView* pageView;
    GtkWidget* xournalWidget;

    bool isVisible = false;

    bool italic = false;
    bool bold = false;
    bool underlined = false;
    bool strikethrough = false;
    Color color;

    /**
     * UI Elements
     */
    GtkPopover* contextMenu = nullptr;

    GtkFontButton* fontBtn = nullptr;

    Button tglBoldBtn;
    Button tglItalicBtn;
    Button tglUnderlineBtn;
    Button tglStrikethrough;

    GtkImage* colorIcon = nullptr;

    xoj::util::GObjectSPtr<GSimpleAction> alignmentAction;
};
