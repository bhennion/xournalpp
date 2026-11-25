/**
 *
 *
 */
#pragma once

#include <type_traits>

#include <control/tools/TextEditor.h>

#include "control/AudioController.h"
#include "control/Control.h"
#include "control/NavigationHistory.h"
#include "control/ScrollHandler.h"
#include "control/ToolEnums.h"
#include "control/UndoRedoController.h"
#include "control/layer/LayerController.h"
#include "control/settings/Settings.h"
#include "control/zoom/ZoomControl.h"
#include "enums/Action.enum.h"
#include "gui/DocumentTab.h"
#include "gui/MainWindow.h"
#include "gui/SearchBar.h"
#include "gui/XournalView.h"
#include "gui/dialog/RenameLayerDialog.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"
#include "model/Document.h"
#include "model/Font.h"
#include "model/StrokeStyle.h"
#include "model/XojPage.h"
#include "plugin/PluginController.h"
#include "util/Assert.h"
#include "util/PopupWindowWrapper.h"
#include "util/Util.h"
#include "util/XojMsgBox.h"
#include "util/i18n.h"

#include "ActionDatabase.h"

/**
 * Template class to store Action properties. Expected members are:
 *      * a static member function
 *              static void callback(GSimpleAction*, GVariant*, MainControl*);
 *      * (optional) a member type state_type. If a state_type is provided, there must be a static member function
 *              static state_type initialState(MainControl* ctrl);
 *      * (optional) a member type parameter_type. If both are provided, parameter_type and state_type must agree.
 *      * (optional) a member static constexpr const char* accelerators[] = {"accel1", "accel2", nullptr} for
 *          accelerators that need to be set in the code rather than in the .xml files
 *      * (optional) a static member function
 *              static bool initiallyEnabled(MainControl*);
 *          Defaults to [](MainControl*){return true;}
 *      * (optional) a member type app_namespace = std::true_type if the action should be added to the app. namespace
 *          Otherwise, it is added to the win. namespace
 *
 * Note that both state_type and parameter_type must be convertible to GVariant. See util/GVariantTemplate.h
 */
template <Action action>
struct ActionProperties {};

/**
 * @brief true if the Action has a parameter
 */
template <Action a, class U = void>
struct has_param: std::false_type {};
template <Action a>
struct has_param<a, std::void_t<typename ActionProperties<a>::parameter_type>>: std::true_type {};

/**
 * @brief true if the Action has a state
 */
template <Action a, class U = void>
struct has_state: std::false_type {};
template <Action a>
struct has_state<a, std::void_t<typename ActionProperties<a>::state_type>>: std::true_type {};


/*** SPECIALIZATIONS ***/

/** File Menu **/
template <>
struct ActionProperties<Action::NEW_FILE> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->newFile(); }
};

template <>
struct ActionProperties<Action::OPEN> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->askToOpenFile(); }
};

template <>
struct ActionProperties<Action::ANNOTATE_PDF> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->askToAnnotatePdf(); }
};

template <>
struct ActionProperties<Action::SAVE> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::save>();
    }
};

template <>
struct ActionProperties<Action::SAVE_AS> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::saveAs>();
    }
};

template <>
struct ActionProperties<Action::EXPORT_AS_PDF> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::exportAsPdf>();
    }
};

template <>
struct ActionProperties<Action::EXPORT_AS> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::exportAs>();
    }
};
template <>
struct ActionProperties<Action::PRINT> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::print>();
    }
};
template <>
struct ActionProperties<Action::QUIT> {
    using app_namespace = std::true_type;
#ifdef __APPLE__
    static constexpr const char* accelerators[] = {"<Meta>Q", nullptr};
#else
    static constexpr const char* accelerators[] = {"<Ctrl>Q", nullptr};
#endif
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->quit(); }
};


