#include "SearchControl.h"

#include <algorithm>  // for min
#include <memory>     // for __shared_ptr_access
#include <utility>    // for move

#include "model/Element.h"  // for Element, ELEMENT_TEXT
#include "model/Layer.h"    // for Layer
#include "model/Text.h"     // for Text
#include "model/XojPage.h"  // for XojPage
#include "view/TextView.h"  // for TextView

using std::string;

SearchControl::SearchControl(const PageRef& page, XojPdfPageSPtr pdf) {
    this->page = page;
    this->pdf = std::move(pdf);
}

SearchControl::~SearchControl() { freeSearchResults(); }

void SearchControl::freeSearchResults() { this->results.clear(); }

void SearchControl::paint(cairo_t* cr, double zoom, Color color) {
    // set the line always the same size on display
    cairo_set_line_width(cr, 1 / zoom);

    for (XojPdfRectangle rect: this->results) {
        cairo_rectangle(cr, rect.x1, rect.y1, rect.x2 - rect.x1, rect.y2 - rect.y1);
        Util::cairo_set_source_rgbi(cr, color);
        cairo_stroke_preserve(cr);
        Util::cairo_set_source_rgbi(cr, color, 0.3);
        cairo_fill(cr);
    }
}

auto SearchControl::search(std::string text, int* occures, double* top) -> bool {
    freeSearchResults();

    if (text.empty()) {
        return true;
    }

    if (this->pdf) {
        this->results = this->pdf->findText(text);
    }

    for (Layer* l: *this->page->getLayers()) {
        if (!l->isVisible()) {
            continue;
        }

        for (Element* e: l->getElements()) {
            if (e->getType() == ELEMENT_TEXT) {
                Text* t = dynamic_cast<Text*>(e);

                std::vector<XojPdfRectangle> textResult = xoj::view::TextView::findText(t, text);

                this->results.insert(this->results.end(), textResult.begin(), textResult.end());
            }
        }
    }

    if (occures) {
        *occures = this->results.size();
    }

    if (top) {
        if (this->results.empty()) {
            *top = 0;
        } else {

            XojPdfRectangle first = this->results[0];

            double min = first.y1;
            for (XojPdfRectangle rect: this->results) { min = std::min(min, rect.y1); }

            *top = min;
        }
    }

    return !this->results.empty();
}
