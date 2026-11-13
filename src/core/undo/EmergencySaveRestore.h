/*
 * Xournal++
 *
 * Element used to mark the document as changed, so it is changed after restore.
 * Without this, it's unchanged and does not ask for save
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "UndoAction.h"  // for UndoAction

class DocumentControl;


class EmergencySaveRestore: public UndoAction {
public:
    EmergencySaveRestore();
    ~EmergencySaveRestore() override;

public:
    bool undo(DocumentControl* control) override;
    bool redo(DocumentControl* control) override;

    std::string getText() override;

private:
};
