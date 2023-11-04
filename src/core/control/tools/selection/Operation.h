/*
 * Xournal++
 *
 * Operations for moving selections
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/geometry/Position.h"
#include "undo/UndoAction.h"  // for UndoAction (ptr only)

#include "CursorSelectionType.h"  // for CursorSelectionType, CURS...

class SnapToGridInputHandler;
class XournalppCursor;
class SelectedContent;

class Operation {
public:
    Operation(const Position& pos, const SnapToGridInputHandler& snapper);
    virtual ~Operation() = default;

    /**
     * Process a pointer motion
     * @param p The new pointer location, in page coordinates
     */
    virtual void processPoint(const xoj::util::Point<double>& p, bool alt) = 0;

    inline const Position& getPosition() const { return position; }

protected:
    Position position;
    const SnapToGridInputHandler& snapper;
};

class Rotation final: public Operation {
public:
    Rotation(const Position& pos, const SnapToGridInputHandler& snapper, XournalppCursor& cursor);
    ~Rotation() override = default;

    void processPoint(const xoj::util::Point<double>& p, bool alt) override;

private:
    XournalppCursor& cursor;
};

class Translation final: public Operation {
public:
    Translation(const Position& pos, const SnapToGridInputHandler& snapper,
                const xoj::util::Point<double>& mouseDownPoint);
    ~Translation() override = default;

    void processPoint(const xoj::util::Point<double>& p, bool alt) override;

private:
    /// Coordinates relative to this->position.center
    xoj::util::Point<double> mouseDownPoint;
    /// Coordinates relative to this->position.center
    xoj::util::Point<double> snappedCorner;
};

class Rescaling final: public Operation {
public:
    Rescaling(const Position& pos, const SnapToGridInputHandler& snapper, CursorSelectionType type,
              const xoj::util::Point<double> mouseDownPoint, double minSize, bool allowMirroring);
    ~Rescaling() override = default;

    void processPoint(const xoj::util::Point<double>& p, bool alt) override;

    /// Returns the center of the homothety, in page coordinates
    xoj::util::Point<double> getCenter() const;

private:
    /// Normalized rescaling direction
    xoj::util::Point<double> rescalingDirection;

    /// In page coordinates
    xoj::util::Point<double> mouseDownPoint;

    /// Original position of the dragged corner, in page coordinates
    xoj::util::Point<double> draggedCorner;

    Position originalPosition;

    double minimalSpan;
    double originalSpan;
    bool allowMirroring;
    double signX;
    double signY;
};
