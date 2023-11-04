#include "EditSelection.h"

#include <algorithm>  // for min, max, stable_sort
#include <cmath>      // for abs, cos, sin, cop...
#include <cstddef>    // for size_t
#include <limits>     // for numeric_limits
#include <memory>     // for make_unique, __sha...
#include <numeric>    // for reduce
#include <string>     // for string
#include <utility>

#include <gdk/gdk.h>  // for gdk_cairo_set_sour...

#include "control/Control.h"                       // for Control
#include "control/settings/Settings.h"             // for Settings
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "control/zoom/ZoomControl.h"              // for ZoomControl
#include "gui/Layout.h"                            // for Layout
#include "gui/PageView.h"                          // for XojPageView
#include "gui/XournalView.h"                       // for XournalView
#include "gui/XournalppCursor.h"                   // for XournalppCursor
#include "gui/widgets/XournalWidget.h"             // for gtk_xournal_get_la...
#include "model/Document.h"                        // for Document
#include "model/Element.h"                         // for Element::Index
#include "model/ElementInsertionPosition.h"
#include "model/Layer.h"                          // for Layer
#include "model/LineStyle.h"                      // for LineStyle
#include "model/Point.h"                          // for Point
#include "model/XojPage.h"                        // for XojPage
#include "undo/AddUndoAction.h"                   // for AddUndoAction
#include "undo/InsertUndoAction.h"                // for InsertsUndoAction
#include "undo/UndoRedoHandler.h"                 // for UndoRedoHandler
#include "util/Range.h"                           // for Range
#include "util/Util.h"                            // for cairo_set_dash_from_vector
#include "util/glib_casts.h"                      // for wrap_v
#include "util/i18n.h"                            // for _
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream
#include "view/overlays/EditSelectionView.h"      // for EditSelectionView

#include "CursorSelectionType.h"
#include "Operation.h"
#include "UIMeasures.h"

class XojFont;

/// Smallest can scale down to, in pixels.
constexpr size_t MINPIXSIZE = 5;

/// Number of times to trigger edge pan timer per second
constexpr unsigned int PAN_TIMER_RATE = 30;


EditSelection::EditSelection(Control* ctrl, InsertionOrder elts, const PageRef& page, Layer* layer, XojPageView* view,
                             const Range& bounds, const Range& snappingBounds):
        btnWidth(std::max(10, ctrl->getSettings()->getDisplayDpi() / 8)),
        sourcePage(page),
        sourceLayer(layer),
        view(view),
        undo(ctrl->getUndoRedoHandler()),
        snappingHandler(ctrl->getSettings()),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::EditSelectionView>>()) {
    for (auto&& [e, _]: elts) {
        adaptFlagsToNewElement(e.get());
    }
    this->content =
            std::make_unique<SelectedContent>(page, layer, std::move(elts), xoj::util::Rectangle<double>(bounds),
                                              xoj::util::Rectangle<double>(snappingBounds));
}

EditSelection::EditSelection(Control* ctrl, const PageRef& page, Layer* layer, XojPageView* view):
        btnWidth(std::max(10, ctrl->getSettings()->getDisplayDpi() / 8)),
        sourcePage(page),
        sourceLayer(layer),
        view(view),
        undo(ctrl->getUndoRedoHandler()),
        snappingHandler(ctrl->getSettings()),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::EditSelectionView>>()) {}

EditSelection::~EditSelection() {
    finalizeSelection();

    if (this->edgePanHandler) {
        g_source_destroy(this->edgePanHandler);
        g_source_unref(this->edgePanHandler);
    }
}

void EditSelection::fillFromStream(ObjectInputStream& in, const xoj::util::Point<double>& initialPos) {
    xoj_assert(this->content = nullptr);
    this->content = std::make_unique<SelectedContent>(this->sourcePage, this->sourceLayer);
    this->content->readSerialized(in);

    if (this->content->empty()) {
        return;
    }

    this->preserveAspectRatio = false;
    this->supportMirroring = true;
    this->supportRotation = true;
    auto undo = std::make_unique<AddUndoAction>(sourcePage, false);
    Element::Index index = 0;

    // Translate the elements to their "initial" position
    auto delta = initialPos - this->content->getPosition().center;
    this->content->forEachElement([&](Element* e) {
        e->move(delta.x, delta.y);
        undo->addElement(this->sourceLayer, e, index++);
        adaptFlagsToNewElement(e);
    });
    this->content->setOriginalCenter(initialPos);
    this->undo->addUndoAction(std::move(undo));
}

