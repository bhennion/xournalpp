/*
 * Xournal++
 *
 * Previews of the pages in the document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <string>   // for string
#include <tuple>    // for tuple
#include <vector>   // for vector

#include <glib.h>     // for gulong
#include <gtk/gtk.h>  // for GtkWidget

#include "gui/IconNameHelper.h"                            // for IconNameHe...
#include "gui/sidebar/previews/base/SidebarPreviewBase.h"  // for SidebarPre...
#include "gui/sidebar/previews/base/SidebarToolbar.h"      // for SidebarAct...
#include "util/raii/GObjectSPtr.h"

class Control;
class GladeGui;


class SidebarPreviewPages: public SidebarPreviewBase {
public:
    SidebarPreviewPages(Control* control, GladeGui* gui, SidebarToolbar* toolbar);
    ~SidebarPreviewPages() override;

public:
    void enableSidebar() override;

    /**
     * @overwrite
     */
    std::string getName() override;

    /**
     * @overwrite
     */
    std::string getIconName() override;

    /**
     * Update the preview images
     * @overwrite
     */
    void updatePreviews() override;

    /**
     * Opens the page preview context menu, at the current cursor position, for
     * the given page.
     */
    void openPreviewContextMenu(double x, double y, GtkWidget* entry) override;

public:
    // DocumentListener interface (only the part which is not handled by SidebarPreviewBase)
    void pageSizeChanged(size_t page) override;
    void pageChanged(size_t page) override;
    void pageSelected(size_t page) override;
    void pageInserted(size_t page) override;
    void pageDeleted(size_t page) override;

private:
    /**
     * Unselect the last selected page, if any
     */
    void unselectPage();

    /**
     * Updates the indices of the pages
     */
    void updateIndices();

private:
    /**
     * The context menu to display when a page is right-clicked.
     */
    xoj::util::WidgetSPtr contextMenu;

    IconNameHelper iconNameHelper;
};
