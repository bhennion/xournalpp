/*
 * Xournal++
 *
 * Toolbar drag & drop controller
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <vector>

class MainControl;
class ToolbarAdapter;
class ToolbarCustomizeDialog;

class ToolbarDragDropHandler final {
public:
    ToolbarDragDropHandler(MainControl* control);
    ~ToolbarDragDropHandler();

public:
    void configure();

    void toolbarConfigDialogClosed();

public:
    void prepareToolbarsForDragAndDrop();
    void clearToolbarsFromDragAndDrop();

    inline MainControl* getControl() const { return control; }

private:
    MainControl* control;

    std::vector<std::unique_ptr<ToolbarAdapter>> toolbars;
    std::unique_ptr<ToolbarCustomizeDialog> customizeDialog;
};
