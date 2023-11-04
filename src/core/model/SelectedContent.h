/*
 * Xournal++
 *
 * The model of selected content being edited
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <utility>
#include <vector>

#include "control/ToolEnums.h"                    // for ToolSize
#include "control/tools/selection/OrderChange.h"  // for SelectionOrderChange
#include "model/Element.h"                        // for Element:;Index
#include "model/ElementContainer.h"               // for ElementContainer
#include "model/ElementInsertionPosition.h"       // for InsertionPosition, InsertionOrder
#include "model/PageRef.h"                        // for PageRef
#include "model/geometry/Position.h"              // for Position
#include "undo/UndoAction.h"                      // for UndoAction (ptr only)
#include "util/Color.h"                           // for Color
#include "util/Rectangle.h"                       // for Rectangle
#include "util/serializing/Serializable.h"        // for Serializable

class Range;
class Layer;
class XojPageView;
class DeleteUndoAction;
class LineStyle;
class ObjectInputStream;
class ObjectOutputStream;
class XojFont;

class SelectedContent: public ElementContainer, public Serializable {
public:
    SelectedContent(PageRef page, Layer* layer);
    SelectedContent(PageRef page, Layer* layer, InsertionOrder order, const xoj::util::Rectangle<double>& box,
                    const xoj::util::Rectangle<double>& snapBounds);
    ~SelectedContent() override = default;

    void forEachElement(std::function<void(Element*)> f) const override;

public:
    /**
     * @brief Drops the elements where they are, to the layer/page. Finalizes the selection
     * @return The (coarse) bounding box of the dropped elements
     */
    Range drop(bool scaleLineWidth);

    /// Cancels any transformation and drop the elements where they were first selected.
    void cancel();

    /// Moves the elements according to the current position and returns the InsertionOrder.
    InsertionOrder makeMoveEffective(bool scaleLineWidth);

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

public:
    inline const Position& getPosition() const { return position; }
    inline void setPosition(const Position& pos) { position = pos; }

    inline const Position& getOriginalPosition() const { return originalPosition; }
    inline void setOriginalCenter(const xoj::util::Point<double>& c) {
        position.center = c;
        originalPosition.center = c;
    }

    inline const PageRef& getPage() const { return page; }

    inline double getHorizontalMargin() const { return horizontalMargin; }
    inline double getVerticalMargin() const { return verticalMargin; }

    /// Compute the range (in page coordinates) containing the (possible rotated) selection rectangle in the given pos.
    Range computeCoarseBoundingBox(const Position& pos) const;
    Range computeUnrotatedBoundingBox() const;

    /// Get the coefficient for rescaling the margins from this->originalPosition to pos
    double getMarginScale(const Position& pos) const;

    inline bool empty() const { return insertionOrder.empty(); }

private:
    Position originalPosition;
    Position position;

    /// Margins between the snapping box and the actual bounding box
    double horizontalMargin;
    double verticalMargin;

    /**
     * Mapping of elements in the selection to the indexes from the original selection layer.
     * Defines a insert order over the selection.
     *
     * Invariant: the insertion order must be sorted by index in ascending order.
     */
    InsertionOrder insertionOrder;

    /**
     * Page on which the selected elements "are". Note that the selected elements do not actually belong to the
     * Layer::elements of a layer on that page. This is used to create UndoAction as if the elements were on the page.
     */
    PageRef page;
    /**
     * Layer to which the selected elements "belong". Note that the selected elements do not actually belong to the
     * Layer::elements of the layer. This is used to create UndoAction as if the elements were on that layer.
     */
    Layer* layer;


    //     SelectedContent(xoj::util::Rectangle<double> bounds, xoj::util::Rectangle<double> snappedBounds,
    //                           const PageRef& sourcePage, Layer* sourceLayer, XojPageView* sourceView);
    //     ~SelectedContent() override;
    //
