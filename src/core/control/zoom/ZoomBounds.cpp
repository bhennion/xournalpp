#include "ZoomBounds.h"

#include <algorithm>  // for find, clamp
#include <cmath>      // for isfinite

#include <glib.h>  // for g_warning


double ZoomBounds::getMin() const { return value100percent * DEFAULT_ZOOM_MIN; }
double ZoomBounds::getMax() const { return value100percent * DEFAULT_ZOOM_MAX; }
double ZoomBounds::convertToZoomReal(double zoom) const { return zoom / value100percent; }
double ZoomBounds::convertFromZoomReal(double zoom) const { return zoom * value100percent; }
double ZoomBounds::clamp(double zoom) const { return std::clamp(zoom, getMin(), getMax()); }

auto ZoomBoundsControl::getZoomBounds() const -> const ZoomBounds& { return bounds; }

void ZoomBoundsControl::setDPIratio(double ratio) {
    // Reject invalid values: they would zero out zoomMin/zoomMax and poison paste/selection math with NaNs/Infs
    // (e.g. corrupted displayDpi settings, or bogus monitor metrics on Windows)
    if (!std::isfinite(ratio) || ratio <= 0.0) {
        g_warning("Ignoring invalid zoom100Value: %f", ratio);
        return;
    }
    // zoom100Value = DPI / Util::DPI_NORMALIZATION_FACTOR, so these bounds mirror the [36, 720] DPI range the
    // automatic detection clamps to in xoj::util::gtk::getWidgetDPI() (36 / 72 = 0.5, 720 / 72 = 10.0). They also
    // contain the manual calibration slider's range [50, 400] DPI (see the adjustmentZoom in settings.glade).
    this->bounds.value100percent = std::clamp(ratio, 0.5, 10.0);
}
