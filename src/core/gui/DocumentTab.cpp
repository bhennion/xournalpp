#include "DocumentTab.h"

#include "control/Control.h"
#include "control/DeviceListHelper.h"
#include "control/settings/Settings.h"
#include "control/zoom/ZoomControl.h"
#include "gui/sidebar/Sidebar.h"
#include "scroll/ScrollHandling.h"
#include "util/gtk4_helper.h"

#include "Builder.h"
#include "SearchBar.h"
#include "XournalView.h"

constexpr auto UI_FILE = "tabContent.glade";

DocumentTab::DocumentTab(GladeSearchpath* gladeSearchPath, Control* control, GtkBox* parent): control(control) {
    Builder builder(gladeSearchPath, UI_FILE);
    panedContainerWidget.reset(builder.get("panelMainContents"), xoj::util::ref);
    boxContainerWidget.reset(builder.get("mainContentContainer"), xoj::util::ref);
    mainContentWidget.reset(builder.get("boxContents"), xoj::util::ref);
    sidebarWidget.reset(builder.get("sidebar"), xoj::util::ref);
    mainWidget.reset(builder.get("tab"), xoj::util::ref);

    winXournal.reset(gtk_scrolled_window_new(), xoj::util::adopt);

    gtk_box_append(parent, mainWidget.get());

    setGtkTouchscreenScrollingForDeviceMapping();

    gtk_box_append(GTK_BOX(builder.get("boxContents")), winXournal.get());

    scrollHandling = std::make_unique<ScrollHandling>(GTK_SCROLLED_WINDOW(winXournal.get()));
    xournal = std::make_unique<XournalView>(winXournal.get(), control, scrollHandling.get());
    searchbar = std::make_unique<SearchBar>(control, builder);
    sidebar = std::make_unique<Sidebar>(control, builder);

    control->getZoomControl()->initZoomHandler(winXournal.get(), xournal.get(), control);
    gtk_widget_show_all(winXournal.get());

    setSidebarVisible(control->getSettings()->isSidebarVisible());
    updateScrollbarSidebarPosition();
}

DocumentTab::~DocumentTab() = default;

void DocumentTab::setSidebarVisible(bool visible) {
    if (!visible && (this->getSidebar() != nullptr)) {
        this->getSidebar()->saveSize();
    }

    if (visible != this->sidebarVisible) {
        // Due to a GTK bug, we can't just hide the sidebar widget in the GtkPaned.
        // If we do this, we create a dead region where the pane separator was previously.
        // In this region, we can't use the touchscreen to start horizontal strokes.
        // As such:
        if (!visible) {
// hide sidebar
#if GTK_MAJOR_VERSION == 3
            gtk_container_remove(GTK_CONTAINER(panedContainerWidget.get()), mainContentWidget.get());
#else
            if (control->getSettings()->isSidebarOnRight()) {
                gtk_paned_set_start_child(GTK_PANED(panedContainerWidget.get()), nullptr);
            } else {
                gtk_paned_set_end_child(GTK_PANED(panedContainerWidget.get()), nullptr);
            }
#endif
            gtk_box_remove(GTK_BOX(boxContainerWidget.get()), panedContainerWidget.get());
            gtk_box_append(GTK_BOX(boxContainerWidget.get()), mainContentWidget.get());
            this->sidebarVisible = false;
        } else {
            // show sidebar

            // Allocation is reset when we switch up the contained elements. Fetch the
            // width here in case we need it afterwards.
            int contentWidth = gtk_widget_get_width(boxContainerWidget.get());

            gtk_box_remove(GTK_BOX(boxContainerWidget.get()), mainContentWidget.get());

#if GTK_MAJOR_VERSION == 3
            if (control->getSettings()->isSidebarOnRight()) {
                gtk_paned_pack1(GTK_PANED(panedContainerWidget.get()), mainContentWidget.get(), true, false);
            } else {
                gtk_paned_pack2(GTK_PANED(panedContainerWidget.get()), mainContentWidget.get(), true, false);
            }
#else
            if (control->getSettings()->isSidebarOnRight()) {
                gtk_paned_set_start_child(GTK_PANED(panedContainerWidget.get()), mainContentWidget.get());
            } else {
                gtk_paned_set_end_child(GTK_PANED(panedContainerWidget.get()), mainContentWidget.get());
            }
#endif

            gtk_box_append(GTK_BOX(boxContainerWidget.get()), panedContainerWidget.get());
            this->sidebarVisible = true;

            updatePanedPosition(contentWidth);
        }
    }

    gtk_widget_set_visible(sidebarWidget.get(), visible);
}

bool DocumentTab::getSidebarVisible() const { return sidebarVisible; }

/**
 * Invert the position of the paned widget and disconnect from the signal.
 * @param handlerId should be the ID of the signal handler that should be disconnected.
 */
static void invertPanedPosition(GtkWidget* widget, GtkAllocation* allocation, gulong* handlerId) {
    int newDividerPos = allocation->width - gtk_paned_get_position(GTK_PANED(widget));
    gtk_paned_set_position(GTK_PANED(widget), newDividerPos);

    // We only need to switch the position once, so disconnect the signal right away.
    g_signal_handler_disconnect(widget, *handlerId);
}

