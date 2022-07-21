#include "StrokeHandler.h"

#include <algorithm>  // for max, min
#include <cassert>    // for assert
#include <cfloat>     // for DBL_EPSILON
#include <cmath>      // for ceil, pow, abs
#include <memory>     // for unique_ptr, mak...
#include <utility>    // for move
#include <vector>     // for vector

#include <gdk/gdk.h>  // for GdkEventKey

#include "control/Control.h"                          // for Control
#include "control/ToolEnums.h"                        // for DRAWING_TYPE_ST...
#include "control/ToolHandler.h"                      // for ToolHandler
#include "control/layer/LayerController.h"            // for LayerController
#include "control/settings/Settings.h"                // for Settings
#include "control/shaperecognizer/ShapeRecognizer.h"  // for ShapeRecognizer
#include "control/tools/InputHandler.h"               // for InputHandler::P...
#include "control/tools/SnapToGridInputHandler.h"     // for SnapToGridInput...
#include "gui/XournalView.h"                          // for XournalView
#include "gui/inputdevices/PositionInputData.h"       // for PositionInputData
#include "model/Layer.h"                              // for Layer
#include "model/Stroke.h"                             // for Stroke, STROKE_...
#include "model/XojPage.h"                            // for XojPage
#include "undo/InsertUndoAction.h"                    // for InsertUndoAction
#include "undo/RecognizerUndoAction.h"                // for RecognizerUndoA...
#include "undo/UndoRedoHandler.h"                     // for UndoRedoHandler
#include "util/DispatchPool.h"                        // for DispatchPool
#include "util/Range.h"                               // for Range
#include "util/Rectangle.h"                           // for Rectangle, util
#include "view/overlays/StrokeToolFilledHighlighterView.h"  // for StrokeToolFilledHighlighterView
#include "view/overlays/StrokeToolFilledView.h"             // for StrokeToolFilledView
#include "view/overlays/StrokeToolView.h"                   // for StrokeToolView

#include "StrokeStabilizer.h"  // for Base, get

using namespace xoj::util;

guint32 StrokeHandler::lastStrokeTime;  // persist for next stroke

StrokeHandler::StrokeHandler(XournalView* xournal, const PageRef& page):
        InputHandler(xournal, page),
        snappingHandler(xournal->getControl()->getSettings()),
        stabilizer(StrokeStabilizer::get(xournal->getControl()->getSettings())),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::StrokeToolView>>()) {}

StrokeHandler::~StrokeHandler() = default;

auto StrokeHandler::onKeyEvent(GdkEventKey* event) -> bool { return false; }


auto StrokeHandler::onMotionNotifyEvent(const PositionInputData& pos) -> bool {
    if (!stroke) {
        return false;
    }

    if (pos.pressure == 0) {
        /**
         * Some devices emit a move event with pressure 0 when lifting the stylus tip
         * Ignore those events
         */
        return true;
    }

    stabilizer->processEvent(pos);
    return true;
}

void StrokeHandler::paintTo(Point point) {
    if (this->hasPressure && point.z > 0.0) {
        point.z *= this->stroke->getWidth();
    }

    auto pointCount = stroke->getPointCount();

    if (pointCount > 0) {
        Point endPoint = stroke->getPoint(pointCount - 1);
        double distance = point.lineLengthTo(endPoint);
        if (distance < PIXEL_MOTION_THRESHOLD) {  //(!validMotion(point, endPoint)) {
            if (pointCount == 1 && this->hasPressure && endPoint.z < point.z) {
                // Record the possible increase in pressure for the first point
                this->stroke->setLastPressure(point.z);
                this->viewPool->dispatch(xoj::view::StrokeToolView::THICKEN_FIRST_POINT_REQUEST, point.z);
            }
            return;
        }
        if (this->hasPressure) {
            /**
             * Both device and tool are pressure sensitive
             */
            if (const double widthDelta = point.z - endPoint.z;
                - widthDelta > MAX_WIDTH_VARIATION || widthDelta > MAX_WIDTH_VARIATION) {
                /**
                 * If the width variation is to big, decompose into shorter segments.
                 * Those segments can not be shorter than PIXEL_MOTION_THRESHOLD
                 */
                double nbSteps = std::min(std::ceil(std::abs(widthDelta) / MAX_WIDTH_VARIATION),
                                          std::floor(distance / PIXEL_MOTION_THRESHOLD));
                double stepLength = 1.0 / nbSteps;
                Point increment((point.x - endPoint.x) * stepLength, (point.y - endPoint.y) * stepLength,
                                widthDelta * stepLength);
                endPoint.z += increment.z;

                for (int i = 1; i < static_cast<int>(nbSteps); i++) {  // The last step is done below
                    endPoint.x += increment.x;
                    endPoint.y += increment.y;
                    endPoint.z += increment.z;
                    drawSegmentTo(endPoint);
                }
            }
        }
    }
    drawSegmentTo(point);
}

