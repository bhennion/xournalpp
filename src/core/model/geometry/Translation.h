/*
 * Xournal++
 *
 * Translation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include "Transformation.h"

namespace xoj::geometry {
class Translation final: public Transformation {
public:
    Translation(double dx, double dy);
    ~Translation() noexcept override = default;

    void apply(double& x, double& y) const override;

    Point operator()(Point p) const override;

    inline bool transformsBoundingBoxes() const override { return true; }
    inline double getScaleFactor() const override { return 1.0; }

private:
    double dx;
    double dy;
};
};  // namespace xoj::geometry
