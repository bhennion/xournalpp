#include "SplineHandler.h"

#include <algorithm>  // for max, max_element
#include <cassert>    // for assert
#include <cmath>      // for pow, M_PI, cos, sin
#include <cstddef>    // for size_t
#include <list>       // for list, operator!=
#include <memory>     // for allocator_traits<>...
#include <optional>   // for optional
#include <utility>    // for move

#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Escape

#include "control/Control.h"                       // for Control
#include "control/layer/LayerController.h"         // for LayerController
#include "control/settings/Settings.h"             // for Settings
#include "control/tools/InputHandler.h"            // for InputHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "gui/XournalView.h"                       // for XournalView
#include "gui/XournalppCursor.h"                   // for XournalppCursor
#include "gui/inputdevices/PositionInputData.h"    // for PositionInputData
#include "model/Layer.h"                           // for Layer
#include "model/SplineSegment.h"                   // for SplineSegment
#include "model/Stroke.h"                          // for Stroke
#include "model/XojPage.h"                         // for XojPage
#include "undo/InsertUndoAction.h"                 // for InsertUndoAction
#include "undo/UndoRedoHandler.h"                  // for UndoRedoHandler
#include "util/DispatchPool.h"
#include "view/overlays/SplineToolView.h"

SplineHandler::SplineHandler(XournalView* xournal, const PageRef& page):
        InputHandler(xournal, page),
        snappingHandler(xournal->getControl()->getSettings()),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::SplineToolView>>()) {}

SplineHandler::~SplineHandler() = default;

std::unique_ptr<xoj::view::OverlayView> SplineHandler::createView(const xoj::view::Repaintable* parent) const {
    return std::make_unique<xoj::view::SplineToolView>(this, parent);
}

constexpr double SHIFT_AMOUNT = 1.0;
constexpr double ROTATE_AMOUNT = 5.0;
constexpr double SCALE_AMOUNT = 1.05;
constexpr double MAX_TANGENT_LENGTH = 2000.0;
constexpr double MIN_TANGENT_LENGTH = 1.0;

auto SplineHandler::onKeyEvent(GdkEventKey* event) -> bool {
    if (!stroke ||
        (event->type != GDK_KEY_PRESS && event->keyval != GDK_KEY_Escape)) {  // except for escape key only act on key
                                                                              // press event, not on key release event
        return false;
    }

    std::unique_lock lock(this->dataMutex);
    assert(!this->knots.empty() && this->knots.size() == this->tangents.size());
    Range rg = this->computeLastSegmentRepaintRange(lock);

    switch (event->keyval) {
        case GDK_KEY_Escape: {
            lock.unlock();
            this->finalizeSpline();
            return true;
        }
        case GDK_KEY_BackSpace: {
            if (this->knots.size() == 1) {
                lock.unlock();
                this->finalizeSpline();
                return true;
            }
            this->deleteLastKnotWithTangent();
            assert(!this->knots.empty() && this->knots.size() == this->tangents.size());
            const Point& p = this->knots.back();
            const Point& t = this->tangents.back();
            rg.addPoint(p.x - t.x, p.y - t.y);  // Ensure the tangent vector gets its color updated
            break;
        }
        case GDK_KEY_Right: {
            this->movePoint(SHIFT_AMOUNT, 0);
            rg = rg.unite(this->computeLastSegmentRepaintRange(lock));
            break;
        }
        case GDK_KEY_Left: {
            this->movePoint(-SHIFT_AMOUNT, 0);
            rg = rg.unite(this->computeLastSegmentRepaintRange(lock));
            break;
        }
        case GDK_KEY_Up: {
            this->movePoint(0, -SHIFT_AMOUNT);
            rg = rg.unite(this->computeLastSegmentRepaintRange(lock));
            break;
        }
        case GDK_KEY_Down: {
            this->movePoint(0, SHIFT_AMOUNT);
            rg = rg.unite(this->computeLastSegmentRepaintRange(lock));
            break;
        }
        case GDK_KEY_r:
        case GDK_KEY_R: {  // r like "rotate"
            double angle = (event->state & GDK_SHIFT_MASK) ? -ROTATE_AMOUNT : ROTATE_AMOUNT;
            double xOld = this->tangents.back().x;
            double yOld = this->tangents.back().y;
            double xNew = cos(angle * M_PI / 180) * xOld + sin(angle * M_PI / 180) * yOld;
            double yNew = -sin(angle * M_PI / 180) * xOld + cos(angle * M_PI / 180) * yOld;
            this->modifyLastTangent(Point(xNew, yNew));
            rg = rg.unite(this->computeLastSegmentRepaintRange(lock));
            break;
        }
        case GDK_KEY_s:
        case GDK_KEY_S: {  // s like "scale"
            double xOld = this->tangents.back().x;
            double yOld = this->tangents.back().y;
            double length = 2 * sqrt(pow(xOld, 2) + pow(yOld, 2));
            double factor = 1;
            if ((event->state & GDK_SHIFT_MASK) && length >= MIN_TANGENT_LENGTH) {
                factor = 1 / SCALE_AMOUNT;
            } else if (!(event->state & GDK_SHIFT_MASK) && length <= MAX_TANGENT_LENGTH) {
                factor = SCALE_AMOUNT;
            }
            double xNew = xOld * factor;
            double yNew = yOld * factor;
            this->modifyLastTangent(Point(xNew, yNew));
            rg = rg.unite(this->computeLastSegmentRepaintRange(lock));
            break;
        }
        default:
            return false;
    }
    lock.unlock();

    this->viewPool->dispatch(xoj::view::SplineToolView::PAINT_REQUEST, rg);
    return true;
}

