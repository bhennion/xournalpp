#include "StrokeHandler.h"

#include <algorithm>  // for max, min
#include <cassert>    // for assert
#include <cmath>      // for ceil, pow, abs
#include <limits>     // for numeric_limits
#include <memory>     // for unique_ptr, mak...
#include <utility>    // for move
#include <vector>     // for vector

#include <gdk/gdk.h>  // for GdkEventKey

#include "control/Control.h"                          // for Control
#include "control/ToolEnums.h"                        // for DRAWING_TYPE_ST...
#include "control/ToolHandler.h"                      // for ToolHandler
#include "control/layer/LayerController.h"            // for LayerController
#include "control/settings/Settings.h"                // for Settings
#include "control/settings/SettingsEnums.h"           // for EmptyLastPageAppendType
#include "control/shaperecognizer/ShapeRecognizer.h"  // for ShapeRecognizer
#include "control/tools/InputHandler.h"               // for InputHandler::P...
#include "control/tools/SnapToGridInputHandler.h"     // for SnapToGridInput...
#include "control/zoom/ZoomControl.h"
#include "gui/MainWindow.h"
#include "gui/PageView.h"                        // for XojPageView
#include "gui/XournalView.h"                     // for XournalView
#include "gui/inputdevices/PositionInputData.h"  // for PositionInputData
#include "model/Document.h"                      // for Document
#include "model/Layer.h"                         // for Layer
#include "model/LineStyle.h"                     // for LineStyle
#include "model/Stroke.h"                        // for Stroke, STROKE_...
#include "model/XojPage.h"                       // for XojPage
#include "model/path/PiecewiseLinearPath.h"
#include "splineapproximation/SplineApproximatorLive.h"
#include "undo/InsertUndoAction.h"               // for InsertUndoAction
#include "undo/RecognizerUndoAction.h"           // for RecognizerUndoA...
#include "undo/UndoRedoHandler.h"                // for UndoRedoHandler
#include "util/Color.h"                          // for cairo_set_sourc...
#include "util/Range.h"                          // for Range
#include "util/Rectangle.h"                      // for Rectangle, util
#include "view/StrokeView.h"                     // for StrokeView, Str...
#include "view/View.h"                           // for Context

#include "StrokeStabilizer.h"  // for Base, get

using namespace xoj::util;

guint32 StrokeHandler::lastStrokeTime;  // persist for next stroke

StrokeHandler::StrokeHandler(Control* control, XojPageView* pageView, const PageRef& page):
        InputHandler(control, page),
        snappingHandler(control->getSettings()),
        stabilizer(StrokeStabilizer::get(control->getSettings())),
        pageView(pageView) {}

void StrokeHandler::draw(cairo_t* cr) {
    assert(stroke && stroke->hasPath() && !stroke->getPath().empty());

    auto setColorAndBlendMode = [stroke = this->stroke.get(), cr]() {
        if (stroke->getToolType() == StrokeTool::HIGHLIGHTER) {
            if (auto fill = stroke->getFill(); fill != -1) {
                Util::cairo_set_source_rgbi(cr, stroke->getColor(), static_cast<double>(fill) / 255.0);
            } else {
                Util::cairo_set_source_rgbi(cr, stroke->getColor(), xoj::view::StrokeView::OPACITY_HIGHLIGHTER);
            }
            cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY);
        } else {
            Util::cairo_set_source_rgbi(cr, stroke->getColor());
            cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        }
    };

    if (this->mask) {
        setColorAndBlendMode();
        if (this->splineLiveApproximation && this->approximatedSpline->nbSegments() != 0) {
            this->liveSegmentStroke->setPath(std::make_shared<Spline>(this->liveApprox->liveSegment));
            this->liveSegmentStroke->clearPointCache();

            cairo_set_operator(maskForLiveSegment->cr, CAIRO_OPERATOR_CLEAR);
            cairo_paint(maskForLiveSegment->cr);

            xoj::view::StrokeView v(this->liveSegmentStroke.get());
            v.draw(xoj::view::Context::createColorBlind(maskForLiveSegment->cr));

            cairo_set_operator(maskForLiveSegment->cr, CAIRO_OPERATOR_SOURCE);
            cairo_mask_surface(maskForLiveSegment->cr, mask->surf, 0, 0);
            cairo_mask_surface(cr, maskForLiveSegment->surf, 0, 0);
        } else {
            cairo_mask_surface(cr, mask->surf, 0, 0);
        }
    } else {
        if (const Path& p = this->stroke->getPath(); p.nbSegments() == 0) {
            // StrokeView does not handle single dots
            const Point& pt = p.getFirstKnot();
            double width = this->hasPressure ? pt.z : this->stroke->getWidth();
            setColorAndBlendMode();
            this->paintDot(cr, pt.x, pt.y, width);
        } else {
            if (this->splineLiveApproximation && this->hasPressure) {
                // Update the liveSegment in the point cache
                this->stroke->resizePointCache(this->liveSegmentPointCacheBegin);
                this->stroke->addToPointCache(this->liveApprox->liveSegment);
            }
            strokeView->draw(xoj::view::Context::createDefault(cr));
        }
    }
