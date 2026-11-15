/*
 * Xournal++
 *
 * The main Control
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>   // for size_t
#include <memory>    // for unique_ptr
#include <optional>  // for optional
#include <string>    // for string, allocator
#include <vector>    // for vector

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gio/gio.h>                // for GApplication
#include <glib.h>                   // for guint
#include <gtk/gtk.h>                // for GtkLabel

#include "control/ToolEnums.h"                      // for ToolSize, ToolType
#include "control/settings/ViewModes.h"             // for ViewModeId
#include "control/tools/EditSelection.h"            // for OrderChange
#include "enums/Action.enum.h"                      // for Action
#include "gui/toolbarMenubar/model/ColorPalette.h"  // for ColorPalette
#include "model/DocumentHandler.h"                  // for DocumentHandler
#include "model/DocumentListener.h"                 // for DocumentListener
#include "model/GeometryTool.h"                     // for GeometryTool
#include "model/PageRef.h"                          // for PageRef
#include "undo/UndoRedoHandler.h"                   // for UndoRedoHandler (ptr only)

#include "ClipboardHandler.h"  // for ClipboardListener
#include "ToolHandler.h"       // for ToolListener
#include "filesystem.h"        // for path

class LoadHandler;
class GeometryToolController;
class AudioController;
class FullscreenHandler;
class Sidebar;
class GladeSearchpath;
class MetadataManager;
class XournalppCursor;
class ToolbarDragDropHandler;
class PageBackgroundChangeController;
class PageTemplateSettings;
class PageTypeHandler;
class BaseExportJob;
class LayerController;
class PluginController;
class Document;
class EditSelection;
class Element;
class MainWindow;
class ObjectInputStream;
class ScrollHandler;
class SearchBar;
class Settings;
class TextEditor;
class XournalScheduler;
class ZoomControl;
class ToolMenuHandler;
class XojFont;
class XojPdfRectangle;
class ActionDatabase;
class NavigationHistory;
class ProgressListener;
class DocumentTab;
class PageType;
class MainControl;
class FileOpener;

class DocumentControl: public DocumentHandler, public UndoRedoListener, public ClipboardListener {
public:
    /**
     * @param recovery: undo action added to the undo/redo stack upon creation
     */
    DocumentControl(MainControl* mainCtrl);  //, std::unique_ptr<Document> doc, UndoActionPtr recovery);
    ~DocumentControl() override;

    // Getters
    ZoomControl* getZoomControl() const;
    Document* getDocument() const;
    UndoRedoHandler* getUndoRedoHandler() const;
    ScrollHandler* getScrollHandler() const;
    PageRef getCurrentPage();
    size_t getCurrentPageNo() const;
    Sidebar* getSidebar() const;
    SearchBar* getSearchBar() const;
    PageBackgroundChangeController* getPageBackgroundChangeController() const;
    LayerController* getLayerController() const;
    NavigationHistory* getNavigationHistory() const;
    TextEditor* getTextEditor();
    MainControl* getMainControl() const;
    Settings* getSettings() const;
    DocumentTab* getDocumentTab() const;

    void setDocumentTab(DocumentTab* tab);


    void addChangedDocumentListener(DocumentListener* dl);
    void removeChangedDocumentListener(DocumentListener* dl);

    /**
     * Fire page selected, but first check if the page Number is valid
     *
     * @return the page ID or size_t_npos if the page is not found
     */
    size_t firePageSelected(const PageRef& page);
    void firePageSelected(size_t page);

    /**
     * @brief Search text on the given page. The matches (if any) are stored in the XojPageView::SearchControl instance.
     * @param occurrences If not nullptr, the pointed variable will contain the number of matches on the page
     * @param matchRect If not nullptr, will contain the topleft point of the first match on the page
     *                          (Used for scrolling to the first match)
     * @return true if at least one match was found
     */
    bool searchTextOnPage(const std::string& text, size_t pageNumber, size_t index, size_t* occurrences,
                          XojPdfRectangle* matchRect);

    /**
     * @brief Get the pen line style to select in the toolbar
     *
     * @return style to select, empty if no style should be selected (active
     * selection with differing line styles)
     */
    auto getLineStyleOfSelection() -> std::optional<LineStyle> const;
    void setColorOfSelection(Color color);  ///< Change the color of the currently selected  elements
    bool setLineStyleOfSelection(const LineStyle& style);  ///< returns true if there was a selection
    void setFillToSelection(int penfill, int highlighterfill);
    void setThicknessToSelection(double penthickness, double highlighterthickness, double eraserthickness);
    void setFontToTextAndSelection(const XojFont& font);

    /// If a text is under edition, close it and make it an EditSelection
    void makeSelectionOfActiveText();

    void onToolNoLongerSpline();  ///< End the active spline

    // Change layout parameters
    void setViewPairedPages(bool enabled);
    void setPairsOffset(int numOffset);
    void setViewColumns(int numColumns);
    void setViewRows(int numRows);
    void setViewLayoutVert(bool vert);
    void setViewLayoutR2L(bool r2l);
    void setViewLayoutB2T(bool b2t);

    void switchPresentationMode(bool enable);  ///< Turn on/off presentation mode

    // ClipboardListener interface
    void clipboardPasteText(std::string text) override;
    void clipboardPasteImage(GdkPixbuf* img) override;
    void clipboardPasteXournal(ObjectInputStream& in) override;
    EditSelection* clipboardGetSelection() const override;
    void deleteSelection() override;

    void clipboardPaste(ElementPtr e);

    // Other selection handling
    void clearSelection();
    void clearSelectionEndText();
    void selectAllOnPage();
    void moveSelectionToLayer(size_t layerNo);
    void reorderSelection(EditSelection::OrderChange change);

    // Undo-redo
    void undo();
    void redo();

    // Page handling
    void duplicatePage();
    void insertNewPage(size_t position, bool automatedInsertion = false);
    void appendNewPdfPages();
    void insertPage(const PageRef& page, size_t position, bool shouldScrollToPage = true);
    void deletePage();
    void movePageTowardsBeginning();
    void movePageTowardsEnd();

    void onPageSelected(size_t page, size_t pdfPage);  ///< Inform components that a page has been selected

    /// Ask the user whether a page with the given id should be added to the document.
    void askInsertPdfPage(size_t pdfPage);

    // Save and exports
    void print();
    void exportAsPdf();
    void exportAs();
    /**
     * @brief Asynchronously saves the document and calls callback afterwards with boolean parameter true on success.
     * May ask the user for a place to save if necessary.
     */
    void save();
    /**
     * @brief Asks the user for a new location, asynchronously saves the document there and calls callback afterwards.
     */
    void saveAs();

    /// Marks the current document as saved if it is currently marked as unsaved.
    void resetSavedStatus();

    void setLastAutosaveFile(fs::path newAutosaveFile);
    void deleteLastAutosaveFile();

    void enableAutosave(bool enable);

    struct ChangedSettings {
        bool forceSidebarLayout = false;
        bool borderColorChanged = false;
        bool needLayout = false;
        bool unlimitedScrollingChanged = false;
        xoj::util::Point<double> scrollOffset = {0., 0.};  ///< Applied if needLayout
    };
    void updateSettingsChanged(const ChangedSettings& changes);

    // Dialogs acting on the current doc
    void showPaperFormatDialog();
    void showChangePageBackgroundColorDialog();
    void showGotoPageDialog();

    // UndoRedoListener interface
    void undoRedoChanged() override;
    void undoRedoPageChanged(PageRef page) override;

    /**
     * Close the current document, prompting to save unsaved changes.
     *
     * @param callback Called after trying to close the document, with param true in case of success, false otherwise.
     * @param allowDestroy Whether clicking "Discard" should destroy the current document.
     * @param allowCancel Whether the user should be able to cancel closing the document.
     * @param forceClose Whether to skip the save dialog and unconditionally discard unsaved changes.
     * @return true if the user closed the document, otherwise false.
     */
    void close(std::function<void(bool)> callback, bool allowDestroy = false, bool allowCancel = true,
               bool forceClose = false);

    /**
     * Forcibly replaces the opened document.
     * WARNING: Be sure the active document has been saved (or discarded) before calling replaceDocument()
     */
    void replaceDocument(std::unique_ptr<Document> doc, int scrollToPage);

    void closeDocument();  ///< Closes the active document

