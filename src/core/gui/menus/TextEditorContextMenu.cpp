#include "TextEditorContextMenu.h"

#include <iostream>
#include <string>

#include "control/Control.h"                // for Control
#include "control/tools/TextEditor.h"       // for TextEditor
#include "gui/Builder.h"                    // for Builder
#include "gui/GladeSearchpath.h"            // for GladeSearchPath
#include "gui/PageView.h"                   // for PageView
#include "model/Font.h"                     // for XojFont
#include "model/Text.h"                     // for Text
#include "util/GVariantTemplate.h"          //
#include "util/GtkUtil.h"                   //
#include "view/overlays/TextEditionView.h"  // for TextEditionView
#include "util/serdesstream.h" //
#include "util/gtk4_helper.h" //
#include "util/raii/GLibGuards.h" //
#include "util/XojMsgBox.h" //
#include "util/i18n.h"  //


static constexpr const char* ACTION_NAME = "text-alignment";
static constexpr const char* NAMESPACED_ACTION_NAME = "win.text-alignment";

template <typename GtkType, auto fn, auto... a>
static void wrap(GtkType*, gpointer data) {
    ((static_cast<TextEditorContextMenu*>(data))->*fn)(a...);
}

template <auto fn, auto... a>
static void addToggledCallback(TextEditorContextMenu::Button& btn, TextEditorContextMenu* self) {
    btn.signalId = g_signal_connect(btn.btn, "toggled", G_CALLBACK((wrap<GtkToggleButton, fn, a...>)), self);
}
template <auto fn, auto... a>
static void addClickedCallback(GtkButton* btn, TextEditorContextMenu* self) {
    g_signal_connect(btn, "clicked", G_CALLBACK((wrap<GtkButton, fn, a...>)), self);
}


/// Returns a floating GtkImage
static auto newTextColorIcon(Color color) {
    auto stream = serdes_stream<std::stringstream>();
    stream << "<svg width=\"24\" height=\"24\" stroke-width=\"1\" stroke-linecap=\"round\" stroke-linejoin=\"round\" xmlns=\"http://www.w3.org/2000/svg\">"
    "<text x=\"12\" y=\"15\" fill=\"black\" font-weight=\"bold\" font-size=\"20\" text-anchor=\"middle\">A</text>"
    "<rect width=\"20\" height=\"4\" x=\"2\" y=\"18\" rx=\"1\" ry=\"1\" style=\"stroke:#000000;stroke-opacity:1;fill:#"
           << std::hex << std::setw(6) << std::setfill('0') << (uint32_t(color) & 0x00ffffff)
           << "\"/></svg>";

    std::string str = stream.str();  // Keep this alive as long as gstream is
    xoj::util::GObjectSPtr<GInputStream> gstream(g_memory_input_stream_new_from_data(str.c_str(), -1, nullptr),
                                                 xoj::util::adopt);

    xoj::util::GErrorGuard err;
    auto icon = xoj::util::GObjectSPtr<GdkPixbuf>(gdk_pixbuf_new_from_stream(gstream.get(), NULL, xoj::util::out_ptr(err)), xoj::util::adopt);
    xoj_assert_message(!err, err->message);

    return icon;
}