#ifdef DEBUG_FPS
    frameCnt++;
#endif
}

auto StrokeHandler::onKeyEvent(GdkEventKey* event) -> bool { return false; }

auto StrokeHandler::onMotionNotifyEvent(const PositionInputData& pos, double zoom) -> bool {
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
    if (this->hasPressure) {
        point.z *= this->stroke->getWidth();
    }
    if (!this->splineLiveApproximation) {
        const Point endPoint = this->path->getLastKnot();  // Make a copy as push to the vector might invalidate a ref.
        const double distance = point.lineLengthTo(endPoint);
        if (distance < PIXEL_MOTION_THRESHOLD) {
            if (this->path->nbSegments() == 0 && this->hasPressure && endPoint.z < point.z) {
                // Record the possible increase in pressure for the first point
                this->path->setFirstKnotPressure(point.z);

                if (mask) {
                    this->paintDot(mask->cr, endPoint.x, endPoint.y, point.z);
                }
                // Trigger a call to `draw`. If mask == nullopt, the `paintDot` is called in `draw`
                this->pageView->repaintRect(endPoint.x - 0.5 * point.z, endPoint.y - 0.5 * point.z, point.z, point.z);
            }
            return;
        }
        if (this->hasPressure && std::abs(point.z - endPoint.z) > MAX_WIDTH_VARIATION) {
            /**
             * Both device and tool are pressure sensitive.
             * If the width variation is to big, decompose into shorter segments.
             * Those segments can not be shorter than PIXEL_MOTION_THRESHOLD
             */
            MathVect3 increment(endPoint, point);
            double nbSteps = std::min(std::ceil(std::abs(increment.dz) / MAX_WIDTH_VARIATION),
                                      std::floor(distance / PIXEL_MOTION_THRESHOLD));
            double stepLength = 1.0 / nbSteps;
            increment *= stepLength;

            MathVect3 diffVector{0, 0, 0};
            for (int i = 1; i < static_cast<int>(nbSteps); i++) {  // The last step is done below
                diffVector += increment;
                drawSegmentTo(diffVector.translatePoint(endPoint));
            }
        }
    } else {
        Point& endPoint = *this->liveApprox->P0;
        if (point.lineLengthTo(endPoint) < PIXEL_MOTION_THRESHOLD) {
            if (this->liveApprox->dataCount == 1 && this->hasPressure && endPoint.z < point.z) {
                // Record the possible increase in pressure for the first point
                endPoint.z = point.z;
                this->approximatedSpline->setFirstKnotPressure(point.z);

                // Paint the dot for the user to see
                if (mask) {
                    this->paintDot(mask->cr, endPoint.x, endPoint.y, point.z);
                }
                // Trigger a call to `draw`. If mask == nullopt, the `paintDot` is called in `draw`
                this->pageView->repaintRect(endPoint.x - 0.5 * point.z, endPoint.y - 0.5 * point.z, point.z, point.z);
            }
            return;
        }
    }
    drawSegmentTo(point);
}

