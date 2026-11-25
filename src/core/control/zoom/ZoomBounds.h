/*
 * Xournal++
 *
 * Controls the bounds of zoom values, depending on the display's DPI level
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>

constexpr auto DEFAULT_ZOOM_MAX{7};
constexpr auto DEFAULT_ZOOM_MIN{0.3};

struct ZoomBounds {
    double value100percent = 1.;  ///< Zoom ratio to get a 1-to-1 size ratio on the current screen
    double getMin() const;
    double getMax() const;
    double convertToZoomReal(double zoom) const;
    double convertFromZoomReal(double zoom) const;
    double clamp(double zoom) const;
};

class ZoomBoundsControl {
public:
    virtual ~ZoomBoundsControl() = default;

    const ZoomBounds& getZoomBounds() const;
    void setDPIratio(double ratio);  ///< adapts value100percent

private:
    ZoomBounds bounds;
};
