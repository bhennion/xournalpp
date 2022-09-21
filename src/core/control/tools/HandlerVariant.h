/*
 * Xournal++
 *
 * Variant for storing all possible tool handlers and avoid constant reallocation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "util/SameBaseVariant.h"

#include "ArrowHandler.h"
#include "CoordinateSystemHandler.h"
#include "EllipseHandler.h"
#include "RectangleHandler.h"
#include "RulerHandler.h"
#include "SplineHandler.h"
#include "StrokeHandler.h"

// The following enum must correspond to the list of type of the SameBaseVariant below
enum class InputHandlerType : size_t {
    NONE,
    ARROW_HANDLER,
    COORDINATE_SYSTEM_HANDLER,
    ELLIPSE_HANDLER,
    RECTANGLE_HANDLER,
    RULER_HANDLER,
    SPLINE_HANDLER,
    STROKE_HANDLER
};

class InputHandlerVariant:
        public SameBaseVariant<InputHandlerType, InputHandler,  // Base class
                               ArrowHandler,                    // ARROW_HANDLER
                               CoordinateSystemHandler,         // COORDINATE_SYSTEM_HANDLER
                               EllipseHandler,                  // ELLIPSE_HANDLER
                               RectangleHandler,                // RECTANGLE_HANDLER
                               RulerHandler,                    // RULER_HANDLER
                               SplineHandler,                   // SPLINE_HANDLER
                               StrokeHandler>                   // STROKE_HANDLER
{};
