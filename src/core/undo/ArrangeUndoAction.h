/*
 * Xournal++
 *
 * Undo move action (EditSelection)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <deque>    // for deque
#include <string>   // for string
#include <utility>  // for pair
#include <vector>   // for vector

#include "model/Element.h"  // for Element::Index, Element
#include "model/ElementInsertionPosition.h"  // for InsertionOrder
#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class DocumentControl;
class Layer;

class ArrangeUndoAction: public UndoAction {
public:
    ArrangeUndoAction(const PageRef& page, Layer* layer, std::string desc, InsertionOrderRef oldOrder,
                      InsertionOrderRef newOrder);
    ~ArrangeUndoAction() override;

public:
    bool undo(DocumentControl* control) override;
    bool redo(DocumentControl* control) override;
    std::string getText() override;

private:
    void applyRearrange(DocumentControl* control);

private:
    Layer* layer;

    /** Description of the ordering action. */
    std::string description;

    // These track the ordering of elements
    InsertionOrderRef oldOrder;
    InsertionOrderRef newOrder;
};
