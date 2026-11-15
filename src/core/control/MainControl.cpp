#include "MainControl.h"

#include <algorithm>   // for max
#include <cstdlib>     // for size_t
#include <exception>   // for exce...
#include <functional>  // for bind
#include <iterator>    // for end
#include <memory>      // for make...
#include <optional>    // for opti...
#include <regex>       // for regex
#include <string>      // for string
#include <utility>     // for move

#include "control/AudioController.h"                             // for Audi...
#include "control/ClipboardHandler.h"                            // for Clip...
#include "control/RecentManager.h"                               // for Rece...
#include "control/Tool.h"                                        // for Tool
#include "control/ToolHandler.h"                                 // for Tool...
#include "control/actions/ActionDatabase.h"                      // for Acti...
#include "control/jobs/Scheduler.h"                              // for JOB_...
#include "control/jobs/XournalScheduler.h"                       // for Xour...
#include "control/pagetype/PageTypeHandler.h"                    // for Page...
#include "control/settings/ButtonConfig.h"                       // for Butt...
#include "control/settings/PageTemplateSettings.h"               // for Page...
#include "control/settings/Settings.h"                           // for Sett...
#include "control/settings/SettingsEnums.h"                      // for Button
#include "control/settings/ViewModes.h"                          // for ViewM..
#include "gui/DocumentTab.h"                                     // for DocumentTab
#include "gui/FloatingToolbox.h"                                 // for Floa...
#include "gui/MainWindow.h"                                      // for Main...
#include "gui/PdfFloatingToolbox.h"                              // for PdfF...
#include "gui/StatusBar.h"                                       // for StatusBar
#include "gui/XournalView.h"                                     // for Xour... TODO Remove
#include "gui/XournalppCursor.h"                                 // for Xour...
#include "gui/dialog/AboutDialog.h"                              // for Abou...
#include "gui/dialog/FormatDialog.h"                             // for Form...
#include "gui/dialog/PageTemplateDialog.h"                       // for Page...
#include "gui/dialog/SelectOpacityDialog.h"                      // for Opac...
#include "gui/dialog/SettingsDialog.h"                           // for Sett...
#include "gui/dialog/ToolbarManageDialog.h"                      // for Tool...
#include "gui/dialog/XojOpenDlg.h"                               // for XojOpenDlg
#include "gui/dialog/toolbarCustomize/ToolbarDragDropHandler.h"  // for Tool...
#include "gui/inputdevices/HandRecognition.h"                    // for Hand...
#include "gui/inputdevices/SetsquareInputHandler.h"              // for Sets...
#include "gui/menus/menubar/Menubar.h"                           // for Menubar
#include "gui/menus/menubar/PageTypeSubmenu.h"                   // for PageTypeSubmenu
#include "gui/sidebar/Sidebar.h"                                 // for Sidebar
#include "gui/toolbarMenubar/ToolMenuHandler.h"                  // for Tool...
#include "gui/toolbarMenubar/model/ToolbarData.h"                // for Tool...
#include "gui/toolbarMenubar/model/ToolbarModel.h"               // for Tool...
#include "model/Document.h"                                      // for Docu...
#include "model/Font.h"                                          // for XojFont
#include "model/LineStyle.h"                                     // for Line...
#include "model/PageType.h"                                      // for Page...
#include "model/StrokeStyle.h"                                   // for StrokeStyle
#include "plugin/PluginController.h"                             // for Plug...
#include "settings/RecolorParameters.h"                          // for RecolorParameters
#include "util/Assert.h"                                         // for xoj_assert
#include "util/Color.h"                                          // for oper...
#include "util/PathUtil.h"                                       // for clea...
#include "util/PlaceholderString.h"                              // for Plac...
#include "util/PopupWindowWrapper.h"                             // for PopupWindowWrapper
#include "util/Stacktrace.h"                                     // for Stac...
#include "util/StringUtils.h"                                    // for char_cast
#include "util/Util.h"                                           // for exec...
#include "util/XojMsgBox.h"                                      // for XojM...
#include "util/glib_casts.h"                                     // for wrap_v
#include "util/i18n.h"                                           // for _, FS

#include "Control.h"  // TODO remove
#include "CrashHandler.h"
#include "DocumentControl.h"
#include "FileOpener.h"
#include "config-dev.h"
#include "config.h"

static auto makeSettings() {
    auto name = Util::getConfigFileWithFallback(SETTINGS_XML_FILE);
    auto saveName = Util::getConfigFile(SETTINGS_XML_FILE);

    return std::make_unique<Settings>(std::move(saveName), std::move(name));
}

MainControl::MainControl(GApplication* gtkApp, GladeSearchpath* gladeSearchPath):
        gtkApp(gtkApp),
        gladeSearchPath(gladeSearchPath),
        settings(makeSettings()),
        toolHandler(std::make_unique<ToolHandler>(this, this->settings.get())),
        cursor(std::make_unique<XournalppCursor>(this)),
        scheduler(std::make_unique<XournalScheduler>()),
        pageTypes(std::make_unique<PageTypeHandler>(gladeSearchPath)),
        pluginController(std::make_unique<PluginController>(this)),
        isBlocking(false) {
    this->settings->load();
    this->loadPaletteFromSettings();
    this->pluginController->registerToolbar();
    this->toolHandler->loadState();
    this->toolHandler->loadButtonSettings();
}

MainControl::~MainControl() { this->scheduler->stop(); }

void MainControl::saveState() {
    this->toolHandler->saveState();

    gint width = 0;
    gint height = 0;
    gtk_window_get_size(getGtkWindow(), &width, &height);

    if (!this->win->isMaximized()) {
        this->settings->setMainWndSize(width, height);
    }
    this->settings->setMainWndMaximized(this->win->isMaximized());

    this->documentControl->getSidebar()->saveSize();
}

