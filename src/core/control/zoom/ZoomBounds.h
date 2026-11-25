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

class ZoomBoundsListener {
public:
    virtual void zoomBoundsChanged(const ZoomBounds& bounds) = 0;
    virtual ~ZoomBoundsListener() = default;
};

class ZoomBoundsControl {
public:
    virtual ~ZoomBoundsControl() = default;

    void addListener(ZoomBoundsListener* l);
    void removeListener(ZoomBoundsListener* l);
    void fireZoomBoundsChanged() const;

    const ZoomBounds& getBounds() const;
    void setDPIratio(double ratio);  ///< sets value100percent and fires fireZoomBoundsChanged()

private:
    std::vector<ZoomBoundsListener*> listeners;
    ZoomBounds bounds;
};