void StrokeHandler::drawSegmentTo(const Point& point) {

    this->stroke->addPoint(this->hasPressure ? point : Point(point.x, point.y));
    this->viewPool->dispatch(xoj::view::StrokeToolView::ADD_POINT_REQUEST, this->stroke->getPointVector().back());
    return;
}

void StrokeHandler::onSequenceCancelEvent() {
    if (this->stroke) {
        this->viewPool->dispatch(xoj::view::StrokeToolView::CANCELLATION_REQUEST, Range(this->stroke->boundingRect()));
        stroke.reset();
    }
}

void StrokeHandler::onButtonReleaseEvent(const PositionInputData& pos) {
    if (!stroke) {
        return;
    }

    /**
     * The stabilizer may have added a gap between the end of the stroke and the input device
     * Fill this gap.
     */
    stabilizer->finalizeStroke();


    Control* control = xournal->getControl();
    Settings* settings = control->getSettings();

    if (settings->getStrokeFilterEnabled())  // Note: For shape tools see BaseStrokeHandler which has a slightly
                                             // different version of this filter. See //!
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
            if (pos.timestamp - StrokeHandler::lastStrokeTime > strokeFilterSuccessiveTime) {
                // stroke not being added to layer... delete here but clear first!

                this->viewPool->dispatch(xoj::view::StrokeToolView::CANCELLATION_REQUEST,
                                         Range(this->stroke->boundingRect()));
                stroke.reset();

                this->userTapped = true;

                StrokeHandler::lastStrokeTime = pos.timestamp;

                return;
            }
        }
        StrokeHandler::lastStrokeTime = pos.timestamp;
    }

    // Backward compatibility and also easier to handle for me;-)
    // I cannot draw a line with one point, to draw a visible line I need two points,
    // twice the same Point is also OK
    if (auto const& pv = stroke->getPointVector(); pv.size() == 1) {
        const Point& pt = pv.front();
        if (this->hasPressure) {
            // Pressure inference provides a pressure value to the last event. Most devices set this value to 0.
            const double newPressure = std::max(pt.z, pos.pressure * this->stroke->getWidth());
            this->stroke->setLastPressure(newPressure);
            this->viewPool->dispatch(xoj::view::StrokeToolView::THICKEN_FIRST_POINT_REQUEST, newPressure);
        }
        stroke->addPoint(pt);
    }

    stroke->freeUnusedPointItems();

    control->getLayerController()->ensureLayerExists(page);

    Layer* layer = page->getSelectedLayer();

    UndoRedoHandler* undo = control->getUndoRedoHandler();

    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke.get()));

    ToolHandler* h = control->getToolHandler();

    if (h->getDrawingType() == DRAWING_TYPE_STROKE_RECOGNIZER) {
        ShapeRecognizer reco;

        Stroke* recognized = reco.recognizePatterns(stroke.get());

        if (recognized) {
            strokeRecognizerDetected(recognized, layer);

            // Full repaint is done anyway
            // So repaint don't need to be done here

            stroke.release();  // The stroke is now owned by the UndoRedoHandler (to undo the recognition)
            return;
        }
    }

    layer->addElement(stroke.get());
    page->fireElementChanged(stroke.get());
    stroke.release();
}