auto SplineHandler::onMotionNotifyEvent(const PositionInputData& pos) -> bool {
    if (!stroke) {
        return false;
    }

    std::unique_lock lock(this->dataMutex);
    assert(!this->knots.empty() && this->knots.size() == this->tangents.size());

    double zoom = xournal->getZoom();
    Range rg = this->computeLastSegmentRepaintRange(lock);
    if (this->isButtonPressed) {
        Point newTangent = Point(pos.x / zoom - this->currPoint.x, pos.y / zoom - this->currPoint.y);
        if (validMotion(newTangent, this->tangents.back())) {
            this->modifyLastTangent(newTangent);
        }
    } else {
        this->buttonDownPoint = Point(pos.x / zoom, pos.y / zoom);
        this->currPoint = snappingHandler.snap(this->buttonDownPoint, knots.back(), pos.isAltDown());
    }
    rg = rg.unite(this->computeLastSegmentRepaintRange(lock));
    lock.unlock();

    this->viewPool->dispatch(xoj::view::SplineToolView::PAINT_REQUEST, rg);
    return true;
}

void SplineHandler::onSequenceCancelEvent() {
    //  Touch screen sequence changed from normal to swipe/zoom/scroll sequence
    isButtonPressed = false;
    if (!stroke) {
        return;
    }
    std::unique_lock lock(this->dataMutex);
    auto rg = this->computeLastSegmentRepaintRange(lock);
    if (this->knots.size() == 1) {
        this->knots.clear();
        this->tangents.clear();
        this->stroke.reset();
    } else {
        this->deleteLastKnotWithTangent();
    }
    lock.unlock();
    this->viewPool->dispatch(xoj::view::SplineToolView::PAINT_REQUEST, rg);
}

void SplineHandler::onButtonReleaseEvent(const PositionInputData& pos) { isButtonPressed = false; }

