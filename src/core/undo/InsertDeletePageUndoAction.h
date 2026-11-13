/*
 * Xournal++
 *
 * Undo action for insert page / delete page
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

class DocumentControl;


class InsertDeletePageUndoAction: public UndoAction {
public:
    InsertDeletePageUndoAction(const PageRef& page, size_t pagePos, bool inserted);
    ~InsertDeletePageUndoAction() override;

public:
    bool undo(DocumentControl* control) override;
    bool redo(DocumentControl* control) override;

    std::string getText() override;

private:
    bool insertPage(DocumentControl* control);
    bool deletePage(DocumentControl* control);

private:
    bool inserted;
    size_t pagePos;
};