protected:
    void updateWindowTitle();  ///< Changes the window title, depending on the document's status

    void fileLoaded(int scrollToPage = -1);

    static bool checkChangedDocument(DocumentControl* control);
    static bool autosaveCallback(DocumentControl* control);

    /// Implemetation of save() and saveAs()
    void saveImpl(bool saveAs, std::function<void(bool)> callback);

    /**
     * @brief Creates the specified geometric tool if it's not on the current page yet. Deletes it if it already exists.
     * @return true if a geometric tool was created
     */
    template <class ToolClass, class ViewClass, class ControllerClass, class InputHandlerClass,
              GeometryToolType toolType>
    bool toggleGeometryTool();
    void resetGeometryTool();

public:
    /**
     * @brief Creates a compass if it's not on the current page yet. Deletes it if it already exists.
     * @return true if a compass was created
     */
    bool toggleCompass();
    /**
     * @brief Creates a setsquare if it's not on the current page yet. Deletes it if it already exists.
     * @return true if a setsquare was created
     */
    bool toggleSetsquare();

protected:
    MainControl* mainControl = nullptr;
    DocumentTab* tab = nullptr;

    /// autosave timer
    guint autosaveTimeout = 0;
    fs::path lastAutosaveFilename;

    guint changeTimout = 0;             ///< Timer watching the changes and actualizes the previews from time to time
    std::vector<PageRef> changedPages;  ///< pages modified since the last update (for preview update)
    std::list<DocumentListener*> changedDocumentListeners;  ///< Listeners to be updated by checkChangedDocument.

    std::unique_ptr<Document> doc;
    std::unique_ptr<UndoRedoHandler> undoRedo;  ///< Document's undo-redo stack
    std::unique_ptr<MetadataManager> metadata;  ///< Document's metadata

    std::unique_ptr<ZoomControl> zoom;
    std::unique_ptr<ScrollHandler> scrollHandler;

    std::unique_ptr<PageBackgroundChangeController> pageBackgroundChangeController;
    std::unique_ptr<LayerController> layerController;

    std::unique_ptr<GeometryTool> geometryTool;
    std::unique_ptr<GeometryToolController> geometryToolController;

    std::unique_ptr<NavigationHistory> navHistory;
};

