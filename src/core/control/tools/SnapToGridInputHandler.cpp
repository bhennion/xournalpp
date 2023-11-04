#include "SnapToGridInputHandler.h"

#include "control/settings/Settings.h"
#include "model/Snapping.h"
#include "util/Point.h"

SnapToGridInputHandler::SnapToGridInputHandler(const Settings* settings): settings(settings) {}

double SnapToGridInputHandler::snapVertically(double y, bool alt) const {
    if (alt != settings->isSnapGrid()) {
        return Snapping::snapVertically(y, settings->getSnapGridSize(), settings->getSnapGridTolerance());
    }
    return y;
}

double SnapToGridInputHandler::snapHorizontally(double x, bool alt) const {
    if (alt != settings->isSnapGrid()) {
        return Snapping::snapHorizontally(x, settings->getSnapGridSize(), settings->getSnapGridTolerance());
    }
    return x;
}

double SnapToGridInputHandler::snapAlong(Point const& pos, xoj::util::Point<double> direction, bool alt) const {
    if (alt != settings->isSnapGrid()) {
        double deltaSnappedX = std::numeric_limits<double>::infinity();
        if (std::abs(direction.x) > std::numeric_limits<double>::epsilon()) {
            double x = Snapping::snapHorizontally(pos.x, settings->getSnapGridSize(), settings->getSnapGridTolerance());
            if (double d = x - pos.x; d != 0.0) {
                deltaSnappedX = d / direction.x;
            }
        }
        double deltaSnappedY = std::numeric_limits<double>::infinity();
        if (std::abs(direction.y) > std::numeric_limits<double>::epsilon()) {
            double y = Snapping::snapVertically(pos.y, settings->getSnapGridSize(), settings->getSnapGridTolerance());
            if (double d = y - pos.y; d != 0.0) {
                deltaSnappedY = d / direction.y;
            }
        }
        const double tolerance = 0.5 * settings->getSnapGridSize() * settings->getSnapGridTolerance();
        if (std::abs(deltaSnappedX) < tolerance) {
            return std::abs(deltaSnappedY) < std::abs(deltaSnappedX) ? deltaSnappedY : deltaSnappedX;
        } else if (std::abs(deltaSnappedY) < tolerance) {
            return deltaSnappedY;
        }
    }
    return 0.0;
}

Point SnapToGridInputHandler::snapToGrid(Point const& pos, bool alt) const {
    if (alt != settings->isSnapGrid()) {
        return Snapping::snapToGrid(pos, settings->getSnapGridSize(), settings->getSnapGridTolerance());
    }
    return pos;
}

double SnapToGridInputHandler::snapAngle(double radian, bool alt) const {
    if (alt != settings->isSnapRotation()) {
        return Snapping::snapAngle(radian, settings->getSnapRotationTolerance());
    }
    return radian;
}

Point SnapToGridInputHandler::snapRotation(Point const& pos, Point const& center, bool alt) const {
    if (alt != settings->isSnapRotation()) {
        return Snapping::snapRotation(pos, center, settings->getSnapRotationTolerance());
    }
    return pos;
}

Point SnapToGridInputHandler::snap(Point const& pos, Point const& center, bool alt) const {
    Point rotationSnappedPoint{snapRotation(pos, center, alt)};
    return snapToGrid(rotationSnappedPoint, alt);
}