void MainControl::initWindow(MainWindow* win, bool disableAudio) {
    this->win = win;
    this->getDocumentControl()->setDocumentTab(win->getDocumentTab());

#ifdef ENABLE_AUDIO
    if (!(disableAudio || this->settings->isAudioDisabled())) {
        try {
            this->audioController = std::make_unique<AudioController>(*this->settings, *this);
        } catch (const std::exception& e) {
            XojMsgBox::showErrorToUser(
                    GTK_WINDOW(win->getWindow()),
                    FC(_F("Initialization of audio devices failed. You can disable audio support in Settings->Audio") %
                       e.what()));
        }
    }
#endif

    this->actionDB = std::make_unique<ActionDatabase>(this);

    selectTool(toolHandler->getToolType());

    XojMsgBox::setDefaultWindow(getGtkWindow());

    updatePageNumbers(0, 0, npos);

    this->getDocumentControl()->getDocumentTab()->getXournal()->layoutPages();

    toolLineStyleChanged();

    this->clipboardHandler = std::make_unique<ClipboardHandler>(this, win->getWindow());
    this->clipboardHandler->setListener(this->getDocumentControl());
}

void MainControl::updatePageNumbers(size_t page, size_t pageCount, size_t pdfPage) {
    xoj_assert(this->win != nullptr);

    this->win->updatePageNumbers(page, pageCount, pdfPage);

    updatePageActions(page, pageCount);
}

void MainControl::showFloatingToolbox(int x, int y) {
    GtkWidget* mainWindow = GTK_WIDGET(this->getGtkWindow());
    GtkWidget* mainBox = this->getWindow()->get("mainBox");

    gint mainBoxX, mainBoxY;
    gtk_widget_translate_coordinates(mainWindow, mainBox, x, y, &mainBoxX, &mainBoxY);

    this->getWindow()->getFloatingToolbox()->show(mainBoxX, mainBoxY);
}

auto MainControl::copy() -> bool {
    if (this->documentControl && this->documentControl->getDocumentTab()->getXournal()->copy()) {
        return true;
    }
    return this->clipboardHandler->copy();
}

auto MainControl::cut() -> bool {
    if (this->documentControl && this->documentControl->getDocumentTab()->getXournal()->cut()) {
        return true;
    }
    return this->clipboardHandler->cut();
}

auto MainControl::paste() -> bool {
    if (this->documentControl && this->documentControl->getDocumentTab()->getXournal()->paste()) {
        return true;
    }
    return this->clipboardHandler->paste();
}

void MainControl::showAlphaSelectionDialog(OpacityFeature feature) {
    int alpha = 0;

    switch (feature) {
        case OPACITY_FILL_PEN:
            alpha = this->toolHandler->getPenFill();
            break;
        case OPACITY_FILL_HIGHLIGHTER:
            alpha = this->toolHandler->getHighlighterFill();
            break;
        case OPACITY_SELECT_PDF_TEXT_MARKER:
            alpha = this->toolHandler->getSelectPDFTextMarkerOpacity();
            break;
        default:
            g_warning("Unhandled OpacityFeature for selectAlpha event: %s", opacityFeatureToString(feature).data());
            Stacktrace::printStacktrace();
            break;
    }
    auto dlg = xoj::popup::PopupWindowWrapper<xoj::popup::SelectOpacityDialog>(
            gladeSearchPath, alpha, feature, [&th = *toolHandler](int alpha, OpacityFeature feature) {
                switch (feature) {
                    case OPACITY_FILL_PEN:
                        th.setPenFill(alpha);
                        break;
                    case OPACITY_FILL_HIGHLIGHTER:
                        th.setHighlighterFill(alpha);
                        break;
                    case OPACITY_SELECT_PDF_TEXT_MARKER:
                        th.setSelectPDFTextMarkerOpacity(alpha);
                        break;
                    default:
                        g_warning("Unhandled OpacityFeature for callback of SelectOpacityDialog: %s",
                                  opacityFeatureToString(feature).data());
                        Stacktrace::printStacktrace();
                        break;
                }
            });
    dlg.show(getGtkWindow());
}

void MainControl::manageToolbars() {
    xoj::popup::PopupWindowWrapper<ToolbarManageDialog> dlg(
            this->gladeSearchPath, this->win->getToolbarModel(), [win = this->win]() {
                if (const auto& tbs = win->getToolbarModel()->getToolbars();
                    std::none_of(tbs.begin(), tbs.end(),
                                 [tb = win->getSelectedToolbar()](const auto& t) { return t.get() == tb; })) {
                    // The active toolbar has been deleted!
                    xoj_assert(!tbs.empty());
                    win->toolbarSelected(tbs.front().get());
                    XojMsgBox::showErrorToUser(
                            GTK_WINDOW(win->getWindow()),
                            _("You deleted the active toolbar. Falling back to the default toolbar."));
                }

                win->updateToolbarMenu();
                auto filepath = Util::getConfigFile(TOOLBAR_CONFIG);
                win->getToolbarModel()->save(filepath);
            });
    dlg.show(GTK_WINDOW(this->win->getWindow()));

    this->win->updateToolbarMenu();
    auto filepath = Util::getConfigFile(TOOLBAR_CONFIG);
    this->win->getToolbarModel()->save(filepath);
}

void MainControl::customizeToolbars() {
    xoj_assert(this->win != nullptr);

    if (this->win->getSelectedToolbar()->isPredefined()) {
        enum { YES = 1, NO };
        XojMsgBox::askQuestion(getGtkWindow(),
                               FC(_F("The Toolbarconfiguration \"{1}\" is predefined, "
                                     "would you create a copy to edit?") %
                                  this->win->getSelectedToolbar()->getName()),
                               std::string(), {{_("Yes"), YES}, {_("No"), NO}}, [ctrl = this](int response) {
                                   if (response == YES) {
                                       auto data = std::make_unique<ToolbarData>(*ctrl->win->getSelectedToolbar());
                                       ToolbarModel* model = ctrl->win->getToolbarModel();
                                       model->initCopyNameId(data.get());
                                       ctrl->win->toolbarSelected(model->add(std::move(data)));
                                       ctrl->win->updateToolbarMenu();

                                       xoj_assert(!ctrl->win->getSelectedToolbar()->isPredefined());
                                       ctrl->customizeToolbars();
                                   }
                               });
    } else {
        if (!this->dragDropHandler) {
            this->dragDropHandler = std::make_unique<ToolbarDragDropHandler>(this);
        }
        this->dragDropHandler->configure();
    }
}