void EditSelection::adaptFlagsToNewElement(Element* e) {
    xoj_assert(e != nullptr);
    // Geometric transformations are enabled if every selected element supports them
    this->preserveAspectRatio = this->preserveAspectRatio || e->rescaleOnlyAspectRatio();
    this->supportMirroring = this->supportMirroring && e->rescaleWithMirror();
    this->supportRotation = this->supportRotation && e->supportRotation();
    // Style modification tools are enabled as soon as one of the elements supports them
    this->supportSetLineWidth = this->supportSetLineWidth || e->supportSetLineWidth();
    this->supportSetColor = this->supportSetColor || e->supportSetColor();
    this->supportSetFill = this->supportSetFill || e->supportSetFill();
    this->supportSetLineStyle = this->supportSetLineStyle || e->supportSetLineStyle();
}

/**
 * Finishes all pending changes, move the elements, scale the elements and add
 * them to new layer if any or to the old if no new layer
 */
void EditSelection::finalizeSelection() {
    XojPageView* v = getPageViewUnderCursor();
    if (v == nullptr) {  // Not on any page - move back to original page and position
        this->content->cancel();
        // TODO(undo)
        // TODO(rerender)
        return;
    }

    this->view = v;
    Range rg = this->content->drop(v->getXournal()->getControl()->getSettings()->getRestoreLineWidthEnabled());

    this->view->getPage()->fireRangeChanged(rg);
    this->viewPool->dispatchAndClear(xoj::view::EditSelectionView::FINALIZATION_REQUEST, rg);

    // PageRef page = this->view->getPage();
    // Layer* layer = page->getSelectedLayer();
    // this->contents->finalizeSelection(this->getRect(), this->snappedBounds, this->preserveAspectRatio, layer, page,
    //                                   this->view, this->undo);

    // This is needed if the selection not was 100% on a page
    // this->view->getXournal()->repaintSelection(true);
}

auto EditSelection::makeMoveEffective() -> InsertionOrder {
    return content->makeMoveEffective(view->getXournal()->getControl()->getSettings()->getRestoreLineWidthEnabled());
}

auto EditSelection::getElementAt(double x, double y) const -> Element* {
    const auto& pos = getPosition();
    const auto& originalPos = this->content->getOriginalPosition();
    x -= pos.center.x;
    y -= pos.center.y;

    const double cos = std::cos(pos.angle), sin = std::sin(pos.angle);
    xoj::util::Point<double> p(cos * x + sin * y, -sin * x + cos * y);

    const double scaleX = pos.halfWidth != 0 ? originalPos.halfWidth / pos.halfWidth : 1.0;
    const double scaleY = pos.halfHeight != 0 ? originalPos.halfHeight / pos.halfHeight : 1.0;
    p = xoj::util::Point<double>(p.x * scaleX, p.y * scaleY) + originalPos.center;

    // p is now in the coordinates as if the selection was never moved
    auto elems = content->getElements();
    auto it = std::find_if(elems.begin(), elems.end(),
                           [&](Element* elem) { return elem->intersectsArea(p.x - 5, p.y - 5, 5, 5); });
    return it != elems.end() ? *it : nullptr;
}

auto EditSelection::getUnrotatedBoundingBox() const -> xoj::util::Rectangle<double> {
    return xoj::util::Rectangle<double>(this->content->computeUnrotatedBoundingBox());
}

/**
 * gets the minimal bounding box containing all elements of the selection used for e.g. grid snapping
 */
auto EditSelection::getSnappedBounds() const -> xoj::util::Rectangle<double> {
    const auto& pos = this->content->getPosition();
    const double w = std::abs(pos.halfWidth), h = std::abs(pos.halfHeight);
    return xoj::util::Rectangle<double>(pos.center.x - w, pos.center.y - h, 2 * w, 2 * h);
}

/**
 * get the original bounding rectangle in document coordinates
 */
auto EditSelection::getOriginalBounds() const -> xoj::util::Rectangle<double> {
    const auto& pos = this->content->getOriginalPosition();
    xoj_assert(pos.halfWidth >= 0 && pos.halfHeight >= 0);  // The original position has no mirroring...
    const double w = pos.halfWidth + this->content->getHorizontalMargin();
    const double h = pos.halfHeight + this->content->getVerticalMargin();
    return xoj::util::Rectangle<double>(pos.center.x - w, pos.center.y - h, 2 * w, 2 * h);
}

auto EditSelection::getRotation() const -> double { return this->getPosition().angle; }