class MainControl: public ToolListener, public ClipboardActionStatusListener {
public:
    MainControl(GApplication* gtkApp, GladeSearchpath* gladeSearchPath);
    ~MainControl() override;

    Settings* getSettings() const;
    ToolHandler* getToolHandler() const;
    MainWindow* getWindow() const;
    ProgressListener* getProgressListener() const;
    GtkWindow* getGtkWindow() const;
    XournalppCursor* getCursor() const;
    AudioController* getAudioController() const;
    PageTypeHandler* getPageTypes() const;
    PluginController* getPluginController() const;
    const Palette& getPalette() const;
    GladeSearchpath* getGladeSearchPath() const;
    XournalScheduler* getScheduler() const;
    ClipboardHandler& getClipboardHandler() const;
    inline ActionDatabase* getActionDatabase() const { return actionDB.get(); }
    DocumentControl* getDocumentControl() const;

public:
    // ToolListener interface
    void toolColorChanged() override;        ///< Update the Cursor and the Toolbar based on the active color
    void changeColorOfSelection() override;  ///< Change the color of the current selection based on the active Tool
    void toolChanged() override;
    void toolSizeChanged() override;
    void toolFillChanged() override;
    void toolLineStyleChanged() override;
    void eraserTypeChanged() override;

protected:
    void eraserSizeChanged();
    void penSizeChanged();
    void highlighterSizeChanged();

public:
    void selectTool(ToolType type);
    void selectDefaultTool();

    void setToolDrawingType(DrawingType type);
    void setToolSize(ToolSize size);
    void setFill(bool fill);
    void setFont(const XojFont& font);

    /**
     * Change the line style of the PEN if select, or of selected elements if any
     * Otherwise, select the PEN tool and set its linestyle
     */
    void setLineStyle(const std::string& style);

    /// Change the eraser type. If the eraser is not selected, select it as well.
    void setEraserType(EraserType type);


    void updatePageActions(size_t currentPage, size_t nbPages);  ///< Enable/Disable page action buttons
    void updateBackgroundActions(PageType currentPageType);  ///< Enable/Disable background size/color action buttons
    void updatePageNumbers(size_t page, size_t pageCount, size_t pdfPage);  ///< Update numbers in the UI
    void updateWindowTitle(const std::u8string& title);
    void updateUndoRedoActions(bool canUndo, bool canRedo, const std::string& undoDescription,
                               const std::string& redoDescription);
    void updateNavigationHistoryActions(bool canForward, bool canBackward);
    void updateLayerActions(size_t currentLayer, size_t maxLayer);

public:
    // ClipboardActionStatusListener interface
    void clipboardCutCopyEnabled(bool enabled) override;
    void clipboardPasteEnabled(bool enabled) override;

    void setCopyCutEnabled(bool enabled);

    bool copy();
    bool cut();
    bool paste();

    /// Make a schedule a job
    template <typename JobType, auto priority, typename... Args>
    void schedule(Args&&... args);

    /// Remove all jobs referring the document. May block, waiting for ongoing jobs to complete.
    void removeJobsFor(const DocumentControl* docCtrl);

