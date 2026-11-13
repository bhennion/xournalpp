/*
 * Xournal++
 *
 * Undo action for page size change
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


class PageSizeChangeUndoAction: public UndoAction {
public:
    PageSizeChangeUndoAction(const PageRef& page, double origW, double origH);
    ~PageSizeChangeUndoAction() override;

public:
    bool undo(DocumentControl* control) override;
    bool redo(DocumentControl* control) override;

    std::string getText() override;

private:
    bool swapSizes(DocumentControl* ctrl);
    double otherWidth;
    double otherHeight;
};
