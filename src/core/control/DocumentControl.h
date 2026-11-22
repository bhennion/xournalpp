/*
 * Xournal++
 *
 * Control of a document
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

#include "control/tools/EditSelection.h"  // for OrderChange
#include "model/DocumentHandler.h"        // for DocumentHandler
#include "model/GeometryTool.h"           // for GeometryTool
#include "model/PageRef.h"                // for PageRef
#include "undo/UndoRedoHandler.h"         // for UndoRedoHandler (ptr only)

#include "ClipboardHandler.h"  // for ClipboardListener
#include "filesystem.h"        // for path

class GeometryToolController;
class Sidebar;
class MetadataManager;
class PageBackgroundChangeController;
class LayerController;
class Document;
class EditSelection;
class Element;
class ObjectInputStream;
class ScrollHandler;
class SearchBar;
class Settings;
class TextEditor;
class ZoomControl;
class XojFont;
class XojPdfRectangle;
class NavigationHistory;
class DocumentTab;
class MainControl;
class ToolHandler;

class DocumentControl: public DocumentHandler, public UndoRedoListener, public ClipboardListener {
public:
    DocumentControl(MainControl* mainCtrl);
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
    ToolHandler* getToolHandler() const;
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
    void setColorOfSelection(Color color);                 ///< Change the color of the currently selected  elements
    bool setLineStyleOfSelection(const LineStyle& style);  ///< returns true if there was a selection
    void setFillToSelection(int penfill, int highlighterfill);
    void setThicknessToSelection(double penthickness, double highlighterthickness, double eraserthickness);
    void setFontToTextAndSelection(const XojFont& font);

    /// If a text is under edition, close it and make it an EditSelection
    void makeSelectionOfActiveText();

    void onToolNoLongerText();    ///< End the active text
    void onToolNoLongerLink();    ///< End the active link
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