/** Edit Menu **/
template <>
struct ActionProperties<Action::UNDO> {
#ifdef __APPLE__
    static constexpr const char* accelerators[] = {"<Meta>Z", nullptr};
#else
    static constexpr const char* accelerators[] = {"<Ctrl>Z", nullptr};
#endif
    static bool initiallyEnabled(MainControl*) { return false; }
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::undo>();
    }
};
template <>
struct ActionProperties<Action::REDO> {
#ifdef __APPLE__
    static constexpr const char* accelerators[] = {"<Meta><Shift>Z", "<Meta>Y", nullptr};
#else
    static constexpr const char* accelerators[] = {"<Ctrl><Shift>Z", "<Ctrl>Y", nullptr};
#endif
    static bool initiallyEnabled(MainControl*) { return false; }
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::redo>();
    }
};
template <>
struct ActionProperties<Action::CUT> {
#ifdef __APPLE__
    static constexpr const char* accelerators[] = {"<Meta>X", "Cut", nullptr};
#else
    static constexpr const char* accelerators[] = {"<Ctrl>X", "Cut", nullptr};
#endif
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->cut(); }
};
template <>
struct ActionProperties<Action::COPY> {
#ifdef __APPLE__
    static constexpr const char* accelerators[] = {"<Meta>C", "Copy", nullptr};
#else
    static constexpr const char* accelerators[] = {"<Ctrl>C", "Copy", nullptr};
#endif
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->copy(); }
};
template <>
struct ActionProperties<Action::PASTE> {
#ifdef __APPLE__
    static constexpr const char* accelerators[] = {"<Meta>V", "Paste", nullptr};
#else
    static constexpr const char* accelerators[] = {"<Ctrl>V", "Paste", nullptr};
#endif
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->paste(); }
};
template <>
struct ActionProperties<Action::SEARCH> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->clearSelectionEndText();
            docctrl->getSearchBar()->showSearchBar(true);
        }
    }
};

template <>
struct ActionProperties<Action::SELECT_ALL> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::selectAllOnPage>();
    }
};
template <>
struct ActionProperties<Action::DELETE> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            if (!docctrl->getDocumentTab()->getXournal()->actionDelete()) {
                docctrl->deleteSelection();
            }
        }
    }
};

template <>
struct ActionProperties<Action::ARRANGE_SELECTION_ORDER> {
    using parameter_type = EditSelection::OrderChange;
    static void callback(GSimpleAction*, GVariant* p, MainControl* ctrl) {
        auto change = getGVariantValue<EditSelection::OrderChange>(p);
        xoj_assert(change <= EditSelection::OrderChange::SendToBack);
        ctrl->activeDocDoes<&DocumentControl::reorderSelection>(change);
    }
};

template <>
struct ActionProperties<Action::MOVE_SELECTION_LAYER_UP> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            // moveSelectionToLayer takes layer number (layerid - 1) not id
            // therefore the new layer is "layerid - 1 + 1"
            docctrl->moveSelectionToLayer(docctrl->getCurrentPage()->getSelectedLayerId());
        }
    }
};

template <>
struct ActionProperties<Action::MOVE_SELECTION_LAYER_DOWN> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl();
            docctrl != nullptr && docctrl->getLayerController()->getCurrentLayerId() >= 2) {
            // moveSelectionToLayer takes layer number (layerid - 1) not id
            // therefore the new layer is "layerid - 1 - 1"
            docctrl->moveSelectionToLayer(docctrl->getCurrentPage()->getSelectedLayerId() - 2);
        }
    }
};

template <>
struct ActionProperties<Action::ROTATION_SNAPPING> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->isSnapRotation(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool enable = g_variant_get_boolean(p);
        ctrl->setRotationSnapping(enable);
    }
};

template <>
struct ActionProperties<Action::GRID_SNAPPING> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->isSnapGrid(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool enable = g_variant_get_boolean(p);
        ctrl->setGridSnapping(enable);
    }
};

template <>
struct ActionProperties<Action::PREFERENCES> {
    using app_namespace = std::true_type;
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->showSettings(); }
};


/** View Menu **/
template <>
struct ActionProperties<Action::PAIRED_PAGES_MODE> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->isShowPairedPages(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool enabled = g_variant_get_boolean(p);
        ctrl->activeDocDoes<&DocumentControl::setViewPairedPages>(enabled);
    }
};

template <>
struct ActionProperties<Action::PAIRED_PAGES_OFFSET> {
    using state_type = int;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->getPairsOffset(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        int offset = g_variant_get_int32(p);
        ctrl->activeDocDoes<&DocumentControl::setPairsOffset>(offset);
    }
};

template <>
struct ActionProperties<Action::PRESENTATION_MODE> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->isPresentationMode(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool enabled = g_variant_get_boolean(p);
        ctrl->setViewPresentationMode(enabled);
    }
};

template <>
struct ActionProperties<Action::FULLSCREEN> {
    using state_type = bool;
    static constexpr state_type initialState(MainControl*) { return false; }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool enabled = g_variant_get_boolean(p);
        ctrl->setViewFullscreenMode(enabled);
    }
};

template <>
struct ActionProperties<Action::SHOW_SIDEBAR> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->isSidebarVisible(); }
    static void callback(GSimpleAction*, GVariant* p, MainControl* ctrl) {
        ctrl->setShowSidebar(g_variant_get_boolean(p));
    }
};

template <>
struct ActionProperties<Action::SHOW_TOOLBAR> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->isToolbarVisible(); }
    static void callback(GSimpleAction*, GVariant* p, MainControl* ctrl) {
        ctrl->setShowToolbar(g_variant_get_boolean(p));
    }
};

