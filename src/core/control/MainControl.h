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

#include <gio/gio.h>  // for GApplication
#include <glib.h>     // for guint

#include "control/ToolEnums.h"                      // for ToolSize, ToolType
#include "control/settings/ViewModes.h"             // for ViewModeId
#include "control/zoom/ZoomBounds.h"                // for ZoomBoundsControl
#include "enums/Action.enum.h"                      // for Action
#include "gui/toolbarMenubar/model/ColorPalette.h"  // for ColorPalette

#include "ClipboardHandler.h"  // for ClipboardListener
#include "ToolHandler.h"       // for ToolListener
#include "filesystem.h"        // for path

class AudioController;
class GladeSearchpath;
class XournalppCursor;
class ToolbarDragDropHandler;
class PageTypeHandler;
class PluginController;
class MainWindow;
class Settings;
class XournalScheduler;
class ToolMenuHandler;
class XojFont;
class ActionDatabase;
class ProgressListener;
class PageType;
class FileOpener;
class DocumentControl;
class Document;
class UndoAction;

class MainControl: public ToolListener, public ClipboardActionStatusListener, public ZoomBoundsControl {
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
    void toolColorChanged() override;
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
    void setColor(Color color, bool applyToSelection = true);  ///< The user actively chose a color

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
    void finishedOpeningFileCallback(std::unique_ptr<Document> doc, int scrollToPage,
                                     std::unique_ptr<UndoAction> recoveryAction);

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
