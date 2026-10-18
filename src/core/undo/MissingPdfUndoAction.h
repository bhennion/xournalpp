/*
 * Xournal++
 *
 * Undo action for replacing a missing PDF file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"  // for UndoAction
#include "filesystem.h"  // for path

class Control;

class MissingPdfUndoAction: public UndoAction {
public:
    MissingPdfUndoAction(const xoj::FilePath& oldFilepath, bool oldAttachPdf);
    ~MissingPdfUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    xoj::FilePath filepath;
    bool attachPdf;
};
