#include "ErasableStrokeOverlapTree.h"

#include <algorithm>  // for max, min, minmax
#include <cassert>    // for assert
#include <optional>   // for optional
#include <tuple>      // for tie, tuple

#include "model/Point.h"                  // for Point
#include "model/Stroke.h"                 // for Stroke
#include "model/eraser/ErasableStroke.h"  // for ErasableStroke::SubSection
#include "model/path/Path.h"              // for Path::Parameter
#include "model/path/Spline.h"
#include "util/Range.h"  // for Range

using xoj::util::Rectangle;

void ErasableStroke::OverlapTree::populate(const SubSection& section, const Stroke& stroke) {
    Populator populator(this->data, stroke);
    populator.populate(section, this->root);
    populated = true;
}

bool ErasableStroke::OverlapTree::isPopulated() const { return populated; }

#ifdef DEBUG_ERASABLE_STROKE_BOXES
void ErasableStroke::OverlapTree::addOverlapsToRange(const ErasableStroke::OverlapTree& other, double halfWidth,
                                                     Range& range, cairo_t* cr) const {
#else
void ErasableStroke::OverlapTree::addOverlapsToRange(const ErasableStroke::OverlapTree& other, double halfWidth,
                                                     Range& range) const {
#endif
    assert(this->isPopulated() && other.isPopulated());

#ifdef DEBUG_ERASABLE_STROKE_BOXES
    this->root.addOverlapsToRange(other.root, halfWidth, range, cr);
#else
    this->root.addOverlapsToRange(other.root, halfWidth, range);
#endif
}

void ErasableStroke::OverlapTree::Populator::populate(const SubSection& section, Node& root) {
    assert(stroke.path != nullptr);

    const Path& path = stroke.getPath();
    assert(section.min.index <= section.max.index && section.max.index < path.nbSegments());

    size_t nbLeaves = section.max.index - section.min.index + 1;
    this->data.resize(nbLeaves - 1);

    this->nextFreeSlot = this->data.begin();

    auto pathType = path.getType();
    assert(pathType == Path::PIECEWISE_LINEAR || pathType == Path::SPLINE);

    if (section.min.index == section.max.index) {
        // The section spans on a single segment
        if (pathType == Path::PIECEWISE_LINEAR) {
            root.initializeOnSegment(path.getPoint(section.min), path.getPoint(section.max));
            return;
        }
        const Spline& spline = dynamic_cast<const Spline&>(path);
        root.initializeOnSegment(spline.getSegment(section.min.index).getSubsegment(section.min.t, section.max.t));
        return;
    }

    if (section.max.t == 0.0) {
        // Highly unlikely...
        if (section.min.index + 1 == section.max.index) {
            // The section spans on a single segment
            if (pathType == Path::PIECEWISE_LINEAR) {
                root.initializeOnSegment(path.getPoint(section.min), path.getPoint(section.max));
                return;
            }
            const Spline& spline = dynamic_cast<const Spline&>(path);
            root.initializeOnSegment(spline.getSegment(section.min.index).subdivide(section.min.t).second);
            return;
        }
        if (section.min.t == 0.0) {
            if (pathType == Path::PIECEWISE_LINEAR) {
                this->populateNode(root, section.min.index, section.max.index, path.getData());
                return;
            }
            const Spline& spline = dynamic_cast<const Spline&>(path);
            this->populateNode(root, section.min.index, section.max.index, spline.segments());
            return;
        }
        if (pathType == Path::PIECEWISE_LINEAR) {
            this->populateNode(root, path.getPoint(section.min), section.min.index + 1, section.max.index,
                               path.getData());
            return;
        }
        const Spline& spline = dynamic_cast<const Spline&>(path);
        this->populateNode(root, section.min, section.max.index, spline.segments());
        return;
    }

    if (section.min.t == 0.0) {
        // This happens when erasing the end of a stroke
        if (pathType == Path::PIECEWISE_LINEAR) {
            this->populateNode(root, section.min.index, section.max.index, path.getPoint(section.max), path.getData());
            return;
        }
        const Spline& spline = dynamic_cast<const Spline&>(path);
        this->populateNode(root, section.min.index, section.max, spline.segments());
        return;
    }

    if (pathType == Path::PIECEWISE_LINEAR) {
        this->populateNode(root, path.getPoint(section.min), section.min.index + 1, section.max.index,
                           path.getPoint(section.max), path.getData());
        return;
    }
    const Spline& spline = dynamic_cast<const Spline&>(path);
    this->populateNode(root, section, spline.segments());
}

auto ErasableStroke::OverlapTree::Populator::getNextFreeSlot() -> std::pair<Node, Node>* {
    assert(nextFreeSlot < data.end());
    return &*(nextFreeSlot++);
}

/**
 * Nodes for PiecewiseLinearPath
 */
void ErasableStroke::OverlapTree::Populator::populateNode(Node& node, const Point& firstPoint, size_t min, size_t max,
                                                          const Point& lastPoint, const std::vector<Point>& pts) {
    assert(min <= max && max < pts.size());
    /**
     * Split in two in the middle
     */
    size_t middle = (min + max) / 2;

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, firstPoint, min, middle, pts);
    this->populateNode(node.children->second, middle, max, lastPoint, pts);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Populator::populateNode(Node& node, const Point& firstPoint, size_t min, size_t max,
                                                          const std::vector<Point>& pts) {
    assert(min <= max && max < pts.size());
    if (min == max) {
        // The node corresponds to a single segment
        node.initializeOnSegment(firstPoint, pts[min]);
        return;
    }
    /**
     * Split in two
     */
    size_t middle = (min + max) / 2;
    assert(middle >= min && middle < max);

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, firstPoint, min, middle, pts);
    this->populateNode(node.children->second, middle, max, pts);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Populator::populateNode(Node& node, size_t min, size_t max, const Point& lastPoint,
                                                          const std::vector<Point>& pts) {
    assert(min <= max && max < pts.size());
    if (min == max) {
        // The node corresponds to a single segment
        node.initializeOnSegment(pts[min], lastPoint);
        return;
    }
    /**
     * Split in two
     */
    size_t middle = (min + max + 1) / 2;
    assert(middle > min && middle <= max);

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, min, middle, pts);
    this->populateNode(node.children->second, middle, max, lastPoint, pts);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Populator::populateNode(Node& node, size_t min, size_t max,
                                                          const std::vector<Point>& pts) {
    assert(max > min);
    if (min + 1 == max) {
        // The node corresponds to a single segment
        node.initializeOnSegment(pts[min], pts[max]);
        return;
    }
    /**
     * Split in two
     */
    size_t middle = (min + max) / 2;
    assert(middle > min && middle < max);

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, min, middle, pts);
    this->populateNode(node.children->second, middle, max, pts);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Node::initializeOnSegment(const Point& p1, const Point& p2) {
    std::tie(this->minX, this->maxX) = std::minmax(p1.x, p2.x);
    std::tie(this->minY, this->maxY) = std::minmax(p1.y, p2.y);
}

/**
 * Nodes for splines
 */
void ErasableStroke::OverlapTree::Populator::populateNode(
        Node& node, const SubSection& section, const Path::SegmentIteratable<const SplineSegment>& segments) {
    assert(section.min.index < section.max.index && section.max.index < segments.size());
    /**
     * Split in two in the middle
     */
    size_t middle = (section.min.index + section.max.index + 1) / 2;

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, section.min, middle, segments);
    this->populateNode(node.children->second, middle, section.max, segments);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Populator::populateNode(
        Node& node, const Path::Parameter& startParam, size_t endIndex,
        const Path::SegmentIteratable<const SplineSegment>& segments) {
    assert(startParam.index <= endIndex && endIndex < segments.size());
    if (startParam.index + 1 == endIndex) {
        // The node corresponds to a single segment
        node.initializeOnSegment(segments[endIndex].subdivide(startParam.t).second);
        return;
    }
    /**
     * Split in two
     */
    size_t middle = (startParam.index + endIndex) / 2;
    assert(middle >= startParam.index && middle < endIndex);

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, startParam, middle, segments);
    this->populateNode(node.children->second, middle, endIndex, segments);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Populator::populateNode(
        Node& node, size_t startIndex, const Path::Parameter& endParam,
        const Path::SegmentIteratable<const SplineSegment>& segments) {
    assert(startIndex <= endParam.index && endParam.index < segments.size());
    if (startIndex == endParam.index) {
        // The node corresponds to a single segment
        node.initializeOnSegment(segments[startIndex].subdivide(endParam.t).first);
        return;
    }
    /**
     * Split in two
     */
    size_t middle = (startIndex + endParam.index + 1) / 2;
    assert(middle > startIndex && middle <= endParam.index);

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, startIndex, middle, segments);
    this->populateNode(node.children->second, middle, endParam, segments);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Populator::populateNode(
        Node& node, size_t startIndex, size_t endIndex, const Path::SegmentIteratable<const SplineSegment>& segments) {
    assert(endIndex > startIndex);
    if (startIndex + 1 == endIndex) {
        // The node corresponds to a single segment
        node.initializeOnSegment(segments[startIndex]);
        return;
    }
    /**
     * Split in two
     */
    size_t middle = (startIndex + endIndex) / 2;
    assert(middle > startIndex && middle < endIndex);

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, startIndex, middle, segments);
    this->populateNode(node.children->second, middle, endIndex, segments);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Node::initializeOnSegment(const SplineSegment& segment) {
    auto box = segment.getBoundingBox();
    this->minX = box.x;
    this->minY = box.y;
    this->maxX = box.x + box.width;
    this->maxY = box.y + box.height;
}

void ErasableStroke::OverlapTree::Node::computeBoxFromChildren() {
    assert(children != nullptr);
    minX = std::min(children->first.minX, children->second.minX);
    maxX = std::max(children->first.maxX, children->second.maxX);
    minY = std::min(children->first.minY, children->second.minY);
    maxY = std::max(children->first.maxY, children->second.maxY);
}

#ifdef DEBUG_ERASABLE_STROKE_BOXES
void ErasableStroke::OverlapTree::Node::addOverlapsToRange(const Node& other, double halfWidth, Range& range,
                                                           cairo_t* cr) const {
#else
void ErasableStroke::OverlapTree::Node::addOverlapsToRange(const Node& other, double halfWidth, Range& range) const {
#endif
    bool intersect = this->maxX + halfWidth > other.minX - halfWidth &&
                     this->minX - halfWidth<other.maxX + halfWidth&& this->maxY + halfWidth> other.minY - halfWidth &&
                     this->minY - halfWidth < other.maxY + halfWidth;
    if (!intersect) {
        return;
    }
    if (other.children != nullptr) {
#ifdef DEBUG_ERASABLE_STROKE_BOXES
        other.children->first.addOverlapsToRange(*this, halfWidth, range, cr);
        other.children->second.addOverlapsToRange(*this, halfWidth, range, cr);
#else
        other.children->first.addOverlapsToRange(*this, halfWidth, range);
        other.children->second.addOverlapsToRange(*this, halfWidth, range);
#endif
        return;
    }
    /**
     * other is a leaf corresponding to a single segment of the stroke
     * Split *this until it's just a leaf
     */
    if (this->children != nullptr) {
#ifdef DEBUG_ERASABLE_STROKE_BOXES
        this->children->first.addOverlapsToRange(other, halfWidth, range, cr);
        this->children->second.addOverlapsToRange(other, halfWidth, range, cr);
#else
        this->children->first.addOverlapsToRange(other, halfWidth, range);
        this->children->second.addOverlapsToRange(other, halfWidth, range);
#endif
        return;
    }
    /**
     * Both this and other are leaves corresponding to single segments
     * Repaint the intersection of their bounding boxes
     */
    auto rectThis = this->toRectangle(halfWidth);
    auto rectOther = other.toRectangle(halfWidth);
    auto inter = rectThis.intersects(rectOther);

    assert(inter);

    const Rectangle<double>& rect = inter.value();
#ifdef DEBUG_ERASABLE_STROKE_BOXES
    paintDebugRect(rectThis, 'g', cr);
    paintDebugRect(rectOther, 'g', cr);
    paintDebugRect(rect, 'b', cr);
#endif
    range.addPoint(rect.x, rect.y);
    range.addPoint(rect.x + rect.width, rect.y + rect.height);
}

auto ErasableStroke::OverlapTree::Node::toRectangle(double padding) const -> Rectangle<double> {
    return {minX - padding, minY - padding, maxX - minX + 2 * padding, maxY - minY + 2 * padding};
}