public:
    /**
     * Sets the new position, page and layer
     * newCenter must be relative to the new page
     * Assumes the provided layer belongs to the provided page
     * Returns a MoveUndoAction
     */
    UndoActionPtr applyTranslation(const xoj::util::Point<double>& newCenter, PageRef newPage, Layer* newLayer);

    /**
     * Sets the new angle
     * Returns a RotateUndoAction
     */
    UndoActionPtr applyRotation(double newAngle);

    /**
     * Sets the new position
     * Assumes the old and new positions are relative to the same page and have the same angle
     * Assumes the provided scaling center is consistent with old and new positions
     * Returns a ScaleUndoAction
     */
    UndoActionPtr applyScaling(const Position& newPos, xoj::util::Point<double> scalingCenter, bool restoreLineWidth);


    /**
     * Drops a clone of the selected elements on the page. Returns a AddUndoAction and a Range containing the elements
     * (or {nullptr ,Range()} if nothing is done)
     */
    std::pair<UndoActionPtr, Range> dropAClone(bool scaleLineWidth) const;

    /**
     * Sets the line style for all strokes, returns an undo action and a Range containing all the modifications
     * (or {nullptr ,Range()} if nothing is done)
     */
    std::pair<UndoActionPtr, Range> setLineStyle(LineStyle style);

    /**
     * Sets the tool size for pen or eraser, returns an undo action
     * (or {nullptr ,Range()} if nothing is done)
     */
    std::pair<UndoActionPtr, Range> setSize(ToolSize size, const double* thicknessPen,
                                            const double* thicknessHighlighter, const double* thicknessEraser);

    /**
     * Set the color of all elements, return an undo action
     * (Or {nullptr ,Range()} if nothing done, e.g. because there is only an image)
     */
    std::pair<UndoActionPtr, Range> setColor(Color color);

    /**
     * Sets the font of all containing text elements, return an undo action
     * (or {nullptr ,Range()} if there are no Text elements)
     */
    std::pair<UndoActionPtr, Range> setFont(const XojFont& font);

    /**
     * Fills the stroke, return an undo action
     * (Or {nullptr ,Range()} if nothing done, e.g. because there is only an image)
     */
    std::pair<UndoActionPtr, Range> setFill(int alphaPen, int alphaHighligther);

    /**
     * Delete the selection, return an undo action and a range for repainting
     * (Or {nullptr ,Range()} if nothing done, e.g. because there is only a newly created Image/TexImage)
     */
    std::pair<UndoActionPtr, Range> deleteContent();

    std::pair<UndoActionPtr, Range> rearrangeInsertionOrder(SelectionOrderChange order);

    /**
     * Returns a view to the elements
     */
    auto getElements() const -> InsertionOrderElementsView;

    //
    //     /**
    //      * Fills the undo item if the selection is deleted
    //      * the selection is cleared after
    //      */
    //     void fillUndoItem(DeleteUndoAction* undo);
    //
    //     /**
    //      * Creates an undo/redo item for translating by (dx, dy), and then updates the bounding boxes accordingly.
    //      */
    //     void addMoveUndo(UndoRedoHandler* undo, double dx, double dy);
    //
    // public:
    //     /**
    //      * paints the selection
    //      */
    //     void paint(cairo_t* cr, double x, double y, double rotation, double width, double height, double zoom);
    //
    //     /**
    //      * Finish the editing
    //      */
    //     void finalizeSelection(xoj::util::Rectangle<double> bounds, xoj::util::Rectangle<double> snappedBounds,
    //                            bool aspectRatio, Layer* layer, const PageRef& targetPage, XojPageView* targetView,
    //                            UndoRedoHandler* undo);
    //
    //     void updateContent(xoj::util::Rectangle<double> bounds, xoj::util::Rectangle<double> snappedBounds, double
    //     rotation,
    //                        bool aspectRatio, Layer* layer, const PageRef& targetPage, XojPageView* targetView,
    //                        UndoRedoHandler* undo, CursorSelectionType type);
    //
    // private:
    //     /**
    //      * Delete our internal View buffer,
    //      * it will be recreated when the selection is painted next time
    //      */
    //     void deleteViewBuffer();
    //
    //     /**
    //      * Callback to redrawing the buffer asynchrony
    //      */
    //     static auto repaintSelection(EditSelectionContents* selection) -> bool;
    //
    // public:
    //     /**
    //      * Gets the original view of the contents
    //      */
    //     XojPageView* getSourceView();
    //
    //
    //     /**
    //      * Gets the original X of the contents
    //      */
    //     double getOriginalX() const;
    //
    //     /**
    //      * Gets the original Y of the contents
    //      */
    //     double getOriginalY() const;
    //
    //     /**
    //      * Gets the complete original bounding box as rectangle
    //      */
    //     xoj::util::Rectangle<double> getOriginalBounds() const;
    //
    //     constexpr static struct {
    //         bool operator()(std::pair<Element*, Element::Index> p1, std::pair<Element*, Element::Index> p2) {
    //             return p1.second < p2.second;
    //         }
    //     } insertOrderCmp{};
    //
    // public:
    //     // Serialize interface
    //     void serialize(ObjectOutputStream& out) const override;
    //     void readSerialized(ObjectInputStream& in) override;
    //
    // private:
    //     /**
    //      * The original dimensions to calculate the zoom factor for rescaling the items and the offset for moving the
    //      * selection
    //      */
    //     xoj::util::Rectangle<double> originalBounds;
    //     xoj::util::Rectangle<double> lastBounds;
    //     xoj::util::Rectangle<double> lastSnappedBounds;
    //
    //     /**
    //      * The given rotation. Original rotation should always be zero (double)
    //      */
    //     double rotation = 0;
    //     double lastRotation = 0;  // for undoing multiple rotations during one selection edit.
    //
    //     /**
    //      * The offset to the original selection
    //      */
    //     double relativeX = -9999999999;
    //     double relativeY = -9999999999;
    //
    //     /**
    //      * The setting for whether line width is restored after resizing operation (checked at creation time)
    //      */
    //     bool restoreLineWidth;
    //
    //     /**
    //      * The selected element (the only one which are handled by this instance)
    //      */
    //     std::vector<Element*> selected;
    //
    //     /**
    //      * Mapping of elements in the selection to the indexes from the original selection layer.
    //      * Defines a insert order over the selection.
    //      *
    //      * Invariant: the insert order must be sorted by index in ascending order.
    //      */
    //     std::deque<std::pair<Element*, Element::Index>> insertOrder;
    //
    //     /**
    //      * The rendered elements
    //      */
    //     cairo_surface_t* crBuffer = nullptr;
    //
    //     /**
    //      * The source id for the rescaling task
    //      */
    //     guint rescaleId = 0;
    //
    //     /**
    //      * Source Page for Undo operations
    //      */
    //     PageRef sourcePage;
    //
    //     /**
    //      * Source Layer for Undo operations
    //      */
    //     Layer* sourceLayer;
    //
    //     /**
    //      * Source View for Undo operations
    //      */
    //     XojPageView* sourceView;
};
