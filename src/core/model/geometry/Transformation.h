/*
 * Xournal++
 *
 * Geometric transformation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <memory>

class Point;
struct Position;
namespace xoj::util {
template <typename T>
struct Point;
};

namespace xoj::geometry {
class Transformation {
public:
    virtual ~Transformation() noexcept = default;

    /// Apply inplace to (x,y)
    virtual void apply(double& x, double& y) const = 0;

    xoj::util::Point<double> operator()(xoj::util::Point<double> p) const;
    virtual Point operator()(Point p) const = 0;

    /// Whether or not applying this transformation to the bounding box gets the right bounding box
    virtual bool transformsBoundingBoxes() const = 0;
    /// Get the scale applied by transformation on pressure values
    virtual double getScaleFactor() const = 0;

    /// Creates a Transformation that moves elements from origPos to pos
    static std::unique_ptr<Transformation> fromTo(const Position& origPos, const Position& pos,
                                                  bool scaleLineWidth = false);
};
};  // namespace xoj::geometry
