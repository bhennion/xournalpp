#include "Translation.h"

#include "model/Point.h"
#include "util/Point.h"

namespace xoj::geometry {
Translation::Translation(double dx, double dy): dx(dx), dy(dy) {}

void Translation::apply(double& x, double& y) const {
    x += dx;
    y += dy;
}

Point Translation::operator()(Point p) const {
    apply(p.x, p.y);
    return p;
}
};  // namespace xoj::geometry
