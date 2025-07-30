#include <gtk/gtk.h>

#include "stroke-demo.h"

#if GTK_MAJOR_VERSION == 3
#include "util/gtk4_helper.h"
#endif

static constexpr int SIZE = 800;                   ///< width/height in pixels
static constexpr size_t N_STROKES = 150;           ///< Number of strokes
static constexpr size_t FPS_UPDATE_PERIOD = 3000;  ///< in ms

static StrokesDemo demo;
static int frame_nb = 0;
static int rendererId = -1;
static GtkLabel *fpsLabel, *rendererLabel;
static gint64 last_time;

static gboolean timeout(gpointer data) {
    gint64 delay = g_get_monotonic_time() - last_time;
    static char buf[20];
    sprintf(buf, "%f", (double)frame_nb * 1000000. / (double)delay);
    printf("Frames: %d -- FPS: %s\n", frame_nb, buf);
    gtk_label_set_text(fpsLabel, buf);
    return G_SOURCE_CONTINUE;
}

static gboolean refresh(gpointer w) {
    if (GTK_IS_WIDGET(w)) {
        gtk_widget_queue_draw(GTK_WIDGET(w));
        return G_SOURCE_CONTINUE;
    } else {
        return G_SOURCE_REMOVE;
    }
}

static void draw_fun(GtkDrawingArea* w, cairo_t* cr, int width, int height, gpointer) {
    // background
    cairo_set_source_rgb(cr, 1., 1., 1.);
    cairo_paint(cr);

    demo.tick(width, height);
    demo.draw(&demo, cr);

    frame_nb++;
}

static void clicked(GtkButton*, gpointer) {
    rendererId = (rendererId + 1) % 4;
    demo.setActiveRenderer(rendererId);
    const char* render = StrokesDemo::getRendererName(rendererId);
    printf("Renderer: %s\n", render);
    gtk_label_set_text(rendererLabel, render);
    last_time = g_get_monotonic_time();
    frame_nb = 0;
}

#if GTK_MAJOR_VERSION == 3
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
static bool isX11() { return GDK_IS_X11_DISPLAY(gdk_display_get_default()); }
#else
static bool isX11() { return false; }
#endif

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
static bool isWayland() { return GDK_IS_WAYLAND_DISPLAY(gdk_display_get_default()); }
#else
static bool isWayland() { return false; }
#endif

#ifdef GDK_WINDOWING_QUARTZ
#include <gdk/gdkquartz.h>
static bool isQuartz() { return GDK_IS_QUARTZ_DISPLAY(gdk_display_get_default()); }
#else
static bool isQuartz() { return false; }
#endif

#ifdef GDK_WINDOWING_BROADWAY
#include <gdk/gdkbroadway.h>
static bool isBroadway() { return GDK_IS_BROADWAY_DISPLAY(gdk_display_get_default()); }
#else
static bool isBroadway() { return false; }
#endif

#ifdef GDK_WINDOWING_WIN32
#include <gdk/gdkwin32.h>
static bool isWin32() { return GDK_IS_WIN32_DISPLAY(gdk_display_get_default()); }
#else
static bool isWin32() { return false; }
#endif

static const char* getGdkBackend() {
    if (gdk_display_get_default()) {
        if (isX11()) {
            return "X11";
        } else if (isWayland()) {
            return "Wayland";
        } else if (isBroadway()) {
            return "Broadway";
        } else if (isQuartz()) {
            return "Quartz";
        } else if (isWin32()) {
            return "Win32";
        } else {
            return "Unknown";
        }
    } else {
        return nullptr;
    }
}