void SplineHandler::onButtonPressEvent(const PositionInputData& pos) {
    isButtonPressed = true;
    const double zoom = xournal->getZoom();
    const double radius = xoj::view::SplineToolView::RADIUS_WITHOUT_ZOOM / zoom;

    if (!stroke) {
        // This should only happen right after the SplineHandler's creation, before any views got attached
        assert(this->viewPool->empty());

        stroke = createStroke(this->xournal->getControl());
        std::lock_guard lock(this->dataMutex);
        assert(this->knots.empty() && this->tangents.empty());
        this->buttonDownPoint = Point(pos.x / zoom, pos.y / zoom);
        this->currPoint = snappingHandler.snapToGrid(this->buttonDownPoint, pos.isAltDown());
        this->addKnot(this->currPoint);
    } else {
        std::unique_lock lock(this->dataMutex);
        assert(!this->knots.empty());
        this->buttonDownPoint = Point(pos.x / zoom, pos.y / zoom);
        this->currPoint = snappingHandler.snap(this->buttonDownPoint, knots.back(), pos.isAltDown());
        double dist = this->buttonDownPoint.lineLengthTo(this->knots.front());
        if (dist < radius) {  // now the spline is closed and finalized
            this->addKnotWithTangent(this->knots.front(), this->tangents.front());
            lock.unlock();
            this->finalizeSpline();
        } else if (validMotion(currPoint, this->knots.back())) {
            this->addKnot(this->currPoint);
            auto rg = this->computeLastSegmentRepaintRange(lock);
            lock.unlock();
            this->viewPool->dispatch(xoj::view::SplineToolView::PAINT_REQUEST, rg);
        }
    }
}

void SplineHandler::onButtonDoublePressEvent(const PositionInputData&) { finalizeSpline(); }

void SplineHandler::movePoint(double dx, double dy) {
    // move last non dynamically changing point
    if (!this->knots.empty()) {
        this->knots.back().x += dx;
        this->knots.back().y += dy;
    }
}

void SplineHandler::finalizeSpline() {
    if (!this->stroke) {
        return;
    }

    Data data;
    {
        std::lock_guard lock(this->dataMutex);
        std::swap(data.knots, this->knots);
        std::swap(data.tangents, this->tangents);
        data.currPoint = currPoint;
        // data.closedSpline is irrelevant here
    }
    auto s = std::move(stroke);

    auto rg = this->computeTotalRepaintRange(data, s->getWidth());

    if (data.knots.size() < 2) {  // This is not a valid spline
        this->viewPool->dispatch(xoj::view::SplineToolView::PAINT_REQUEST, rg);
        return;
    }

    s->setPointVector(linearizeSpline(data));
    s->freeUnusedPointItems();

    Control* control = xournal->getControl();
    control->getLayerController()->ensureLayerExists(page);

    Layer* layer = page->getSelectedLayer();

    UndoRedoHandler* undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, s.get()));

    layer->addElement(s.get());

    /*
     * Triggers a rerendering of the portion of page under the spline.
     * This also erases the drawing aids (tangent vectors and so on)
     *
     * WARNING: this is sort of a hack relying on jobs/RenderJob repainting the entire page
     * (so that our drawing aids indeed get erased).
     * NOTA BENE: For this to work, it is important that this->stroke has been reset already.
     *
     * This hack also leaves room for blinking (sometimes) upon spline finalization.
     * We need to figure out a good way of handling this transition.
     */
    this->page->fireElementChanged(s.release());

    xournal->getCursor()->updateCursor();
}

void SplineHandler::addKnot(const Point& p) { addKnotWithTangent(p, Point(0, 0)); }

void SplineHandler::addKnotWithTangent(const Point& p, const Point& t) {
    this->knots.push_back(p);
    this->tangents.push_back(t);
}

void SplineHandler::modifyLastTangent(const Point& t) {
    assert(!this->tangents.empty());
    this->tangents.back() = t;
}

void SplineHandler::deleteLastKnotWithTangent() {
    assert(this->knots.size() > 1 && this->knots.size() == this->tangents.size());
    this->knots.pop_back();
    this->tangents.pop_back();
}

