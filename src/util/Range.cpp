#include "util/Range.h"
#include "util/Rectangle.h"

#include <algorithm>  // for max, min
#include <limits>     // for max, lowest

Range::Range(): x1(std::numeric_limits<double>::max()), y1(std::numeric_limits<double>::max()), x2(std::numeric_limits<double>::lowest()), y2(std::numeric_limits<double>::lowest()) {}

Range::Range(double x, double y): x1(x), y1(y), x2(x), y2(y) {}

Range::Range(const xoj::util::Rectangle<double>& r): x1(r.x), y1(r.y), x2(r.x + r.width), y2(r.y + r.height) {}

Range::Range(double x1, double y1, double x2, double y2): x1(x1), y1(y1), x2(x2), y2(y2) {}

void Range::addPoint(double x, double y) {
    this->x1 = std::min(this->x1, x);
    this->x2 = std::max(this->x2, x);

    this->y1 = std::min(this->y1, y);
    this->y2 = std::max(this->y2, y);
}

auto Range::getX() const -> double { return this->x1; }

auto Range::getY() const -> double { return this->y1; }

auto Range::getWidth() const -> double { return this->x2 - this->x1; }

auto Range::getHeight() const -> double { return this->y2 - this->y1; }

auto Range::getX2() const -> double { return this->x2; }

auto Range::getY2() const -> double { return this->y2; }

void Range::addPadding(double padding) {
    this->x1 -= padding;
    this->x2 += 2.0 * padding;
    this->y1 -= padding;
    this->y2 += 2.0 * padding;
}


auto Range::empty() const -> bool { return this->x1 > this->x2 || this->y1 > this->y2; }