TextEditorContextMenu::TextEditorContextMenu(Control* control, TextEditor* editor, XojPageView* pageView,
                                             GtkWidget* xournalWidget):
        control(control), editor(editor), pageView(pageView), xournalWidget(xournalWidget), color(editor->getTextElement()->getColor()) {
    // Only for debugging
    std::cout << "TextEditorContextMenu created!" << std::endl;

    Builder builder(this->control->getGladeSearchPath(), "textEditorContextMenu.glade");

    this->contextMenu = GTK_POPOVER(builder.get("textEditorContextMenu"));
    gtk_popover_set_relative_to(this->contextMenu, this->xournalWidget);
    gtk_popover_set_constrain_to(this->contextMenu, GTK_POPOVER_CONSTRAINT_WINDOW);
    gtk_popover_set_modal(this->contextMenu, false);
    gtk_widget_set_can_focus(GTK_WIDGET(this->contextMenu), false);
    gtk_widget_hide(GTK_WIDGET(this->contextMenu));

    this->fontBtn = GTK_FONT_BUTTON(builder.get("btnFontChooser"));

    using Self = TextEditorContextMenu;

    g_signal_connect(this->fontBtn, "font-set", G_CALLBACK((wrap<GtkFontButton, &Self::changeFont>)), this);


    this->tglBoldBtn.btn = GTK_TOGGLE_BUTTON(builder.get("btnDecoBold"));
    this->tglItalicBtn.btn = GTK_TOGGLE_BUTTON(builder.get("btnDecoItalic"));
    this->tglUnderlineBtn.btn = GTK_TOGGLE_BUTTON(builder.get("btnDecoUnderline"));
    this->tglStrikethrough.btn = GTK_TOGGLE_BUTTON(builder.get("btnStrikethrough"));
    addToggledCallback<&Self::toggleBold>(this->tglBoldBtn, this);
    addToggledCallback<&Self::toggleItalic>(this->tglItalicBtn, this);
    addToggledCallback<&Self::toggleUnderline>(this->tglUnderlineBtn, this);
    addToggledCallback<&Self::toggleStrikethrough>(this->tglStrikethrough, this);

    this->colorIcon = GTK_IMAGE(gtk_image_new_from_pixbuf(newTextColorIcon(this->color).get()));
    gtk_widget_show_all(GTK_WIDGET(this->colorIcon));
    gtk_button_set_child(GTK_BUTTON(builder.get("btnFontColor")), GTK_WIDGET(this->colorIcon));
    addClickedCallback<&Self::changeColor>(GTK_BUTTON(builder.get("btnFontColor")), this);

    this->alignmentAction.reset(g_simple_action_new_stateful(ACTION_NAME, gVariantType<TextAlignment>(),
                                                             makeGVariant(TextAlignment::JUSTIFIED)),
                                xoj::util::adopt);
    g_action_map_add_action(G_ACTION_MAP(this->control->getGtkWindow()), G_ACTION(this->alignmentAction.get()));
    g_signal_connect(this->alignmentAction.get(), "change-state",
                     G_CALLBACK(+[](GSimpleAction*, GVariant* p, gpointer self) {
                         static_cast<TextEditorContextMenu*>(self)->changeAlignment(getGVariantValue<TextAlignment>(p));
                     }),
                     this);

    auto bindAlign = [&](TextAlignment a, const char* id) {
        auto* w = GTK_ACTIONABLE(builder.get(id));
        gtk_actionable_set_action_target_value(w, makeGVariant(a));
        gtk_actionable_set_action_name(w, NAMESPACED_ACTION_NAME);
        xoj::util::gtk::setToggleButtonUnreleasable(GTK_TOGGLE_BUTTON(w));
    };
    bindAlign(TextAlignment::LEFT, "btnAlignLeft");
    bindAlign(TextAlignment::CENTER, "btnAlignCenter");
    bindAlign(TextAlignment::RIGHT, "btnAlignRight");
    bindAlign(TextAlignment::JUSTIFIED, "btnJustify");
}

TextEditorContextMenu::~TextEditorContextMenu() {
    gtk_popover_set_relative_to(this->contextMenu, NULL);
    g_action_map_remove_action(G_ACTION_MAP(this->control->getGtkWindow()), ACTION_NAME);
    std::cout << "TextEditorContextMenu destroyed!" << std::endl;
}


void TextEditorContextMenu::show() {
    if (!isVisible) {
        this->switchAlignmentButtons(this->editor->getTextElement()->getAlignment());
        this->reposition();
        gtk_popover_popup(this->contextMenu);
        isVisible = true;
        std::cout << "Popup menu should be shown" << std::endl;
    }
}

void TextEditorContextMenu::hide() {
    if (isVisible) {
        gtk_popover_popdown(this->contextMenu);
        isVisible = false;
        std::cout << "Popup menu should be hidden" << std::endl;
    }
}

void TextEditorContextMenu::reposition() {
    int padding = xoj::view::TextEditionView::PADDING_IN_PIXELS;
    Range r = this->editor->getContentBoundingBox();
    GdkRectangle rect{this->pageView->getX() + int(r.getX() * this->pageView->getZoom()),
                      this->pageView->getY() + int(r.getY() * this->pageView->getZoom()) - padding,
                      int(r.getWidth() * this->pageView->getZoom()), int(r.getHeight() * this->pageView->getZoom())};
    gtk_popover_set_pointing_to(this->contextMenu, &rect);
}

void TextEditorContextMenu::changeFont() {
    xoj::util::PangoFontDescriptionUPtr desc(gtk_font_chooser_get_font_desc(GTK_FONT_CHOOSER(this->fontBtn)));
    this->editor->addInlineAttribute(xoj::util::PangoAttributeUPtr(pango_attr_font_desc_new(desc.get())));
    gtk_widget_grab_focus(this->xournalWidget);
}

void TextEditorContextMenu::changeColor() {
    XojMsgBox::showColorChooserDialog(this->control->getGtkWindow(), _("Choose text color"), this->color, [this](std::optional<Color> c) {
        if (c.has_value()) {
            this->color = c.value();
            auto colorU16 = Util::argb_to_ColorU16(this->color);
            this->editor->addInlineAttribute(xoj::util::PangoAttributeUPtr(pango_attr_foreground_new(colorU16.red, colorU16.green, colorU16.blue)));
            gtk_image_set_from_pixbuf(this->colorIcon, newTextColorIcon(this->color).get());
            gtk_widget_grab_focus(this->xournalWidget);
        }
    });
}

