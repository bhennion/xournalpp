/*
 * Xournal++
 *
 * Displays a text Element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string
#include <vector>  // for vector

#include <pango/pangocairo.h>  // for PangoLayout, cairo_t

#include "util/raii/GObjectSPtr.h"

#include "View.h"  // for ElementView

class Text;
class XojPdfRectangle;

class xoj::view::TextView: public xoj::view::ElementView {
public:
    TextView(const Text* t);
    virtual ~TextView();

    /**
     * Draws a Text model to a cairo surface
     */
    void draw(const Context& ctx) const override;

    /**
     * Searches text within a Text model
     */
    static std::vector<XojPdfRectangle> findText(const Text* t, std::string& search);

    /**
     * Initialize a Pango layout
     */
    static xoj::util::GSPtr<PangoLayout> initPango(cairo_t* cr, const Text* t);

private:
    const Text* text;
};