void MainControl::setToolDrawingType(DrawingType type) {
    if (this->toolHandler->getDrawingType() != type) {
        if (this->toolHandler->getDrawingType() == DRAWING_TYPE_SPLINE) {
            documentControl->onToolNoLongerSpline();
        }
        this->toolHandler->setDrawingType(type);
    }
}

void MainControl::setFullscreen(bool enabled) {
    win->setFullscreen(enabled);
    actionDB->setActionState(Action::FULLSCREEN, enabled);
}

void MainControl::setShowSidebar(bool enabled) {
    win->getDocumentTab()->setSidebarVisible(enabled);
    actionDB->setActionState(Action::SHOW_SIDEBAR, enabled);

    if (settings->isSidebarVisible() != enabled &&
        settings->getActiveViewMode() == PresetViewModeIds::VIEW_MODE_DEFAULT) {
        settings->setSidebarVisible(enabled);
        ViewMode viewMode = settings->getViewModes()[PresetViewModeIds::VIEW_MODE_DEFAULT];
        viewMode.showSidebar = enabled;
        settings->setViewMode(PresetViewModeIds::VIEW_MODE_DEFAULT, viewMode);
    }
}

void MainControl::setShowToolbar(bool enabled) {
    win->setToolbarVisible(enabled);
    actionDB->setActionState(Action::SHOW_TOOLBAR, enabled);

    if (settings->isToolbarVisible() != enabled &&
        settings->getActiveViewMode() == PresetViewModeIds::VIEW_MODE_DEFAULT) {
        settings->setToolbarVisible(enabled);
        ViewMode viewMode = settings->getViewModes()[PresetViewModeIds::VIEW_MODE_DEFAULT];
        viewMode.showToolbar = enabled;
        settings->setViewMode(PresetViewModeIds::VIEW_MODE_DEFAULT, viewMode);
    }
}

void MainControl::setShowMenubar(bool enabled) {
    win->setMenubarVisible(enabled);
    actionDB->setActionState(Action::SHOW_MENUBAR, enabled);
}

void MainControl::updatePageActions(size_t currentPage, size_t pageCount) {
    this->actionDB->enableAction(Action::DELETE_PAGE, pageCount > 1);
    this->actionDB->enableAction(Action::MOVE_PAGE_TOWARDS_BEGINNING, currentPage != 0);
    this->actionDB->enableAction(Action::MOVE_PAGE_TOWARDS_END, currentPage < pageCount - 1);

    this->actionDB->enableAction(Action::GOTO_FIRST, currentPage != 0);
    this->actionDB->enableAction(Action::GOTO_PREVIOUS, currentPage != 0);
    this->actionDB->enableAction(Action::GOTO_PREVIOUS_ANNOTATED_PAGE, currentPage != 0);
    this->actionDB->enableAction(Action::GOTO_PAGE, pageCount > 1);
    this->actionDB->enableAction(Action::GOTO_NEXT, currentPage < pageCount - 1);
    this->actionDB->enableAction(Action::GOTO_LAST, currentPage < pageCount - 1);
    this->actionDB->enableAction(Action::GOTO_NEXT_ANNOTATED_PAGE, currentPage < pageCount - 1);
}

void MainControl::updateBackgroundActions(PageType pt) {
    this->actionDB->enableAction(Action::PAPER_BACKGROUND_COLOR, !pt.isSpecial());
    // PDF page size is defined, you cannot change it
    this->actionDB->enableAction(Action::PAPER_FORMAT, !pt.isPdfPage());
    this->getWindow()->getMenubar()->getPageTypeSubmenu().setSelectedPT(std::move(pt));
}

void MainControl::showPaperTemplateDialog() {
    auto popup = xoj::popup::PopupWindowWrapper<xoj::popup::PageTemplateDialog>(
            this->gladeSearchPath, settings.get(), win->getToolMenuHandler(), pageTypes.get());
    popup.show(GTK_WINDOW(this->win->getWindow()));
}

void MainControl::setViewFullscreenMode(bool enabled) {
    if (enabled) {
        this->loadViewMode(VIEW_MODE_FULLSCREEN);
    } else {
        this->loadViewMode(VIEW_MODE_DEFAULT);
    }
}

void MainControl::setViewPresentationMode(bool enabled) {
    settings->setPresentationMode(enabled);
    if (enabled) {
        this->loadViewMode(VIEW_MODE_PRESENTATION);
    } else {
        this->loadViewMode(VIEW_MODE_DEFAULT);
    }
    this->documentControl->switchPresentationMode(enabled);

    // Disable Zoom
    this->actionDB->enableAction(Action::ZOOM_IN, !enabled);
    this->actionDB->enableAction(Action::ZOOM_OUT, !enabled);
    this->actionDB->enableAction(Action::ZOOM_FIT, !enabled);
    this->actionDB->enableAction(Action::ZOOM_100, !enabled);
    this->actionDB->enableAction(Action::ZOOM, !enabled);

    this->actionDB->enableAction(Action::SET_LAYOUT_BOTTOM_TO_TOP, !enabled);
    this->actionDB->enableAction(Action::SET_LAYOUT_RIGHT_TO_LEFT, !enabled);
    this->actionDB->enableAction(Action::SET_LAYOUT_VERTICAL, !enabled);
    this->actionDB->enableAction(Action::SET_COLUMNS_OR_ROWS, !enabled);
}

void MainControl::updateUndoRedoActions(bool canUndo, bool canRedo, const std::string& undoDescription,
                                        const std::string& redoDescription) {

    this->actionDB->enableAction(Action::UNDO, canUndo);
    this->actionDB->enableAction(Action::REDO, canRedo);

    win->setUndoDescription(undoDescription);
    win->setRedoDescription(redoDescription);
}

void MainControl::updateNavigationHistoryActions(bool canForward, bool canBackward) {
    actionDB->enableAction(Action::NAVIGATE_BACK, canBackward);
    actionDB->enableAction(Action::NAVIGATE_FORWARD, canForward);
}