void TextEditorContextMenu::changeAlignment(TextAlignment align) {
    this->switchAlignmentButtons(align);
    this->editor->setTextAlignment(align);
    gtk_widget_grab_focus(this->xournalWidget);
}

static void setToggleButtonState(const TextEditorContextMenu::Button& btn, bool active) {
    g_signal_handler_block(btn.btn, btn.signalId);
    gtk_toggle_button_set_active(btn.btn, active);
    g_signal_handler_unblock(btn.btn, btn.signalId);
}

void TextEditorContextMenu::toggleItalic() {
    this->italic = !this->italic;
    this->editor->addInlineAttribute(xoj::util::PangoAttributeUPtr(pango_attr_style_new(this->italic ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL)));
}

void TextEditorContextMenu::toggleBold() {
    this->bold = !this->bold;
    this->editor->addInlineAttribute(xoj::util::PangoAttributeUPtr(pango_attr_weight_new(this->bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL)));
}

void TextEditorContextMenu::toggleUnderline() {
    this->underlined = !this->underlined;
    this->editor->addInlineAttribute(xoj::util::PangoAttributeUPtr(pango_attr_underline_new(this->underlined ? PANGO_UNDERLINE_SINGLE : PANGO_UNDERLINE_NONE)));
}

void TextEditorContextMenu::toggleStrikethrough() {
    this->strikethrough = !this->strikethrough;
    this->editor->addInlineAttribute(xoj::util::PangoAttributeUPtr(pango_attr_strikethrough_new(this->strikethrough)));
}

template<typename PangoEnumType>
static PangoEnumType getEnumAttr(PangoAttribute* p) { return static_cast<PangoEnumType>(pango_attribute_as_int(p)->value); }

void TextEditorContextMenu::setAttributes(std::vector<xoj::util::PangoAttributeUPtr> attributes) {
    std::cout << "ContextMenu.setAttributes: " << attributes.size() << std::endl;

    bool italic = false;
    bool underlined = false;
    bool strikethrough = false;
    bool bold = false;
    Color color = this->editor->getTextElement()->getColor();
    bool inlineFont = false;

    for (auto&& p: attributes) {
        switch (p->klass->type) {
            case PANGO_ATTR_FONT_DESC:
                gtk_font_chooser_set_font_desc(GTK_FONT_CHOOSER(this->fontBtn), pango_attribute_as_font_desc(p.get())->desc);
                inlineFont = true;
                break;
            case PANGO_ATTR_FOREGROUND: {
                PangoColor c = pango_attribute_as_color(p.get())->color;
                color = Color{static_cast<uint8_t>(c.red >> 8U), static_cast<uint8_t>(c.green >> 8U),
                                    static_cast<uint8_t>(c.blue >> 8U)};
                break;
            }
            case PANGO_ATTR_STYLE:
                italic = getEnumAttr<PangoStyle>(p.get()) == PANGO_STYLE_ITALIC;
                break;
            case PANGO_ATTR_WEIGHT:
                bold = getEnumAttr<PangoWeight>(p.get()) == PANGO_WEIGHT_BOLD;
                break;
            case PANGO_ATTR_UNDERLINE:
                underlined = getEnumAttr<PangoUnderline>(p.get()) == PANGO_UNDERLINE_SINGLE;
                break;
            case PANGO_ATTR_STRIKETHROUGH:
                strikethrough = getEnumAttr<bool>(p.get());
                break;
            default:
                break;
        }
    }
    if (!inlineFont) {
        // Use the global font in the font chooser button
        xoj::util::PangoFontDescriptionUPtr desc(pango_font_description_from_string(editor->getTextElement()->getFontName().c_str()));
        pango_font_description_set_absolute_size(desc.get(), editor->getTextElement()->getFontSize() * PANGO_SCALE);
        gtk_font_chooser_set_font_desc(GTK_FONT_CHOOSER(this->fontBtn), desc.get());
    }
    if (italic != this->italic) {
        this->italic = italic;
        setToggleButtonState(tglItalicBtn, this->italic);
    }
    if (underlined != this->underlined) {
        this->underlined = underlined;
        setToggleButtonState(tglUnderlineBtn, this->underlined);
    }
    if (strikethrough != this->strikethrough) {
        this->strikethrough = strikethrough;
        setToggleButtonState(tglStrikethrough, this->strikethrough);
    }
    if (bold != this->bold) {
        this->bold = bold;
        setToggleButtonState(tglBoldBtn, this->bold);
    }
    if (color != this->color) {
        this->color = color;
        gtk_image_set_from_pixbuf(this->colorIcon, newTextColorIcon(this->color).get());
    }
}

void TextEditorContextMenu::switchAlignmentButtons(TextAlignment alignment) {
    g_simple_action_set_state(alignmentAction.get(), makeGVariant(alignment));
}
