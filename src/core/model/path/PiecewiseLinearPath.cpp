/**
 * Xournal++
 *
 * A piecewise linear (PL) path
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#include "PiecewiseLinearPath.h"

#include <cmath>
#include <numeric>

#include "model/Element.h"
#include "model/MathVect.h"
#include "util/LoopUtil.h"
#include "util/SmallVector.h"
#include "util/TinyVector.h"
#include "util/serializing/ObjectInputStream.h"
#include "util/serializing/ObjectOutputStream.h"

#include "IntersectWithPaddedBoxTemplate.h"
#include "SegmentIteratable.h"

using xoj::util::Rectangle;

PiecewiseLinearPath::PiecewiseLinearPath(const std::vector<Point>& vector) { data = vector; }
PiecewiseLinearPath::PiecewiseLinearPath(std::vector<Point>&& vector) { std::swap(data, vector); }

PiecewiseLinearPath& PiecewiseLinearPath::operator=(const std::vector<Point>& vector) {
    data = vector;
    return *this;
}
PiecewiseLinearPath& PiecewiseLinearPath::operator=(std::vector<Point>&& vector) {
    std::swap(data, vector);
    return *this;
}

PiecewiseLinearPath::PiecewiseLinearPath(const Point& firstPoint) { data.push_back(firstPoint); }

PiecewiseLinearPath::PiecewiseLinearPath(const Point& firstPoint, size_t size) {
    data.reserve(size + 1);
    data.push_back(firstPoint);
}

PiecewiseLinearPath::PiecewiseLinearPath(const Point& firstPoint, const Point& secondPoint) {
    data = {firstPoint, secondPoint};
}

PiecewiseLinearPath::PiecewiseLinearPath(ObjectInputStream& in) {
    in.readObject("PiecewiseLinearPath");

    Point* p{};
    int count{};
    in.readData(reinterpret_cast<void**>(&p), &count);
    this->data = std::vector<Point>{p, p + count};
    g_free(p);

    in.endObject();
}

void PiecewiseLinearPath::serialize(ObjectOutputStream& out) const {
    out.writeObject("PiecewiseLinearPath");
    out.writeData(this->data.data(), this->data.size(), sizeof(Point));
    out.endObject();
}

auto PiecewiseLinearPath::segments() -> SegmentIteratable<LineSegment> {
    if (data.empty()) {
        return SegmentIteratable<LineSegment>(nullptr, nullptr);
    }
    return SegmentIteratable<LineSegment>(&data.front(), &data.back());
}

auto PiecewiseLinearPath::segments() const -> SegmentIteratable<const LineSegment> {
    if (data.empty()) {
        return SegmentIteratable<const PiecewiseLinearPath::LineSegment>(nullptr, nullptr);
    }
    return SegmentIteratable<const LineSegment>(&data.front(), &data.back());
}

void PiecewiseLinearPath::setFirstPoint(const Point& p) {
    if (data.empty()) {
        data.push_back(p);
    } else {
        data.front() = p;
    }
}

void PiecewiseLinearPath::close() {
    if (!data.empty()) {
        data.push_back(data.front());
    }
}

const PiecewiseLinearPath::LineSegment& PiecewiseLinearPath::getSegment(size_t index) const {
    return *(reinterpret_cast<const LineSegment*>(this->data.data() + index));
}

auto PiecewiseLinearPath::getPoint(const Path::Parameter& parameter) const -> Point {
    if (data.empty()) {
        g_warning("PiecewiseLinearPath::getPoint: Empty path");
        return Point(NAN, NAN);
    }

    size_t index = parameter.index;
    double t = parameter.t;
    size_t n = nbSegments();

    if (index == n - 1 && t == 1.0) {
        return data.back();
    }

    if (index >= n || t < 0.0 || t > 1.0) {
        g_warning("PiecewiseLinearPath::getPoint: Parameter out of range: %zu >= %zu || %f < 0.0 || %f >= 1.0", index,
                  n, t, t);
        return Point(NAN, NAN);
    }

    if (t == 0.0) {
        return data[index];
    }

    const Point& p = data[index];
    const Point& q = data[index + 1];
    double u = 1 - t;
    return Point(u * p.x + t * q.x, u * p.y + t * q.y, u * p.z + t * q.z);
}

auto PiecewiseLinearPath::getPoint(const size_t index) const -> const Point& {
    assert(!data.empty());
    if (index >= data.size()) {
        g_warning("PiecewiseLinearPath::getPoint: index out of range: %zu >= %zu", index, data.size());
        return data.back();
    }
    return data[index];
}

auto PiecewiseLinearPath::clone() const -> std::unique_ptr<Path> {
    std::unique_ptr<PiecewiseLinearPath> clone = std::make_unique<PiecewiseLinearPath>();
    std::copy(this->data.cbegin(), this->data.cend(), std::back_inserter(clone->data));
    return clone;
}

auto PiecewiseLinearPath::cloneSection(const Path::Parameter& lowerBound, const Path::Parameter& upperBound) const
        -> std::unique_ptr<Path> {

    if (upperBound.index == lowerBound.index) {
        const Point& p = this->getPoint(lowerBound);
        const Point& q = this->getPoint(upperBound);
        return std::make_unique<PiecewiseLinearPath>(p, q);
    } else {
        // Create and reserve memory for the clone
        std::unique_ptr<PiecewiseLinearPath> clone = std::make_unique<PiecewiseLinearPath>(
                this->getPoint(lowerBound), upperBound.index - lowerBound.index + 1);

        // second point of segment of index lowerBound.index
        auto it = this->data.cbegin() + (std::ptrdiff_t)lowerBound.index + 1;

        // second point of segment of index upperBound.index
        auto endIt = this->data.cbegin() + (std::ptrdiff_t)upperBound.index + 1;

        std::copy(it, endIt, std::back_inserter(clone->data));

        clone->addLineSegmentTo(this->getPoint(upperBound));

        assert(clone->data.size() == upperBound.index - lowerBound.index + 2);
        return clone;
    }
}

std::unique_ptr<Path> PiecewiseLinearPath::cloneCircularSectionOfClosedPath(const Path::Parameter& startParam,
                                                                            const Path::Parameter& endParam) const {
    std::unique_ptr<PiecewiseLinearPath> clone = std::make_unique<PiecewiseLinearPath>(
            this->getPoint(startParam), this->data.size() - startParam.index + endParam.index);

    auto startIt = this->data.cbegin() + (std::ptrdiff_t)startParam.index + 1;
    assert(startIt != this->data.cend());
    std::copy(startIt, this->data.cend() - 1, std::back_inserter(clone->data));

    auto endIt = this->data.cbegin() + (std::ptrdiff_t)endParam.index + 1;
    std::copy(this->data.cbegin(), endIt, std::back_inserter(clone->data));

    // Last point
    clone->addLineSegmentTo(this->getPoint(endParam));

    assert(clone->data.size() == this->data.size() - startParam.index + endParam.index + 1);
    return clone;
}

Rectangle<double> PiecewiseLinearPath::getThinBoundingBox() const {
    if (data.empty()) {
        return {0.0, 0.0, 0.0, 0.0};
    }

    double minSnapX = DBL_MAX;
    double maxSnapX = DBL_MIN;
    double minSnapY = DBL_MAX;
    double maxSnapY = DBL_MIN;

    for (auto&& p: data) {
        minSnapX = std::min(minSnapX, p.x);
        minSnapY = std::min(minSnapY, p.y);

        maxSnapX = std::max(maxSnapX, p.x);
        maxSnapY = std::max(maxSnapY, p.y);
    }

    return {minSnapX, minSnapY, maxSnapX - minSnapX, maxSnapY - minSnapY};
}

Rectangle<double> PiecewiseLinearPath::getSubSectionThinBoundingBox(const Path::Parameter& startParam,
                                                                    const Path::Parameter& endParam) const {
    Point p = this->getPoint(startParam);
    double minX = p.x;
    double maxX = p.x;
    double minY = p.y;
    double maxY = p.y;

    auto endIt = this->data.cbegin() + (std::ptrdiff_t)endParam.index + 1;
    for (auto it = this->data.cbegin() + (std::ptrdiff_t)startParam.index + 1; it != endIt; ++it) {
        minX = std::min(minX, it->x);
        maxX = std::max(maxX, it->x);
        minY = std::min(minY, it->y);
        maxY = std::max(maxY, it->y);
    }

    Point q = this->getPoint(endParam);
    minX = std::min(minX, q.x);
    maxX = std::max(maxX, q.x);
    minY = std::min(minY, q.y);
    maxY = std::max(maxY, q.y);

    return {minX, minY, maxX - minX, maxY - minY};
}

void PiecewiseLinearPath::setSecondToLastPressure(double pressure) {
    if (size_t size = this->data.size(); size >= 2) {
        this->data[size - 2].z = pressure;
    }
}

void PiecewiseLinearPath::extrapolateLastPressureValue() {
    size_t size = this->nbSegments();
    if (size == 0) {  // No segment (maybe a first point)
        return;
    }
    if (size == 1) {  // Only 1 segment
        this->data.back().z = this->data.front().z;
    } else {
        // Linearly extrapolate the pressure using the second and third to last points
        // Remember that size is the number of segments (= nb of points - 1)
        this->data.back().z = std::max(2 * data[size - 1].z - data[size - 2].z, 0.05);
    }
}

double PiecewiseLinearPath::getAveragePressure() {
    return std::accumulate(begin(this->data), end(this->data), 0.0, [](double l, Point const& p) { return l + p.z; }) /
           (double)this->data.size();
}

auto PiecewiseLinearPath::nbSegments() const -> size_t { return data.empty() ? 0 : data.size() - 1; }

void PiecewiseLinearPath::resize(size_t n) {
    ++n;  // corresponding number of points
    if (n < data.size()) {
        data.resize(n);
    }
}

auto PiecewiseLinearPath::LineSegment::intersectWithRectangle(const Rectangle<double> rectangle) const
        -> TinyVector<double, 2> {
    std::optional<Interval<double>> intersections =
            intersectLineWithRectangle(this->firstKnot, this->secondKnot, rectangle);

    if (intersections) {
        TinyVector<double, 2> result;
        if (intersections->min > 0.0 && intersections->min <= 1.0) {
            result.emplace_back(intersections->min);
        }
        if (intersections->max > 0.0 && intersections->max <= 1.0) {
            result.emplace_back(intersections->max);
        }
        return result;
    }
    return {};
}

std::pair<std::reference_wrapper<const Point>, std::reference_wrapper<const Point>>
        PiecewiseLinearPath::LineSegment::getLeftHalfTangent() const {
    return std::make_pair<std::reference_wrapper<const Point>, std::reference_wrapper<const Point>>(this->firstKnot,
                                                                                                    this->secondKnot);
}

std::pair<std::reference_wrapper<const Point>, std::reference_wrapper<const Point>>
        PiecewiseLinearPath::LineSegment::getRightHalfTangent() const {
    return std::make_pair<std::reference_wrapper<const Point>, std::reference_wrapper<const Point>>(this->firstKnot,
                                                                                                    this->secondKnot);
}

auto PiecewiseLinearPath::LineSegment::getPoint(double t) const -> point_t {
    return firstKnot.relativeLineTo(secondKnot, t);
}

auto PiecewiseLinearPath::intersectWithPaddedBox(const PaddedBox& box, size_t firstIndex, size_t lastIndex) const
        -> IntersectionParametersContainer {
    return this->intersectWithPaddedBoxTemplate<LineSegment>(box, firstIndex, lastIndex, this->segments());
}

// auto PiecewiseLinearPath::intersectWithRectangle(const Rectangle<double>& rectangle) const
//         -> std::vector<Path::Parameter> {
//     if (data.empty()) {
//         return {};
//     }
//
//     bool startInside = data.front().isInside(rectangle);
//     if (data.size() == 1 || (data.size() == 2 && data[0].equalsPos(data[1]))) {
//         if (startInside) {
//             return {{0, 0.0}, {0, 1.0}};
//         } else {
//             return {};
//         }
//     }
//
//     std::vector<Parameter> result;
//     if (startInside) {
//         result.emplace_back(0, 0.0);
//     }
//
//     size_t index = 0;
//     auto it1 = data.cbegin();
//     auto it2 = std::next(it1);
//     auto endIt = data.cend();
//     for (auto it1 = data.cbegin(), it2 = std::next(it1); it2 != endIt; it1 = it2++, index++) {
//         std::vector<double> intersections = intersectLineSegmentWithRectangle(*it1, *it2, rectangle);
//         std::transform(intersections.cbegin(), intersections.cend(), std::back_inserter(result),
//                        [&index](double t) { return Parameter(index, t); });
//     }
//     if (data.back().isInside(rectangle)) {
//         size_t n = data.size() >= 2 ? data.size() - 2 : 0;
//         result.emplace_back(n, 1.0);
//     }
//     /**
//      * Do we need to take care of the very improbable cases where the first or last knot lie on the boundary?
//      * Could some snapping make those cases actually possible?
//      */
//     assert(result.size() % 2 == 0);
//
//     return result;
// }
//
// auto PiecewiseLinearPath::intersectWithRectangle(const Rectangle<double>& rectangle, size_t firstIndex,
//                                                  size_t lastIndex) const -> std::vector<Path::Parameter> {
//
//     std::vector<Parameter> result;
//     auto inserter = std::back_inserter(result);
//     size_t index = firstIndex;
//     auto it1 = data.cbegin() + (std::ptrdiff_t)index;
//     auto it2 = std::next(it1);
//
//     /**
//      * The first segment
//      */
//     std::vector<double> intersections = intersectLineSegmentWithRectangle(*it1, *it2, rectangle);
//
//     if (it1->isInside(rectangle)) {
//         /**
//          * The path starts in the rectangle. Add a fake intersection parameter
//          */
//         result.emplace_back(index, 0.0);
//     } else if (isPointOnBoundary(*it1, rectangle)) {
//         /**
//          * Improbable case: the segment begins on the rectangle's boundary
//          */
//         Point p;
//         if (intersections.empty()) {
//             p = *it2;
//         } else {
//             double t = 0.5 * intersections.front();
//             double u = 1 - t;
//             p.x = u * it1->x + t * it2->x;
//             p.y = u * it1->y + t * it2->y;
//         }
//         if (p.isInside(rectangle)) {
//             /**
//              * Exceptional case: The segment begins on the rectangle's boundary and goes inwards.
//              * Add an intersection point
//              */
//             result.emplace_back(index, 0.0);
//         }
//     }
//
//     std::transform(intersections.begin(), intersections.end(), inserter,
//                    [index](double v) { return Parameter(index, v); });
//
//     auto endIt = data.cbegin() + (std::ptrdiff_t)(lastIndex + 2);
//     it1 = it2++;  // We already took care of the first segment
//     index++;
//
//     for (; it2 != endIt; it1 = it2++, index++) {
//         std::vector<double> intersection = intersectLineSegmentWithRectangle(*it1, *it2, rectangle);
//         std::transform(intersection.cbegin(), intersection.cend(), inserter,
//                        [&index](double t) { return Parameter(index, t); });
//     }
//
//     if (result.size() % 2) {
//         if (it1->isInside(rectangle)) {
//             /**
//              * The spline ends in the rectangle (not on the boundary). Add a fake intersection parameter
//              */
//             result.emplace_back(lastIndex, 1.0);
//         } else {
//             /**
//              * The only possibility:
//              * The segment ends on the rectangle's boundary, coming from outside
//              * Drop this last intersection point
//              */
//             result.pop_back();
//         }
//     }
//     return result;
// }

