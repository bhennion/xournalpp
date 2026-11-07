/*
 * Xournal++
 *
 * The Main window
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>    // for array
#include <atomic>   // for atomic_bool
#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <string>   // for string

#include <gdk/gdk.h>      // for GdkDragContext, GdkEvent
#include <glib-object.h>  // for GClosure
#include <glib.h>         // for gpointer, gboolean, gint
#include <gtk/gtk.h>      // for GtkWidget, GtkCheckMenu...

#include "util/Point.h"
#include "util/raii/GObjectSPtr.h"

#include "GladeGui.h"            // for GladeGui
#include "ToolbarDefinitions.h"  // for TOOLBAR_DEFINITIONS_LEN

class Control;
class Layout;
class SpinPageAdapter;
class ScrollHandling;
class ToolMenuHandler;
class ToolbarData;
class ToolbarModel;
class XournalView;
class PdfFloatingToolbox;
class FloatingToolbox;
class GladeSearchpath;
class StatusBar;
class DocumentTab;

class Menubar;

typedef std::array<xoj::util::WidgetSPtr, TOOLBAR_DEFINITIONS_LEN> ToolbarWidgetArray;

class MainWindow: public GladeGui {
public:
    MainWindow(GladeSearchpath* gladeSearchPath, Control* control, GtkApplication* parent);
    ~MainWindow() override;

    void populate(GladeSearchpath* gladeSearchPath);

public:
    GMenuModel* getMenuModel() const;

    void show(GtkWindow* parent) override;

    void toolbarSelected(const std::string& id);
    void toolbarSelected(ToolbarData* d);
    ToolbarData* getSelectedToolbar() const;

private:
    const ToolbarData* clearToolbar();
    void loadToolbar(ToolbarData* d);

public:
    /**
     * reloadToolbars reloads the currently selected toolbar
     *
     * This is especially useful when a change in the SettingsDialog should be reflected right away
     */
    void reloadToolbars();

    void updatePageNumbers(size_t page, size_t pagecount, size_t pdfpage);

    void setMaximized(bool maximized);
    bool isMaximized() const;

    void setFullscreen(bool enabled) const;

    bool isDarkTheme() const;

    XournalView* getXournal() const;

    void setMenubarVisible(bool visible);
    void setToolbarVisible(bool visible);

    Control* getControl() const;

    PdfFloatingToolbox* getPdfToolbox() const;
    FloatingToolbox* getFloatingToolbox() const;

    void setUndoDescription(const std::string& description);
    void setRedoDescription(const std::string& description);

    ToolbarModel* getToolbarModel() const;
    ToolMenuHandler* getToolMenuHandler() const;

    void setDynamicallyGeneratedSubmenuDisabled(bool disabled);

    void updateToolbarMenu();
    void updateColorscheme();

    const ToolbarWidgetArray& getToolbarWidgets() const;
    const char* getToolbarName(GtkToolbar* toolbar) const;

    [[maybe_unused]] Menubar* getMenubar() const;

    /// Infer the window's DPI from available monitor info and use it to set the default zoom value.
    void setDPI() const;

    inline StatusBar* getStatusBar() const { return statusbar.get(); }
    inline DocumentTab* getDocumentTab() const { return documentTab.get(); }

private:
    void createToolbar();

    /**
     * Window close Button is pressed
     */
    static bool deleteEventCallback(GtkWidget* widget, GdkEvent* event, Control* control);

    /**
     * Window is maximized/minimized
     */
    static void windowMaximizedCallback(GObject* window, GParamSpec*, MainWindow* win);

    /**
     * Callback for drag & drop files
     */
    static void dragDataRecived(GtkWidget* widget, GdkDragContext* dragContext, gint x, gint y, GtkSelectionData* data,
                                guint info, guint time, MainWindow* win);

    /**
     * Load Overall CSS file with custom icons, other styling and potentially, user changes
     */
    static void loadMainCSS(GladeSearchpath* gladeSearchPath, const gchar* cssFilename);

private:
    Control* control;

    std::unique_ptr<PdfFloatingToolbox> pdfFloatingToolBox;
    std::unique_ptr<FloatingToolbox> floatingToolbox;

    // Toolbars
    std::unique_ptr<ToolMenuHandler> toolbar;
    ToolbarData* selectedToolbar = nullptr;

    std::unique_ptr<Menubar> menubar;

    bool maximized = false;
    bool darkMode = false;
    bool modifiedGtkSettingsTheme = false;

    ToolbarWidgetArray toolbarWidgets;

    /// The last monitor the window has been moved to -- used for setting dpi
    GdkMonitor* lastMonitor = nullptr;

    std::unique_ptr<StatusBar> statusbar;
    std::unique_ptr<DocumentTab> documentTab;
};
