#include "Transformation.h"

#include "util/Assert.h"

#include "AffineTransformation.h"
#include "Position.h"
#include "Translation.h"

namespace xoj::geometry {
xoj::util::Point<double> Transformation::operator()(xoj::util::Point<double> p) const {
    this->apply(p.x, p.y);
    return p;
}

std::unique_ptr<Transformation> Transformation::fromTo(const Position& origPos, const Position& pos,
                                                       bool scaleLineWidth) {
    xoj_assert(origPos.angle == 0.);
    const double mx = origPos.halfWidth == 0.0 ? 1.0 : pos.halfWidth / origPos.halfWidth;
    const double my = origPos.halfHeight == 0.0 ? 1.0 : pos.halfHeight / origPos.halfHeight;

    if (pos.angle == 0.0 && mx == 1.0 && my == 1.0) {
        return std::make_unique<Translation>(pos.center.x - origPos.center.x, pos.center.y - origPos.center.y);
    }

    const double cos = std::cos(pos.angle);
    const double sin = std::sin(pos.angle);
    const double mz = scaleLineWidth ? std::sqrt(std::abs(mx * my)) : 1.0;

    return std::make_unique<AffineTransformation>(
            cos * mx, -sin * my, sin * mx, cos * my,
            -cos * mx * origPos.center.x + sin * my * origPos.center.y + pos.center.x,
            -sin * mx * origPos.center.x - cos * my * origPos.center.y + pos.center.y, mz);
}
};  // namespace xoj::geometry