void MainControl::updateLayerActions(size_t currentLayer, size_t maxLayer) {
    actionDB->enableAction(Action::LAYER_DELETE, currentLayer > 0);
    actionDB->enableAction(Action::LAYER_MERGE_DOWN, currentLayer > 1);
    actionDB->enableAction(Action::LAYER_MOVE_UP, currentLayer < maxLayer);
    actionDB->enableAction(Action::LAYER_MOVE_DOWN, currentLayer > 1);
    actionDB->enableAction(Action::MOVE_SELECTION_LAYER_UP, currentLayer < maxLayer);
    actionDB->enableAction(Action::MOVE_SELECTION_LAYER_DOWN, currentLayer > 1);
    actionDB->enableAction(Action::LAYER_GOTO_NEXT, currentLayer < maxLayer);
    actionDB->enableAction(Action::LAYER_GOTO_PREVIOUS, currentLayer > 0);
    actionDB->enableAction(Action::LAYER_GOTO_TOP, currentLayer < maxLayer);

    actionDB->setActionState(Action::LAYER_ACTIVE, currentLayer);
}

void MainControl::selectTool(ToolType type) {
    auto oldTool = getToolHandler()->getActiveTool();
    if (oldTool && oldTool->getToolType() == type) {
        return;
    }

    auto* docCtrl = this->getDocumentControl();
    if (docCtrl && oldTool) {
        if (oldTool->getToolType() == ToolType::TOOL_TEXT) {
            if (isSelectToolType(type)) {
                // keep text-selection when switching from text to seletion tool
                docCtrl->makeSelectionOfActiveText();
            } else {
                docCtrl->onToolNoLongerText();
            }
        } else if (oldTool->getToolType() == ToolType::TOOL_LINK) {
            docCtrl->onToolNoLongerLink();
        } else if (oldTool->getToolType() == ToolType::TOOL_DRAW_SPLINE) {
            docCtrl->onToolNoLongerSpline();
        }
    }

    toolHandler->selectTool(type);
    toolHandler->fireToolChanged();
}

void MainControl::selectDefaultTool() {
    ButtonConfig* cfg = settings->getButtonConfig(Button::BUTTON_DEFAULT);
    cfg->applyConfigToToolbarTool(toolHandler.get());

    if (toolHandler->getToolType() != TOOL_NONE) {
        selectTool(toolHandler->getToolType());
    }
}

void MainControl::toolChanged() {
    ToolType type = toolHandler->getToolType();

    this->actionDB->setActionState(Action::SELECT_TOOL, type);

    this->actionDB->enableAction(Action::TOOL_DRAW_LINE, toolHandler->hasCapability(TOOL_CAP_RULER));
    this->actionDB->enableAction(Action::TOOL_DRAW_RECTANGLE, toolHandler->hasCapability(TOOL_CAP_RECTANGLE));
    this->actionDB->enableAction(Action::TOOL_DRAW_ELLIPSE, toolHandler->hasCapability(TOOL_CAP_ELLIPSE));
    this->actionDB->enableAction(Action::TOOL_DRAW_ARROW, toolHandler->hasCapability(TOOL_CAP_ARROW));
    this->actionDB->enableAction(Action::TOOL_DRAW_DOUBLE_ARROW, toolHandler->hasCapability(TOOL_CAP_DOUBLE_ARROW));
    this->actionDB->enableAction(Action::TOOL_DRAW_COORDINATE_SYSTEM, toolHandler->hasCapability(TOOL_CAP_ARROW));
    this->actionDB->enableAction(Action::TOOL_DRAW_SPLINE, toolHandler->hasCapability(TOOL_CAP_SPLINE));
    this->actionDB->enableAction(Action::TOOL_DRAW_SHAPE_RECOGNIZER, toolHandler->hasCapability(TOOL_CAP_RECOGNIZER));

    DrawingType dt = toolHandler->getDrawingType();
    this->actionDB->setActionState(Action::TOOL_DRAW_LINE, dt == DRAWING_TYPE_LINE);
    this->actionDB->setActionState(Action::TOOL_DRAW_RECTANGLE, dt == DRAWING_TYPE_RECTANGLE);
    this->actionDB->setActionState(Action::TOOL_DRAW_ELLIPSE, dt == DRAWING_TYPE_ELLIPSE);
    this->actionDB->setActionState(Action::TOOL_DRAW_ARROW, dt == DRAWING_TYPE_ARROW);
    this->actionDB->setActionState(Action::TOOL_DRAW_DOUBLE_ARROW, dt == DRAWING_TYPE_DOUBLE_ARROW);
    this->actionDB->setActionState(Action::TOOL_DRAW_COORDINATE_SYSTEM, dt == DRAWING_TYPE_COORDINATE_SYSTEM);
    this->actionDB->setActionState(Action::TOOL_DRAW_SPLINE, dt == DRAWING_TYPE_SPLINE);
    this->actionDB->setActionState(Action::TOOL_DRAW_SHAPE_RECOGNIZER, dt == DRAWING_TYPE_SHAPE_RECOGNIZER);

    bool enableSize = toolHandler->hasCapability(TOOL_CAP_SIZE);
    this->actionDB->enableAction(Action::TOOL_SIZE, enableSize);
    if (enableSize) {
        toolSizeChanged();
    }

    // Set or reset the lineStyle
    toolLineStyleChanged();

    bool enableFill = toolHandler->hasCapability(TOOL_CAP_FILL);
    this->actionDB->enableAction(Action::TOOL_FILL, enableFill);
    this->actionDB->enableAction(Action::TOOL_FILL_OPACITY, enableFill);
    if (enableFill) {
        toolFillChanged();
    }

    bool enableColor = toolHandler->hasCapability(TOOL_CAP_COLOR);
    this->actionDB->enableAction(Action::TOOL_COLOR, enableColor);
    this->actionDB->enableAction(Action::SELECT_COLOR, enableColor);
    if (enableColor) {
        toolColorChanged();
    }

    bool enableTextAlign = toolHandler->hasCapability(TOOL_CAP_ALIGN);
    this->actionDB->enableAction(Action::TEXT_ALIGNMENT, enableTextAlign);
    if (enableTextAlign) {
        this->actionDB->setActionState(Action::TEXT_ALIGNMENT, toolHandler->getActiveTool()->getTextAlignment());
    }
    bool enableTextJustify = toolHandler->hasCapability(TOOL_CAP_JUSTIFY);
    this->actionDB->enableAction(Action::TEXT_JUSTIFY, enableTextJustify);
    if (enableTextAlign) {
        this->actionDB->setActionState(Action::TEXT_JUSTIFY, toolHandler->getActiveTool()->getTextJustify());
    }

    getCursor()->updateCursor();
}

