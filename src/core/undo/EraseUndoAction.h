/*
 * Xournal++
 *
 * Undo action for eraser
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <set>     // for multiset
#include <string>  // for string

#include "model/PageRef.h"  // for PageRef
#include "model/Stroke.h"   // for Stroke

#include "PageLayerPosEntry.h"  // for PageLayerPosEntry
#include "UndoAction.h"         // for UndoAction

class DocumentControl;
class Layer;

class EraseUndoAction: public UndoAction {
public:
    EraseUndoAction(const PageRef& page);

public:
    bool undo(DocumentControl* control) override;
    bool redo(DocumentControl* control) override;

    void addOriginal(Layer* layer, Stroke* element, Element::Index pos);
    void addEdited(Layer* layer, Stroke* element, Element::Index pos);
    [[maybe_unused]] void removeEdited(Stroke* element);

    void finalize();

    std::string getText() override;

private:
    std::multiset<PageLayerPosEntry<Stroke>> edited{};
    std::multiset<PageLayerPosEntry<Stroke>> original{};
};
