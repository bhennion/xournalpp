#include "InputHandler.h"

InputHandler::InputHandler(Control* control, const PageRef& page): control(control), page(page) {}
InputHandler::~InputHandler() = default;

bool InputHandler::isReadyForDeletion() const { return readyForDeletion; }
