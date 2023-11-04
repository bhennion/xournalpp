#include "Operation.h"

#include <algorithm>  // for min
#include <cmath>      // for abs, cos, sin, hypot...

#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "gui/XournalppCursor.h"                   // for XournalppCursor
#include "model/SelectedContent.h"                 // for SelectedContent
#include "util/Assert.h"                           // for xoj_assert, xoj_assert_message

#include "Operation.h"  // for Selection

Operation::Operation(const Position& pos, const SnapToGridInputHandler& snapper): position(pos), snapper(snapper) {}

Rotation::Rotation(const Position& pos, const SnapToGridInputHandler& snapper, XournalppCursor& cursor):
        Operation(pos, snapper), cursor(cursor) {}

void Rotation::processPoint(const xoj::util::Point<double>& p, bool alt) {
    this->position.angle = snapper.snapAngle(atan2(p.y - this->position.center.y, p.x - this->position.center.x), alt);
    this->cursor.setRotationAngle(180.0 / M_PI * this->position.angle);
}


Translation::Translation(const Position& pos, const SnapToGridInputHandler& snapper,
                         const xoj::util::Point<double>& mouseDownPoint):
        Operation(pos, snapper), mouseDownPoint(mouseDownPoint - pos.center) {
    const double sin = std::sin(pos.angle), cos = std::cos(pos.angle);
    const auto& p = this->mouseDownPoint;
    const xoj::util::Point<double> unrotated(cos * p.x + sin * p.y, -sin * p.x + cos * p.y);
    xoj::util::Point<double> unrotatedCorner(std::copysign(pos.halfWidth, unrotated.x),
                                             std::copysign(pos.halfHeight, unrotated.y));
    this->snappedCorner = xoj::util::Point<double>(cos * unrotatedCorner.x - sin * unrotatedCorner.y,
                                                   sin * unrotatedCorner.x + cos * unrotatedCorner.y);
}

void Translation::processPoint(const xoj::util::Point<double>& p, bool alt) {
    const Point q = snapper.snapToGrid(
            Point(p.x - mouseDownPoint.x + snappedCorner.x, p.y - mouseDownPoint.y + snappedCorner.y), alt);
    this->position.center = xoj::util::Point<double>(q.x, q.y) - snappedCorner;
}