auto EditSelection::getSourcePage() const -> PageRef { return this->sourcePage; }

auto EditSelection::getSourceLayer() const -> Layer* { return this->sourceLayer; }

static void afterOp(std::pair<UndoActionPtr, Range> p, UndoRedoHandler* h,
                    xoj::util::DispatchPool<xoj::view::EditSelectionView>& pool) {
    if (p.first) {
        h->addUndoAction(std::move(p.first));
    }
    if (p.second.isValid()) {
        pool.dispatch(xoj::view::EditSelectionView::FLAG_DIRTY_REGION, p.second);
    }
}

void EditSelection::setSize(ToolSize size, const double* thicknessPen, const double* thicknessHighlighter,
                            const double* thicknessEraser) {
    afterOp(this->content->setSize(size, thicknessPen, thicknessHighlighter, thicknessEraser), undo, *viewPool);
}

void EditSelection::setFill(int alphaPen, int alphaHighligther) {
    afterOp(this->content->setFill(alphaPen, alphaHighligther), undo, *viewPool);
}

void EditSelection::setLineStyle(LineStyle style) { afterOp(this->content->setLineStyle(style), undo, *viewPool); }

void EditSelection::setColor(Color color) { afterOp(this->content->setColor(color), undo, *viewPool); }

void EditSelection::setFont(const XojFont& font) { afterOp(this->content->setFont(font), undo, *viewPool); }

void EditSelection::rearrangeInsertionOrder(const SelectionOrderChange change) {
    afterOp(this->content->rearrangeInsertionOrder(change), undo, *viewPool);
}

void EditSelection::dropAClone() const {
    const auto* settings = view->getXournal()->getControl()->getSettings();
    afterOp(this->content->dropAClone(settings->getRestoreLineWidthEnabled()), undo, *viewPool);
}

void EditSelection::deleteSelection() {
    auto [undo, rg] = this->content->deleteContent();
    if (undo) {
        this->undo->addUndoAction(std::move(undo));
    }
    this->viewPool->dispatchAndClear(xoj::view::EditSelectionView::FINALIZATION_REQUEST, rg);
}

auto EditSelection::getElements() const -> InsertionOrderElementsView { return content->getElements(); }

void EditSelection::onButtonReleaseEvent(double x, double y) {
    if (this->mouseDownType == CURSOR_SELECTION_DELETE) {
        this->view->getXournal()->deleteSelection();
        return;
    }
    xoj_assert(currentOperation);

    UndoActionPtr undoact;
    if (this->mouseDownType == CURSOR_SELECTION_MOVE) {
        // Translations may result in elements changing page
        Layout* layout = gtk_xournal_get_layout(this->view->getXournal()->getWidget());
        XojPageView* v = layout->getPageViewAt(round_cast<int>(x), round_cast<int>(y));
        if (!v || v == this->view) {
            // Stay on the same page
            undoact = this->content->applyTranslation(this->currentOperation->getPosition().center, nullptr, nullptr);
        } else {
            // Change page
            auto p = v->getPage();
            auto l = p->getSelectedLayer();

            xoj::util::Point<double> center(view->getX() - v->getX(), view->getY() - v->getY());
            center /= this->view->getXournal()->getZoom();
            center += this->currentOperation->getPosition().center;  // Center in the new page coordinates

            undoact = this->content->applyTranslation(center, p, l);
            this->view = v;
        }
    } else if (this->mouseDownType == CURSOR_SELECTION_ROTATE) {
        undoact = this->content->applyRotation(this->currentOperation->getPosition().angle);
    } else {  // Rescaling
        xoj_assert(dynamic_cast<Rescaling*>(this->currentOperation.get()));
        undoact = this->content->applyScaling(
                this->currentOperation->getPosition(),
                dynamic_cast<Rescaling*>(this->currentOperation.get())->getCenter(),
                view->getXournal()->getControl()->getSettings()->getRestoreLineWidthEnabled());
    }

    this->undo->addUndoAction(std::move(undoact));
    this->currentOperation.reset();
    this->mouseDownType = CURSOR_SELECTION_NONE;

    const bool wasEdgePanning = this->isEdgePanning();
    this->setEdgePan(false);
    if (wasEdgePanning) {
        this->ensureWithinVisibleArea();
    }
}

