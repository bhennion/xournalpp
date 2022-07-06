#include "BaseStrokeToolView.h"

#include <cmath>

#include <cairo.h>

#include "model/Stroke.h"
#include "util/Range.h"
#include "view/Repaintable.h"
#include "view/overlays/OverlayView.h"

using namespace xoj::view;

static Color strokeColorWithAlpha(const Stroke& s) {
    Color c = s.getColor();
    if (s.getToolType() == STROKE_TOOL_HIGHLIGHTER) {
        c.alpha = s.getFill() == -1 ? 120U : static_cast<uint8_t>(s.getFill());
    } else {
        c.alpha = 255U;
    }
    return c;
}

BaseStrokeToolView::BaseStrokeToolView(const Repaintable* parent, const Stroke& stroke):
        OverlayView(parent),
        cairoOp(stroke.getToolType() == STROKE_TOOL_HIGHLIGHTER ? CAIRO_OPERATOR_MULTIPLY : CAIRO_OPERATOR_OVER),
        strokeColor(strokeColorWithAlpha(stroke)),
        lineStyle(stroke.getLineStyle()),
        strokeWidth(stroke.getWidth()) {}

BaseStrokeToolView::~BaseStrokeToolView() noexcept = default;

static void printSurfaceType(cairo_surface_t*);

auto BaseStrokeToolView::createMask(cairo_t* tgtcr) const -> Mask {
    const double scale = this->parent->getAbsoluteScale();
    Range visibleRange = this->parent->getVisiblePart();
    visibleRange.addPadding(0.5 * this->strokeWidth);

    const int xOffset = -static_cast<int>(std::floor(visibleRange.minX * scale));
    const int yOffset = -static_cast<int>(std::floor(visibleRange.minY * scale));
    const int width = static_cast<int>(std::ceil(visibleRange.maxX * scale)) + xOffset;
    const int height = static_cast<int>(std::ceil(visibleRange.maxY * scale)) + yOffset;

    cairo_surface_t* surf;
    if (tgtcr) {
        surf = cairo_surface_create_similar(cairo_get_target(tgtcr), CAIRO_CONTENT_ALPHA, width, height);
        printf("Creating mask of type: ");
        printSurfaceType(surf);
    } else {
        surf = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);
        printf("Creating mask of default image type\n");
    }

    Mask mask(cairo_create(surf));
    cairo_t* cr = mask.get();
    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_surface_set_device_offset(surf, xOffset, yOffset);
    cairo_surface_set_device_scale(surf, scale, scale);
    cairo_surface_destroy(surf);  // surf is now owned by cr

    return mask;
}

static void printSurfaceType(cairo_surface_t* surf) {
    auto surftype = cairo_surface_get_type(surf);
    switch (surftype) {
        case CAIRO_SURFACE_TYPE_IMAGE:
            printf("image\n");
            break;
        case CAIRO_SURFACE_TYPE_PDF:
            printf("pdf\n");
            break;
        case CAIRO_SURFACE_TYPE_PS:
            printf("ps\n");
            break;
        case CAIRO_SURFACE_TYPE_XLIB:
            printf("xlib\n");
            break;
        case CAIRO_SURFACE_TYPE_XCB:
            printf("xcb\n");
            break;
        case CAIRO_SURFACE_TYPE_GLITZ:
            printf("glitz\n");
            break;
        case CAIRO_SURFACE_TYPE_QUARTZ:
            printf("quartz\n");
            break;
        case CAIRO_SURFACE_TYPE_WIN32:
            printf("win32\n");
            break;
        case CAIRO_SURFACE_TYPE_BEOS:
            printf("beos\n");
            break;
        case CAIRO_SURFACE_TYPE_DIRECTFB:
            printf("directfb\n");
            break;
        case CAIRO_SURFACE_TYPE_SVG:
            printf("svg\n");
            break;
        case CAIRO_SURFACE_TYPE_OS2:
            printf("os2\n");
            break;
        case CAIRO_SURFACE_TYPE_WIN32_PRINTING:
            printf("win32 printing surface\n");
            break;
        case CAIRO_SURFACE_TYPE_QUARTZ_IMAGE:
            printf("quartz_image\n");
            break;
        case CAIRO_SURFACE_TYPE_SCRIPT:
            printf("script\n");
            break;
        case CAIRO_SURFACE_TYPE_QT:
            printf("Qt\n");
            break;
        case CAIRO_SURFACE_TYPE_RECORDING:
            printf("recording\n");
            break;
        case CAIRO_SURFACE_TYPE_VG:
            printf("OpenVG surface\n");
            break;
        case CAIRO_SURFACE_TYPE_GL:
            printf("OpenGL\n");
            break;
        case CAIRO_SURFACE_TYPE_DRM:
            printf("Direct Render Manager\n");
            break;
        case CAIRO_SURFACE_TYPE_TEE:
            printf("'tee' (a multiplexing surface)\n");
            break;
        case CAIRO_SURFACE_TYPE_XML:
            printf("XML (for debugging)\n");
            break;
        case CAIRO_SURFACE_TYPE_SKIA:
            printf("CAIRO_SURFACE_TYPE_SKIA\n");
            break;
        case CAIRO_SURFACE_TYPE_SUBSURFACE:
            printf("subsurface created with cairo_surface_create_for_rectangle()\n");
            break;
        case CAIRO_SURFACE_TYPE_COGL:
            printf("Cogl\n");
            break;
        default:
            printf("Unknown surface type\n");
    }
}