Rescaling::Rescaling(const Position& pos, const SnapToGridInputHandler& snapper, CursorSelectionType type,
                     const xoj::util::Point<double> mouseDownPoint, double minSize, bool allowMirroring):
        Operation(pos, snapper), mouseDownPoint(mouseDownPoint), originalPosition(pos), allowMirroring(allowMirroring) {
    xoj::util::Point<double> draggedCornerBeforeRotation;
    switch (type) {
        case CURSOR_SELECTION_TOP_LEFT:
            signX = -1;
            signY = -1;
            draggedCornerBeforeRotation = xoj::util::Point<double>(-pos.halfWidth, -pos.halfHeight);
            break;
        case CURSOR_SELECTION_TOP:
            signX = 0;
            signY = -1;
            draggedCornerBeforeRotation = xoj::util::Point<double>(0.0, -pos.halfHeight);
            break;
        case CURSOR_SELECTION_TOP_RIGHT:
            signX = 1;
            signY = -1;
            draggedCornerBeforeRotation = xoj::util::Point<double>(pos.halfWidth, -pos.halfHeight);
            break;
        case CURSOR_SELECTION_LEFT:
            signX = -1;
            signY = 0;
            draggedCornerBeforeRotation = xoj::util::Point<double>(-pos.halfWidth, 0.0);
            break;
        case CURSOR_SELECTION_RIGHT:
            signX = 1;
            signY = 0;
            draggedCornerBeforeRotation = xoj::util::Point<double>(pos.halfWidth, 0.0);
            break;
        case CURSOR_SELECTION_BOTTOM_LEFT:
            signX = -1;
            signY = 1;
            draggedCornerBeforeRotation = xoj::util::Point<double>(-pos.halfWidth, pos.halfHeight);
            break;
        case CURSOR_SELECTION_BOTTOM:
            signX = 0;
            signY = 1;
            draggedCornerBeforeRotation = xoj::util::Point<double>(0.0, pos.halfHeight);
            break;
        case CURSOR_SELECTION_BOTTOM_RIGHT:
            signX = 1;
            signY = 1;
            draggedCornerBeforeRotation = xoj::util::Point<double>(pos.halfWidth, pos.halfHeight);
            break;
        default:
            xoj_assert_message(false, "Rescaling created with illegal type: " + std::to_string(type));
    }
    const double sin = std::sin(pos.angle), cos = std::cos(pos.angle);
    this->draggedCorner = {cos * draggedCornerBeforeRotation.x - sin * draggedCornerBeforeRotation.y,
                           sin * draggedCornerBeforeRotation.x + cos * draggedCornerBeforeRotation.y};
    double hypot = std::hypot(draggedCorner.x, draggedCorner.y);
    rescalingDirection = draggedCorner / hypot;
    draggedCorner += pos.center;

    originalSpan = 2.0 * hypot;

    // get the smallest component of draggedCornerBeforeRotation that is non-zero.
    double smallestDir =
            draggedCornerBeforeRotation.x != 0.0 ?
                    (draggedCornerBeforeRotation.y != 0.0 ? std::min(std::abs(draggedCornerBeforeRotation.y),
                                                                     std::abs(draggedCornerBeforeRotation.x)) :
                                                            std::abs(draggedCornerBeforeRotation.x)) :
                    std::abs(draggedCornerBeforeRotation.y);
    xoj_assert(smallestDir != 0.0);
    minimalSpan = std::abs(0.5 * minSize / smallestDir);
}

auto Rescaling::getCenter() const -> xoj::util::Point<double> { return 2. * originalPosition.center - draggedCorner; }

void Rescaling::processPoint(const xoj::util::Point<double>& p, bool alt) {
    auto motion = p - mouseDownPoint;
    // Projection of the motion vector onto the rescaling direction
    double deltaSpan = motion.x * rescalingDirection.x + motion.y * rescalingDirection.y;

    // Avoid making boxes with size 0
    if (double span = originalSpan + deltaSpan; span < minimalSpan) {
        if (!allowMirroring || span > 0.0) {
            span = minimalSpan;
        } else {
            span = std::min(span, -minimalSpan);
        }
        deltaSpan = span - originalSpan;
    }
    auto cornerMotion = deltaSpan * rescalingDirection;
    Point corner(draggedCorner.x + cornerMotion.x, draggedCorner.y + cornerMotion.y);

    if (auto delta = snapper.snapAlong(corner, rescalingDirection, alt); delta != 0.0) {
        // Ensure snapping would not make too small a box
        if (std::abs(deltaSpan + delta + originalSpan) > minimalSpan) {
            deltaSpan += delta;
            cornerMotion = deltaSpan * rescalingDirection;
        }
    }

    cornerMotion *= 0.5;  // The halfWidth/halfHeight/center only change by half the motion
    position.center = originalPosition.center + cornerMotion;
    const double sin = std::sin(position.angle), cos = std::cos(position.angle);
    // halfWidth/halfHeight are computed in the unrotated coordinates
    cornerMotion = {cos * cornerMotion.x + sin * cornerMotion.y, -sin * cornerMotion.x + cos * cornerMotion.y};
    // cornerMotion.x/y could be negative, so std::copysign doesn't work here
    cornerMotion = {std::signbit(signX) ? -cornerMotion.x : cornerMotion.x,
                    std::signbit(signY) ? -cornerMotion.y : cornerMotion.y};
    position.halfWidth = originalPosition.halfWidth + cornerMotion.x;
    position.halfHeight = originalPosition.halfHeight + cornerMotion.y;
}