void StrokeHandler::onSequenceCancelEvent() { stroke.reset(); }

void StrokeHandler::onButtonReleaseEvent(const PositionInputData& pos, double zoom) {
    if (!this->stroke ||
        ((!this->path || this->path->empty()) && (!this->approximatedSpline || this->approximatedSpline->empty()))) {
        return;
    }

    /**
     * The stabilizer may have added a gap between the end of the stroke and the input device
     * Fill this gap.
     */
    stabilizer->finalizeStroke();

    Settings* settings = control->getSettings();

    if (settings->getStrokeFilterEnabled())  // Note: For shape tools see BaseStrokeHandler which has a slightly
                                             // different version of this filter. See //!
    {
        int strokeFilterIgnoreTime = 0, strokeFilterSuccessiveTime = 0;
        double strokeFilterIgnoreLength = NAN;

        settings->getStrokeFilter(&strokeFilterIgnoreTime, &strokeFilterIgnoreLength, &strokeFilterSuccessiveTime);
        double dpmm = settings->getDisplayDpi() / 25.4;

        double lengthSqrd = (pow(((pos.x / zoom) - (this->buttonDownPoint.x)), 2) +
                             pow(((pos.y / zoom) - (this->buttonDownPoint.y)), 2)) *
                            pow(zoom, 2);

        if (lengthSqrd < pow((strokeFilterIgnoreLength * dpmm), 2) &&
            pos.timestamp - this->startStrokeTime < strokeFilterIgnoreTime) {
            if (pos.timestamp - StrokeHandler::lastStrokeTime > strokeFilterSuccessiveTime) {
                // stroke not being added to layer... delete here but clear first!

                this->pageView->rerenderRect(stroke->getX(), stroke->getY(), stroke->getElementWidth(),
                                             stroke->getElementHeight());  // clear onMotionNotifyEvent drawing //!

                stroke.reset();
                this->userTapped = true;

                StrokeHandler::lastStrokeTime = pos.timestamp;

                return;
            }
        }
        StrokeHandler::lastStrokeTime = pos.timestamp;
    }

    bool enoughPoints = true;
    if (this->splineLiveApproximation) {
        enoughPoints = this->liveApprox->dataCount >= 3;
        if (enoughPoints) {
            // Try to approximate the last points
            bool lastFitSuccess = this->liveApprox->finalize();

            // Draw the last spline segment
            const SplineSegment& liveSegment = this->liveApprox->liveSegment;

            Rectangle<double> rect;
            if (this->hasPressure) {
                rect = liveSegment.getThickBoundingBox();
            } else {
                rect = liveSegment.getThinBoundingBox();
                rect.addPadding(0.5 * this->stroke->getWidth());
            }

            if (mask) {
                Stroke liveSegmentStroke;
                std::shared_ptr<Spline> segs;
                if (lastFitSuccess) {
                    segs = std::make_shared<Spline>(liveSegment);
                } else {
                    segs = std::make_shared<Spline>(this->liveApprox->lastDefinitiveSegment);
                    segs->addCubicSegment(liveSegment);
                }
                liveSegmentStroke.setPath(segs);
                liveSegmentStroke.setWidth(this->stroke->getWidth());
                liveSegmentStroke.setPressureSensitive(this->hasPressure);

                xoj::view::StrokeView sView(&liveSegmentStroke);
                sView.draw(xoj::view::Context::createColorBlind(mask->cr));
            } else {
                if (this->stroke->getFill() != -1) {
                    /**
                     * Need to fill the area delimited by liveSegment and firstKnot
                     * Add the first knot to the rectangle
                     */
                    const Point& firstPoint = this->approximatedSpline->getFirstKnot();

                    double maxX = std::max(rect.x + rect.width, firstPoint.x);
                    double maxY = std::max(rect.y + rect.height, firstPoint.y);
                    rect.x = std::min(rect.x, firstPoint.x);
                    rect.y = std::min(rect.y, firstPoint.y);
                    rect.width = maxX - rect.x;
                    rect.height = maxY - rect.y;
                }
            }
            this->pageView->repaintRect(rect.x, rect.y, rect.width, rect.height);
            this->liveApprox->printStats();
        } else {
            // The stroke only has 1 or 2 points.
            // Either a degenerate line segment, or an actual line segment
            if (this->liveApprox->dataCount == 1) {
                Point& pt = *this->liveApprox->P0;
                if (this->hasPressure) {
                    // Pressure inference provides a pressure value to the last event. Most devices set this value to 0.
                    pt.z = std::max(pt.z, pos.pressure * this->stroke->getWidth());
                }
                this->stroke->setPath(std::make_shared<PiecewiseLinearPath>(pt, pt));
            } else {
                // 2 points
                this->stroke->setPath(std::make_shared<PiecewiseLinearPath>(this->liveApprox->liveSegment.firstKnot,
                                                                            this->liveApprox->liveSegment.secondKnot));
            }
        }
    } else {
        enoughPoints = this->path->nbSegments() >= 1;
        if (!enoughPoints) {
            // We cannot draw a line with one point, to draw a visible line we need two points,
            // Twice the same Point is also OK
            const Point& pt = this->path->getFirstKnot();
            if (this->hasPressure) {
                // Pressure inference provides a pressure value to the last event. Most devices set this value to 0.
                this->path->setFirstKnotPressure(std::max(pt.z, pos.pressure * this->stroke->getWidth()));
            }
            this->path->close();  // Copy the first knot to make a second point.
        }
    }

    stroke->freeUnusedPointItems();

    Layer* layer = page->getSelectedLayer();

    UndoRedoHandler* undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke.get()));

    if (settings->getEmptyLastPageAppend() == EmptyLastPageAppendType::OnDrawOfLastPage) {
        auto* doc = control->getDocument();
        doc->lock();
        auto pdfPageCount = doc->getPdfPageCount();
        doc->unlock();
        if (pdfPageCount == 0) {
            auto currentPage = control->getCurrentPageNo();
            doc->lock();
            auto lastPage = doc->getPageCount() - 1;
            doc->unlock();
            if (currentPage == lastPage) {
                control->insertNewPage(currentPage + 1, false);
            }
        }
    }

    ToolHandler* h = control->getToolHandler();
    if (!this->splineLiveApproximation && h->getDrawingType() == DRAWING_TYPE_STROKE_RECOGNIZER) {
        ShapeRecognizer reco(*this->path);

        std::shared_ptr<Path> result = reco.recognizePatterns(control->getSettings()->getStrokeRecognizerMinSize());

        if (result) {
            strokeRecognizerDetected(result, layer);

            // Full repaint is done anyway
            // So repaint don't need to be done here

            stroke.release();  // The stroke is now owned by the UndoRedoHandler (to undo the recognition)
            return;
        }
    }

    const bool postApproximation =
            enoughPoints && this->control->getSettings()->getSplineApproximatorType() == SplineApproximator::Type::POST;
    if (postApproximation) {
        /**
         * Approximate the stroke by a spline using Schneider's algorithm
         */
        auto rect = stroke->boundingRect();
        stroke->splineFromPLPath();
        rect.unite(stroke->boundingRect());
        layer->addElement(stroke.release());
        this->pageView->rerenderRect(rect.x, rect.y, rect.width, rect.height);
    } else {
        // Add the element
        layer->addElement(stroke.get());
        page->fireElementChanged(stroke.get());
        stroke.release();
    }
}


