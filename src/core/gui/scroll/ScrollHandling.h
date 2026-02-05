/*
 * Xournal++
 *
 * Scroll handling for different scroll implementations
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>  // for GtkAdjustment, GtkWidget, GtkScrollable

#include "util/raii/GObjectSPtr.h"

class Layout;

class ScrollHandling {
public:
    ScrollHandling(GtkAdjustment* adjHorizontal, GtkAdjustment* adjVertical);
    ScrollHandling(GtkScrolledWindow* scrollable);
    ~ScrollHandling();

public:
    GtkAdjustment* getHorizontal() const;
    GtkAdjustment* getVertical() const;
    void setHorizontal(GtkAdjustment* adj);
    void setVertical(GtkAdjustment* adj);

private:
    xoj::util::GObjectSPtr<GtkAdjustment> adjHorizontal;
    xoj::util::GObjectSPtr<GtkAdjustment> adjVertical;
};
