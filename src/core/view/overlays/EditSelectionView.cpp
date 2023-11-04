#include "EditSelectionView.h"

#include "control/ToolHandler.h"
#include "control/tools/selection/EditSelection.h"
#include "control/tools/selection/UIMeasures.h"
#include "model/SelectedContent.h"
#include "util/Color.h"
#include "util/Util.h"
#include "util/raii/CairoWrappers.h"
#include "view/ElementContainerView.h"
#include "view/Repaintable.h"
#include "view/View.h"

using namespace xoj::view;

constexpr uint8_t FILLING_ALPHA = 76;
constexpr double DASH_SIZE = 10;  ///< in pixels

EditSelectionView::EditSelectionView(const EditSelection* selection, Repaintable* parent, Color aidColor,
                                     ToolHandler* toolHandler):
        ToolView(parent), selection(selection), aidColor(aidColor), toolHandler(toolHandler) {
    this->registerToPool(selection->getViewPool());
    // this->on(FLAG_DIRTY_REGION, Range());
    // TODO
}

EditSelectionView::~EditSelectionView() noexcept { this->unregisterFromPool(); }

static void drawRotationHandle(cairo_t* cr, double x, double y, double zoom, Color color) {
    const double halfWidth = EditSelectionMeasures::ROTATION_HANDLE_SIZE / zoom;
    cairo_rectangle(cr, x - halfWidth, y - halfWidth, 2. * halfWidth, 2. * halfWidth);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_fill_preserve(cr);
    Util::cairo_set_source_argb(cr, color);
    cairo_stroke(cr);
}

static void drawStretchHandle(cairo_t* cr, double x, double y, double zoom, Color color) {
    const double halfWidth = EditSelectionMeasures::STRETCH_HANDLE_SIZE / zoom;
    cairo_rectangle(cr, x - halfWidth, y - halfWidth, 2. * halfWidth, 2. * halfWidth);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_fill_preserve(cr);
    Util::cairo_set_source_argb(cr, color);
    cairo_stroke(cr);
}

static void drawDeletionHandle(cairo_t* cr, double x, double y, double zoom, Color color) {
    const double halfWidth = EditSelectionMeasures::DELETION_HANDLE_SIZE / zoom;
    cairo_rectangle(cr, x - halfWidth, y - halfWidth, 2. * halfWidth, 2. * halfWidth);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_fill_preserve(cr);
    Util::cairo_set_source_argb(cr, color);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_move_to(cr, x - halfWidth, y - halfWidth);
    cairo_rel_move_to(cr, 2. * halfWidth, 0);
    cairo_rel_line_to(cr, -2. * halfWidth, 2. * halfWidth);
    cairo_rel_move_to(cr, 2. * halfWidth, 0);
    cairo_rel_line_to(cr, -2. * halfWidth, -2. * halfWidth);
    cairo_stroke(cr);
}

EditSelectionView::SelectionMask::SelectionMask(cairo_surface_t* target, double halfWidth, double halfHeight,
                                                double hPadding, double vPadding, double zoom):
        Mask(target, Range(-halfWidth - hPadding, -halfHeight - vPadding, halfWidth + hPadding, halfHeight + vPadding),
             zoom, CAIRO_CONTENT_COLOR_ALPHA),
        halfWidth(halfWidth),
        halfHeight(halfHeight) {}

static void rotateContextAndDrawContent(cairo_t* cr, const EditSelection& sel, EditSelectionView::SelectionMask& mask,
                                        double zoom) {
    const auto* content = sel.getContent();
    xoj_assert(content);
    const auto& pos = sel.getPosition();
    cairo_translate(cr, pos.center.x, pos.center.y);

    if (std::abs(pos.angle) > std::numeric_limits<double>::epsilon()) {
        cairo_rotate(cr, pos.angle);

        // Draw the rotation point for debugging
        // cairo_set_source_rgb(cr, 0, 1, 0);
        // cairo_rectangle(cr, 0, 0, 10, 10);
        // cairo_stroke(cr);
    }

    if (!mask.isInitialized()) {
        const auto& originalPos = content->getOriginalPosition();
        double scaleX = originalPos.halfWidth != 0.0 ? pos.halfWidth / originalPos.halfWidth : 1.0;
        double scaleY = originalPos.halfHeight != 0.0 ? pos.halfHeight / originalPos.halfHeight : 1.0;
        double marginScale = std::max({1.0, scaleX, scaleY});

        mask = EditSelectionView::SelectionMask(cairo_get_target(cr), std::abs(pos.halfWidth), std::abs(pos.halfHeight),
                                                marginScale * content->getHorizontalMargin(),
                                                marginScale * content->getVerticalMargin(), zoom);

        xoj::util::CairoSaveGuard saveGuard(mask.get());
        cairo_scale(mask.get(), scaleX, scaleY);
        cairo_translate(mask.get(), -originalPos.center.x, -originalPos.center.y);

        xoj::view::ElementContainerView view(content);
        view.draw(xoj::view::Context::createDefault(mask.get()));
    }

    xoj::util::CairoSaveGuard saveGuard(cr);
    double scaleX = mask.getHalfWidth() != 0.0 ? pos.halfWidth / mask.getHalfWidth() : 1.0;
    double scaleY = mask.getHalfHeight() != 0.0 ? pos.halfHeight / mask.getHalfHeight() : 1.0;
    cairo_scale(cr, scaleX, scaleY);
    mask.paintTo(cr);

    // int wImg = cairo_image_surface_get_width(mask.get());
    // int hImg = cairo_image_surface_get_height(this->crBuffer);
    //
    // int wTarget = static_cast<int>(std::abs(width) * zoom);
    // int hTarget = static_cast<int>(std::abs(height) * zoom);
    //
    // double sx = static_cast<double>(wTarget) / wImg;
    // double sy = static_cast<double>(hTarget) / hImg;
    //
    // if (wTarget != wImg || hTarget != hImg || std::abs(rotation) > std::numeric_limits<double>::epsilon()) {
    //     if (!this->rescaleId) {
    //         this->rescaleId = g_idle_add(xoj::util::wrap_v<repaintSelection>, this);
    //     }
    //     cairo_scale(cr, sx, sy);
    // }
    //
    // double dx = static_cast<int>(std::min(x, x + width) * zoom / sx);
    // double dy = static_cast<int>(std::min(y, y + height) * zoom / sy);
    //
    // cairo_set_source_surface(cr, this->crBuffer, dx, dy);
    // cairo_paint(cr);
}