static const char* getSurfaceTypeName(cairo_surface_t* surf) {
    switch (cairo_surface_get_type(surf)) {
        // Strings from https://cairographics.org/manual/cairo-cairo-surface-t.html#cairo-surface-type-t
        case CAIRO_SURFACE_TYPE_IMAGE:
            return "image";
        case CAIRO_SURFACE_TYPE_PDF:
            return "pdf";
        case CAIRO_SURFACE_TYPE_PS:
            return "ps";
        case CAIRO_SURFACE_TYPE_XLIB:
            return "xlib";
        case CAIRO_SURFACE_TYPE_XCB:
            return "xcb";
        case CAIRO_SURFACE_TYPE_GLITZ:
            return "glitz";
        case CAIRO_SURFACE_TYPE_QUARTZ:
            return "quartz";
        case CAIRO_SURFACE_TYPE_WIN32:
            return "win32";
        case CAIRO_SURFACE_TYPE_BEOS:
            return "beos";
        case CAIRO_SURFACE_TYPE_DIRECTFB:
            return "directfb";
        case CAIRO_SURFACE_TYPE_SVG:
            return "svg";
        case CAIRO_SURFACE_TYPE_OS2:
            return "os2";
        case CAIRO_SURFACE_TYPE_WIN32_PRINTING:
            return "win32 printing surface";
        case CAIRO_SURFACE_TYPE_QUARTZ_IMAGE:
            return "quartz_image";
        case CAIRO_SURFACE_TYPE_SCRIPT:
            return "script";
        case CAIRO_SURFACE_TYPE_QT:
            return "Qt";
        case CAIRO_SURFACE_TYPE_RECORDING:
            return "recording";
        case CAIRO_SURFACE_TYPE_VG:
            return "OpenVG surface";
        case CAIRO_SURFACE_TYPE_GL:
            return "OpenGL";
        case CAIRO_SURFACE_TYPE_DRM:
            return "Direct Render Manager";
        case CAIRO_SURFACE_TYPE_TEE:
            return "'tee' (a multiplexing surface)";
        case CAIRO_SURFACE_TYPE_XML:
            return "XML (for debugging)";
        case CAIRO_SURFACE_TYPE_SKIA:
            return "CAIRO_SURFACE_TYPE_SKIA";
        case CAIRO_SURFACE_TYPE_SUBSURFACE:
            return "subsurface";
        case CAIRO_SURFACE_TYPE_COGL:
            return "Cogl";
        default:
            return "Unknown surface type";
    }
}

static gboolean first_draw(GtkWidget* widget, cairo_t* cr, gpointer data) {
    char buf[256];
    sprintf(buf, "Backend: %s  --  surface: %s", getGdkBackend(), getSurfaceTypeName(cairo_get_target(cr)));
    gtk_label_set_text(GTK_LABEL(widget), buf);
    g_signal_handlers_disconnect_by_func(widget, (gpointer)first_draw, data);
    return false;
}
#endif

static void activate(GtkApplication* app, gpointer) {
    GtkWidget *window, *drawing_area, *boxv, *boxh, *btn, *lbl;

    demo.setParameters(N_STROKES, SIZE, SIZE);

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, SIZE, SIZE);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_fun, nullptr, nullptr);

    boxh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

    btn = gtk_button_new_with_label("change renderer");
    g_signal_connect(btn, "clicked", G_CALLBACK(clicked), nullptr);
    gtk_box_append(GTK_BOX(boxh), btn);

    rendererLabel = GTK_LABEL(gtk_label_new(StrokesDemo::getRendererName(rendererId)));
    gtk_box_append(GTK_BOX(boxh), GTK_WIDGET(rendererLabel));

    gtk_box_append(GTK_BOX(boxh), gtk_label_new("FPS: "));
    fpsLabel = GTK_LABEL(gtk_label_new("0.0"));
    gtk_box_append(GTK_BOX(boxh), GTK_WIDGET(fpsLabel));

    boxv = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_box_append(GTK_BOX(boxv), drawing_area);
    gtk_box_append(GTK_BOX(boxv), boxh);

    GtkWidget* backendLabel = gtk_label_new(nullptr);
    gtk_box_append(GTK_BOX(boxv), backendLabel);
#if GTK_MAJOR_VERSION == 3
    g_signal_connect(backendLabel, "draw", G_CALLBACK(first_draw), nullptr);
#else
    // TODO - use realize signal
#endif


    gtk_window_set_child(GTK_WINDOW(window), boxv);

    g_timeout_add(FPS_UPDATE_PERIOD, timeout, nullptr);
    g_idle_add(refresh, drawing_area);

    clicked(nullptr, nullptr);  // Print renderer

#if GTK_MAJOR_VERSION == 3
    gtk_widget_show_all(window);
#else
    gtk_window_present(GTK_WINDOW(window));
#endif
}

int main(int argc, char** argv) {
    GtkApplication* app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), nullptr);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
