/*
 * Xournal++
 *
 * Undo action for insert (write text, draw stroke...)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class Element;
class Layer;
class DocumentControl;

using ElementPtr = std::unique_ptr<Element>;

class TextBoxUndoAction: public UndoAction {
public:
    TextBoxUndoAction(const PageRef& page, Layer* layer, Element* element, ElementPtr oldelement);
    ~TextBoxUndoAction() override;

public:
    auto undo(DocumentControl* control) -> bool override;
    auto redo(DocumentControl* control) -> bool override;

    auto getText() -> std::string override;

private:
    Layer* layer;
    Element* element;
    ElementPtr oldelement;
};