template <>
struct ActionProperties<Action::SET_LAYOUT_VERTICAL> {
    using state_type = bool;
    using parameter_type = state_type;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->getViewLayoutVert(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool vertical = g_variant_get_boolean(p);
        ctrl->activeDocDoes<&DocumentControl::setViewLayoutVert>(vertical);
    }
};

template <>
struct ActionProperties<Action::SET_LAYOUT_RIGHT_TO_LEFT> {
    using state_type = bool;
    using parameter_type = state_type;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->getViewLayoutR2L(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool rightToLeft = g_variant_get_boolean(p);
        ctrl->activeDocDoes<&DocumentControl::setViewLayoutR2L>(rightToLeft);
    }
};

template <>
struct ActionProperties<Action::SET_LAYOUT_BOTTOM_TO_TOP> {
    using state_type = bool;
    using parameter_type = state_type;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->getViewLayoutB2T(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool bottomToTop = g_variant_get_boolean(p);
        ctrl->activeDocDoes<&DocumentControl::setViewLayoutB2T>(bottomToTop);
    }
};

template <>
struct ActionProperties<Action::SET_COLUMNS_OR_ROWS> {
    using state_type = int;
    using parameter_type = state_type;
    static state_type initialState(MainControl* ctrl) {
        auto* settings = ctrl->getSettings();
        return settings->isViewFixedRows() ? -settings->getViewRows() : settings->getViewColumns();
    }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        int value = g_variant_get_int32(p);
        xoj_assert(value != 0);
        if (value > 0) {
            ctrl->activeDocDoes<&DocumentControl::setViewColumns>(value);
        } else {
            ctrl->activeDocDoes<&DocumentControl::setViewRows>(std::abs(value));
        }
    }
};

template <>
struct ActionProperties<Action::MANAGE_TOOLBAR> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->manageToolbars(); }
};
template <>
struct ActionProperties<Action::CUSTOMIZE_TOOLBAR> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->customizeToolbars(); }
};
template <>
struct ActionProperties<Action::SHOW_MENUBAR> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->isMenubarVisible(); }
    static void callback(GSimpleAction*, GVariant* p, MainControl* ctrl) {
        ctrl->setShowMenubar(g_variant_get_boolean(p));
    }
};


/*
 * Zoom callbacks are postponed to later in the UI Thread
 * On slower machine this feels more fluent.
 */
template <>
struct ActionProperties<Action::ZOOM_IN> {
#ifdef __APPLE__
    static constexpr const char* accelerators[] = {"<Meta>plus", "<Meta>KP_Add", "<Meta>equal", nullptr};
#else
    static constexpr const char* accelerators[] = {"<Ctrl>plus", "<Ctrl>KP_Add", "<Ctrl>equal", nullptr};
#endif
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            Util::execInUiThread([zoom = docctrl->getZoomControl()]() { zoom->zoomOneStep(ZOOM_IN); });
        }
    }
};

template <>
struct ActionProperties<Action::ZOOM_OUT> {
#ifdef __APPLE__
    static constexpr const char* accelerators[] = {"<Meta>minus", "<Meta>KP_Subtract", nullptr};
#else
    static constexpr const char* accelerators[] = {"<Ctrl>minus", "<Ctrl>KP_Subtract", nullptr};
#endif
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            Util::execInUiThread([zoom = docctrl->getZoomControl()]() { zoom->zoomOneStep(ZOOM_OUT); });
        }
    }
};

template <>
struct ActionProperties<Action::ZOOM_100> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            Util::execInUiThread([zoom = docctrl->getZoomControl()]() { zoom->zoom100(); });
        }
    }
};

template <>
struct ActionProperties<Action::ZOOM_FIT> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->getForceZoomToFitOnLoad(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool enabled = g_variant_get_boolean(p);
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            Util::execInUiThread([enabled, zoom = docctrl->getZoomControl()]() { zoom->setZoomFitMode(enabled); });
        }
    }
};

template <>
struct ActionProperties<Action::ZOOM> {
    using state_type = double;
    static state_type initialState(MainControl*) { return 1.; }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        double scale = g_variant_get_double(p);
        xoj_assert(scale >= DEFAULT_ZOOM_MIN && scale <= DEFAULT_ZOOM_MAX);
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            Util::execInUiThread([scale, ctrl, zoomctrl = docctrl->getZoomControl()]() {
                double newZoom = ctrl->getBounds().convertFromZoomReal(scale);
                zoomctrl->setZoomFitMode(false);
                zoomctrl->startZoomSequence();
                zoomctrl->zoomSequenceChange(newZoom, false);
                zoomctrl->endZoomSequence();
            });
        }
    }
};