void StrokeHandler::strokeRecognizerDetected(Stroke* recognized, Layer* layer) {
    recognized->setWidth(stroke->hasPressure() ? stroke->getAvgPressure() : stroke->getWidth());

    // snapping
    Stroke* snappedStroke = recognized->cloneStroke();
    if (xournal->getControl()->getSettings()->getSnapRecognizedShapesEnabled()) {
        Rectangle<double> oldSnappedBounds = recognized->getSnappedBounds();
        Point topLeft = Point(oldSnappedBounds.x, oldSnappedBounds.y);
        Point topLeftSnapped = snappingHandler.snapToGrid(topLeft, false);

        snappedStroke->move(topLeftSnapped.x - topLeft.x, topLeftSnapped.y - topLeft.y);
        Rectangle<double> snappedBounds = snappedStroke->getSnappedBounds();
        Point belowRight = Point(snappedBounds.x + snappedBounds.width, snappedBounds.y + snappedBounds.height);
        Point belowRightSnapped = snappingHandler.snapToGrid(belowRight, false);

        double fx = (std::abs(snappedBounds.width) > DBL_EPSILON) ?
                            (belowRightSnapped.x - topLeftSnapped.x) / snappedBounds.width :
                            1;
        double fy = (std::abs(snappedBounds.height) > DBL_EPSILON) ?
                            (belowRightSnapped.y - topLeftSnapped.y) / snappedBounds.height :
                            1;
        snappedStroke->scale(topLeftSnapped.x, topLeftSnapped.y, fx, fy, 0, false);
    }

    auto recognizerUndo = std::make_unique<RecognizerUndoAction>(page, layer, stroke.get(), snappedStroke);

    UndoRedoHandler* undo = xournal->getControl()->getUndoRedoHandler();
    undo->addUndoAction(std::move(recognizerUndo));
    layer->addElement(snappedStroke);

    Range range(snappedStroke->getX(), snappedStroke->getY());
    range.addPoint(snappedStroke->getX() + snappedStroke->getElementWidth(),
                   snappedStroke->getY() + snappedStroke->getElementHeight());

    range.addPoint(stroke->getX(), stroke->getY());
    range.addPoint(stroke->getX() + stroke->getElementWidth(), stroke->getY() + stroke->getElementHeight());

    page->fireRangeChanged(range);
}

void StrokeHandler::onButtonPressEvent(const PositionInputData& pos) {
    assert(!stroke);

    const double zoom = xournal->getZoom();

    this->buttonDownPoint.x = pos.x / zoom;
    this->buttonDownPoint.y = pos.y / zoom;

    stroke = createStroke(this->xournal->getControl());

    this->hasPressure = this->stroke->getToolType() == STROKE_TOOL_PEN && pos.pressure != Point::NO_PRESSURE;

    const double width = this->hasPressure ? pos.pressure * stroke->getWidth() : Point::NO_PRESSURE;
    stroke->addPoint(Point(this->buttonDownPoint.x, this->buttonDownPoint.y, width));

    stabilizer->initialize(this, zoom, pos);

    this->startStrokeTime = pos.timestamp;
}

void StrokeHandler::onButtonDoublePressEvent(const PositionInputData& pos) {
    // nothing to do
}

auto StrokeHandler::createView(const xoj::view::Repaintable* parent) const -> std::unique_ptr<xoj::view::OverlayView> {
    assert(this->stroke);
    const Stroke& s = *this->stroke;
    if (s.getFill() != -1) {
        if (s.getToolType() == STROKE_TOOL_HIGHLIGHTER) {
            // Filled highlighter requires to wipe the mask entirely at every iteration
            // It has a dedicated view class.
            return std::make_unique<xoj::view::StrokeToolFilledHighlighterView>(this, s, parent);
        } else {
            return std::make_unique<xoj::view::StrokeToolFilledView>(this, s, parent);
        }
    } else {
        return std::make_unique<xoj::view::StrokeToolView>(this, s, parent);
    }
}

auto StrokeHandler::getViewPool() const -> const std::shared_ptr<xoj::util::DispatchPool<xoj::view::StrokeToolView>>& {
    return viewPool;
}
