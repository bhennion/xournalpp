/*
 * Xournal++
 *
 * A selection for editing, every selection (Rect, Lasso...) is
 * converted to this one if the selection is finished
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>   // for unique_ptr
#include <utility>  // for pair
#include <vector>   // for vector

#include <cairo.h>  // for cairo_t, cairo_matrix_t
#include <glib.h>   // for GSource

#include "control/ToolEnums.h"                     // for ToolSize
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHandler
#include "model/Element.h"                         // for Element, Element::Index
#include "model/ElementContainer.h"                // for ElementContainer
#include "model/ElementInsertionPosition.h"        // for InsertionOrder
#include "model/OverlayBase.h"                     // for OverlayBase
#include "model/PageRef.h"                         // for PageRef
#include "model/SelectedContent.h"                 // for SelectedContent
#include "undo/UndoAction.h"                       // for UndoAction (ptr only)
#include "util/Color.h"                            // for Color
#include "util/Rectangle.h"                        // for Rectangle
#include "util/serializing/Serializable.h"         // for Serializable

#include "CursorSelectionType.h"  // for CursorSelectionType, CURS...
#include "OrderChange.h"          // for SelectionOrderChange

class UndoRedoHandler;
class Layer;
class XojPageView;
class Selection;
class EditSelectionContents;
class DeleteUndoAction;
class LineStyle;
class ObjectInputStream;
class ObjectOutputStream;
class XojFont;
class Document;
class EditSelection;
class Operation;

namespace xoj::util {
template <class T>
class DispatchPool;
};

namespace xoj::view {
class EditSelectionView;
};

class EditSelection: public OverlayBase {
public:
    EditSelection(Control* ctrl, InsertionOrder elts, const PageRef& page, Layer* layer, XojPageView* view,
                  const Range& bounds, const Range& snappingBounds);

    /// Construct an empty selection
    EditSelection(Control* ctrl, const PageRef& page, Layer* layer, XojPageView* view);

    ~EditSelection() override;

    inline const std::shared_ptr<xoj::util::DispatchPool<xoj::view::EditSelectionView>>& getViewPool() const {
        return viewPool;
    }


public:
    /**
     * @brief Finds the element (if any) at page coordinate (x,y).
     * Takes the selection's translation/rotation/scaling into account
     **/
    Element* getElementAt(double x, double y) const;

    /**
     * @brief Get the unrotated bounding rectangle in document coordinates (multiple with zoom)
     */
    xoj::util::Rectangle<double> getUnrotatedBoundingBox() const;

    /**
     * gets the minimal bounding box containing all elements of the selection used for e.g. grid snapping
     */
    xoj::util::Rectangle<double> getSnappedBounds() const;

    /**
     * get the original bounding rectangle in document coordinates
     */
    xoj::util::Rectangle<double> getOriginalBounds() const;

    /**
     * Get the rotation angle of the selection
     */
    double getRotation() const;

    /**
     * Get the source page (where the selection was done)
     */
    PageRef getSourcePage() const;

    /**
     * Get the source layer (form where the Elements come)
     */
    Layer* getSourceLayer() const;

    inline XojPageView* getView() const { return view; }
    inline const SelectedContent* getContent() const { return content.get(); }

public:
    /**
     * Sets the tool size for pen or eraser
     */
    void setSize(ToolSize size, const double* thicknessPen, const double* thicknessHighlighter,
                 const double* thicknessEraser);

    /**
     * Set the line style of all strokes
     */
    void setLineStyle(LineStyle style);

    /**
     * Set the color of all elements
     */
    void setColor(Color color);

    /**
     * Sets the font of all containing text elements
     */
    void setFont(const XojFont& font);

    /**
     * Clears the selection and returns a DeleteUndoAction
     */
    void deleteSelection();

    /**
     * Fills the stroke
     */
    void setFill(int alphaPen, int alphaHighligther);

    /**
     * @brief Drop a clone of the selected objects on the page at the current position
     */
    void dropAClone() const;

    /**
     * @brief Fill *this with elements unserialized from the given stream and center the selection at initialPos
     * Assumes *this has no content yet
     */
    void fillFromStream(ObjectInputStream& in, const xoj::util::Point<double>& initialPos);

    /**
     * @brief Change the insert order of this selection.
     */
    void rearrangeInsertionOrder(const SelectionOrderChange change);