/** Navigation menu **/
template <>
struct ActionProperties<Action::GOTO_FIRST> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getScrollHandler()->goToFirstPage();
        }
    }
};
template <>
struct ActionProperties<Action::GOTO_PREVIOUS> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getScrollHandler()->goToPreviousPage();
        }
    }
};

template <>
struct ActionProperties<Action::GOTO_PAGE> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::showGotoPageDialog>();
    }
};
template <>
struct ActionProperties<Action::GOTO_NEXT> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getScrollHandler()->goToNextPage();
        }
    }
};
template <>
struct ActionProperties<Action::GOTO_LAST> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getScrollHandler()->goToLastPage();
        }
    }
};

template <>
struct ActionProperties<Action::GOTO_NEXT_ANNOTATED_PAGE> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getScrollHandler()->scrollToAnnotatedPage(true);
        }
    }
};

template <>
struct ActionProperties<Action::GOTO_PREVIOUS_ANNOTATED_PAGE> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getScrollHandler()->scrollToAnnotatedPage(false);
        }
    }
};

template <>
struct ActionProperties<Action::NAVIGATE_BACK> {
    static bool initiallyEnabled(MainControl*) { return false; }
    static constexpr const char* accelerators[] = {"<Alt>Left", nullptr};
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getNavigationHistory()->navigate(-1);
        }
    }
};

template <>
struct ActionProperties<Action::NAVIGATE_FORWARD> {
    static bool initiallyEnabled(MainControl*) { return false; }
    static constexpr const char* accelerators[] = {"<Alt>Right", nullptr};
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getNavigationHistory()->navigate(1);
        }
    }
};


/** Journal Menu **/
template <>
struct ActionProperties<Action::NEW_PAGE_BEFORE> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->insertNewPage(docctrl->getCurrentPageNo());
        }
    }
};

template <>
struct ActionProperties<Action::NEW_PAGE_AFTER> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->insertNewPage(docctrl->getCurrentPageNo() + 1);
        }
    }
};

template <>
struct ActionProperties<Action::NEW_PAGE_AT_END> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->insertNewPage(docctrl->getDocument()->getPageCount());
        }
    }
};

template <>
struct ActionProperties<Action::DUPLICATE_PAGE> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::duplicatePage>();
    }
};
template <>
struct ActionProperties<Action::MOVE_PAGE_TOWARDS_BEGINNING> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::movePageTowardsBeginning>();
    }
};
template <>
struct ActionProperties<Action::MOVE_PAGE_TOWARDS_END> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::movePageTowardsEnd>();
    }
};

template <>
struct ActionProperties<Action::APPEND_NEW_PDF_PAGES> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::appendNewPdfPages>();
    }
};
template <>
struct ActionProperties<Action::CONFIGURE_PAGE_TEMPLATE> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->showPaperTemplateDialog(); }
};
template <>
struct ActionProperties<Action::DELETE_PAGE> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::deletePage>();
    }
};

template <>
struct ActionProperties<Action::PAPER_FORMAT> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::showPaperFormatDialog>();
    }
};
template <>
struct ActionProperties<Action::PAPER_BACKGROUND_COLOR> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->activeDocDoes<&DocumentControl::showChangePageBackgroundColorDialog>();
    }
};


/** Tool menu **/
template <>
struct ActionProperties<Action::SELECT_TOOL> {
    using state_type = ToolType;
    using parameter_type = state_type;
    static state_type initialState(MainControl* ctrl) { return ctrl->getToolHandler()->getActiveTool()->getToolType(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        ToolType tt = getGVariantValue<ToolType>(p);
        xoj_assert(tt < TOOL_END_ENTRY);
        if (requiresClearedSelection(tt)) {
            ctrl->activeDocDoes<&DocumentControl::clearSelection>();
        }
        ctrl->selectTool(tt);
    }
};

template <>
struct ActionProperties<Action::SELECT_DEFAULT_TOOL> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->selectDefaultTool(); }
};

/*
 * To have the menu entries drawn as check boxes and not as radio buttons, each drawing type has a boolean-state
 * action (on/off). This action cannot take a parameter: we need one callback per drawing type
 */
template <DrawingType type>
struct ActionPropDrawingTypes {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) {
        return ctrl->getToolHandler()->getActiveTool()->getDrawingType() == type;
    }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        auto* actionDB = ctrl->getActionDatabase();

        actionDB->setActionState(Action::TOOL_DRAW_SHAPE_RECOGNIZER, false);
        actionDB->setActionState(Action::TOOL_DRAW_RECTANGLE, false);
        actionDB->setActionState(Action::TOOL_DRAW_ELLIPSE, false);
        actionDB->setActionState(Action::TOOL_DRAW_ARROW, false);
        actionDB->setActionState(Action::TOOL_DRAW_DOUBLE_ARROW, false);
        actionDB->setActionState(Action::TOOL_DRAW_COORDINATE_SYSTEM, false);
        actionDB->setActionState(Action::TOOL_DRAW_LINE, false);
        actionDB->setActionState(Action::TOOL_DRAW_SPLINE, false);

        g_simple_action_set_state(ga, p);
        bool enabled = g_variant_get_boolean(p);

        ctrl->setToolDrawingType(enabled ? type : DRAWING_TYPE_DEFAULT);
    }
};

