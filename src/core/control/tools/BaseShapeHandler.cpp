#include "BaseShapeHandler.h"

#include <cassert>  // for assert
#include <cmath>    // for pow, NAN
#include <memory>   // for make_unique, __share...

#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Alt_L, GDK_K...

#include "control/Control.h"                       // for Control
#include "control/layer/LayerController.h"         // for LayerController
#include "control/settings/Settings.h"             // for Settings
#include "control/tools/InputHandler.h"            // for InputHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHandler
#include "gui/XournalView.h"                       // for XournalView
#include "gui/XournalppCursor.h"                   // for XournalppCursor
#include "gui/inputdevices/PositionInputData.h"    // for PositionInputData
#include "model/Layer.h"                           // for Layer
#include "model/Stroke.h"                          // for Stroke
#include "model/XojPage.h"                         // for XojPage
#include "undo/InsertUndoAction.h"                 // for InsertUndoAction
#include "undo/UndoRedoHandler.h"                  // for UndoRedoHandler
#include "util/DispatchPool.h"                     // for DispatchPool
#include "util/Range.h"                            // for Range
#include "util/Rectangle.h"                        // for Rectangle
#include "view/overlays/ShapeToolView.h"           // for ShapeToolView

using xoj::util::Rectangle;

guint32 BaseShapeHandler::lastStrokeTime;  // persist for next stroke


BaseShapeHandler::BaseShapeHandler(XournalView* xournal, const PageRef& page, bool flipShift, bool flipControl):
        InputHandler(xournal, page),
        flipShift(flipShift),
        flipControl(flipControl),
        snappingHandler(xournal->getControl()->getSettings()),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::ShapeToolView>>()) {}

BaseShapeHandler::~BaseShapeHandler() = default;

void BaseShapeHandler::updateShape(const PositionInputData& pos) {
    auto [shape, rg] = this->createShape(pos);
    std::swap(shape, this->shape);

    rg.addPadding(0.5 * this->stroke->getWidth());
    Range repaintRange = rg.unite(lastDrawingRange);
    lastDrawingRange = rg;
    viewPool->dispatch(xoj::view::ShapeToolView::FLAG_DIRTY_REGION, repaintRange);
}

void BaseShapeHandler::cancelStroke() {
    this->shape.clear();
    this->viewPool->dispatch(xoj::view::ShapeToolView::FLAG_DIRTY_REGION, this->lastDrawingRange);
    this->lastDrawingRange = Range();
}

auto BaseShapeHandler::onKeyEvent(GdkEventKey* event) -> bool {
    if (event->is_modifier) {
        PositionInputData pos{};
        pos.x = pos.y = pos.pressure = 0;  // not used in redraw
        if (event->keyval == GDK_KEY_Shift_L || event->keyval == GDK_KEY_Shift_R) {
            pos.state = static_cast<GdkModifierType>(
                    event->state ^ GDK_SHIFT_MASK);  // event state does not include current this modifier keypress - so
                                                     // ^toggle will work for press and release.
        } else if (event->keyval == GDK_KEY_Control_L || event->keyval == GDK_KEY_Control_R) {
            pos.state = static_cast<GdkModifierType>(event->state ^ GDK_CONTROL_MASK);
        } else if (event->keyval == GDK_KEY_Alt_L || event->keyval == GDK_KEY_Alt_R) {
            pos.state = static_cast<GdkModifierType>(event->state ^ GDK_MOD1_MASK);
        } else {
            return false;
        }

        this->updateShape(pos);

        return true;
    }
    return false;
}

auto BaseShapeHandler::onMotionNotifyEvent(const PositionInputData& pos) -> bool {
    double zoom = xournal->getZoom();
    double x = pos.x / zoom;
    double y = pos.y / zoom;

    Point newPoint(x, y);
    if (!validMotion(newPoint, this->currPoint)) {
        return true;
    }
    this->currPoint = newPoint;

    this->updateShape(pos);

    return true;
}

void BaseShapeHandler::onSequenceCancelEvent() { this->cancelStroke(); }

