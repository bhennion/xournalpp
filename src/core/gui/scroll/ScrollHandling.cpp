#include "ScrollHandling.h"

#include "gui/Layout.h"


ScrollHandling::ScrollHandling(GtkAdjustment* adjHorizontal, GtkAdjustment* adjVertical):
adjHorizontal(adjHorizontal, xoj::util::refsink), adjVertical(adjVertical, xoj::util::refsink) {}

ScrollHandling::ScrollHandling(GtkScrolledWindow* win):
        ScrollHandling(gtk_scrolled_window_get_hadjustment(win), gtk_scrolled_window_get_vadjustment(win)) {}


ScrollHandling::~ScrollHandling() = default;

auto ScrollHandling::getHorizontal() const -> GtkAdjustment* { return adjHorizontal.get(); }

auto ScrollHandling::getVertical() const -> GtkAdjustment* { return adjVertical.get(); }

void ScrollHandling::setHorizontal(GtkAdjustment* adj) {
    adjHorizontal.reset(adj, xoj::util::refsink);
}
void ScrollHandling::setVertical(GtkAdjustment* adj) {
    adjVertical.reset(adj, xoj::util::refsink);
}