template <>
struct ActionProperties<Action::TOOL_DRAW_SHAPE_RECOGNIZER>: ActionPropDrawingTypes<DRAWING_TYPE_SHAPE_RECOGNIZER> {};
template <>
struct ActionProperties<Action::TOOL_DRAW_RECTANGLE>: ActionPropDrawingTypes<DRAWING_TYPE_RECTANGLE> {};
template <>
struct ActionProperties<Action::TOOL_DRAW_ELLIPSE>: ActionPropDrawingTypes<DRAWING_TYPE_ELLIPSE> {};
template <>
struct ActionProperties<Action::TOOL_DRAW_ARROW>: ActionPropDrawingTypes<DRAWING_TYPE_ARROW> {};
template <>
struct ActionProperties<Action::TOOL_DRAW_DOUBLE_ARROW>: ActionPropDrawingTypes<DRAWING_TYPE_DOUBLE_ARROW> {};
template <>
struct ActionProperties<Action::TOOL_DRAW_COORDINATE_SYSTEM>: ActionPropDrawingTypes<DRAWING_TYPE_COORDINATE_SYSTEM> {};
template <>
struct ActionProperties<Action::TOOL_DRAW_LINE>: ActionPropDrawingTypes<DRAWING_TYPE_LINE> {};
template <>
struct ActionProperties<Action::TOOL_DRAW_SPLINE>: ActionPropDrawingTypes<DRAWING_TYPE_SPLINE> {};

template <>
struct ActionProperties<Action::SETSQUARE> {
    using state_type = bool;
    static constexpr state_type initialState(MainControl*) { return false; }
    static void callback(GSimpleAction* ga, GVariant*, MainControl* ctrl) {
        // Only one GeometryTool is activated at a time
        ctrl->getActionDatabase()->setActionState(Action::COMPASS, false);
        auto* docctrl = ctrl->getDocumentControl();
        bool value = docctrl ? docctrl->toggleSetsquare() : false;
        g_simple_action_set_state(ga, makeGVariant(value));
    }
};

template <>
struct ActionProperties<Action::COMPASS> {
    using state_type = bool;
    static constexpr state_type initialState(MainControl*) { return false; }
    static void callback(GSimpleAction* ga, GVariant*, MainControl* ctrl) {
        // Only one GeometryTool is activated at a time
        ctrl->getActionDatabase()->setActionState(Action::SETSQUARE, false);
        auto* docctrl = ctrl->getDocumentControl();
        bool value = docctrl ? docctrl->toggleCompass() : false;
        g_simple_action_set_state(ga, makeGVariant(value));
    }
};

// Pen submenu
template <>
struct ActionProperties<Action::TOOL_PEN_SIZE> {
    using state_type = ToolSize;
    using parameter_type = state_type;
    static state_type initialState(MainControl* ctrl) { return ctrl->getToolHandler()->getPenSize(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        ToolSize size = getGVariantValue<ToolSize>(p);
        xoj_assert(size < TOOL_SIZE_NONE);
        ctrl->getToolHandler()->setPenSize(size);
    }
};

template <>
struct ActionProperties<Action::TOOL_PEN_LINE_STYLE> {
    using state_type = const std::string&;
    using parameter_type = state_type;
    static std::string initialState(MainControl* ctrl) {
        if (auto* tool = ctrl->getToolHandler()->getActiveTool(); tool->getToolType() == TOOL_PEN) {
            return StrokeStyle::formatStyle(tool->getLineStyle());
        }
        return "none";
    }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        const char* styleName = g_variant_get_string(p, nullptr);
        ctrl->setLineStyle(styleName);
    }
};

template <>
struct ActionProperties<Action::TOOL_PEN_FILL> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) { return ctrl->getToolHandler()->getTool(TOOL_PEN).getFill(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool enabled = g_variant_get_boolean(p);
        ctrl->getToolHandler()->setPenFillEnabled(enabled);
    }
};

