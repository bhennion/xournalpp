#include "Position.h"

void Position::rescale(FixedCorner corner, double halfW, double halfH) {
    const double dx = corner.ew == FixedCorner::EAST ? halfWidth - halfW : halfW - halfWidth;
    const double dy = corner.ns == FixedCorner::SOUTH ? halfHeight - halfH : halfH - halfHeight;
    const double cos = std::cos(angle);
    const double sin = std::sin(angle);

    center.x += cos * dx - sin * dy;
    center.x += sin * dx + cos * dy;
    halfWidth = halfW;
    halfHeight = halfH;
}

void Position::setAngle(double angle) { this->angle = angle; }
void Position::moveTo(const xoj::util::Point<double>& c) { center = c; }