void BaseShapeHandler::onButtonReleaseEvent(const PositionInputData& pos) {
    xournal->getCursor()->activateDrawDirCursor(false);  // in case released within  fixate_Dir_Mods_Dist

    Control* control = xournal->getControl();
    Settings* settings = control->getSettings();

    if (settings->getStrokeFilterEnabled())  // Note: For simple strokes see StrokeHandler which has a slightly
                                             // different version of this filter.  See //!
    {
        int strokeFilterIgnoreTime = 0, strokeFilterSuccessiveTime = 0;
        double strokeFilterIgnoreLength = NAN;

        settings->getStrokeFilter(&strokeFilterIgnoreTime, &strokeFilterIgnoreLength, &strokeFilterSuccessiveTime);
        double dpmm = settings->getDisplayDpi() / 25.4;

        double zoom = xournal->getZoom();
        double lengthSqrd = (pow(((pos.x / zoom) - (this->buttonDownPoint.x)), 2) +
                             pow(((pos.y / zoom) - (this->buttonDownPoint.y)), 2)) *
                            pow(xournal->getZoom(), 2);

        if (lengthSqrd < pow((strokeFilterIgnoreLength * dpmm), 2) &&
            pos.timestamp - this->startStrokeTime < strokeFilterIgnoreTime) {
            if (pos.timestamp - BaseShapeHandler::lastStrokeTime > strokeFilterSuccessiveTime) {
                // stroke not being added to layer... delete here.
                this->cancelStroke();

                this->userTapped = true;

                BaseShapeHandler::lastStrokeTime = pos.timestamp;

                xournal->getCursor()->updateCursor();

                return;
            }
        }
        BaseShapeHandler::lastStrokeTime = pos.timestamp;
    }

    control->getLayerController()->ensureLayerExists(page);

    Layer* layer = page->getSelectedLayer();

    UndoRedoHandler* undo = control->getUndoRedoHandler();

    auto [shape, snappingBox] = this->createShape(pos);
    stroke->setPointVector(shape, &snappingBox);

    /*
     * Update the shape, for one last drawing operation triggered by page->fireElementChanged below
     * This avoids the stroke blinking.
     */
    std::swap(shape, this->shape);

    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke.get()));

    layer->addElement(stroke.get());
    page->fireElementChanged(stroke.get());
    stroke.release();

    xournal->getCursor()->updateCursor();
}

void BaseShapeHandler::onButtonPressEvent(const PositionInputData& pos) {
    assert(this->viewPool->empty());
    double zoom = xournal->getZoom();
    this->buttonDownPoint.x = pos.x / zoom;
    this->buttonDownPoint.y = pos.y / zoom;

    this->startStrokeTime = pos.timestamp;
    this->startPoint = snappingHandler.snapToGrid(this->buttonDownPoint, pos.isAltDown());

    this->stroke = createStroke(this->xournal->getControl());
}

void BaseShapeHandler::onButtonDoublePressEvent(const PositionInputData& pos) {
    // nothing to do
}

void BaseShapeHandler::modifyModifiersByDrawDir(double width, double height, bool changeCursor) {
    bool gestureShift = this->flipShift;
    bool gestureControl = this->flipControl;

    if (this->drawModifierFixed == NONE) {
        // User hasn't dragged out past DrawDirModsRadius  i.e. modifier not yet locked.
        gestureShift = (width < 0) != gestureShift;
        gestureControl = (height < 0) != gestureControl;

        this->modShift = this->modShift == !gestureShift;
        this->modControl = this->modControl == !gestureControl;

        double zoom = xournal->getZoom();
        double fixate_Dir_Mods_Dist = xournal->getControl()->getSettings()->getDrawDirModsRadius() / zoom;
        assert(fixate_Dir_Mods_Dist > 0.0);
        if (std::abs(width) > fixate_Dir_Mods_Dist || std::abs(height) > fixate_Dir_Mods_Dist) {
            this->drawModifierFixed = static_cast<DIRSET_MODIFIERS>(SET | (gestureShift ? SHIFT : NONE) |
                                                                    (gestureControl ? CONTROL : NONE));
            if (changeCursor) {
                xournal->getCursor()->activateDrawDirCursor(false);
            }
        } else {
            if (changeCursor) {
                xournal->getCursor()->activateDrawDirCursor(true, this->modShift, this->modControl);
            }
        }
    } else {
        gestureShift = gestureShift == !(this->drawModifierFixed & SHIFT);
        gestureControl = gestureControl == !(this->drawModifierFixed & CONTROL);
        this->modShift = this->modShift == !gestureShift;
        this->modControl = this->modControl == !gestureControl;
    }
}

auto BaseShapeHandler::getShape() const -> const std::vector<Point>& { return this->shape; }

auto BaseShapeHandler::getViewPool() const
        -> const std::shared_ptr<xoj::util::DispatchPool<xoj::view::ShapeToolView>>& {
    return viewPool;
}
