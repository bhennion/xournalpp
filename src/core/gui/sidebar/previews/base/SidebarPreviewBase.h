/*
 * Xournal++
 *
 * Base class for prviews in the sidebar
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <vector>   // for vector

#include <gtk/gtk.h>  // for GtkWidget, GtkAllocation

#include "gui/sidebar/AbstractSidebarPage.h"  // for AbstractSidebarPage
#include "model/DocumentChangeType.h"         // for DocumentChangeType
#include "util/Util.h"
#include "util/raii/GObjectSPtr.h"

class PdfCache;
class SidebarLayout;
class SidebarPreviewBaseEntry;
class Control;

class SidebarPreviewBase: public AbstractSidebarPage {
public:
    SidebarPreviewBase(Control* control);
    ~SidebarPreviewBase() override;

public:
    void enableSidebar() override;
    void disableSidebar() override;

    /**
     * Layout the pages to the current size of the sidebar
     */
    void layout() override;

    /**
     * Update the preview images
     */
    virtual void updatePreviews() = 0;

    /**
     * @overwrite
     */
    bool hasData() override;

    /**
     * @overwrite
     */
    GtkWidget* getWidget() override;

    /**
     * Gets the zoom factor for the previews
     */
    double getZoom() const;

    /**
     * Gets the PDF cache for preview rendering
     */
    PdfCache* getCache();

public:
    // DocumentListener interface (only the part handled by SidebarPreviewBase)
    void documentChanged(DocumentChangeType type) override;
    void pageInserted(size_t page) override;
    void pageDeleted(size_t page) override;

protected:
    /**
     * Timeout callback to scroll to a page
     */
    static bool scrollToPreview(SidebarPreviewBase* sidebar);

    /**
     * The size of the sidebar has chnaged
     */
    static void sizeChanged(GtkWidget* widget, GtkAllocation* allocation, SidebarPreviewBase* sidebar);

public:
    /**
     * Opens a context menu, at the current cursor position.
     */
    virtual void openPreviewContextMenu(double x, double y, GtkWidget* entry) = 0;

private:
    /**
     * The Zoom of the previews
     */
    double zoom = 0.15;

    /**
     * For preview rendering
     */
    std::unique_ptr<PdfCache> cache;

    /**
     * The layouting class for the prviews
     */
    SidebarLayout* layoutmanager = nullptr;

protected:
    /// The scrollable area with the miniatures
    xoj::util::WidgetSPtr scrollableBox;

    /// Main box, containing the scrollable area and the toolbar.
    xoj::util::WidgetSPtr mainBox;

    /// The widget within the scrollable area with the page miniatures
    xoj::util::WidgetSPtr miniaturesContainer;

    /**
     * The currently selected entry in the sidebar, starting from 0
     * -1 means no valid selection
     */
    size_t selectedEntry = npos;

    /**
     * The previews
     */
    std::vector<std::unique_ptr<SidebarPreviewBaseEntry>> previews;

    /**
     * The sidebar is enabled
     */
    bool enabled = false;

    friend class SidebarLayout;
};
