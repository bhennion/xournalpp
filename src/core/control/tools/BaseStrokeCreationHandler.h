/*
 * Xournal++
 *
 * Handles input and optimizes the stroke
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr

#include "model/PageRef.h"  // for PageRef

#include "InputHandler.h"

class Control;
class Point;
class Stroke;

/**
 * @brief A base class to handle tools which create a stroke
 */
class BaseStrokeCreationHandler: public InputHandler {
public:
    BaseStrokeCreationHandler(Control* control, const PageRef& page);
    virtual ~BaseStrokeCreationHandler();

public:
    Stroke* getStroke() const;

    bool handlesElement(const Element* e) const override;

protected:
    [[nodiscard]] static std::unique_ptr<Stroke> createStroke(Control* control);

    static bool validMotion(Point p, Point q);

    /**
     * Smaller movements will be ignored.
     * Expressed in page coordinates
     */
    static constexpr double PIXEL_MOTION_THRESHOLD = 0.3;

protected:
    std::unique_ptr<Stroke> stroke;
};