void StrokeHandler::strokeRecognizerDetected(std::shared_ptr<Path> result, Layer* layer) {

    // snapping
    if (control->getSettings()->getSnapRecognizedShapesEnabled()) {
        Rectangle<double> oldSnappedBounds = result->getThinBoundingBox();

        Point topLeft = Point(oldSnappedBounds.x, oldSnappedBounds.y);
        Point topLeftSnapped = snappingHandler.snapToGrid(topLeft, false);

        result->move(topLeftSnapped.x - topLeft.x, topLeftSnapped.y - topLeft.y);

        Point belowRight = Point(topLeftSnapped.x + oldSnappedBounds.width, topLeftSnapped.y + oldSnappedBounds.height);
        Point belowRightSnapped = snappingHandler.snapToGrid(belowRight, false);

        double fx = (std::abs(oldSnappedBounds.width) > std::numeric_limits<double>::epsilon()) ?
                            (belowRightSnapped.x - topLeftSnapped.x) / oldSnappedBounds.width :
                            1;
        double fy = (std::abs(oldSnappedBounds.height) > std::numeric_limits<double>::epsilon()) ?
                            (belowRightSnapped.y - topLeftSnapped.y) / oldSnappedBounds.height :
                            1;
        result->scale(topLeftSnapped.x, topLeftSnapped.y, fx, fy, 0, false);
    }

    Stroke* recognized = new Stroke();
    recognized->setPath(result);

    recognized->applyStyleFrom(this->stroke.get());
    recognized->setWidth(this->stroke->hasPressure() ? this->path->getAveragePressure() : this->stroke->getWidth());

    auto recognizerUndo = std::make_unique<RecognizerUndoAction>(page, layer, stroke.get(), recognized);

    UndoRedoHandler* undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::move(recognizerUndo));
    layer->addElement(recognized);

    Range range(recognized->getX(), recognized->getY());
    range.addPoint(recognized->getX() + recognized->getElementWidth(),
                   recognized->getY() + recognized->getElementHeight());

    range.addPoint(stroke->getX(), stroke->getY());
    range.addPoint(stroke->getX() + stroke->getElementWidth(), stroke->getY() + stroke->getElementHeight());

    page->fireRangeChanged(range);
}

