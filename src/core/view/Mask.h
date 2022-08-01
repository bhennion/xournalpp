/*
 * Xournal++
 *
 * Virtual class for showing overlays (e.g. active tools, selections and so on)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>

#include "util/Range.h"
#include "util/raii/CairoWrappers.h"

namespace xoj::view {

class Mask {
public:
    Mask() = default;
    Mask(cairo_surface_t* target, const Range& extent, double zoom, int DPIScaline,
         cairo_content_t contentType = CAIRO_CONTENT_ALPHA);
    cairo_t* get();
    bool isInitialized() const;
    void blitTo(cairo_t* targetCr) const;
    void wipe();

private:
    xoj::util::CairoSPtr cr;
};
};  // namespace xoj::view
