#include "TextView.h"

#include <algorithm>  // for max
#include <cstddef>    // for size_t

#include <glib-object.h>  // for g_object_unref

#include "model/Text.h"           // for Text
#include "pdf/base/XojPdfPage.h"  // for XojPdfRectangle
#include "util/Color.h"           // for cairo_set_source_rgbi
#include "util/StringUtils.h"     // for StringUtils
#include "util/raii/CairoWrappers.h"
#include "util/raii/GObjectSPtr.h"
#include "view/View.h"            // for Context, OPACITY_NO_AUDIO, view

#include "filesystem.h"  // for path

using namespace xoj::view;

TextView::TextView(const Text* text): text(text) {}

TextView::~TextView() = default;

auto TextView::initPango(cairo_t* cr, const Text* t) -> xoj::util::GSPtr<PangoLayout> {
    auto layout = t->createPangoLayout();
    pango_cairo_update_layout(cr, layout.get());
    pango_context_set_matrix(pango_layout_get_context(layout.get()), nullptr);

    return layout;
}

void TextView::draw(const Context& ctx) const {
    if (text->isInEditing()) {
        // The drawing is handled by gui/TextEditor
        return;
    }

    xoj::util::CairoSaveGuard saveGuard(ctx.cr);

    // make elements without audio translucent when highlighting elements with audio
    if (ctx.fadeOutNonAudio && text->getAudioFilename().empty()) {
        cairo_set_operator(ctx.cr, CAIRO_OPERATOR_OVER);
        Util::cairo_set_source_rgbi(ctx.cr, text->getColor(), OPACITY_NO_AUDIO);
    } else {
        cairo_set_operator(ctx.cr, CAIRO_OPERATOR_SOURCE);
        Util::cairo_set_source_rgbi(ctx.cr, text->getColor());
    }

    cairo_translate(ctx.cr, text->getX(), text->getY());

    auto layout = initPango(ctx.cr, text);
    const std::string& content = text->getText();
    pango_layout_set_text(layout.get(), content.c_str(), static_cast<int>(content.length()));

    pango_cairo_show_layout(ctx.cr, layout.get());
}

auto TextView::findText(const Text* t, std::string& search) -> std::vector<XojPdfRectangle> {
    size_t patternLength = search.length();
    if (patternLength == 0) {
        return {};
    }

    auto layout = t->createPangoLayout();
    const std::string& content = t->getText();
    pango_layout_set_text(layout.get(), content.c_str(), static_cast<int>(content.length()));


    std::string text = StringUtils::toLowerCase(content);

    std::string pattern = StringUtils::toLowerCase(search);

    std::vector<XojPdfRectangle> list;

    for (size_t pos = text.find(pattern); pos != std::string::npos; pos = text.find(pattern, pos + 1)) {
        XojPdfRectangle mark;
        PangoRectangle rect = {0};
        pango_layout_index_to_pos(layout.get(), static_cast<int>(pos), &rect);
        mark.x1 = (static_cast<double>(rect.x)) / PANGO_SCALE + t->getX();
        mark.y1 = (static_cast<double>(rect.y)) / PANGO_SCALE + t->getY();

        pango_layout_index_to_pos(layout.get(), static_cast<int>(pos + patternLength - 1), &rect);
        mark.x2 = (static_cast<double>(rect.x) + rect.width) / PANGO_SCALE + t->getX();
        mark.y2 = (static_cast<double>(rect.y) + rect.height) / PANGO_SCALE + t->getY();

        list.push_back(mark);
    }

    return list;
}