bool PiecewiseLinearPath::isInSelection(ShapeContainer* container) {
    for (auto&& p: this->data) {
        double px = p.x;
        double py = p.y;

        if (!container->contains(px, py)) {
            return false;
        }
    }

    return true;
}

double PiecewiseLinearPath::squaredDistanceToPoint(const Point& p, double veryClose, double toFar) {
    if (this->data.empty()) {
        return toFar;
    }
    MathVect2 u(p, this->data.front());
    double min = std::min(toFar, u.squaredNorm());
    for (auto& segment: this->segments()) {
        MathVect2 v(segment.firstKnot, segment.secondKnot);
        double t = -MathVect2::scalarProduct(u, v) / v.squaredNorm();
        MathVect2 w(p, segment.secondKnot);
        if (t > 0.0) {
            double m = (t >= 1.0 ? w.squaredNorm() : (u + t * v).squaredNorm());
            if (m < min) {
                if (m <= veryClose) {
                    return veryClose;
                }
                min = m;
            }
        }
        u = w;
    }
    return min;
}

void PiecewiseLinearPath::addToCairo(cairo_t* cr) const {
    for_first_then_each(
            this->data, [cr](auto const& first) { cairo_move_to(cr, first.x, first.y); },
            [cr](auto const& other) { cairo_line_to(cr, other.x, other.y); });
}

