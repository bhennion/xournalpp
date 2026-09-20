#include "GladeGui.h"

#include <cstdlib>  // for exit

#include <glib-object.h>  // for g_object_unref
#include <glib.h>         // for g_error_free, GError, g_warning

#include "util/PlaceholderString.h"  // for PlaceholderString
#include "util/StringUtils.h"        // for char_cast
#include "util/XojMsgBox.h"          // for XojMsgBox
#include "util/i18n.h"               // for FS, _F

#include "GladeSearchpath.h"  // for GladeSearchpath
#include "filesystem.h"       // for path

GladeGui::GladeGui(GladeSearchpath*, const std::string& glade, const std::string& mainWnd) {
    GError* error = nullptr;
    builder.reset(gtk_builder_new(), xoj::util::adopt);

    if (auto p = std::string("/org/xournalpp/ui/") + glade;
    !gtk_builder_add_from_resource(builder.get(), p.c_str(), &error)) {
        std::string msg = FS(_F("Error loading glade file \"{1}\" (try to load \"{2}\")") % glade %
                             p);

        if (error != nullptr) {
            msg += "\n";
            msg += error->message;
            g_error_free(error);
        }
        XojMsgBox::showErrorAndQuit(msg, -1);
    }

    this->window = get(mainWnd);
}

GladeGui::~GladeGui() {
    if (GTK_IS_WINDOW(window)) {
        gtk_window_destroy(GTK_WINDOW(window));
    }
}

auto GladeGui::get(const std::string& name) -> GtkWidget* {
    GtkWidget* w = GTK_WIDGET(gtk_builder_get_object(builder.get(), name.c_str()));
    if (w == nullptr) {
        g_warning("GladeGui::get: Could not find glade Widget: \"%s\"", name.c_str());
    }
    return w;
}

auto GladeGui::getWindow() const -> GtkWidget* { return this->window; }

auto GladeGui::getBuilder() const -> GtkBuilder* { return this->builder.get(); }
