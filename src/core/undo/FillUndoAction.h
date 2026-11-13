/*
 * Xournal++
 *
 * Undo action resize
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string
#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class Layer;
class FillUndoActionEntry;
class Stroke;
class DocumentControl;

class FillUndoAction: public UndoAction {
public:
    FillUndoAction(const PageRef& page, Layer* layer);
    ~FillUndoAction() override;

public:
    bool undo(DocumentControl* control) override;
    bool redo(DocumentControl* control) override;
    std::string getText() override;

    void addStroke(Stroke* s, int originalFill, int newFill);

private:
    std::vector<FillUndoActionEntry*> data;

    Layer* layer;
};