void PiecewiseLinearPath::addSectionToCairo(cairo_t* cr, const Path::Parameter& lowerBound,
                                            const Path::Parameter& upperBound) const {
    Point p = this->getPoint(lowerBound);
    cairo_move_to(cr, p.x, p.y);

    auto endIt = data.cbegin() + (std::ptrdiff_t)upperBound.index + 1;
    for (auto it = data.cbegin() + (std::ptrdiff_t)lowerBound.index + 1; it != endIt; ++it) {
        cairo_line_to(cr, it->x, it->y);
    }

    Point q = this->getPoint(upperBound);
    cairo_line_to(cr, q.x, q.y);
}

void PiecewiseLinearPath::addCircularSectionToCairo(cairo_t* cr, const Path::Parameter& startParam,
                                                    const Path::Parameter& endParam) const {
    Point p = this->getPoint(startParam);
    cairo_move_to(cr, p.x, p.y);

    auto endIt = data.cend();
    for (auto it = data.cbegin() + (std::ptrdiff_t)startParam.index + 1; it != endIt; ++it) {
        cairo_line_to(cr, it->x, it->y);
    }
    endIt = data.cbegin() + (std::ptrdiff_t)endParam.index + 1;
    for (auto it = data.cbegin(); it != endIt; ++it) {
        cairo_line_to(cr, it->x, it->y);
    }

    Point q = this->getPoint(endParam);
    cairo_line_to(cr, q.x, q.y);
}
