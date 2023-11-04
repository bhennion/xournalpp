#include "AffineTransformation.h"

#include "model/Point.h"
#include "util/Point.h"

namespace xoj::geometry {
AffineTransformation::AffineTransformation(double a, double b, double c, double d, double x0, double y0, double mz):
        a(a), b(b), c(c), d(d), x0(x0), y0(y0), mz(mz) {}

void AffineTransformation::apply(double& x, double& y) const {
    const double copyX = x;
    x = a * x + b * y + x0;
    y = c * copyX + d * y + y0;
}

Point AffineTransformation::operator()(Point p) const {
    apply(p.x, p.y);
    p.z = p.z == Point::NO_PRESSURE ? Point::NO_PRESSURE : p.z * mz;
    return p;
}
};  // namespace xoj::geometry
