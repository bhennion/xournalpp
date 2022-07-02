/*
 * Xournal++
 *
 * Range
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

namespace xoj::util {
    template<typename Float>
    class Rectangle;
};

class Range final {
public:
    Range(); // Empty range
    Range(double x, double y);
    Range(double x1, double y1, double x2, double y2);
    explicit Range(const xoj::util::Rectangle<double>& r);
    ~Range() = default;

    void addPoint(double x, double y);

    double getX() const;
    double getY() const;
    double getWidth() const;
    double getHeight() const;

    double getX2() const;
    double getY2() const;

    void addPadding(double padding);

    [[maybe_unused]] bool empty() const;

private:
    double x1;
    double y1;
    double x2;
    double y2;
};