void DocumentTab::updatePanedPosition(int contentWidth) {
    if (!this->control->getSettings()->isSidebarOnRight()) {
        // Sidebar is on the left side.
        gtk_paned_set_position(GTK_PANED(this->panedContainerWidget.get()),
                               this->control->getSettings()->getSidebarWidth());
    } else {
        // Sidebar is on the right side.
        if (contentWidth > 0) {
            int dividerPos = contentWidth - this->control->getSettings()->getSidebarWidth();
            gtk_paned_set_position(GTK_PANED(this->panedContainerWidget.get()), dividerPos);
        } else {
            // Allocation is unkown (window hasn't been shown yet). We have to wait for the signal.
            // Set position as if the sidebar was on the left side, and let the signal handler
            // simply invert the position when the allocation is known.
            gtk_paned_set_position(GTK_PANED(this->panedContainerWidget.get()),
                                   this->control->getSettings()->getSidebarWidth());
            gulong* signal_id = new gulong{};
            *signal_id = g_signal_connect_data(
                    this->panedContainerWidget.get(), "size-allocate",
                    xoj::util::wrap_for_g_callback_v<invertPanedPosition>, signal_id,
                    [](gpointer d, GClosure*) { delete reinterpret_cast<gulong*>(d); }, GConnectFlags(0));
        }
    }
}

void DocumentTab::updateScrollbarSidebarPosition() {
    // Part 1: update scrollbar position
    if (winXournal != nullptr) {
        GtkScrolledWindow* scrolledWindow = GTK_SCROLLED_WINDOW(winXournal.get());

        ScrollbarHideType type = this->control->getSettings()->getScrollbarHideType();

        bool scrollbarOnLeft = control->getSettings()->isScrollbarOnLeft();
        if (scrollbarOnLeft) {
            gtk_scrolled_window_set_placement(scrolledWindow, GTK_CORNER_TOP_RIGHT);
        } else {
            gtk_scrolled_window_set_placement(scrolledWindow, GTK_CORNER_TOP_LEFT);
        }

        gtk_widget_set_visible(gtk_scrolled_window_get_hscrollbar(scrolledWindow), !(type & SCROLLBAR_HIDE_HORIZONTAL));
        gtk_widget_set_visible(gtk_scrolled_window_get_vscrollbar(scrolledWindow), !(type & SCROLLBAR_HIDE_VERTICAL));

        gtk_scrolled_window_set_overlay_scrolling(scrolledWindow,
                                                  !control->getSettings()->isScrollbarFadeoutDisabled());
    }

    // Part 2: update sidebar position
    GtkPaned* paned = GTK_PANED(this->panedContainerWidget.get());

    // Allocation is reset when we switch up the contained elements. Fetch the
    // width here in case we need it afterwards.
    int contentWidth = gtk_widget_get_width(this->boxContainerWidget.get());

    bool sidebarRight = control->getSettings()->isSidebarOnRight();
    if (sidebarRight != (gtk_paned_get_child2(paned) == this->sidebarWidget.get())) {
        // switch sidebar and main content
        GtkWidget* sidebar = this->sidebarWidget.get();
        GtkWidget* mainContent = this->sidebarVisible ? this->mainContentWidget.get() : nullptr;
#if GTK_MAJOR_VERSION == 3
        if (this->sidebarVisible) {
            gtk_container_remove(GTK_CONTAINER(paned), sidebar);
            gtk_container_remove(GTK_CONTAINER(paned), mainContent);

            if (sidebarRight) {
                gtk_paned_pack1(paned, mainContent, true, false);
                gtk_paned_pack2(paned, sidebar, false, false);
            } else {
                gtk_paned_pack1(paned, sidebar, false, false);
                gtk_paned_pack2(paned, mainContent, true, false);
            }
        } else {
            // The sidebar is hidden. That means the paned widget only contains the
            // sidebar while the main contents are shown alone in the box container.
            gtk_container_remove(GTK_CONTAINER(paned), sidebar);

            if (sidebarRight) {
                gtk_paned_pack2(paned, sidebar, false, false);
            } else {
                gtk_paned_pack1(paned, sidebar, false, false);
            }
        }
#else
        gtk_paned_set_start_child(paned, nullptr);
        gtk_paned_set_end_child(paned, nullptr);
        if (sidebarRight) {
            gtk_paned_set_start_child(paned, mainContent);
            gtk_paned_set_resize_start_child(paned, true);
            gtk_paned_set_shrink_start_child(paned, false);
            gtk_paned_set_end_child(paned, sidebar);
            gtk_paned_set_resize_end_child(paned, false);
            gtk_paned_set_shrink_end_child(paned, false);
        } else {
            gtk_paned_set_end_child(paned, mainContent);
            gtk_paned_set_resize_end_child(paned, true);
            gtk_paned_set_shrink_end_child(paned, false);
            gtk_paned_set_start_child(paned, sidebar);
            gtk_paned_set_resize_start_child(paned, false);
            gtk_paned_set_shrink_start_child(paned, false);
        }
#endif
    }

    if (this->sidebarVisible) {
        updatePanedPosition(contentWidth);
    }
}

void DocumentTab::setGtkTouchscreenScrollingForDeviceMapping() {
    InputDeviceClass touchscreenClass =
            DeviceListHelper::getSourceMapping(GDK_SOURCE_TOUCHSCREEN, this->control->getSettings());

    setGtkTouchscreenScrollingEnabled(touchscreenClass == INPUT_DEVICE_TOUCHSCREEN &&
                                      !control->getSettings()->getTouchDrawingEnabled());
}

void DocumentTab::setGtkTouchscreenScrollingEnabled(bool enabled) {
    if (!control->getSettings()->getGtkTouchInertialScrollingEnabled()) {
        enabled = false;
    }
    gtk_scrolled_window_set_kinetic_scrolling(GTK_SCROLLED_WINDOW(winXournal.get()), enabled);
}

Layout* DocumentTab::getLayout() const { return this->xournal->getLayout(); }
Sidebar* DocumentTab::getSidebar() const { return this->sidebar.get(); }
SearchBar* DocumentTab::getSearchBar() const { return this->searchbar.get(); }
XournalView* DocumentTab::getXournal() const { return xournal.get(); }
GtkWidget* DocumentTab::getWidget() const { return mainWidget.get(); }
