#include "SidebarPreviewBaseEntry.h"

#include <glib-object.h>  // for G_CALLBACK, g_object_ref
#include <gtk/gtk.h>      //

#include "control/Control.h"                // for Control
#include "control/jobs/XournalScheduler.h"  // for XournalScheduler
#include "model/XojPage.h"                  // for XojPage
#include "util/safe_casts.h"                // for floor_cast

#include "SidebarPreviewBase.h"  // for SidebarPreviewBase

SidebarPreviewBaseEntry::SidebarPreviewBaseEntry(SidebarPreviewBase* sidebar, const PageRef& page):
        sidebar(sidebar), page(page), widget(gtk_button_new(), xoj::util::adopt) {
    // A spinner while it's loading
    auto* spin = gtk_spinner_new();
    gtk_spinner_start(GTK_SPINNER(spin));
    gtk_button_set_child(GTK_BUTTON(widget.get()), spin);
    gtk_button_set_has_frame(GTK_BUTTON(widget.get()), false);

    updateSize();

    g_signal_connect(this->widget.get(), "clicked", G_CALLBACK(+[](GtkButton*, gpointer self) {
                         static_cast<SidebarPreviewBaseEntry*>(self)->mouseButtonPressCallback();
                         return true;
                     }),
                     this);

    // Set up right button clicks to pop up the context menu
    auto* ctrl = gtk_gesture_click_new();
    gtk_widget_add_controller(widget.get(), GTK_EVENT_CONTROLLER(ctrl));
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(ctrl), 3);  // 3 = right button
    g_signal_connect(ctrl, "pressed",
                     G_CALLBACK(+[](GtkGestureClick* g, gint n_press, gdouble x, gdouble y, gpointer d) {
                         if (n_press == 1) {
                             auto* self = static_cast<SidebarPreviewBaseEntry*>(d);
                             self->mouseButtonPressCallback();
                             self->sidebar->openPreviewContextMenu(
                                     x, y, gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(g)));
                         }
                     }),
                     this);

    repaint();
}

SidebarPreviewBaseEntry::~SidebarPreviewBaseEntry() {
    this->sidebar->getControl()->getScheduler()->removeSidebar(this);
    gtk_fixed_remove(GTK_FIXED(gtk_widget_get_parent(widget.get())), widget.get());
}

void SidebarPreviewBaseEntry::setSelected(bool selected) {
    if (this->selected == selected) {
        return;
    }
    this->selected = selected;
    if (selected) {
        gtk_widget_add_css_class(this->widget.get(), "page-selected");
    } else {
        gtk_widget_remove_css_class(this->widget.get(), "page-selected");
    }
}

void SidebarPreviewBaseEntry::repaint() { sidebar->getControl()->getScheduler()->addRepaintSidebar(this); }

void SidebarPreviewBaseEntry::updateSize() {
    this->DPIscaling = gtk_widget_get_scale_factor(this->widget.get());
    // To avoid having a black line, we use floor rather than ceil
    this->imageWidth = floor_cast<int>(page->getWidth() * sidebar->getZoom());
    this->imageHeight = floor_cast<int>(page->getHeight() * sidebar->getZoom());
    gtk_widget_set_size_request(gtk_button_get_child(GTK_BUTTON(this->widget.get())), imageWidth, imageHeight);
}

auto SidebarPreviewBaseEntry::getWidth() -> int { return this->imageWidth; }

auto SidebarPreviewBaseEntry::getHeight() -> int { return this->imageHeight; }

auto SidebarPreviewBaseEntry::getWidgetWidth() -> int { return gtk_widget_get_width(widget.get()); }

auto SidebarPreviewBaseEntry::getWidgetHeight() -> int { return gtk_widget_get_height(widget.get()); }

auto SidebarPreviewBaseEntry::getWidget() -> GtkWidget* { return this->widget.get(); }

void SidebarPreviewBaseEntry::setChild(xoj::util::WidgetSPtr child) {
    Util::execInUiThread(
            [w = this->widget, child = std::move(child)]() { gtk_button_set_child(GTK_BUTTON(w.get()), child.get()); });
}