template <>
struct ActionProperties<Action::TOOL_PEN_FILL_OPACITY> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->showAlphaSelectionDialog(OPACITY_FILL_PEN);
    }
};
// Eraser submenu
template <>
struct ActionProperties<Action::TOOL_ERASER_SIZE> {
    using state_type = ToolSize;
    using parameter_type = state_type;
    static state_type initialState(MainControl* ctrl) { return ctrl->getToolHandler()->getEraserSize(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        ToolSize size = getGVariantValue<ToolSize>(p);
        xoj_assert(size < TOOL_SIZE_NONE);
        ctrl->getToolHandler()->setEraserSize(size);
    }
};

template <>
struct ActionProperties<Action::TOOL_ERASER_TYPE> {
    using state_type = EraserType;
    using parameter_type = state_type;
    static state_type initialState(MainControl* ctrl) { return ctrl->getToolHandler()->getEraserType(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        EraserType type = getGVariantValue<EraserType>(p);
        ctrl->setEraserType(type);
    }
};

// Highlighter submenu
template <>
struct ActionProperties<Action::TOOL_HIGHLIGHTER_SIZE> {
    using state_type = ToolSize;
    using parameter_type = state_type;
    static state_type initialState(MainControl* ctrl) { return ctrl->getToolHandler()->getHighlighterSize(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        ToolSize size = getGVariantValue<ToolSize>(p);
        xoj_assert(size < TOOL_SIZE_NONE);
        ctrl->getToolHandler()->setHighlighterSize(size);
    }
};

template <>
struct ActionProperties<Action::TOOL_HIGHLIGHTER_FILL> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) {
        return ctrl->getToolHandler()->getTool(TOOL_HIGHLIGHTER).getFill();
    }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool enabled = g_variant_get_boolean(p);
        ctrl->getToolHandler()->setHighlighterFillEnabled(enabled);
    }
};

template <>
struct ActionProperties<Action::TOOL_HIGHLIGHTER_FILL_OPACITY> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->showAlphaSelectionDialog(OPACITY_FILL_HIGHLIGHTER);
    }
};

template <>
struct ActionProperties<Action::TOOL_SELECT_PDF_TEXT_MARKER_OPACITY> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->showAlphaSelectionDialog(OPACITY_SELECT_PDF_TEXT_MARKER);
    }
};

template <>
struct ActionProperties<Action::TOGGLE_TOUCH_DRAWING> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->getTouchDrawingEnabled(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool enabled = g_variant_get_boolean(p);
        ctrl->getSettings()->setTouchDrawingEnabled(enabled);
    }
};

template <>
struct ActionProperties<Action::SELECT_FONT> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->showFontDialog(); }
};

template <>
struct ActionProperties<Action::FONT> {
    using state_type = const std::string&;
    using parameter_type = state_type;
    static std::string initialState(MainControl* ctrl) { return ctrl->getSettings()->getFont().asString(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        ctrl->setFont(XojFont(g_variant_get_string(p, nullptr)));
    }
};

template <>
struct ActionProperties<Action::TEXT_ALIGNMENT> {
    using state_type = TextAlignment::Value;
    using parameter_type = state_type;
    static state_type initialState(MainControl* ctrl) { return ctrl->getToolHandler()->getTextAlignment(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        TextAlignment al = getGVariantValue<TextAlignment::Value>(p);
        ctrl->getToolHandler()->setTextAlignment(al);
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            if (auto* te = docctrl->getTextEditor(); te) {
                te->setAlignment(al);
            }
        }
    }
};

template <>
struct ActionProperties<Action::TEXT_JUSTIFY> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) { return ctrl->getToolHandler()->getTextJustify(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool justify = g_variant_get_boolean(p);
        ctrl->getToolHandler()->setTextJustify(justify);
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            if (auto* te = docctrl->getTextEditor(); te) {
                te->setJustify(justify);
            }
        }
    }
};

template <>
struct ActionProperties<Action::AUDIO_RECORD> {
    using state_type = bool;
    static constexpr state_type initialState(MainControl*) { return false; }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
#ifdef ENABLE_AUDIO
        if (!ctrl->getAudioController()) {
            g_warning("Audio has been disabled");
            return;
        }
        bool enabled = g_variant_get_boolean(p);
        bool success = false;
        if (enabled) {
            success = ctrl->getAudioController()->startRecording();
        } else {
            success = ctrl->getAudioController()->stopRecording();
        }

        if (success) {
            g_simple_action_set_state(ga, p);
        } else {
            g_simple_action_set_state(ga, g_variant_new_boolean(!enabled));
            Util::execInUiThread([win = ctrl->getGtkWindow()]() {
                XojMsgBox::showErrorToUser(win, _("Recorder could not be started."));
            });
        }
#else
        g_warning("Audio has been disabled at compile time");
#endif
    }
};
template <>
struct ActionProperties<Action::AUDIO_PAUSE_PLAYBACK> {
    using state_type = bool;
    static constexpr state_type initialState(MainControl*) { return false; }
    static constexpr bool initiallyEnabled(MainControl*) { return false; }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
#ifdef ENABLE_AUDIO
        if (!ctrl->getAudioController()) {
            g_warning("Audio has been disabled");
            return;
        }
        g_simple_action_set_state(ga, p);
        bool enabled = g_variant_get_boolean(p);
        if (enabled) {
            ctrl->getAudioController()->pausePlayback();
        } else {
            ctrl->getAudioController()->continuePlayback();
        }
#else
        g_warning("Audio has been disabled at compile time");
#endif
    }
};

