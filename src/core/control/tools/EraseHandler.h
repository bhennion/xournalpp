/*
 * Xournal++
 *
 * Handles the erase of stroke, in special split into different parts etc.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>

#include "model/PageRef.h"
#include "view/PageViewBase.h"  // for PageViewPoolRef

class DeleteUndoAction;
class Document;
class EraseUndoAction;
class Layer;
class Range;
class Redrawable;
class Stroke;
class ToolHandler;
class UndoRedoHandler;

class EraseHandler {
public:
    EraseHandler(UndoRedoHandler* undo, Document* doc, const PageRef& page, ToolHandler* handler, const xoj::view::PageViewPoolRef&);
    virtual ~EraseHandler();

public:
    void erase(double x, double y);
    void finalize();

private:
    void eraseStroke(Layer* l, Stroke* s, double x, double y, Range& range);

private:
    PageRef page;
    ToolHandler* handler;
    xoj::view::PageViewPoolRef pageViewPool;
    Document* doc;
    UndoRedoHandler* undo;

    DeleteUndoAction* eraseDeleteUndoAction;
    EraseUndoAction* eraseUndoAction;

    double halfEraserSize;

private:
    /**
     * Coefficient for adding padding to the erased sections of strokes.
     * It depends on the stroke cap style ROUND, BUTT or SQUARE.
     * The order must match the enum StrokeCapStyle in Stroke.h
     */
    static constexpr double PADDING_COEFFICIENT_CAP[] = {0.4, 0.01, 0.5};
};