void EditSelection::onButtonPressEvent(CursorSelectionType type, double x, double y) {
    double zoom = this->view->getXournal()->getZoom();

    this->mouseDownType = type;
    const auto& pos = this->content->getPosition();
    auto cursorPos = xoj::util::Point<double>(x, y) / zoom;

    xoj_assert(!currentOperation);
    switch (type) {
        case CURSOR_SELECTION_MOVE:
            currentOperation = std::make_unique<Translation>(pos, snappingHandler, cursorPos);
            break;
        case CURSOR_SELECTION_ROTATE:
            currentOperation = std::make_unique<Rotation>(pos, snappingHandler, *this->view->getXournal()->getCursor());
            break;
        case CURSOR_SELECTION_NONE:
            xoj_assert_message(false, "Mouse down event outside of active selection handles");
            break;
        case CURSOR_SELECTION_DELETE:
            // We don't need a current operation for deletion
            break;
        default:  // Scaling
            currentOperation = std::make_unique<Rescaling>(pos, snappingHandler, type, cursorPos, MINPIXSIZE / zoom,
                                                           this->supportMirroring);
            break;
    }
}

void EditSelection::onMotionNotifyEvent(double mouseX, double mouseY, bool alt) {
    double zoom = this->view->getXournal()->getZoom();

    xoj_assert(currentOperation);
    currentOperation->processPoint(xoj::util::Point<double>(mouseX / zoom, mouseY / zoom), alt);

    this->viewPool->dispatch(xoj::view::EditSelectionView::FLAG_DIRTY_REGION,
                             this->content->computeCoarseBoundingBox(this->getPosition()));


    if (this->mouseDownType == CURSOR_SELECTION_MOVE) {
        Layout* layout = gtk_xournal_get_layout(this->view->getXournal()->getWidget());
        XojPageView* v = layout->getPageViewAt(round_cast<int>(mouseX) + this->view->getX(),
                                               round_cast<int>(mouseY) + this->view->getY());
        if (v && v != this->view) {
            XournalView* xournal = this->view->getXournal();
            const auto pageNr = xournal->getControl()->getDocument()->indexOf(v->getPage());

            xournal->pageSelected(pageNr);
        }
    }
}

auto EditSelection::getPageViewUnderCursor() const -> XojPageView* {
    double zoom = view->getXournal()->getZoom();

    // get grabbing hand position
    // double hx = this->view->getX() + (this->snappedBounds.x + this->relMousePosX) * zoom;
    // double hy = this->view->getY() + (this->snappedBounds.y + this->relMousePosY) * zoom;


    // Layout* layout = gtk_xournal_get_layout(this->view->getXournal()->getWidget());
    // XojPageView* v = layout->getPageViewAt(static_cast<int>(hx), static_cast<int>(hy));

    return view;  // v;
}

/**
 * Translate all coordinates which are relative to the current view to the new view,
 * and set the attribute view to the new view
 */
void EditSelection::translateToView(XojPageView* v) {
    // double zoom = view->getXournal()->getZoom();

    // double ox = this->snappedBounds.x - this->x;
    // double oy = this->snappedBounds.y - this->y;
    // int aX1 = this->view->getX() + static_cast<int>(this->x * zoom)
    // int aY1 = this->view->getY() + static_cast<int>(this->y * zoom)
    //
    // this->x = (aX1 - v->getX()) / zoom;
    // this->y = (aY1 - v->getY()) / zoom;
    // this->snappedBounds.x = this->x + ox;
    // this->snappedBounds.y = this->y + oy;

    this->view = v;
}

auto EditSelection::isMoving() const -> bool {
    xoj_assert((mouseDownType != CURSOR_SELECTION_NONE && mouseDownType != CURSOR_SELECTION_DELETE) ==
               (currentOperation != nullptr));
    return this->currentOperation != nullptr;
}

auto EditSelection::getPosition() const -> const Position& {
    return currentOperation ? currentOperation->getPosition() : content->getPosition();
}

void EditSelection::setEdgePan(bool pan) {
    if (pan && !this->edgePanHandler) {
        this->edgePanHandler = g_timeout_source_new(1000 / PAN_TIMER_RATE);
        g_source_set_callback(this->edgePanHandler, xoj::util::wrap_v<EditSelection::handleEdgePan>, this, nullptr);
        g_source_attach(this->edgePanHandler, nullptr);
    } else if (!pan && this->edgePanHandler) {
        g_source_destroy(this->edgePanHandler);
        g_source_unref(this->edgePanHandler);
        this->edgePanHandler = nullptr;
        this->edgePanInhibitNext = false;
    }
}

bool EditSelection::isEdgePanning() const { return this->edgePanHandler; }