    void saveState();  ///< Save current state (selected tool etc.)
    void registerPluginToolButtons(ToolMenuHandler* toolMenuHandler);
    void loadPaletteFromSettings();


    void setViewFullscreenMode(bool enabled);
    void setViewPresentationMode(bool enabled);

    void manageToolbars();
    void customizeToolbars();
    void setFullscreen(bool enabled);
    void setShowToolbar(bool enabled);
    void setShowMenubar(bool enabled);
    void setShowSidebar(bool enabled);

    /// Loads the view mode (hide/show menu-,tool-&sidebar)
    bool loadViewMode(ViewModeId mode);

    void setRotationSnapping(bool enable);
    void setGridSnapping(bool enable);

    // Show dialogs
    void showSettings();
    void showAbout();
    void showGtkDemo();
    void showPaperTemplateDialog();
    void showAlphaSelectionDialog(OpacityFeature feature);
    void showFontDialog();
    void showColorChooserDialog();

    /**
     * Show floating toolbox at specified coordinates
     * @param x x coordinate relative to main window
     * @param y y coordinate relative to main window
     */
    void showFloatingToolbox(int x, int y);

    void blockUI(const std::string& name);  ///< Freezes the UI, giving name as a reason to the user
    void unblockUI();                       ///< Unfreezes


    /// Asynchronously closes the current document and replaces it by a new file
    void newFile(fs::path filepath = {});

    /// @brief Shows an open file dialog and opens the selected file after closing the previously opened file
    void askToOpenFile();

    /**
     * @brief Asynchronously opens the provided path, safely closes the current opened document and replaces it with the
     * newly parsed file. Calls callback afterwards, with boolean parameter true on success. Does nothing to this->doc
     * in case of failure at any point.
     */
    void openFile(fs::path filepath, int scrollToPage = -1, bool forceOpen = false);

    /// Shows an open file dialog and opens the selected file
    void askToAnnotatePdf();

    /// A document has finished opening. Takes over `doc` and clears `this->fileOpener`
    void finishedOpeningFileCallback(std::unique_ptr<Document> doc, int scrollToPage, UndoActionPtr recoveryAction);

    /**
     * (Potentially asynchronously) Opens the given file without saving any previously opened Document. Treat the file
     * as the result of an emergency save: the file is then deleted and the document marked as unsaved.
     */
    void openEmergencyFile(fs::path filepath);

    //  TODO remove
    void openFileWithoutSavingTheCurrentDocument(fs::path filepath, bool attachToDocument, int scrollToPage);

    void quit(bool allowCancel = true);
    void initWindow(MainWindow* win, bool disableAudio);

    /// Runs a member function of the active document, if there is an active document
    template <auto fun, typename... Args>
    void activeDocDoes(Args&&... args) {
        if (this->getDocumentControl()) {
            (this->getDocumentControl()->*fun)(std::forward<Args>(args)...);
        }
    }

protected:
    GApplication* gtkApp = nullptr;
    GladeSearchpath* gladeSearchPath = nullptr;
    std::unique_ptr<ActionDatabase> actionDB;

    // Keep after the ActionDatabase so it is destroyed first: ~AudioController refers to the ActionDatabase
    std::unique_ptr<AudioController> audioController;
    std::unique_ptr<Settings> settings;
    std::unique_ptr<Palette> palette;                    ///< Colors
    std::unique_ptr<ToolHandler> toolHandler;            ///< Handles tool seleciton and configuration
    std::unique_ptr<XournalppCursor> cursor;             ///< Cursor abstraction layer
    std::unique_ptr<ClipboardHandler> clipboardHandler;  ///< Clipboard abstraction layer
    std::unique_ptr<XournalScheduler> scheduler;         ///< Job scheduler
    std::unique_ptr<PageTypeHandler> pageTypes;          ///< Preconfigured page types
    std::unique_ptr<PluginController> pluginController;
    std::unique_ptr<ToolbarDragDropHandler> dragDropHandler;  ///< Used during toolbar customization only

    MainWindow* win = nullptr;

    DocumentControl* documentControl = nullptr;

    bool isBlocking;  ///< true when a blocking job is running, freezing the UI

    std::unique_ptr<FileOpener> fileOpener;
};

class Control: public MainControl, public DocumentControl {
public:
    Control(GApplication* gtkApp, GladeSearchpath* gladeSearchPath);
    Control(Control const&) = delete;
    Control(Control&&) = delete;
    auto operator=(Control const&) -> Control& = delete;
    auto operator=(Control&&) -> Control& = delete;
    ~Control() override;

    inline auto* getSettings() const { return MainControl::getSettings(); }

public:

    template <Action a>
    friend struct ActionProperties;
};
