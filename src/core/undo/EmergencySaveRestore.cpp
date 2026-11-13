#include "EmergencySaveRestore.h"

#include "undo/UndoAction.h"  // for UndoAction
#include "util/i18n.h"        // for _

class DocumentControl;

EmergencySaveRestore::EmergencySaveRestore(): UndoAction("EmergencySaveRestore") {}

EmergencySaveRestore::~EmergencySaveRestore() = default;

auto EmergencySaveRestore::redo(DocumentControl* control) -> bool {
    // Does nothing, only used to mark the document as changed
    return true;
}

auto EmergencySaveRestore::undo(DocumentControl* control) -> bool {
    // Does nothing, only used to mark the document as changed
    return true;
}

auto EmergencySaveRestore::getText() -> std::string { return _("Emergency saved document"); }