template <>
struct ActionProperties<Action::AUDIO_SEEK_FORWARDS> {
    static constexpr bool initiallyEnabled(MainControl*) { return false; }
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
#ifdef ENABLE_AUDIO
        if (!ctrl->getAudioController()) {
            g_warning("Audio has been disabled");
            return;
        }
        ctrl->getAudioController()->seekForwards();
#else
        g_warning("Audio has been disabled at compile time");
#endif
    }
};

template <>
struct ActionProperties<Action::AUDIO_SEEK_BACKWARDS> {
    static constexpr bool initiallyEnabled(MainControl*) { return false; }
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
#ifdef ENABLE_AUDIO
        if (!ctrl->getAudioController()) {
            g_warning("Audio has been disabled");
            return;
        }
        ctrl->getAudioController()->seekBackwards();
#else
        g_warning("Audio has been disabled at compile time");
#endif
    }
};

template <>
struct ActionProperties<Action::AUDIO_STOP_PLAYBACK> {
    static constexpr bool initiallyEnabled(MainControl*) { return false; }
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
#ifdef ENABLE_AUDIO
        if (!ctrl->getAudioController()) {
            g_warning("Audio has been disabled");
            return;
        }
        ctrl->getAudioController()->stopPlayback();
#else
        g_warning("Audio has been disabled at compile time");
#endif
    }
};


/** Plugin menu **/
template <>
struct ActionProperties<Action::PLUGIN_MANAGER> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        ctrl->getPluginController()->showPluginManager();
    }
};


/** Help menu **/
template <>
struct ActionProperties<Action::HELP> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { XojMsgBox::showHelp(ctrl->getGtkWindow()); }
};

template <>
struct ActionProperties<Action::DEMO> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->showGtkDemo(); }
};

template <>
struct ActionProperties<Action::ABOUT> {
    using app_namespace = std::true_type;
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->showAbout(); }
};


/** Generic tool config, for the toolbars **/
template <>
struct ActionProperties<Action::TOOL_SIZE> {
    using state_type = ToolSize;
    using parameter_type = state_type;
    static state_type initialState(MainControl* ctrl) {
        bool sizeEnable = ctrl->getToolHandler()->hasCapability(TOOL_CAP_SIZE);
        return sizeEnable ? ctrl->getToolHandler()->getActiveTool()->getSize() : TOOL_SIZE_NONE;
    }
    static bool initiallyEnabled(MainControl* ctrl) { return ctrl->getToolHandler()->hasCapability(TOOL_CAP_SIZE); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        ToolSize size = getGVariantValue<ToolSize>(p);
        xoj_assert(size < TOOL_SIZE_NONE);
        ctrl->setToolSize(size);
    }
};

template <>
struct ActionProperties<Action::TOOL_FILL> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) {
        return ctrl->getToolHandler()->hasCapability(TOOL_CAP_FILL) &&
               ctrl->getToolHandler()->getActiveTool()->getFill();
    }
    static bool initiallyEnabled(MainControl* ctrl) { return ctrl->getToolHandler()->hasCapability(TOOL_CAP_FILL); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool enabled = g_variant_get_boolean(p);
        ctrl->setFill(enabled);
    }
};

template <>
struct ActionProperties<Action::TOOL_FILL_OPACITY> {
    static bool initiallyEnabled(MainControl* ctrl) { return ctrl->getToolHandler()->hasCapability(TOOL_CAP_FILL); }
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto tt = ctrl->getToolHandler()->getToolType(); tt == TOOL_PEN) {
            ctrl->showAlphaSelectionDialog(OPACITY_FILL_PEN);
        } else if (tt == TOOL_HIGHLIGHTER) {
            ctrl->showAlphaSelectionDialog(OPACITY_FILL_HIGHLIGHTER);
        } else if (tt == TOOL_SELECT_PDF_TEXT_LINEAR || tt == TOOL_SELECT_PDF_TEXT_RECT) {
            ctrl->showAlphaSelectionDialog(OPACITY_SELECT_PDF_TEXT_MARKER);
        }
    }
};

