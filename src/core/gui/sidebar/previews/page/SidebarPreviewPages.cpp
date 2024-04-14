#include "SidebarPreviewPages.h"

#include <algorithm>  // for max
#include <map>        // for map
#include <memory>     // for uniqu...
#include <utility>    // for pair

#include <glib-object.h>  // for g_obj...

#include "control/Control.h"                                    // for Control
#include "control/ScrollHandler.h"                              // for Scrol...
#include "gui/Builder.h"                                        // for Builder
#include "gui/GladeGui.h"                                       // for GladeGui
#include "gui/sidebar/previews/base/SidebarPreviewBaseEntry.h"  // for Sideb...
#include "gui/sidebar/previews/base/SidebarToolbar.h"           // for Sideb...
#include "model/Document.h"                                     // for Document
#include "model/PageRef.h"                                      // for PageRef
#include "model/XojPage.h"                                      // for XojPage
#include "undo/SwapUndoAction.h"                                // for SwapU...
#include "undo/UndoRedoHandler.h"                               // for UndoR...
#include "util/Assert.h"                                        // for xoj_assert
#include "util/Util.h"                                          // for npos
#include "util/i18n.h"                                          // for _
#include "util/safe_casts.h"                                    // for as_signed

#include "SidebarPreviewPageEntry.h"  // for Sideb...

constexpr auto XML_FILE = "sidebar.ui";
constexpr auto MENU_ID = "PreviewPagesContextMenu";
constexpr auto TOOLBAR_ID = "PreviewPagesToolbar";

SidebarPreviewPages::SidebarPreviewPages(Control* control):
        SidebarPreviewBase(control), iconNameHelper(control->getSettings()) {
    Builder builder(control->getGladeSearchPath(), XML_FILE);

    GMenuModel* menu = G_MENU_MODEL(builder.get(MENU_ID));
    contextMenu.reset(gtk_popover_menu_new_from_model(menu), xoj::util::adopt);
    gtk_widget_set_parent(contextMenu.get(), miniaturesContainer.get());

    gtk_box_append(GTK_BOX(mainBox.get()), builder.get(TOOLBAR_ID));
}

SidebarPreviewPages::~SidebarPreviewPages() {
    gtk_widget_unparent(contextMenu.get());  // Prevents a warning...
}

void SidebarPreviewPages::enableSidebar() {
    SidebarPreviewBase::enableSidebar();

    pageSelected(this->selectedEntry);
}

auto SidebarPreviewPages::getName() -> std::string { return _("Page Preview"); }

auto SidebarPreviewPages::getIconName() -> std::string { return this->iconNameHelper.iconName("sidebar-page-preview"); }

void SidebarPreviewPages::updatePreviews() {
    this->previews.clear();

    Document* doc = this->getControl()->getDocument();
    doc->lock();
    size_t len = doc->getPageCount();
    for (size_t i = 0; i < len; i++) {
        auto p = std::make_unique<SidebarPreviewPageEntry>(this, doc->getPage(i), i);
        gtk_fixed_put(GTK_FIXED(this->miniaturesContainer.get()), p->getWidget(), 0, 0);
        this->previews.emplace_back(std::move(p));
    }
    doc->unlock();

    layout();
}

void SidebarPreviewPages::pageSizeChanged(size_t page) {
    if (page == npos || page >= this->previews.size()) {
        return;
    }
    auto& p = this->previews[page];
    p->updateSize();
    p->repaint();

    layout();
}

void SidebarPreviewPages::pageChanged(size_t page) {
    if (page == npos || page >= this->previews.size()) {
        return;
    }

    auto& p = this->previews[page];
    p->repaint();
}

void SidebarPreviewPages::pageDeleted(size_t page) {
    if (page >= previews.size()) {
        return;
    }

    previews.erase(previews.begin() + as_signed(page));

    // Unselect page, to prevent double selection displaying
    unselectPage();

    updateIndices();

    layout();
}

void SidebarPreviewPages::pageInserted(size_t page) {
    Document* doc = control->getDocument();
    doc->lock();

    auto p = std::make_unique<SidebarPreviewPageEntry>(this, doc->getPage(page), page);

    doc->unlock();

    gtk_fixed_put(GTK_FIXED(this->miniaturesContainer.get()), p->getWidget(), 0, 0);
    this->previews.insert(this->previews.begin() + as_signed(page), std::move(p));

    // Unselect page, to prevent double selection displaying
    unselectPage();

    updateIndices();

    layout();
}

/**
 * Unselect the last selected page, if any
 */
void SidebarPreviewPages::unselectPage() {
    for (auto& p: this->previews) {
        p->setSelected(false);
    }
}

void SidebarPreviewPages::pageSelected(size_t page) {
    if (this->selectedEntry != npos && this->selectedEntry < this->previews.size()) {
        this->previews[this->selectedEntry]->setSelected(false);
    }
    this->selectedEntry = page;

    if (!this->enabled) {
        return;
    }

    if (this->selectedEntry != npos && this->selectedEntry < this->previews.size()) {
        auto& p = this->previews[this->selectedEntry];
        p->setSelected(true);
        scrollToPreview(this);
    }
}

void SidebarPreviewPages::openPreviewContextMenu(double x, double y, GtkWidget* entry) {
    double newX, newY;
    gtk_widget_translate_coordinates(entry, miniaturesContainer.get(), x, y, &newX, &newY);
    GdkRectangle r = {round_cast<int>(newX), round_cast<int>(newY), 0, 0};
    gtk_popover_set_pointing_to(GTK_POPOVER(this->contextMenu.get()), &r);
    gtk_popover_popup(GTK_POPOVER(this->contextMenu.get()));
}

void SidebarPreviewPages::updateIndices() {
    size_t index = 0;
    for (auto& preview: this->previews) {
        dynamic_cast<SidebarPreviewPageEntry*>(preview.get())->setIndex(index++);
    }
}