void StrokeHandler::onButtonPressEvent(const PositionInputData& pos, double zoom) {
    assert(!stroke);

    stroke = createStroke(this->control);

    // Set boolean flags
    this->hasPressure = this->stroke->getToolType().isPressureSensitive() && pos.pressure != Point::NO_PRESSURE;
    this->stroke->setPressureSensitive(this->hasPressure);

    this->splineLiveApproximation =
            this->control->getSettings()->getSplineApproximatorType() == SplineApproximator::Type::LIVE;

    const bool needAMask = this->stroke->getFill() == -1 && !stroke->getLineStyle().hasDashes();

    this->buttonDownPoint.x = pos.x / zoom;
    this->buttonDownPoint.y = pos.y / zoom;

    // Setup stroke path
    if (this->splineLiveApproximation) {
        this->approximatedSpline = std::make_shared<Spline>(
                Point(this->buttonDownPoint.x, this->buttonDownPoint.y,
                      this->hasPressure ? pos.pressure * stroke->getWidth() : Point::NO_PRESSURE));
        this->liveApprox = std::make_unique<SplineApproximator::Live>(this->approximatedSpline);
        this->stroke->setPath(this->approximatedSpline);
        this->liveSegmentStroke = std::make_unique<Stroke>();
        this->liveSegmentStroke->setWidth(this->stroke->getWidth());
        this->liveSegmentStroke->setPressureSensitive(this->hasPressure);

        this->drawEvent = needAMask ?
                                  std::bind(&StrokeHandler::normalDrawLiveApproximator, this, std::placeholders::_1) :
                                  std::bind(&StrokeHandler::fullRedrawLiveApproximator, this, std::placeholders::_1);
    } else {
        this->path = std::make_shared<PiecewiseLinearPath>(
                Point(this->buttonDownPoint.x, this->buttonDownPoint.y,
                      this->hasPressure ? pos.pressure * stroke->getWidth() : Point::NO_PRESSURE));
        this->stroke->setPath(this->path);
        this->drawEvent = needAMask ? std::bind(&StrokeHandler::normalDraw, this, std::placeholders::_1) :
                                      std::bind(&StrokeHandler::fullRedraw, this, std::placeholders::_1);
    }

    stabilizer->initialize(this, zoom, pos);

    double width = this->hasPressure ? this->stroke->getWidth() * pos.pressure : this->stroke->getWidth();

    if (needAMask) {
        // Strokes that require a full redraw don't use a mask
        this->createMask(mask);
        this->paintDot(mask->cr, this->buttonDownPoint.x, this->buttonDownPoint.y, width);
        if (this->splineLiveApproximation) {
            this->createMask(maskForLiveSegment);
        }
    } else {
        strokeView.emplace(stroke.get());
    }
    this->pageView->repaintRect(this->buttonDownPoint.x - 0.5 * width, this->buttonDownPoint.y - 0.5 * width, width,
                                width);

    this->startStrokeTime = pos.timestamp;
}

