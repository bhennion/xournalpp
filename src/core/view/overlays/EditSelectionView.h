/*
 * Xournal++
 *
 * View active selection
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cairo.h>  // for cairo_t

#include "util/Color.h"
#include "util/DispatchPool.h"  // for Listener
#include "view/Mask.h"
#include "view/overlays/OverlayView.h"

class OverlayBase;
class Range;
class EditSelection;
class ToolHandler;

namespace xoj::view {
class Repaintable;

class EditSelectionView final: public ToolView, public xoj::util::Listener<EditSelectionView> {

public:
    EditSelectionView(const EditSelection* handler, Repaintable* parent, Color aidColor, ToolHandler* toolHandler);
    ~EditSelectionView() noexcept override;

    /**
     * @brief Draws the overlay to the given context
     */
    void draw(cairo_t* cr) const override;
    void drawWithoutDrawingAids(cairo_t* cr) const override;

    bool isViewOf(const OverlayBase* overlay) const override;

    /**
     * Listener interface
     */
    static constexpr struct FlagDirtyRegionRequest {
    } FLAG_DIRTY_REGION = {};
    void on(FlagDirtyRegionRequest, Range rg);

    static constexpr struct FinalizationRequest {
    } FINALIZATION_REQUEST = {};
    void deleteOn(FinalizationRequest, Range rg);

    class SelectionMask final: public Mask {
    public:
        SelectionMask() = default;
        SelectionMask(cairo_surface_t* target, double halfWidth, double halfHeight, double hPadding, double vPadding,
                      double zoom);
        inline double getHalfWidth() const { return halfWidth; }
        inline double getHalfHeight() const { return halfHeight; }

    private:
        double halfWidth;
        double halfHeight;
    };

private:
    const EditSelection* selection;

    mutable SelectionMask mask;

    const Color aidColor;

    ToolHandler* toolHandler;

public:
    // Width of the lines making the frame
    static constexpr int BORDER_WIDTH_IN_PIXELS = 1;
};
};  // namespace xoj::view