void MainControl::eraserSizeChanged() {
    this->actionDB->setActionState(Action::TOOL_ERASER_SIZE, toolHandler->getEraserSize());
}

void MainControl::penSizeChanged() { this->actionDB->setActionState(Action::TOOL_PEN_SIZE, toolHandler->getPenSize()); }

void MainControl::highlighterSizeChanged() {
    this->actionDB->setActionState(Action::TOOL_HIGHLIGHTER_SIZE, toolHandler->getHighlighterSize());
}

void MainControl::toolSizeChanged() {
    if (toolHandler->getToolType() == TOOL_PEN) {
        penSizeChanged();
    } else if (toolHandler->getToolType() == TOOL_ERASER) {
        eraserSizeChanged();
    } else if (toolHandler->getToolType() == TOOL_HIGHLIGHTER) {
        highlighterSizeChanged();
    }

    this->actionDB->setActionState(Action::TOOL_SIZE, toolHandler->getSize());

    getCursor()->updateCursor();
}

void MainControl::toolFillChanged() {
    this->actionDB->setActionState(Action::TOOL_FILL, toolHandler->getFill() != -1);
    this->actionDB->setActionState(Action::TOOL_PEN_FILL, toolHandler->getPenFillEnabled());
    this->actionDB->setActionState(Action::TOOL_HIGHLIGHTER_FILL, toolHandler->getHighlighterFillEnabled());
}

void MainControl::toolLineStyleChanged() {
    std::optional<LineStyle> style = this->documentControl->getLineStyleOfSelection();
    if (!style) {
        if (auto* tool = toolHandler->getActiveTool(); tool->getToolType() == TOOL_PEN) {
            style = tool->getLineStyle();
        }
    }
    std::string value = style ? StrokeStyle::formatStyle(style.value()) : "none";
    this->actionDB->setActionState(Action::TOOL_PEN_LINE_STYLE, value.c_str());
}

void MainControl::eraserTypeChanged() {
    this->actionDB->setActionState(Action::TOOL_ERASER_TYPE, this->toolHandler->getEraserType());
}

void MainControl::toolColorChanged() {
    this->actionDB->setActionState(Action::TOOL_COLOR, getToolHandler()->getColorMaskAlpha());
    getCursor()->updateCursor();
}

void MainControl::showSettings() {
    // take note of some settings before to compare with after
    struct {
        Color selectionColor;
        bool verticalSpace;
        int verticalSpaceAmountAbove;
        int verticalSpaceAmountBelow;
        bool horizontalSpace;
        int horizontalSpaceAmountRight;
        int horizontalSpaceAmountLeft;
        bool unlimitedScrolling;
        StylusCursorType stylusCursorType;
        bool highlightPosition;
        SidebarNumberingStyle sidebarStyle;
        std::optional<std::filesystem::path> colorPaletteSetting;
        RecolorParameters recolorParameters;
    } settingsBeforeDialog = {
            settings->getBorderColor(),
            settings->getAddVerticalSpace(),
            settings->getAddVerticalSpaceAmountAbove(),
            settings->getAddVerticalSpaceAmountBelow(),
            settings->getAddHorizontalSpace(),
            settings->getAddHorizontalSpaceAmountRight(),
            settings->getAddHorizontalSpaceAmountLeft(),
            settings->getUnlimitedScrolling(),
            settings->getStylusCursorType(),
            settings->isHighlightPosition(),
            settings->getSidebarNumberingStyle(),
            settings->getColorPaletteSetting(),
            settings->getRecolorParameters(),
    };

    auto dlg = xoj::popup::PopupWindowWrapper<SettingsDialog>(
            this->gladeSearchPath, settings.get(), this,
            std::vector<fs::path>{Util::getBuiltInPaletteDirectoryPath(), Util::getCustomPaletteDirectoryPath()},
            [ctrl = this, settingsBeforeDialog]() {
                Settings* settings = ctrl->getSettings();

                DocumentControl::ChangedSettings changes;

                changes.borderColorChanged = settingsBeforeDialog.selectionColor != settings->getBorderColor();
                changes.forceSidebarLayout = settingsBeforeDialog.sidebarStyle != settings->getSidebarNumberingStyle();

                if (!settings->getUnlimitedScrolling() &&
                    (settingsBeforeDialog.verticalSpace != settings->getAddVerticalSpace() ||
                     settingsBeforeDialog.horizontalSpace != settings->getAddHorizontalSpace() ||
                     settingsBeforeDialog.verticalSpaceAmountAbove != settings->getAddVerticalSpaceAmountAbove() ||
                     settingsBeforeDialog.horizontalSpaceAmountRight != settings->getAddHorizontalSpaceAmountRight() ||
                     settingsBeforeDialog.verticalSpaceAmountBelow != settings->getAddVerticalSpaceAmountBelow() ||
                     settingsBeforeDialog.horizontalSpaceAmountLeft != settings->getAddHorizontalSpaceAmountLeft())) {
                    changes.needLayout = true;
                    changes.scrollOffset = xoj::util::Point<double>(
                            (settings->getAddHorizontalSpace() ? settings->getAddHorizontalSpaceAmountLeft() : 0) -
                                    (settingsBeforeDialog.horizontalSpace ?
                                             settingsBeforeDialog.horizontalSpaceAmountLeft :
                                             0),
                            (settings->getAddVerticalSpace() ? settings->getAddVerticalSpaceAmountAbove() : 0) -
                                    (settingsBeforeDialog.verticalSpace ?
                                             settingsBeforeDialog.verticalSpaceAmountAbove :
                                             0));
                }

                if (settingsBeforeDialog.unlimitedScrolling != settings->getUnlimitedScrolling()) {
                    changes.needLayout = true;
                    changes.unlimitedScrollingChanged = true;
                    changes.scrollOffset +=
                            xoj::util::Point<double>(settingsBeforeDialog.unlimitedScrolling ?
                                                             (settingsBeforeDialog.horizontalSpace ?
                                                                      settingsBeforeDialog.horizontalSpaceAmountLeft :
                                                                      0) :
                                                             -(settingsBeforeDialog.horizontalSpace ?
                                                                       settingsBeforeDialog.horizontalSpaceAmountLeft :
                                                                       0),
                                                     settingsBeforeDialog.unlimitedScrolling ?
                                                             (settingsBeforeDialog.verticalSpace ?
                                                                      settingsBeforeDialog.verticalSpaceAmountAbove :
                                                                      0) :
                                                             -(settingsBeforeDialog.verticalSpace ?
                                                                       settingsBeforeDialog.verticalSpaceAmountAbove :
                                                                       0));
                }

                if (settingsBeforeDialog.stylusCursorType != settings->getStylusCursorType() ||
                    settingsBeforeDialog.highlightPosition != settings->isHighlightPosition()) {
                    ctrl->getCursor()->updateCursor();
                }

                bool reloadToolbars = false;
                if (settingsBeforeDialog.colorPaletteSetting.has_value() &&
                    settingsBeforeDialog.colorPaletteSetting.value() != settings->getColorPaletteSetting()) {
                    ctrl->loadPaletteFromSettings();
                    ctrl->getWindow()->getToolMenuHandler()->updateColorToolItems(ctrl->getPalette());
                    reloadToolbars = true;
                }

                if (settingsBeforeDialog.recolorParameters != settings->getRecolorParameters()) {
                    ctrl->getWindow()->getToolMenuHandler()->updateColorToolItemsRecoloring(
                            settings->getRecolorParameters().recolorizeMainView ?
                                    std::make_optional(settings->getRecolorParameters().recolor) :
                                    std::nullopt);
                    reloadToolbars = true;
                }

                if (reloadToolbars) {
                    ctrl->getWindow()->reloadToolbars();
                }

                ctrl->getWindow()->setDPI();

                ctrl->getDocumentControl()->updateSettingsChanged(changes);

                ctrl->getWindow()->updateColorscheme();

                ctrl->getActionDatabase()->setActionState(Action::TOGGLE_TOUCH_DRAWING,
                                                          settings->getTouchDrawingEnabled());
            });
    dlg.show(this->getGtkWindow());
}

