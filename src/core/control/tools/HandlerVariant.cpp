#include "HandlerVariant.h"

#include "ArrowHandler.h"
#include "CoordinateSystemHandler.h"
#include "EllipseHandler.h"
#include "RectangleHandler.h"
#include "RulerHandler.h"
#include "SplineHandler.h"
#include "StrokeHandler.h"

void InputHandlerVariant::reset() {
    this->data.emplace<std::monostate>();
    this->ptr = nullptr;
}

InputHandlerVariant::operator bool() const { return this->getType() != NONE; }

auto InputHandlerVariant::getType() const -> HandlerType { return static_cast<HandlerType>(this->data.index()); }