void StrokeHandler::onButtonDoublePressEvent(const PositionInputData&, double) {
    // nothing to do
}

void StrokeHandler::paintDot(cairo_t* cr, const double x, const double y, const double width) const {
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(cr, width);
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x, y);
    cairo_stroke(cr);
}

StrokeHandler::Mask::Mask(int width, int height) {
    surf = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);
    cr = cairo_create(surf);
    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
}

StrokeHandler::Mask::~Mask() noexcept {
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

void StrokeHandler::createMask(std::optional<Mask>& m) {
    // todo(bhennion) Make xournal-independent on splitting
    auto xournal = control->getWindow()->getXournal();
    const double ratio = control->getZoomControl()->getZoom() * static_cast<double>(xournal->getDpiScaleFactor());

    std::unique_ptr<Rectangle<double>> visibleRect(xournal->getVisibleRect(pageView));

    // We add a padding to limit graphical bugs when scrolling right after completing a stroke
    const double strokeWidth = this->stroke->getWidth();
    const int width = static_cast<int>(std::ceil((visibleRect->width + strokeWidth) * ratio));
    const int height = static_cast<int>(std::ceil((visibleRect->height + strokeWidth) * ratio));

    m.emplace(width, height);

    cairo_surface_set_device_offset(m->surf, std::round((0.5 * strokeWidth - visibleRect->x) * ratio),
                                    std::round((0.5 * strokeWidth - visibleRect->y) * ratio));
    cairo_surface_set_device_scale(m->surf, ratio, ratio);
}

void StrokeHandler::normalDraw(const Point& p) {
    Point previousPoint(this->path->getLastKnot());

    this->path->addLineSegmentTo(this->hasPressure ? p : Point(p.x, p.y));
    this->stroke->unsetSizeCalculated();

    const double strokeWidth = this->stroke->getWidth();

    Stroke lastSegment;
    lastSegment.setPath(std::make_shared<PiecewiseLinearPath>(previousPoint, p));
    lastSegment.setWidth(strokeWidth);
    lastSegment.setPressureSensitive(this->hasPressure);

    xoj::view::StrokeView sView(&lastSegment);
    sView.draw(xoj::view::Context::createColorBlind(mask->cr));

    const double width = this->hasPressure ? previousPoint.z : strokeWidth;

    Range rg(previousPoint.x, previousPoint.y);
    rg.addPoint(p.x, p.y);

    this->pageView->repaintRect(rg.getX() - 0.5 * width, rg.getY() - 0.5 * width, rg.getWidth() + width,
                                rg.getHeight() + width);
}

void StrokeHandler::normalDrawLiveApproximator(const Point& p) {
    const bool newDefinitiveSegment = !this->liveApprox->feedPoint(p);

    const SplineSegment& liveSplineSegment = this->liveApprox->liveSegment;

    Rectangle<double> rect;
    if (this->hasPressure) {
        rect = liveSplineSegment.getThickBoundingBox();
    } else {
        rect = liveSplineSegment.getThinBoundingBox();
        rect.addPadding(0.5 * this->stroke->getWidth());
    }

    Range rg(rect.x, rect.y);
    rg.addPoint(rect.x + rect.width, rect.y + rect.height);

    if (newDefinitiveSegment) {
        // Fitting failed. Use the last cached segment and start a new live segment
        const SplineSegment& seg = this->liveApprox->lastDefinitiveSegment;
        this->liveSegmentStroke->setPath(std::make_shared<Spline>(seg));
        this->liveSegmentStroke->clearPointCache();

        xoj::view::StrokeView sView(this->liveSegmentStroke.get());
        sView.draw(xoj::view::Context::createColorBlind(mask->cr));

        Rectangle<double> r;
        if (this->hasPressure) {
            r = seg.getThickBoundingBox();
        } else {
            r = seg.getThinBoundingBox();
            r.addPadding(0.5 * this->stroke->getWidth());
        }
        rg.addPoint(r.x, r.y);
        rg.addPoint(r.x + r.width, r.y + r.height);
    }
    this->pageView->repaintRect(rg.getX(), rg.getY(), rg.getWidth(), rg.getHeight());
}

void StrokeHandler::fullRedraw(const Point& p) {
    const Point& previousPoint = this->path->getLastKnot();

    this->path->addLineSegmentTo(this->hasPressure ? p : Point(p.x, p.y));
    this->stroke->unsetSizeCalculated();

    const double width = this->hasPressure ? previousPoint.z : this->stroke->getWidth();

    Range rg(previousPoint.x, previousPoint.y);
    rg.addPoint(p.x, p.y);

    if (this->stroke->getFill() != -1) {
        /**
         * Need to fill the triangle (firstKnot -- previousPoint -- p)
         */
        const Point& firstPoint = this->path->getFirstKnot();
        rg.addPoint(firstPoint.x, firstPoint.y);
    }

    this->pageView->repaintRect(rg.getX() - 0.5 * width, rg.getY() - 0.5 * width, rg.getWidth() + width,
                                rg.getHeight() + width);
}

void StrokeHandler::fullRedrawLiveApproximator(const Point& p) {
    const bool newDefinitiveSegment = !this->liveApprox->feedPoint(p);

    const SplineSegment& liveSegment = this->liveApprox->liveSegment;
    Rectangle<double> rect;
    if (this->hasPressure) {
        rect = liveSegment.getThickBoundingBox();
    } else {
        rect = liveSegment.getThinBoundingBox();
        rect.addPadding(0.5 * this->stroke->getWidth());
    }
    Range rg(rect.x, rect.y);
    rg.addPoint(rect.x + rect.width, rect.y + rect.height);

    if (newDefinitiveSegment) {
        const SplineSegment& seg = this->liveApprox->lastDefinitiveSegment;
        if (this->hasPressure) {
            this->stroke->resizePointCache(this->liveSegmentPointCacheBegin);
            this->stroke->addToPointCache(seg);
            this->liveSegmentPointCacheBegin = this->stroke->getCacheSize();
        }

        Rectangle<double> r;
        if (this->hasPressure) {
            r = seg.getThickBoundingBox();
        } else {
            r = seg.getThinBoundingBox();
            r.addPadding(0.5 * this->stroke->getWidth());
        }
        rg.addPoint(r.x, r.y);
        rg.addPoint(r.x + r.width, r.y + r.height);
    }

    if (this->stroke->getFill() != -1) {
        /**
         * Need to fill the area delimited by liveSegment and firstKnot
         */
        const Point& firstPoint = this->approximatedSpline->getFirstKnot();
        rg.addPoint(firstPoint.x, firstPoint.y);
    }

    double width = this->hasPressure ? liveSegment.getMaximalWidth() : this->stroke->getWidth();

    this->pageView->repaintRect(rg.getX() - 0.5 * width, rg.getY() - 0.5 * width, rg.getWidth() + width,
                                rg.getHeight() + width);
}
