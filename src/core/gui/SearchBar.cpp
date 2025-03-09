#include "SearchBar.h"

#include <string>  // for allocator, string

#include <gdk/gdk.h>         // for GdkEventKey, GDK_SHIFT_MASK
#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Return
#include <glib.h>            // for g_free, g_strdup_printf

#include "control/Control.h"         // for Control
#include "control/ScrollHandler.h"   // for ScrollHandler
#include "gui/MainWindow.h"          // for MainWindow
#include "model/Document.h"          // for Document
#include "util/PlaceholderString.h"  // for PlaceholderString
#include "util/i18n.h"               // for _, FC, _F
#include "util/gtk-signals.h"  // for xoj_signal_connect

SearchBar::SearchBar(Control* control): control(control) {
    MainWindow* win = control->getWindow();

    GtkWidget* close = win->get("buttonCloseSearch");
    xoj_signal_connect(GTK_BUTTON(close), "clicked", xoj::util::wrap_v<buttonCloseSearchClicked>, this);

    GtkWidget* next = win->get("btSearchForward");
    GtkWidget* previous = win->get("btSearchBack");
    xoj_signal_connect(GTK_BUTTON(next), "clicked", +[](GtkButton* button, gpointer self) { static_cast<SearchBar*>(self)->searchNext(); }, this);
    xoj_signal_connect(GTK_BUTTON(previous), "clicked", +[](GtkButton* button, gpointer self) { static_cast<SearchBar*>(self)->searchPrevious(); }, this);

    // TODO(fabian): When keybindings are implemented, handle previous search keybinding properly
    GtkWidget* searchTextField = win->get("searchTextField");
    xoj_signal_connect(GTK_SEARCH_ENTRY(searchTextField), "search-changed", xoj::util::wrap_v<searchTextChangedCallback>, this);
    // Enable next/previous search when pressing Enter / Shift+Enter
    xoj_signal_connect(searchTextField, "key-press-event",
                     +[](GtkWidget* entry, GdkEvent* e, gpointer s) -> gboolean {
                         auto* event = (GdkEventKey*)e;
                         auto* self = static_cast<SearchBar*>(s);
                         if (event->keyval == GDK_KEY_Return) {
                             if (event->state & GDK_SHIFT_MASK) {
                                 self->searchPrevious();
                             } else {
                                 self->searchNext();
                             }
                             // Grab focus again since searching will take away focus
                             gtk_widget_grab_focus(entry);
                             return true;
                         } else if (event->keyval == GDK_KEY_Escape) {
                             self->showSearchBar(false);
                             return true;
                         }
                         return false;
                     },
                     this);

    cssTextFild = gtk_css_provider_new();
    gtk_style_context_add_provider(gtk_widget_get_style_context(win->get("searchTextField")),
                                   GTK_STYLE_PROVIDER(cssTextFild), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

SearchBar::~SearchBar() { this->control = nullptr; }

auto SearchBar::searchTextonCurrentPage(const char* text, size_t* occurrences, double* yOfUpperMostMatch) -> bool {
    size_t p = control->getCurrentPageNo();

    return control->searchTextOnPage(text, p, occurrences, yOfUpperMostMatch);
}

void SearchBar::search(const char* text) {
    MainWindow* win = control->getWindow();
    GtkWidget* lbSearchState = win->get("lbSearchState");

    bool found = true;
    size_t occurrences = 0;

    if (*text != 0) {
        found = searchTextonCurrentPage(text, &occurrences, nullptr);
        if (found) {
            if (occurrences == 1) {
                gtk_label_set_text(GTK_LABEL(lbSearchState), _("Text found on this page"));
            } else {
                char* msg = g_strdup_printf(_("Text %zu times found on this page"), occurrences);
                gtk_label_set_text(GTK_LABEL(lbSearchState), msg);
                g_free(msg);
            }
        } else {
            gtk_label_set_text(GTK_LABEL(lbSearchState), _("Text not found"));
        }
    } else {
        searchTextonCurrentPage("", nullptr, nullptr);
        gtk_label_set_text(GTK_LABEL(lbSearchState), "");
    }

    if (found) {
        gtk_css_provider_load_from_data(cssTextFild, "GtkSearchEntry {}", -1, nullptr);
    } else {
        gtk_css_provider_load_from_data(cssTextFild, "GtkSearchEntry { color: #ff0000; }", -1, nullptr);
    }
}

void SearchBar::searchTextChangedCallback(GtkSearchEntry* entry, SearchBar* searchBar) {
    const char* text = gtk_entry_get_text(GTK_ENTRY(entry));
    searchBar->search(text);
}

void SearchBar::buttonCloseSearchClicked(GtkButton* button, SearchBar* searchBar) { searchBar->showSearchBar(false); }

template <class Fun>
void SearchBar::search(Fun next) const {
    size_t currentPage = control->getCurrentPageNo();
    size_t count = control->getDocument()->getPageCount();
    if (count < 2) {
        // Nothing to do
        return;
    }

    MainWindow* win = control->getWindow();
    GtkWidget* searchTextField = win->get("searchTextField");
    const char* text = gtk_entry_get_text(GTK_ENTRY(searchTextField));
    GtkWidget* lbSearchState = win->get("lbSearchState");
    if (*text == 0) {
        return;
    }

    double yOfUpperMostMatch = 0;
    size_t occurrences = 0;

    // Search backwards through the pages, wrapping around if needed.
    for (size_t searchedPage = next(currentPage); searchedPage != currentPage; searchedPage = next(searchedPage)) {

        bool found = control->searchTextOnPage(text, searchedPage, &occurrences, &yOfUpperMostMatch);
        if (found) {
            control->getScrollHandler()->scrollToPage(searchedPage, yOfUpperMostMatch);
            gtk_label_set_text(
                    GTK_LABEL(lbSearchState),
                    (occurrences == 1 ? FC(_F("Text found once on page {1}") % (searchedPage + 1)) :
                                        FC(_F("Text found {1} times on page {2}") % occurrences % (searchedPage + 1))));
            return;
        }
    }

    gtk_label_set_text(GTK_LABEL(lbSearchState), _("Text not found, searched on all pages"));
}

void SearchBar::searchNext() const {
    size_t count = control->getDocument()->getPageCount();
    auto next = [count](size_t n) { return (n + 1) % count; };
    search(next);
}

void SearchBar::searchPrevious() const {
    size_t count = control->getDocument()->getPageCount();
    auto backwardsNext = [count](size_t n) { return n == 0 ? count - 1 : n - 1; };
    search(backwardsNext);
}

void SearchBar::showSearchBar(bool show) {
    MainWindow* win = control->getWindow();
    GtkWidget* searchBar = win->get("searchBar");

    if (show) {
        GtkWidget* searchTextField = win->get("searchTextField");
        gtk_widget_show_all(searchBar);
        gtk_widget_grab_focus(searchTextField);
    } else {
        gtk_widget_hide(searchBar);
        for (int i = control->getDocument()->getPageCount() - 1; i >= 0; i--) {
            control->searchTextOnPage("", i, nullptr, nullptr);
        }
    }
}
