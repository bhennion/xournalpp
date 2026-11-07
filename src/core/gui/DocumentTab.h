/*
 * Xournal++
 *
 * A frame containing a document's view
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <atomic>
#include <memory>  // for unique_ptr

#include <gtk/gtk.h>  // for GtkWidget, GtkCheckMenu...

#include "util/raii/GObjectSPtr.h"

class Control;
class Layout;
class ScrollHandling;
class XournalView;
class GladeSearchpath;
class SearchBar;
class Sidebar;

class DocumentTab final {
public:
    DocumentTab(GladeSearchpath* gladeSearchPath, Control* control, GtkBox* parent);
    ~DocumentTab();

public:
    void setSidebarVisible(bool visible);
    bool getSidebarVisible() const;

    void updateScrollbarSidebarPosition();

    Layout* getLayout() const;
    Sidebar* getSidebar() const;
    SearchBar* getSearchBar() const;
    XournalView* getXournal() const;
    GtkWidget* getWidget() const;


    /**
     * Disable kinetic scrolling if there is a touchscreen device that was manually mapped to another enabled input
     * device class. This is required so the GtkScrolledWindow does not swallow all the events.
     */
    void setGtkTouchscreenScrollingForDeviceMapping();
    void setGtkTouchscreenScrollingEnabled(bool enabled);

private:
    /**
     * Update the position of the separator in the paned container, adjusting it to the saved sidebar width.
     * @param contentWidth should be the width of the paned container. The caller should retrieve the width
     * of the container before any modifications to it, as that will reset its allocation.
     */
    void updatePanedPosition(int contentWidth);

private:
    Control* control;

    std::unique_ptr<XournalView> xournal;
    std::unique_ptr<ScrollHandling> scrollHandling;
    std::unique_ptr<SearchBar> searchbar;
    std::unique_ptr<Sidebar> sidebar;

    bool sidebarVisible = true;
    std::atomic_bool gtkTouchscreenScrollingEnabled{true};

    xoj::util::WidgetSPtr boxContainerWidget;
    xoj::util::WidgetSPtr panedContainerWidget;
    xoj::util::WidgetSPtr mainContentWidget;
    xoj::util::WidgetSPtr sidebarWidget;
    xoj::util::WidgetSPtr mainWidget;
    xoj::util::WidgetSPtr winXournal;
};