bool EditSelection::handleEdgePan(EditSelection* self) {
    if (self->view->getXournal()->getControl()->getZoomControl()->isZoomPresentationMode() || !self->isMoving()) {
        self->setEdgePan(false);
        return false;
    }

    Translation* currentTranslation = dynamic_cast<Translation*>(self->currentOperation.get());
    if (!currentTranslation) {
        self->setEdgePan(false);
        return false;
    }

    Layout* layout = gtk_xournal_get_layout(self->view->getXournal()->getWidget());
    const Settings* const settings = self->getView()->getXournal()->getControl()->getSettings();
    const double zoom = self->view->getXournal()->getZoom();

    // Helper function to compute scroll amount for a single dimension, based on visible region and selection bbox
    const auto computeScrollAmt = [&](double visMin, double visLen, double bboxMin, double bboxLen, double layoutSize,
                                      double relMousePos) -> double {
        const bool belowMin = bboxMin < visMin;
        const bool aboveMax = bboxMin + bboxLen > visMin + visLen;
        const double visMax = visMin + visLen;
        const double bboxMax = bboxMin + bboxLen;

        const bool isLargeSelection = bboxLen > visLen;
        const auto centerVis = (visMin + visLen / 2);
        const auto mouseDiff = (bboxMin + .5 * bboxLen + relMousePos * zoom - centerVis);

        // Scroll amount multiplier
        double mult = 0.0;

        const double maxMult = settings->getEdgePanMaxMult();
        int panDir = 0;

        // If the selection is larger than the view, scroll based on mouse position relative to the center of the
        // visible view Otherwise calculate bonus scroll amount due to proportion of selection out of view.
        if (isLargeSelection) {
            mult = maxMult * std::abs(mouseDiff) / (visLen);
            if (mouseDiff > 0.1 * visLen / 2.0) {
                panDir = 1;
            } else if (mouseDiff < -0.1 * visLen / 2.0) {
                panDir = -1;
            }
        } else {
            if (aboveMax) {
                panDir = 1;
                mult = maxMult * std::min(bboxLen, bboxMax - visMax) / bboxLen;
            } else if (belowMin) {
                panDir = -1;
                mult = maxMult * std::min(bboxLen, visMin - bboxMin) / bboxLen;
            }
        }

        // Base amount to translate selection (in document coordinates) per timer tick
        const double panSpeed = settings->getEdgePanSpeed();
        const double translateAmt = visLen * panSpeed / (100.0 * PAN_TIMER_RATE);

        // Amount to scroll the visible area by (in layout coordinates), accounting for multiplier
        double layoutScroll = zoom * panDir * (translateAmt * mult);

        // If scrolling past layout boundaries, clamp scroll amount to boundary
        if (visMin + layoutScroll < 0) {
            layoutScroll = -visMin;
        } else if (visMax + layoutScroll > layoutSize) {
            layoutScroll = std::max(0.0, layoutSize - visMax);
        }

        return layoutScroll;
    };

    // Compute scroll (for layout) and translation (for selection) for x and y
    const int layoutWidth = layout->getMinimalWidth();
    const int layoutHeight = layout->getMinimalHeight();
    const auto visRect = layout->getVisibleRect();
    const auto bbox = self->getBoundingBoxInView();
    const double layoutScrollX = 0.;  // computeScrollAmt(visRect.x, visRect.width, bbox.x, bbox.width, layoutWidth,
                                      //              currentTranslation->mouseDownPoint.x);
    const double layoutScrollY = 0.;  // computeScrollAmt(visRect.y, visRect.height, bbox.y, bbox.height, layoutHeight,
                                      //            currentTranslation->mouseDownPoint.y);
    const auto translateX = layoutScrollX / zoom;
    const auto translateY = layoutScrollY / zoom;

    // Perform the scrolling
    if (self->isMoving() && (layoutScrollX != 0.0 || layoutScrollY != 0.0)) {
        layout->scrollRelative(layoutScrollX, layoutScrollY);

        auto pos = self->content->getPosition();
        pos.center += {translateX, translateY};
        self->content->setPosition(pos);
        self->view->getXournal()->repaintSelection();

        // To prevent the selection from jumping and to reduce jitter, block the selection movement triggered by user
        // input
        self->edgePanInhibitNext = true;
        return true;
    } else {
        // No panning, so disable the timer.
        self->setEdgePan(false);
        return false;
    }
}