void MainControl::newFile(fs::path filepath) {
    this->getDocumentControl()->close(
            [ctrl = this, filepath = std::move(filepath)](bool closed) {
                if (closed) {
                    ctrl->getDocumentControl()->replaceDocument(
                            FileOpener::createNewDocument(ctrl->getSettings(), std::move(filepath), std::nullopt), -1);
                }
            },
            true);
}

/**
 * Check if this is an autosave file, return false in this case and display a user instruction
 */
static auto shouldFileOpen(fs::path const& filepath, GtkWindow* win) -> bool {
    auto basePath = Util::getCacheSubfolder("");
    auto isChild = Util::isChildOrEquivalent(filepath, basePath);
    if (isChild) {
        std::string msg = FS(_F("Do not open Autosave files. They may will be overwritten!\n"
                                "Copy the files to another folder.\n"
                                "Files from Folder {1} cannot be opened.") %
                             basePath.u8string());
        XojMsgBox::showErrorToUser(win, msg);
    }
    return !isChild;
}

void MainControl::askToOpenFile() {
    /**
     * Question: in case the current file has not been saved yet, do we want:
     *      1. First ask to save it, save it or discard it and then show the FileChooserDialog to open a new file
     *      2. First show the FileChooserDialog, and if a valid file has been selected and successfully opened, ask to
     *         save or discard the current file
     * For now, this implements option 1.
     */
    this->getDocumentControl()->close([ctrl = this](bool closed) {
        if (closed) {
            xoj::OpenDlg::showOpenFileDialog(ctrl->getGtkWindow(), ctrl->settings.get(), [ctrl](fs::path path) {
                g_message("%s", (_F("file: {1}") % path.string()).c_str());
                ctrl->openFileWithoutSavingTheCurrentDocument(std::move(path), false, -1);
            });
        }
    });
}

void MainControl::finishedOpeningFileCallback(std::unique_ptr<Document> doc, int scrollToPage,
                                              UndoActionPtr recoveryAction) {
    xoj_assert(this->fileOpener);
    this->fileOpener.reset();
    if (!doc) {
        // Something went wrong. Errors have already been show to the user
        return;
    }
    this->getDocumentControl()->replaceDocument(std::move(doc), scrollToPage);
    if (recoveryAction) {
        this->getDocumentControl()->getUndoRedoHandler()->addUndoAction(std::move(recoveryAction));
    }
}

void MainControl::openFile(fs::path filepath, int scrollToPage, bool forceOpen) {
    if (filepath.empty() || (!forceOpen && !shouldFileOpen(filepath, getGtkWindow()))) {
        return;
    }

    this->getDocumentControl()->close(
            [ctrl = this, filepath = std::move(filepath), scrollToPage](bool closed) mutable {
                if (closed) {
                    ctrl->fileOpener = std::make_unique<FileOpener>(ctrl);
                    ctrl->fileOpener->openFile(std::move(filepath), false, scrollToPage);
                }
            },
            false, true, forceOpen);
}

void MainControl::askToAnnotatePdf() {
    this->getDocumentControl()->close([ctrl = this](bool closed) {
        if (closed) {
            xoj::OpenDlg::showAnnotatePdfDialog(ctrl->getGtkWindow(), ctrl->settings.get(),
                                                [ctrl](fs::path path, bool attachPdf) {
                                                    if (!path.empty()) {
                                                        ctrl->fileOpener = std::make_unique<FileOpener>(ctrl);
                                                        ctrl->fileOpener->openPdfFile(std::move(path), attachPdf, -1);
                                                    }
                                                });
        }
    });
}

