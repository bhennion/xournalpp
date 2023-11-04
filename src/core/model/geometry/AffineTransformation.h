/*
 * Xournal++
 *
 * Affine transformation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include "Transformation.h"

namespace xoj::geometry {
class AffineTransformation final: public Transformation {
public:
    /**
     * AffineTransformation with matrix
     *     a  b  x0
     *     c  d  y0
     */
    AffineTransformation(double a, double b, double c, double d, double x0, double y0, double mz);
    ~AffineTransformation() noexcept override = default;

    void apply(double& x, double& y) const override;

    Point operator()(Point p) const override;

    inline bool transformsBoundingBoxes() const override { return false; }
    inline double getScaleFactor() const override { return mz; }

private:
    const double a, b, c, d, x0, y0;  // Augmented matrix applied to 2d-points
    const double mz;                  // Multiplier of the Point::z coordinate (=pressure)
};
};  // namespace xoj::geometry
