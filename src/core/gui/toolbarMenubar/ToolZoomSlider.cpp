#include "ToolZoomSlider.h"

#include <cmath>    // for exp, log
#include <sstream>  // for stringstream, bas...
#include <utility>  // for move

#include <glib.h>  // for g_get_monotonic_time

#include "control/actions/ActionDatabase.h"
#include "control/zoom/ZoomBounds.h"  // for DEFAULT_ZOOM_MIN, DEFAULT_ZOOM_MAX
#include "util/i18n.h"                // for _

#include "SliderItemCreationHelper.h"

constexpr double SCALE_LOG_OFFSET = 0.20753;

constexpr int FINE_STEP_COUNT = 100;
constexpr int COARSE_STEP_COUNT = 10;

// TODO(personalizedrefrigerator): Update the range to reflect the min and max zoom settings.
constexpr auto SLIDER_RANGE =
        AbstractSliderItem::SliderRange{DEFAULT_ZOOM_MIN, DEFAULT_ZOOM_MAX, FINE_STEP_COUNT, COARSE_STEP_COUNT};

ToolZoomSlider::ToolZoomSlider(std::string id, IconNameHelper iconNameHelper, ActionDatabase& db):
        AbstractSliderItem{std::move(id), Category::NAVIGATION, SLIDER_RANGE, db.getAction(Action::ZOOM)},
        iconName(iconNameHelper.iconName("zoom-slider")) {}

auto ToolZoomSlider::formatSliderValue(double value) -> std::string {
    std::stringstream out;
    out << static_cast<int>(std::round(100 * scaleInverseFunction(value)));
    out << "%";
    return out.str();
}

auto ToolZoomSlider::createItem(bool horizontal) -> xoj::util::WidgetSPtr {
    return SliderItemCreationHelper<ToolZoomSlider>::createItem(this, horizontal);
}

auto ToolZoomSlider::getToolDisplayName() const -> std::string { return _("Zoom Slider"); }

auto ToolZoomSlider::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(iconName.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR);
}

auto ToolZoomSlider::scaleFunction(double x) -> double { return std::log(x - SCALE_LOG_OFFSET); }

auto ToolZoomSlider::scaleInverseFunction(double x) -> double { return std::exp(x) + SCALE_LOG_OFFSET; }