void MainControl::openFileWithoutSavingTheCurrentDocument(fs::path filepath, bool attachToDocument, int scrollToPage) {
    this->fileOpener = std::make_unique<FileOpener>(this);
    this->fileOpener->openFile(std::move(filepath), attachToDocument, scrollToPage);
}

void MainControl::openEmergencyFile(fs::path filepath) {
    this->fileOpener = std::make_unique<FileOpener>(this);
    this->fileOpener->openXoppFile(std::move(filepath), -1, true);
}

void MainControl::blockUI(const std::string& name) {
    if (this->isBlocking) {
        return;
    }

    // Disable all gui Control, to get full control over the application
    win->setDynamicallyGeneratedSubmenuDisabled(true);
    actionDB->disableAll();
    getCursor()->setCursorBusy(true);
    getDocumentControl()->getSidebar()->setTmpDisabled(true);

    auto* bar = win->getStatusBar();
    bar->setStateLabel(name);
    bar->setCurrentState(0);
    bar->setMaximumState(100);
    bar->setVisible(true);

    this->isBlocking = true;
}

void MainControl::unblockUI() {
    if (!this->isBlocking) {
        return;
    }

    this->win->setDynamicallyGeneratedSubmenuDisabled(false);
    actionDB->resetEnableStatus();
    getCursor()->setCursorBusy(false);
    getDocumentControl()->getSidebar()->setTmpDisabled(false);

    this->win->getStatusBar()->setVisible(false);

    this->isBlocking = false;
}

auto MainControl::getProgressListener() const -> ProgressListener* { return win->getStatusBar(); }

void MainControl::showFontDialog() {
    this->actionDB->enableAction(Action::SELECT_FONT, false);  // Only one dialog
    auto* dlg = gtk_font_chooser_dialog_new(_("Select font"), GTK_WINDOW(this->win->getWindow()));
    gtk_font_chooser_set_font(GTK_FONT_CHOOSER(dlg), settings->getFont().asString().c_str());

    auto popup = xoj::popup::PopupWindowWrapper<XojMsgBox>(
            GTK_DIALOG(dlg),
            [this, dlg](int response) {
                if (response == GTK_RESPONSE_OK) {
                    auto font =
                            xoj::util::OwnedCString::assumeOwnership(gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dlg)));
                    this->actionDB->fireChangeActionState(Action::FONT, font.get());
                }
                this->actionDB->enableAction(Action::SELECT_FONT, true);
            },
            XojMsgBox::IMMEDIATE);  // We need IMMEDIATE so accessing GTK_FONT_CHOOSER(dlg) is not UB
    popup.show(GTK_WINDOW(this->win->getWindow()));
}

void MainControl::showColorChooserDialog() {
    this->actionDB->enableAction(Action::SELECT_COLOR, false);  // Only one dialog
    auto* dlg = gtk_color_chooser_dialog_new(_("Select color"), GTK_WINDOW(this->win->getWindow()));
    GdkRGBA c = Util::argb_to_GdkRGBA(toolHandler->getColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dlg), &c);
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(dlg), false);

    auto popup = xoj::popup::PopupWindowWrapper<XojMsgBox>(
            GTK_DIALOG(dlg),
            [this, dlg](int response) {
                if (response == GTK_RESPONSE_OK) {
                    GdkRGBA c;
                    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dlg), &c);
                    this->actionDB->fireChangeActionState(Action::TOOL_COLOR, Util::GdkRGBA_to_rgb(c));
                }
                this->actionDB->enableAction(Action::SELECT_COLOR, true);
            },
            XojMsgBox::IMMEDIATE);  // We need IMMEDIATE so accessing GTK_COLOR_CHOOSER(dlg) is not UB
    popup.show(GTK_WINDOW(this->win->getWindow()));
}

void MainControl::updateWindowTitle(const std::u8string& title) {
    gtk_window_set_title(getGtkWindow(), char_cast(title.c_str()));
}

template <typename JobType, auto priority, typename... Args>
void MainControl::schedule(Args&&... args) {
    auto* job = new JobType(dynamic_cast<Control*>(this), std::forward<Args>(args)...);
    this->scheduler->addJob(job, priority);
    job->unref();
}

void MainControl::removeJobsFor(const DocumentControl* docCtrl) {
    // TODO Stop the jobs for this specific document
    this->scheduler->removeAllJobs();
    this->scheduler->finishTask();
}

void MainControl::quit(bool allowCancel) {
    g_message("Quitting");
    auto afterClosed = [this, allowCancel](bool closed) {
        if (!closed) {
            g_message("Closing document failed or cancelled");
            if (!allowCancel) {
                // Cancel is not allowed, and the user close or did not save
                // This is probably called from macOS, where the Application
                // now will be killed - therefore do an emergency save.
                emergencySave();
            }
        } else {
#ifdef ENABLE_AUDIO
            if (audioController) {
                audioController->stopRecording();
            }
#endif
            this->getDocumentControl()->closeDocument();
            this->scheduler->stop();
            settings->save();
            g_application_quit(G_APPLICATION(gtkApp));
        }
    };

    this->getDocumentControl()->close(std::move(afterClosed), true, allowCancel);
}

void MainControl::showAbout() {
    auto popup = xoj::popup::PopupWindowWrapper<xoj::popup::AboutDialog>(this->gladeSearchPath);
    popup.show(GTK_WINDOW(this->win->getWindow()));
}

static void onGtkDemoShown(GObject* proc_object, GAsyncResult* res, gpointer) {
    gboolean success = g_subprocess_wait_finish(G_SUBPROCESS(proc_object), res, NULL);

    if (success) {
        g_message("Gtk demo run successfully!\n");
    } else {
        g_message("Something went wrong running the Gtk demo!\n");
    }
}