template <>
struct ActionProperties<Action::TOOL_COLOR> {
    using state_type = Color;
    using parameter_type = state_type;
    static state_type initialState(MainControl* ctrl) {
        bool enable = ctrl->getToolHandler()->hasCapability(TOOL_CAP_COLOR);
        return enable ? ctrl->getToolHandler()->getActiveTool()->getColor() : Colors::black;
    }
    static bool initiallyEnabled(MainControl* ctrl) { return ctrl->getToolHandler()->hasCapability(TOOL_CAP_COLOR); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        ctrl->getToolHandler()->setColor(getGVariantValue<Color>(p), true);
    }
};

template <>
struct ActionProperties<Action::SELECT_COLOR> {
    static bool initiallyEnabled(MainControl* ctrl) { return ctrl->getToolHandler()->hasCapability(TOOL_CAP_COLOR); }
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) { ctrl->showColorChooserDialog(); }
};

// Layers
template <>
struct ActionProperties<Action::LAYER_SHOW_ALL> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getLayerController()->showAllLayer();
        }
    }
};
template <>
struct ActionProperties<Action::LAYER_HIDE_ALL> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getLayerController()->hideAllLayer();
        }
    }
};

template <>
struct ActionProperties<Action::LAYER_NEW_ABOVE_CURRENT> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getLayerController()->addNewLayer(false);
        }
    }
};

template <>
struct ActionProperties<Action::LAYER_NEW_BELOW_CURRENT> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getLayerController()->addNewLayer(true);
        }
    }
};

template <>
struct ActionProperties<Action::LAYER_COPY> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getLayerController()->copyCurrentLayer();
        }
    }
};

template <>
struct ActionProperties<Action::LAYER_MOVE_UP> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getLayerController()->moveCurrentLayer(true);
        }
    }
};

template <>
struct ActionProperties<Action::LAYER_MOVE_DOWN> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getLayerController()->moveCurrentLayer(false);
        }
    }
};

template <>
struct ActionProperties<Action::LAYER_DELETE> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getLayerController()->deleteCurrentLayer();
        }
    }
};

template <>
struct ActionProperties<Action::LAYER_MERGE_DOWN> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            docctrl->getLayerController()->mergeCurrentLayerDown();
        }
    }
};

template <>
struct ActionProperties<Action::LAYER_RENAME> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            xoj::popup::PopupWindowWrapper<RenameLayerDialog> dialog(
                    ctrl->getGladeSearchPath(), docctrl->getUndoRedoHandler(), docctrl->getLayerController(),
                    docctrl->getLayerController()->getCurrentPage()->getSelectedLayer());
            dialog.show(ctrl->getGtkWindow());
        }
    }
};

template <>
struct ActionProperties<Action::LAYER_GOTO_NEXT> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            PageRef p = docctrl->getCurrentPage();
            auto layer = p->getSelectedLayerId();
            if (layer < p->getLayerCount()) {
                docctrl->getLayerController()->switchToLay(layer + 1, true);
            }
        }
    }
};
template <>
struct ActionProperties<Action::LAYER_GOTO_PREVIOUS> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            PageRef p = docctrl->getCurrentPage();
            auto layer = p->getSelectedLayerId();
            if (layer > 0) {
                docctrl->getLayerController()->switchToLay(layer - 1, true);
            }
        }
    }
};
template <>
struct ActionProperties<Action::LAYER_GOTO_TOP> {
    static void callback(GSimpleAction*, GVariant*, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            PageRef p = docctrl->getCurrentPage();
            docctrl->getLayerController()->switchToLay(p->getLayerCount(), true);
        }
    }
};
template <>
struct ActionProperties<Action::LAYER_ACTIVE> {
    using state_type = Layer::Index;
    using parameter_type = state_type;
    static state_type initialState(MainControl*) { return 0; }
    static void callback(GSimpleAction*, GVariant* p, MainControl* ctrl) {
        if (auto* docctrl = ctrl->getDocumentControl(); docctrl != nullptr) {
            Layer::Index max = docctrl->getLayerController()->getLayerCount();
            Layer::Index current = docctrl->getLayerController()->getCurrentLayerId();
            Layer::Index n = getGVariantValue<Layer::Index>(p);
            if (n != current && n <= max) {
                docctrl->getLayerController()->switchToLay(n);  // Will set the right state for this action
            }
        }
    }
};

template <>
struct ActionProperties<Action::POSITION_HIGHLIGHTING> {
    using state_type = bool;
    static state_type initialState(MainControl* ctrl) { return ctrl->getSettings()->isHighlightPosition(); }
    static void callback(GSimpleAction* ga, GVariant* p, MainControl* ctrl) {
        g_simple_action_set_state(ga, p);
        bool enable = g_variant_get_boolean(p);
        ctrl->getSettings()->setHighlightPosition(enable);
    }
};
