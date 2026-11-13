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
class SizeUndoActionEntry;
class Stroke;
class DocumentControl;

class SizeUndoAction: public UndoAction {
public:
    SizeUndoAction(const PageRef& page, Layer* layer);
    ~SizeUndoAction() override;

public:
    bool undo(DocumentControl* control) override;
    bool redo(DocumentControl* control) override;
    std::string getText() override;

    void addStroke(Stroke* s, double originalWidth, double newWidth, std::vector<double> originalPressure,
                   std::vector<double> newPressure, size_t pressureCount);

public:
    static std::vector<double> getPressure(Stroke* s);

private:
    std::vector<SizeUndoActionEntry*> data;

    Layer* layer;
};