void EditSelectionView::drawWithoutDrawingAids(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);
    rotateContextAndDrawContent(cr, *this->selection, mask, parent->getZoom());
}

void EditSelectionView::draw(cairo_t* cr) const {
    using namespace EditSelectionMeasures;

    xoj::util::CairoSaveGuard saveGuard(cr);
    double zoom = parent->getZoom();
    rotateContextAndDrawContent(cr, *this->selection, mask, zoom);
    // The cairo coordinates are now relative to the center of the box and rotated as necessary

    // Draw the frame
    cairo_set_line_width(cr, BORDER_WIDTH_IN_PIXELS / zoom);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    const auto& pos = this->selection->getPosition();
    const auto* content = this->selection->getContent();
    xoj_assert(content);

    double marginScale = content->getMarginScale(pos);

    double halfWidth = std::abs(pos.halfWidth) + marginScale * content->getHorizontalMargin() + CONTENT_PADDING / zoom;
    double halfHeight = std::abs(pos.halfHeight) + marginScale * content->getVerticalMargin() + CONTENT_PADDING / zoom;

    cairo_rectangle(cr, -halfWidth, -halfHeight, 2 * halfWidth, 2 * halfHeight);
    Util::cairo_set_source_argb(cr, Color(aidColor.red, aidColor.green, aidColor.blue, FILLING_ALPHA));
    cairo_fill_preserve(cr);

    const std::vector<double> dashes = {DASH_SIZE / zoom, DASH_SIZE / zoom};
    Util::cairo_set_dash_from_vector(cr, dashes, 0);
    Util::cairo_set_source_argb(cr, aidColor);
    cairo_stroke(cr);

    if (!selection->isMoving() && toolHandler->getToolType() != TOOL_HAND) {
        cairo_set_dash(cr, nullptr, 0, 0);

        drawStretchHandle(cr, -halfWidth, -halfHeight, zoom, aidColor);  // top left
        drawStretchHandle(cr, -halfWidth, halfHeight, zoom, aidColor);   // bottom left
        drawStretchHandle(cr, halfWidth, -halfHeight, zoom, aidColor);   // top right
        drawStretchHandle(cr, halfWidth, halfHeight, zoom, aidColor);    // bottom right

        if (!selection->getPreserveAspectRatio()) {
            drawStretchHandle(cr, 0, -halfHeight, zoom, aidColor);  // top
            drawStretchHandle(cr, 0, halfHeight, zoom, aidColor);   // bottom
            drawStretchHandle(cr, -halfWidth, 0, zoom, aidColor);   // left
            drawStretchHandle(cr, halfWidth, 0, zoom, aidColor);    // right
        }

        if (selection->isRotationSupported()) {
            drawRotationHandle(cr, halfWidth + ROTATION_HANDLE_DISTANCE / zoom, 0, zoom, aidColor);
        }

        drawDeletionHandle(cr, -halfWidth - DELETION_HANDLE_DISTANCE / zoom, -halfHeight, zoom, aidColor);
    }
}

bool EditSelectionView::isViewOf(const OverlayBase* overlay) const { return overlay == this->selection; }

static double computeRepaintPadding(const EditSelection* sel) {
    if (sel->isMoving()) {
        // The handles are hidden when moving, so the repaint box can be smaller
        // The sqrt(2) factor ensure the corners are fully in the repaint box even if the selection is rotated
        return std::sqrt(2.) * (EditSelectionView::BORDER_WIDTH_IN_PIXELS + EditSelectionMeasures::CONTENT_PADDING);
    } else {
        using namespace EditSelectionMeasures;
        return std::hypot(EditSelectionView::BORDER_WIDTH_IN_PIXELS + CONTENT_PADDING +
                                  std::max(DELETION_HANDLE_DISTANCE, ROTATION_HANDLE_DISTANCE),
                          EditSelectionView::BORDER_WIDTH_IN_PIXELS + CONTENT_PADDING +
                                  std::max(DELETION_HANDLE_SIZE, ROTATION_HANDLE_SIZE));
    }
}

void EditSelectionView::on(EditSelectionView::FlagDirtyRegionRequest, Range rg) {
    rg.addPadding(computeRepaintPadding(this->selection));
    this->parent->flagDirtyRegion(rg);
}

void EditSelectionView::deleteOn(EditSelectionView::FinalizationRequest, Range rg) {
    rg.addPadding(computeRepaintPadding(this->selection));
    this->parent->drawAndDeleteToolView(this, rg);
}
