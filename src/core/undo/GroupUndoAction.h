/*
 * Xournal++
 *
 * Undo action to group a list of undo actions of the same type
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr
#include <string>  // for string
#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class DocumentControl;


class GroupUndoAction: public UndoAction {
public:
    GroupUndoAction();

public:
    void addAction(std::unique_ptr<UndoAction> action);

    /**
     * Get the affected pages
     */
    std::vector<PageRef> getPages() override;

    bool undo(DocumentControl* control) override;
    bool redo(DocumentControl* control) override;

    std::string getText() override;

private:
    std::vector<std::unique_ptr<UndoAction>> actions;
};