public:
    /**
     * Returns all containing elements of this selection
     */
    auto getElements() const -> InsertionOrderElementsView;

    /**
     * Get the cursor type for the current position (if 0 then the default cursor should be used)
     */
    CursorSelectionType getSelectionTypeForPos(double x, double y, double zoom) const;

    /**
     * Gets the selection's bounding box in view coordinates. This takes document zoom
     * and selection rotation into account.
     */
    auto getBoundingBoxInView() const -> xoj::util::Rectangle<double>;

    /**
     * If the selection is outside the visible area correct the coordinates
     */
    void ensureWithinVisibleArea() const;

    /**
     * Finish the current movement. Coordinates are relative to the XournalView!
     */
    void onButtonReleaseEvent(double x, double y);

    /**
     * Handles mouse input for moving and resizing, coordinates are relative to "view"
     */
    void onButtonPressEvent(CursorSelectionType type, double x, double y);

    /**
     * Handles mouse input for moving and resizing, coordinates are relative to "view"
     */
    void onMotionNotifyEvent(double x, double y, bool alt);

    /**
     * If the user is currently moving the selection.
     */
    bool isMoving() const;

    const Position& getPosition() const;

public:
    /// Applies the transformation to the selected elements, empties the selection and return the modified elements
    InsertionOrder makeMoveEffective();

private:
    /**
     * Finishes all pending changes, move the elements, scale the elements and add
     * them to new layer if any or to the old if no new layer
     */
    void finalizeSelection();

    /**
     * Gets the PageView under the cursor
     */
    XojPageView* getPageViewUnderCursor() const;

    /**
     * Translate all coordinates which are relative to the current view to the new view,
     * and set the attribute view to the new view
     */
    void translateToView(XojPageView* v);

    /**
     * Set edge panning signal.
     */
    void setEdgePan(bool edgePan);

    /**
     * Whether the edge pan signal is set.
     */
    bool isEdgePanning() const;

    static bool handleEdgePan(EditSelection* self);

private:
    // Capability flags
    bool preserveAspectRatio = false;  ///< If both scale axes should have the same scale factor, e.g. for Text
    bool supportMirroring = true;      ///< If mirrors are allowed
    bool supportRotation = true;       ///< If rotations are allowed
    bool supportSetLineWidth = false;  ///< If `setSize()` is enabled (sets line width for strokes)
    bool supportSetColor = false;      ///< If `setColor()` is enabled
    bool supportSetFill = false;       ///< If `setFill()` is enabled
    bool supportSetLineStyle = false;  ///< If `setLineStyle()` is enabled

    /// Adapts the values of the above flags as if this element was added to the selection.
    void adaptFlagsToNewElement(Element* e);

public:
    inline bool isRotationSupported() const { return supportRotation; }
    inline bool getPreserveAspectRatio() const { return preserveAspectRatio; }
    inline bool isSetLineWidthSupported() const { return supportSetLineWidth; }
    inline bool isSetLineStyleSupported() const { return supportSetLineStyle; }
    inline bool isSetColorSupported() const { return supportSetColor; }
    inline bool isSetFillSupported() const { return supportSetFill; }

private:
    std::unique_ptr<SelectedContent> content;

    /**
     * Mouse coordinates for moving / resizing
     */
    CursorSelectionType mouseDownType = CURSOR_SELECTION_NONE;

    std::unique_ptr<Operation> currentOperation;

    /**
     * Size of the editing handles
     */
    int btnWidth{8};

    /**
     * The source page (form where the Elements come)
     */
    PageRef sourcePage;

    /**
     * The source layer (form where the Elements come)
     */
    Layer* sourceLayer{};

private:  // HANDLER
    /**
     * The page view for the anchor
     */
    XojPageView* view{};

    /**
     * Undo redo handler
     */
    UndoRedoHandler* undo{};

    /**
     * The handler for snapping points
     */
    SnapToGridInputHandler snappingHandler;

    /**
     * Edge pan timer
     */
    GSource* edgePanHandler = nullptr;

    /**
     * Inhibit the next move event after edge panning finishes. This prevents
     * the selection from teleporting if the page has changed during panning.
     * Additionally, this reduces the amount of "jitter" resulting from moving
     * the selection in mouseDown while edge panning.
     */
    bool edgePanInhibitNext = false;

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::EditSelectionView>> viewPool;
};