auto SplineHandler::computeTotalRepaintRange(const Data& data, double strokeWidth) const -> Range {
    const double zoom = xournal->getZoom();
    const double radius = xoj::view::SplineToolView::RADIUS_WITHOUT_ZOOM / zoom;
    std::vector<double> xCoords = {};
    std::vector<double> yCoords = {};
    for (auto p: data.knots) {
        xCoords.push_back(p.x);
        yCoords.push_back(p.y);
    }
    for (size_t i = 0; i < data.knots.size(); i++) {
        xCoords.push_back(data.knots[i].x + data.tangents[i].x);
        xCoords.push_back(data.knots[i].x - data.tangents[i].x);
        yCoords.push_back(data.knots[i].y + data.tangents[i].y);
        yCoords.push_back(data.knots[i].y - data.tangents[i].y);
    }
    xCoords.push_back(data.currPoint.x);
    yCoords.push_back(data.currPoint.y);

    double minX = *std::min_element(xCoords.begin(), xCoords.end());
    double maxX = *std::max_element(xCoords.begin(), xCoords.end());
    double minY = *std::min_element(yCoords.begin(), yCoords.end());
    double maxY = *std::max_element(yCoords.begin(), yCoords.end());

    Range rg(minX, minY, maxX, maxY);
    rg.addPadding(std::max(radius, strokeWidth));  // Circles around the knots and the spline width
    return rg;
}

Range SplineHandler::computeLastSegmentRepaintRange(std::unique_lock<std::mutex>& lock) const {
    assert(lock.owns_lock() && lock.mutex() == &this->dataMutex);
    assert(!this->knots.empty() && this->knots.size() == this->tangents.size());

    Range rg(this->currPoint.x, this->currPoint.y);
    const Point& p = this->knots.back();
    const Point& t = this->tangents.back();
    rg.addPoint(p.x + t.x, p.y + t.y);
    rg.addPoint(p.x - t.x, p.y - t.y);
    if (auto n = this->knots.size(); n > 1) {
        const Point& q = this->knots[n - 2];
        const Point& s = this->tangents[n - 2];
        rg.addPoint(q.x + s.x, q.y + s.y);
        rg.addPoint(q.x, q.y);  // Enough for the last segment.
    }

    const double zoom = xournal->getZoom();
    const double radius = xoj::view::SplineToolView::RADIUS_WITHOUT_ZOOM / zoom;
    rg.addPadding(std::max(radius, this->stroke->getWidth()));  // Circles around the knots and the spline width
    if (this->stroke->getFill() != -1) {
        const Point& p = this->knots.front();
        rg.addPoint(p.x, p.y);
    }
    return rg;
}

auto SplineHandler::getViewPool() const -> const std::shared_ptr<xoj::util::DispatchPool<xoj::view::SplineToolView>>& {
    return viewPool;
}

auto SplineHandler::getDataClone() const -> std::optional<Data> {
    double radius = xoj::view::SplineToolView::RADIUS_WITHOUT_ZOOM / xournal->getZoom();
    std::lock_guard lock(dataMutex);
    if (this->knots.empty()) {
        return std::nullopt;
    }
    bool closedSpline = this->knots.size() > 1 && this->buttonDownPoint.lineLengthTo(this->knots.front()) < radius;
    Data data = {this->knots, this->tangents, this->currPoint, closedSpline};
    return data;
}

auto SplineHandler::linearizeSpline(const SplineHandler::Data& data) -> std::vector<Point> {
    assert(!data.knots.empty() && data.knots.size() == data.tangents.size());

    std::vector<Point> result;

    auto itKnot1 = data.knots.begin();
    auto itKnot2 = std::next(itKnot1);
    auto itTgt1 = data.tangents.begin();
    auto itTgt2 = std::next(itTgt1);
    auto end = data.knots.end();
    for (; itKnot2 != end; ++itKnot1, ++itKnot2, ++itTgt1, ++itTgt2) {
        SplineSegment seg(*itKnot1, Point(itKnot1->x + itTgt1->x, itKnot1->y + itTgt1->y),
                          Point(itKnot2->x - itTgt2->x, itKnot2->y - itTgt2->y), *itKnot2);
        auto pts = seg.toPointSequence();
        std::move(pts.begin(), pts.end(), std::back_inserter(result));
    }
    result.emplace_back(data.knots.back());

    return result;
}