void MainControl::showGtkDemo() {
    std::string binary = "gtk3-demo";
#ifdef __APPLE__
    if (!xoj::util::OwnedCString::assumeOwnership(g_find_program_in_path(binary.c_str()))) {
        // Try absolute path for binary
        auto path = Util::getExePath() / binary;

        binary = Util::toGFilename(path).c_str();
    }
#endif
    gchar* prog = g_find_program_in_path(binary.c_str());
    if (!prog) {
        XojMsgBox::showErrorToUser(getGtkWindow(), "gtk3-demo was not found in path");
        return;
    }
    GError* err = nullptr;
    GSubprocess* process = g_subprocess_new(G_SUBPROCESS_FLAGS_NONE, &err, prog, nullptr);
    g_free(prog);

    if (err != nullptr) {
        std::string message =
                FS(_F("Creating Gtk demo subprocess failed: {1} (exit code: {2})") % err->message % err->code);
        XojMsgBox::showErrorToUser(getGtkWindow(), message);
        g_error_free(err);
    }

    g_subprocess_wait_async(process, nullptr, reinterpret_cast<GAsyncReadyCallback>(onGtkDemoShown), nullptr);
}

auto MainControl::loadViewMode(ViewModeId mode) -> bool {
    if (!settings->loadViewMode(mode)) {
        return false;
    }
    this->win->setMenubarVisible(settings->isMenubarVisible());
    this->win->setToolbarVisible(settings->isToolbarVisible());
    this->win->getDocumentTab()->setSidebarVisible(settings->isSidebarVisible());
    setFullscreen(settings->isFullscreen());
    return false;
}

void MainControl::clipboardCutCopyEnabled(bool enabled) {
    this->actionDB->enableAction(Action::CUT, enabled);
    this->actionDB->enableAction(Action::COPY, enabled);
}

void MainControl::clipboardPasteEnabled(bool enabled) { this->actionDB->enableAction(Action::PASTE, enabled); }

void MainControl::registerPluginToolButtons(ToolMenuHandler* toolMenuHandler) {
    pluginController->registerToolButtons(toolMenuHandler);
}

void MainControl::setCopyCutEnabled(bool enabled) { this->clipboardHandler->setCopyCutEnabled(enabled); }

void MainControl::setFill(bool fill) {
    documentControl->setFillToSelection(fill ? toolHandler->getPenFill() : -1,
                                        fill ? toolHandler->getHighlighterFill() : -1);
    toolHandler->setFillEnabled(fill);
}

void MainControl::setLineStyle(const std::string& style) {
    LineStyle stl = StrokeStyle::parseStyle(style);
    if (!documentControl->setLineStyleOfSelection(stl) &&
        this->toolHandler->getActiveTool()->getToolType() != TOOL_PEN) {
        this->selectTool(TOOL_PEN);
    }
    this->toolHandler->setLineStyle(stl);
}

void MainControl::setEraserType(EraserType type) {
    if (type != ERASER_TYPE_NONE) {
        if (this->toolHandler->getActiveTool()->getToolType() != TOOL_ERASER) {
            this->selectTool(TOOL_ERASER);
        }
    }
    this->toolHandler->setEraserType(type);
}

void MainControl::setToolSize(ToolSize size) {
    this->documentControl->setThicknessToSelection(toolHandler->getToolThickness(TOOL_PEN)[size],
                                                   toolHandler->getToolThickness(TOOL_HIGHLIGHTER)[size],
                                                   toolHandler->getToolThickness(TOOL_ERASER)[size]);
    this->toolHandler->setSize(size);
}

void MainControl::setFont(const XojFont& font) {
    settings->setFont(font);
    this->activeDocDoes<&DocumentControl::setFontToTextAndSelection>(font);
}

void MainControl::setColor(Color color, bool applyToSelection) {
    if (toolHandler->hasCapability(TOOL_CAP_COLOR)) {
        color.alpha = toolHandler->getActiveTool()->getColor().alpha;
        toolHandler->setColor(color);

        this->activeDocDoes<&DocumentControl::setColorOfSelection>(toolHandler->getColor());
    }
}

auto MainControl::getCursor() const -> XournalppCursor* { return this->cursor.get(); }
auto MainControl::getToolHandler() const -> ToolHandler* { return this->toolHandler.get(); }
auto MainControl::getScheduler() const -> XournalScheduler* { return this->scheduler.get(); }
auto MainControl::getWindow() const -> MainWindow* { return this->win; }
auto MainControl::getGtkWindow() const -> GtkWindow* { return GTK_WINDOW(this->win->getWindow()); }
auto MainControl::getDocumentControl() const -> DocumentControl* { return documentControl; }

void MainControl::setRotationSnapping(bool enable) {
    settings->setSnapRotation(enable);
    this->actionDB->setActionState(Action::ROTATION_SNAPPING, enable);
}

void MainControl::setGridSnapping(bool enable) {
    settings->setSnapGrid(enable);
    this->actionDB->setActionState(Action::GRID_SNAPPING, enable);
}

auto MainControl::getGladeSearchPath() const -> GladeSearchpath* { return this->gladeSearchPath; }

auto MainControl::getSettings() const -> Settings* { return settings.get(); }

auto MainControl::getAudioController() const -> AudioController* { return this->audioController.get(); }

auto MainControl::getPageTypes() const -> PageTypeHandler* { return this->pageTypes.get(); }

auto MainControl::getPluginController() const -> PluginController* { return this->pluginController.get(); }

auto MainControl::getPalette() const -> const Palette& { return *(this->palette); }

auto MainControl::getClipboardHandler() const -> ClipboardHandler& { return *clipboardHandler; }

void MainControl::loadPaletteFromSettings() {
    const auto palettePath = this->settings->getColorPaletteSetting();
    if (palettePath.empty()) {
        this->palette->load_default();
        return;
    }

    auto newPalette = std::make_unique<Palette>(palettePath);
    this->palette = std::move(newPalette);

    // If file does not exist there is no need to attempt parsing it
    if (!fs::exists(this->palette->getFilePath())) {
        this->palette->load_default();
        return;
    }

    try {
        this->palette->load();
    } catch (const std::exception& e) {
        this->palette->parseErrorDialog(e);
        this->palette->load_default();
    }
}
