/*
 * Xournal++
 *
 * Wrapper for undo / redo stuff, to move out the logic of the main Control class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>  // for vector

class DocumentControl;
class Layer;
class Element;

class UndoRedoController {
private:
    UndoRedoController(DocumentControl* control);
    virtual ~UndoRedoController();

private:
    void before();
    void after();

public:
    static void undo(DocumentControl* control);
    static void redo(DocumentControl* control);

private:
    /**
     * Controller
     */
    DocumentControl* control = nullptr;

    /**
     * Layer of the selection before change
     */
    Layer* layer = nullptr;

    /**
     * Selected elements
     */
    std::vector<const Element*> elements;
};