auto EditSelection::getBoundingBoxInView() const -> xoj::util::Rectangle<double> {
    int viewx = this->view->getX();
    int viewy = this->view->getY();
    double zoom = this->view->getXournal()->getZoom();

    auto rg = this->content->computeCoarseBoundingBox(this->getPosition());
    return {viewx + rg.minX * zoom, viewy + rg.minY * zoom, rg.getWidth() * zoom, rg.getHeight() * zoom};
}

void EditSelection::ensureWithinVisibleArea() const {
    const xoj::util::Rectangle<double> viewRect = this->getBoundingBoxInView();
    // need to modify this to take into account the position
    // of the object, plus typecast because XojPageView takes ints
    this->view->getXournal()->ensureRectIsVisible(static_cast<int>(viewRect.x), static_cast<int>(viewRect.y),
                                                  static_cast<int>(viewRect.width), static_cast<int>(viewRect.height));
}

auto EditSelection::getSelectionTypeForPos(double x, double y, double zoom) const -> CursorSelectionType {
    using namespace EditSelectionMeasures;

    const auto& pos = this->content->getPosition();
    double marginScale = this->content->getMarginScale(pos);
    double halfWidth =
            std::abs(pos.halfWidth) + marginScale * this->content->getHorizontalMargin() + CONTENT_PADDING / zoom;
    double halfHeight =
            std::abs(pos.halfHeight) + marginScale * this->content->getVerticalMargin() + CONTENT_PADDING / zoom;
    double signedHalfWidth = std::copysign(halfWidth, pos.halfWidth);
    double signedHalfHeight = std::copysign(halfHeight, pos.halfHeight);

    // Instead of applying the rotation to everything else, we apply the inverse rotation to the cursor position
    const double sin = std::sin(-pos.angle), cos = std::cos(pos.angle);
    x = x / zoom - pos.center.x;
    y = y / zoom - pos.center.y;
    auto p = xoj::util::Point<double>{cos * x - sin * y, sin * x + cos * y};

    auto isCursorInSquare = [&p](double centerX, double centerY, double size) -> bool {
        return centerX - size <= p.x && p.x <= centerX + size && centerY - size <= p.y && p.y <= centerY + size;
    };

    const auto SIZE = STRETCH_HANDLE_SIZE / zoom;

    if (isCursorInSquare(-signedHalfWidth, -signedHalfHeight, SIZE)) {
        return CURSOR_SELECTION_TOP_LEFT;
    }

    if (isCursorInSquare(signedHalfWidth, -signedHalfHeight, SIZE)) {
        return CURSOR_SELECTION_TOP_RIGHT;
    }

    if (isCursorInSquare(-signedHalfWidth, signedHalfHeight, SIZE)) {
        return CURSOR_SELECTION_BOTTOM_LEFT;
    }

    if (isCursorInSquare(signedHalfWidth, signedHalfHeight, SIZE)) {
        return CURSOR_SELECTION_BOTTOM_RIGHT;
    }

    if (isCursorInSquare(-halfWidth - DELETION_HANDLE_DISTANCE / zoom, -signedHalfHeight,
                         DELETION_HANDLE_SIZE / zoom)) {
        return CURSOR_SELECTION_DELETE;
    }


    if (supportRotation &&
        isCursorInSquare(halfWidth + ROTATION_HANDLE_DISTANCE / zoom, 0, ROTATION_HANDLE_SIZE / zoom)) {
        return CURSOR_SELECTION_ROTATE;
    }

    if (!this->preserveAspectRatio) {
        if (-halfWidth <= p.x && p.x <= halfWidth) {
            if (-signedHalfHeight - SIZE <= p.y && p.y <= -signedHalfHeight + SIZE) {
                return CURSOR_SELECTION_TOP;
            }

            if (signedHalfHeight - SIZE <= p.y && p.y <= signedHalfHeight + SIZE) {
                return CURSOR_SELECTION_BOTTOM;
            }
        }

        if (-halfHeight <= p.y && p.y <= halfHeight) {
            if (-signedHalfWidth - SIZE <= p.x && p.x <= -signedHalfWidth + SIZE) {
                return CURSOR_SELECTION_LEFT;
            }

            if (signedHalfWidth - SIZE <= p.x && p.x <= signedHalfWidth + SIZE) {
                return CURSOR_SELECTION_RIGHT;
            }
        }
    }

    if (-halfWidth <= p.x && p.x <= halfWidth && -halfHeight <= p.y && p.y <= halfHeight) {
        return CURSOR_SELECTION_MOVE;
    }

    return CURSOR_SELECTION_NONE;
}
