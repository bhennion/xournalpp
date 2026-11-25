#include "DocumentControl.h"

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

#include "control/ClipboardHandler.h"                   // for Clip...
#include "control/CompassController.h"                  // for Comp...
#include "control/NavigationHistory.h"                  // for Navi...
#include "control/RecentManager.h"                      // for Rece...
#include "control/ScrollHandler.h"                      // for Scro...
#include "control/SetsquareController.h"                // for Sets...
#include "control/Tool.h"                               // for Tool
#include "control/ToolHandler.h"                        // for Tool...
#include "control/jobs/AutosaveJob.h"                   // for Auto...
#include "control/jobs/BaseExportJob.h"                 // for Base...
#include "control/jobs/CustomExportJob.h"               // for Cust...
#include "control/jobs/ExportParameters.h"              // for Cust...
#include "control/jobs/PdfExportJob.h"                  // for PdfE...
#include "control/jobs/SaveJob.h"                       // for SaveJob
#include "control/jobs/Scheduler.h"                     // for JOB_...
#include "control/jobs/XournalScheduler.h"              // for Xour...
#include "control/layer/LayerController.h"              // for Laye...
#include "control/settings/MetadataManager.h"           // for Meta...
#include "control/settings/PageTemplateSettings.h"      // for Page...
#include "control/settings/Settings.h"                  // for Sett...
#include "control/tools/TextEditor.h"                   // for Text...
#include "control/zoom/ZoomControl.h"                   // for Zoom...
#include "gui/DocumentTab.h"                            // for DocumentTab
#include "gui/MainWindow.h"                             // for Main...  TODO  remove
#include "gui/PageView.h"                               // for XojP...
#include "gui/PdfFloatingToolbox.h"                     // for PdfF...
#include "gui/SearchBar.h"                              // for Sear...
#include "gui/XournalView.h"                            // for Xour...
#include "gui/XournalppCursor.h"                        // for Xour...
#include "gui/dialog/ExportDialog.h"                    // for ExportDialog
#include "gui/dialog/FormatDialog.h"                    // for Form...
#include "gui/dialog/GotoDialog.h"                      // for Goto...
#include "gui/dialog/SelectBackgroundColorDialog.h"     // for Sele...
#include "gui/dialog/XojSaveDlg.h"                      // for XojS...
#include "gui/inputdevices/CompassInputHandler.h"       // for Comp...
#include "gui/inputdevices/GeometryToolInputHandler.h"  // for Geom...
#include "gui/inputdevices/HandRecognition.h"           // for Hand...
#include "gui/inputdevices/SetsquareInputHandler.h"     // for Sets...
#include "gui/sidebar/Sidebar.h"                        // for Sidebar
#include "model/BackgroundImage.h"                      // for Back...
#include "model/Compass.h"                              // for Comp...
#include "model/Document.h"                             // for Docu...
#include "model/DocumentChangeType.h"                   // for DOCU...
#include "model/DocumentListener.h"                     // for Docu...
#include "model/Element.h"                              // for Element
#include "model/Font.h"                                 // for XojFont
#include "model/Image.h"                                // for Image
#include "model/Layer.h"                                // for Layer
#include "model/LineStyle.h"                            // for Line...
#include "model/Link.h"                                 // for Link
#include "model/PageType.h"                             // for Page...
#include "model/Setsquare.h"                            // for Sets...
#include "model/Stroke.h"                               // for Stroke
#include "model/StrokeStyle.h"                          // for Stro...
#include "model/TexImage.h"                             // for TexI...
#include "model/Text.h"                                 // for Text
#include "model/XojPage.h"                              // for XojPage
#include "pdf/base/XojPdfPage.h"                        // for XojP...
#include "undo/AddUndoAction.h"                         // for AddU...
#include "undo/InsertDeletePageUndoAction.h"            // for Inse...
#include "undo/InsertUndoAction.h"                      // for Inse...
#include "undo/MoveSelectionToLayerUndoAction.h"        // for Move...
#include "undo/PageSizeChangeUndoAction.h"              // for PageSizeChangeUndoAction
#include "undo/SwapUndoAction.h"                        // for SwapUndoAction
#include "undo/UndoAction.h"                            // for Undo...
#include "util/Assert.h"                                // for xoj_assert
#include "util/Color.h"                                 // for oper...
#include "util/PathUtil.h"                              // for clea...
#include "util/PlaceholderString.h"                     // for Plac...
#include "util/PopupWindowWrapper.h"                    // for PopupWindowWrapper
#include "util/Stacktrace.h"                            // for Stac...
#include "util/StringUtils.h"                           // for char_cast
#include "util/Util.h"                                  // for exec...
#include "util/XojMsgBox.h"                             // for XojM...
#include "util/glib_casts.h"                            // for wrap_v
#include "util/i18n.h"                                  // for _, FS
#include "util/safe_casts.h"                            // for as_unsigned
#include "util/serializing/InputStreamException.h"      // for Inpu...
#include "util/serializing/ObjectInputStream.h"         // for Obje...
#include "util/utf8_view.h"                             // for utf8
#include "view/CompassView.h"                           // for Comp...  TODO  remove
#include "view/SetsquareView.h"                         // for Sets...  TODO  remove
#include "view/overlays/OverlayView.h"                  // for Over...  TODO  remove

#include "CrashHandler.h"                    // for emer...
#include "MainControl.h"                     // for MainControl
#include "MainControlSchedule.h"             // for MainControl::schedule()
#include "PageBackgroundChangeController.h"  // for Page...
#include "PrintHandler.h"                    // for print
#include "UndoRedoController.h"              // for Undo...
#include "config-dev.h"                      // for SETT...
#include "config.h"                          // for PROJ...


DocumentControl::DocumentControl(MainControl* mainCtrl):
        mainControl(mainCtrl),
        doc(std::make_unique<Document>()),
        undoRedo(std::make_unique<UndoRedoHandler>(this)),
        metadata(std::make_unique<MetadataManager>()),
        zoom(std::make_unique<ZoomControl>(mainCtrl->getZoomBounds())),
        scrollHandler(std::make_unique<ScrollHandler>(this)),
        pageBackgroundChangeController(std::make_unique<PageBackgroundChangeController>(this)),
        layerController(std::make_unique<LayerController>(this)),
        navHistory(std::make_unique<NavigationHistory>(this)) {
    this->undoRedo->addUndoRedoListener(this);
    this->layerController->registerListener(this);

    // This is needed to update the previews
    this->changeTimout = g_timeout_add_seconds(5, xoj::util::wrap_v<checkChangedDocument>, this);

    this->enableAutosave(getSettings()->isAutosaveEnabled());

    setEmergencyDocument(this->doc.get());  // for crash handling
}

DocumentControl::~DocumentControl() {
    g_source_remove(this->changeTimout);
    this->enableAutosave(false);
    deleteLastAutosaveFile();
    setEmergencyDocument(nullptr);
}

void DocumentControl::setLastAutosaveFile(fs::path newAutosaveFile) {
    try {
        if (!this->lastAutosaveFilename.empty() && !fs::equivalent(newAutosaveFile, this->lastAutosaveFilename) &&
            fs::exists(newAutosaveFile)) {
            deleteLastAutosaveFile();
        }
    } catch (const fs::filesystem_error& e) {
        auto fmtstr = FS(_F("Filesystem error: {1}") % e.what());
        Util::execInUiThread(
                [fmtstr, win = this->getMainControl()->getGtkWindow()]() { XojMsgBox::showErrorToUser(win, fmtstr); });
    }
    this->lastAutosaveFilename = std::move(newAutosaveFile);
}

void DocumentControl::deleteLastAutosaveFile() {
    try {
        if (fs::exists(this->lastAutosaveFilename)) {
            fs::remove(this->lastAutosaveFilename);
        }
    } catch (const fs::filesystem_error& e) {
        auto fmtstr = FS(_F("Could not remove old autosave file \"{1}\": {2}") % this->lastAutosaveFilename.u8string() %
                         e.what());
        Util::execInUiThread(
                [fmtstr, win = this->getMainControl()->getGtkWindow()]() { XojMsgBox::showErrorToUser(win, fmtstr); });
    }
    this->lastAutosaveFilename.clear();
}

auto DocumentControl::checkChangedDocument(DocumentControl* control) -> bool {
    if (!control->doc->try_lock_shared()) {
        // call again later
        return true;
    }
    for (auto const& page: control->changedPages) {
        auto p = control->doc->indexOf(page);
        if (p != npos) {
            for (DocumentListener* dl: control->changedDocumentListeners) {
                dl->pageChanged(p);
            }
        }
    }
    control->changedPages.clear();
    control->doc->unlock_shared();

    // Call again
    return true;
}

auto DocumentControl::autosaveCallback(DocumentControl* control) -> bool {
    if (control->undoRedo->isChangedAutosave()) {
        control->getMainControl()->schedule<AutosaveJob, JOB_PRIORITY_NONE>();
    }
    return true;
}

void DocumentControl::enableAutosave(bool enable) {
    if (this->autosaveTimeout) {
        g_source_remove(this->autosaveTimeout);
        this->autosaveTimeout = 0;
    }

    if (enable) {
        auto timeout = guint(getSettings()->getAutosaveTimeout()) * 60U;
        this->autosaveTimeout = g_timeout_add_seconds(timeout, xoj::util::wrap_v<autosaveCallback>, this);
    }
}

void DocumentControl::onPageSelected(size_t page, size_t pdfPage) {
    this->getSidebar()->selectPageNr(page, pdfPage);

    this->metadata->storeMetadata(this->doc->getEvMetadataFilename(), static_cast<int>(page),
                                  getZoomControl()->getZoomReal());

    if (navHistory) {
        navHistory->prune();
        navHistory->updateActions();
    }

    getMainControl()->updatePageNumbers(page, this->doc->getPageCount(), pdfPage);

    if (getSettings()->isPageNumberInTitlebarShown()) {
        this->updateWindowTitle();
    }
}

NavigationHistory* DocumentControl::getNavigationHistory() const { return navHistory.get(); }

bool DocumentControl::toggleCompass() {
    return toggleGeometryTool<Compass, xoj::view::CompassView, CompassController, CompassInputHandler,
                              GeometryToolType::COMPASS>();
}
bool DocumentControl::toggleSetsquare() {
    return toggleGeometryTool<Setsquare, xoj::view::SetsquareView, SetsquareController, SetsquareInputHandler,
                              GeometryToolType::SETSQUARE>();
}

template <class ToolClass, class ViewClass, class ControllerClass, class InputHandlerClass, GeometryToolType toolType>
bool DocumentControl::toggleGeometryTool() {
    bool needsNewGeometryTool = !this->geometryToolController || this->geometryToolController->getType() != toolType;
    this->resetGeometryTool();
    if (!needsNewGeometryTool) {
        return false;
    }
    const auto view = this->tab->getXournal()->getViewFor(getCurrentPageNo());
    const auto* xournal = GTK_XOURNAL(this->tab->getXournal()->getWidget());
    auto tool = new ToolClass();
    view->addOverlayView(std::make_unique<ViewClass>(tool, view, zoom.get()));
    this->geometryTool = std::unique_ptr<GeometryTool>(tool);
    this->geometryToolController = std::make_unique<ControllerClass>(view, tool);
    auto geometryToolInputHandler =
            std::make_unique<InputHandlerClass>(this->tab->getXournal(), geometryToolController.get());
    Range range = view->getVisiblePart();
    if (range.isValid()) {
        double originX = (range.minX + range.maxX) * .5;
        double originY = (range.minY + range.maxY) * .5;
        geometryToolController->translate({originX, originY});
    } else {
        geometryToolController->translate({view->getWidth() * .5, view->getHeight() * .5});
    }
    xournal->input->setGeometryToolInputHandler(std::move(geometryToolInputHandler));
    geometryTool->notify();
    return true;
}

void DocumentControl::resetGeometryTool() {
    this->geometryToolController.reset();
    this->geometryTool.reset();
    auto* xournal = GTK_XOURNAL(this->tab->getXournal()->getWidget());
    xournal->input->resetGeometryToolInputHandler();
}

void DocumentControl::clearSelectionEndText() {
    clearSelection();
    if (tab) {
        tab->getXournal()->endTextAllPages();
    }
}

void DocumentControl::selectAllOnPage() {
    auto pageNr = getCurrentPageNo();
    if (pageNr == npos) {
        return;
    }

    tab->getXournal()->clearSelection();

    this->doc->lock();
    XojPageView* view = tab->getXournal()->getViewFor(pageNr);
    if (view == nullptr) {
        this->doc->unlock();
        return;
    }

    PageRef page = this->doc->getPage(pageNr);
    Layer* layer = page->getSelectedLayer();

    auto elements = layer->clearNoFree();
    this->doc->unlock();

    if (!elements.empty()) {
        InsertionOrder insertionOrder;
        insertionOrder.reserve(elements.size());
        Element::Index n = 0;
        for (auto&& e: elements) {
            insertionOrder.emplace_back(std::move(e), n++);
        }
        auto [sel, rg] =
                SelectionFactory::createFromFloatingElements(this, page, layer, view, std::move(insertionOrder));

        page->fireRangeChanged(rg);

        tab->getXournal()->setSelection(sel.release());
    }
}

void DocumentControl::reorderSelection(EditSelection::OrderChange change) {
    EditSelection* sel = tab->getXournal()->getSelection();
    if (!sel) {
        return;
    }

    auto undoAction = sel->rearrangeInsertionOrder(change);
    this->undoRedo->addUndoAction(std::move(undoAction));
}

/**
 * Fire page selected, but first check if the page Number is valid
 *
 * @return the page ID or size_t_npos if the page is not found
 */
auto DocumentControl::firePageSelected(const PageRef& page) -> size_t {
    this->doc->lock_shared();
    size_t pageId = this->doc->indexOf(page);
    this->doc->unlock_shared();
    if (pageId == npos) {
        return npos;
    }

    firePageSelected(pageId);
    return pageId;
}

void DocumentControl::firePageSelected(size_t page) {
    if (page != this->getCurrentPageNo()) {
        DocumentHandler::firePageSelected(page);
    }
}

void DocumentControl::onToolNoLongerSpline() {
    xoj_assert(tab);
    // Shape changed from spline to something else: finish ongoing splines
    tab->getXournal()->endSplineAllPages();
}

void DocumentControl::deletePage() {
    clearSelectionEndText();

    // if the current page contains the geometry tool, reset it
    size_t pNr = getCurrentPageNo();
    if (geometryToolController) {
        doc->lock_shared();
        auto page = doc->indexOf(geometryToolController->getPage());
        doc->unlock_shared();
        if (page == pNr) {
            resetGeometryTool();
        }
    }
    // don't allow delete pages if we have less than 2 pages,
    // so we can be (more or less) sure there is at least one page.
    if (this->doc->getPageCount() < 2) {
        return;
    }

    if (pNr == npos || pNr > this->doc->getPageCount()) {
        // something went wrong...
        return;
    }

    this->doc->lock_shared();
    PageRef page = doc->getPage(pNr);
    this->doc->unlock_shared();

    // first send event, then delete page...
    firePageDeleted(pNr);

    this->doc->lock();
    doc->deletePage(pNr);
    this->doc->unlock();

    this->undoRedo->addUndoAction(std::make_unique<InsertDeletePageUndoAction>(page, pNr, false));

    if (pNr >= this->doc->getPageCount()) {
        pNr = this->doc->getPageCount() - 1;
    }

    scrollHandler->scrollToPage(pNr);
    this->tab->getXournal()->forceUpdatePagenumbers();
}

void DocumentControl::duplicatePage() {
    auto page = getCurrentPage();
    if (!page) {
        return;
    }
    auto pageCopy = std::make_shared<XojPage>(*page);

    insertPage(pageCopy, getCurrentPageNo() + 1);
}

void DocumentControl::movePageTowardsBeginning() {
    auto currentPageNo = this->getCurrentPageNo();
    if (currentPageNo < 1) {
        g_warning("Control::movePageTowardsBeginning() called on the first page");
        return;
    }
    if (currentPageNo == npos) {
        g_warning("Control::movePageTowardsBeginning() called with current page selected");
        return;
    }

    auto lock = std::unique_lock(*this->doc);
    PageRef page = this->doc->getPage(currentPageNo);
    PageRef otherPage = doc->getPage(currentPageNo - 1);
    if (!page || !otherPage) {
        g_warning("Control::movePageTowardsBeginning() called but no page %zu or %zu", currentPageNo,
                  currentPageNo - 1);
        return;
    }

    doc->deletePage(currentPageNo);
    doc->insertPage(page, currentPageNo - 1);
    lock.unlock();

    UndoRedoHandler* undo = this->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<SwapUndoAction>(currentPageNo - 1, true, page, otherPage));

    this->firePageDeleted(currentPageNo);
    this->firePageInserted(currentPageNo - 1);
    this->firePageSelected(currentPageNo - 1);

    this->getScrollHandler()->scrollToPage(currentPageNo - 1);
}


void DocumentControl::movePageTowardsEnd() {
    auto currentPageNo = this->getCurrentPageNo();
    if (currentPageNo == npos) {
        g_warning("Control::movePageTowardsEnd() called with current page selected");
        return;
    }

    auto lock = std::unique_lock(*this->doc);
    auto nbPage = this->doc->getPageCount();
    if (currentPageNo >= nbPage - 1) {
        g_warning("Control::movePageTowardsEnd() called on last page");
        return;
    }

    PageRef page = this->doc->getPage(currentPageNo);
    PageRef otherPage = doc->getPage(currentPageNo + 1);
    if (!page || !otherPage) {
        g_warning("Control::movePageTowardsEnd() called but no page %zu or %zu", currentPageNo, currentPageNo + 1);
        return;
    }

    doc->deletePage(currentPageNo);
    doc->insertPage(page, currentPageNo + 1);
    lock.unlock();

    this->undoRedo->addUndoAction(std::make_unique<SwapUndoAction>(currentPageNo, false, page, otherPage));

    this->firePageDeleted(currentPageNo);
    this->firePageInserted(currentPageNo + 1);
    this->firePageSelected(currentPageNo + 1);

    this->getScrollHandler()->scrollToPage(currentPageNo + 1);
}

/// Remove mnemonic indicators in menu labels
static std::string removeMnemonics(std::string orig) {
    std::regex reg("_(.)");
    return std::regex_replace(orig, reg, "$1");
}

void DocumentControl::askInsertPdfPage(size_t pdfPage) {
    using Responses = enum { CANCEL = 1, AFTER = 2, END = 3 };
    std::vector<XojMsgBox::Button> buttons = {{_("Cancel"), Responses::CANCEL},
                                              {_("Insert after current page"), Responses::AFTER},
                                              {_("Insert at end"), Responses::END}};

    // Must match the labels in main.glade and PageTypeHandler.cpp
    std::string pathToMenuEntry = removeMnemonics(_("_Journal") + std::string(" → ") + _("Paper B_ackground") +
                                                  std::string(" → ") + _("With PDF background"));

    XojMsgBox::askQuestion(this->getMainControl()->getGtkWindow(),
                           FC(_F("Your current document does not contain PDF Page no {1}\n"
                                 "Would you like to insert this page?\n\n"
                                 "Tip: You can select {2} to insert a PDF page.") %
                              static_cast<int64_t>(pdfPage + 1) % pathToMenuEntry),
                           "", buttons, [ctrl = this, pdfPage](int response) {
                               if (response == Responses::AFTER || response == Responses::END) {
                                   Document* doc = ctrl->getDocument();

                                   doc->lock_shared();
                                   size_t position = response == Responses::AFTER ? ctrl->getCurrentPageNo() + 1 :
                                                                                    doc->getPageCount();
                                   XojPdfPageSPtr pdf = doc->getPdfPage(pdfPage);
                                   doc->unlock_shared();

                                   if (pdf) {
                                       auto page = std::make_shared<XojPage>(pdf->getWidth(), pdf->getHeight());
                                       page->setBackgroundPdfPageNr(pdfPage);
                                       ctrl->insertPage(page, position);
                                   }
                               }
                           });
}

void DocumentControl::insertNewPage(size_t position, bool automatedInsertion) {
    pageBackgroundChangeController->insertNewPage(position, automatedInsertion);
}

void DocumentControl::appendNewPdfPages() {
    auto pageCount = this->doc->getPageCount();
    // find last page with pdf background and get its pdf page number
    auto currentPdfPageCount = [&]() {
        for (size_t i = pageCount; i != 0; --i) {
            if (auto page = doc->getPage(i - 1); page && page->getBackgroundType().isPdfPage()) {
                return page->getPdfPageNr() + 1;
            }
        }
        return size_t{0U};
    }();

    auto pdfPageCount = this->doc->getPdfPageCount();
    auto insertCount = pdfPageCount - currentPdfPageCount;

    if (insertCount == 0) {
        std::string msg = FS(_F("No pdf pages available to append. You may need to reopen the document first."));
        XojMsgBox::showErrorToUser(getMainControl()->getGtkWindow(), msg);
    }
    for (size_t i = 0; i != insertCount; ++i) {

        doc->lock_shared();
        XojPdfPageSPtr pdf = doc->getPdfPage(currentPdfPageCount + i);
        doc->unlock_shared();

        if (pdf) {
            auto newPage = std::make_shared<XojPage>(pdf->getWidth(), pdf->getHeight());
            newPage->setBackgroundPdfPageNr(currentPdfPageCount + i);
            insertPage(newPage, pageCount + i);
        } else {
            std::string msg = FS(_F("Unable to retrieve pdf page."));  // should not happen
            XojMsgBox::showErrorToUser(getMainControl()->getGtkWindow(), msg);
        }
    }
}

void DocumentControl::insertPage(const PageRef& page, size_t position, bool shouldScrollToPage) {
    this->doc->lock();
    this->doc->insertPage(page, position);  // insert the new page to the document and update page numbers
    this->doc->unlock();
    undoRedo->addUndoAction(std::make_unique<InsertDeletePageUndoAction>(page, position, true));

    // notify document listeners about the inserted page; this creates the new XojViewPage, recalculates the layout
    // and creates a preview page in the sidebar
    firePageInserted(position);

    getMainControl()->getCursor()->updateCursor();

    // make the inserted page fully visible (or at least as much from the top which fits on the screen),
    // and make the page appear selected
    if (shouldScrollToPage) {
        scrollHandler->scrollToPage(position);
        firePageSelected(position);
    }

    getMainControl()->updatePageActions(getCurrentPageNo(), this->doc->getPageCount());
}

void DocumentControl::showGotoPageDialog() {
    auto popup = xoj::popup::PopupWindowWrapper<xoj::popup::GotoDialog>(
            this->getMainControl()->getGladeSearchPath(), this->getCurrentPageNo(), this->doc->getPageCount(),
            [scroll = this->scrollHandler.get()](size_t pageNumber) {
                xoj_assert(pageNumber != 0);
                scroll->jumpToPage(pageNumber - 1);
            });
    popup.show(this->getMainControl()->getGtkWindow());
}

void DocumentControl::showPaperFormatDialog() {
    auto const& page = getCurrentPage();
    if (!page || page->getBackgroundType().isPdfPage()) {
        return;
    }
    clearSelectionEndText();

    auto popup = xoj::popup::PopupWindowWrapper<xoj::popup::FormatDialog>(
            this->getMainControl()->getGladeSearchPath(), getSettings(), page->getWidth(), page->getHeight(),
            [ctrl = this, page](double width, double height) {
                ctrl->doc->lock();
                double oldW = page->getWidth();
                double oldH = page->getHeight();
                Document::setPageSize(page, width, height);
                size_t pageNo = ctrl->doc->indexOf(page);
                size_t pageCount = ctrl->doc->getPageCount();
                ctrl->doc->unlock();
                ctrl->undoRedo->addUndoAction(std::make_unique<PageSizeChangeUndoAction>(page, oldW, oldH));
                if (pageNo != npos && pageNo < pageCount) {
                    ctrl->firePageSizeChanged(pageNo);
                }
            });
    popup.show(this->getMainControl()->getGtkWindow());
}

void DocumentControl::showChangePageBackgroundColorDialog() {
    auto pNr = getCurrentPageNo();
    this->doc->lock_shared();
    auto const& p = this->doc->getPage(pNr);
    this->doc->unlock_shared();

    if (!p) {
        return;
    }

    clearSelectionEndText();

    PageType bg = p->getBackgroundType();
    if (bg.isSpecial()) {
        return;
    }

    xoj::popup::PopupWindowWrapper<SelectBackgroundColorDialog> dlg(this->getSettings(),
                                                                    [p, pNr, ctrl = this](Color color) {
                                                                        p->setBackgroundColor(color);
                                                                        ctrl->firePageChanged(pNr);
                                                                    });
    dlg.show(this->getMainControl()->getGtkWindow());
}

void DocumentControl::switchPresentationMode(bool enabled) {
    this->getZoomControl()->setZoomPresentationMode(enabled);
    this->getDocumentTab()->getXournal()->layoutPages();
    this->getScrollHandler()->scrollToPage(this->getCurrentPageNo());
}

void DocumentControl::setViewPairedPages(bool enabled) {
    getSettings()->setShowPairedPages(enabled);
    tab->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void DocumentControl::setPairsOffset(int numOffset) {
    getSettings()->setPairsOffset(numOffset);
    tab->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void DocumentControl::setViewColumns(int numColumns) {
    getSettings()->setViewColumns(numColumns);
    getSettings()->setViewFixedRows(false);
    tab->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void DocumentControl::setViewRows(int numRows) {
    getSettings()->setViewRows(numRows);
    getSettings()->setViewFixedRows(true);
    tab->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void DocumentControl::setViewLayoutVert(bool vert) {
    getSettings()->setViewLayoutVert(vert);
    tab->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void DocumentControl::setViewLayoutR2L(bool r2l) {
    getSettings()->setViewLayoutR2L(r2l);
    tab->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void DocumentControl::setViewLayoutB2T(bool b2t) {
    getSettings()->setViewLayoutB2T(b2t);
    tab->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

auto DocumentControl::getCurrentPageNo() const -> size_t {
    if (this->tab) {
        return this->tab->getXournal()->getCurrentPage();
    }
    return 0;
}

auto DocumentControl::searchTextOnPage(const std::string& text, size_t pageNumber, size_t index, size_t* occurrences,
                                       XojPdfRectangle* matchRect) -> bool {
    return this->tab->getXournal()->searchTextOnPage(text, pageNumber, index, occurrences, matchRect);
}

auto DocumentControl::getCurrentPage() -> PageRef {
    this->doc->lock_shared();
    PageRef p = this->doc->getPage(getCurrentPageNo());
    this->doc->unlock_shared();

    return p;
}

void DocumentControl::undoRedoChanged() {
    this->getMainControl()->updateUndoRedoActions(undoRedo->canUndo(), undoRedo->canRedo(), undoRedo->undoDescription(),
                                                  undoRedo->redoDescription());
    this->updateWindowTitle();
}

void DocumentControl::undoRedoPageChanged(PageRef page) {
    if (std::find(begin(this->changedPages), end(this->changedPages), page) == end(this->changedPages)) {
        this->changedPages.emplace_back(std::move(page));
    }
}

void DocumentControl::makeSelectionOfActiveText() {
    auto* xournal = this->tab->getXournal();
    if (auto* te = xournal->getTextEditor(); te && !(te->bufferEmpty())) {
        Text* textobj = te->getTextElement();
        clearSelectionEndText();

        auto pageNr = getCurrentPageNo();
        XojPageView* view = xournal->getViewFor(pageNr);
        xoj_assert(view != nullptr);
        PageRef page = view->getPage();
        auto sel = SelectionFactory::createFromElementOnActiveLayer(this, page, view, textobj);

        xournal->setSelection(sel.release());
    }
}

void DocumentControl::onToolNoLongerLink() { getDocumentTab()->getXournal()->endLinkAllPages(); }

void DocumentControl::onToolNoLongerText() { getDocumentTab()->getXournal()->endTextAllPages(); }

auto DocumentControl::getLineStyleOfSelection() -> std::optional<LineStyle> const {
    const EditSelection* sel = tab->getXournal()->getSelection();
    if (!sel) {
        return std::nullopt;
    }
    std::optional<LineStyle> style;

    // Todo(cpp20) Replace with std::ranges::filter_view and for_first_then_for_each
    for (const Element* e: sel->getElementsView()) {
        if (e->getType() == ELEMENT_STROKE) {
            const auto* s = dynamic_cast<const Stroke*>(e);

            if (s->getToolType().hasLineStyle()) {
                if (style) {
                    if (style.value() != s->getLineStyle()) {
                        return std::nullopt;
                    }
                } else {
                    style = s->getLineStyle();
                }
            }
        }
    }
    return style;
}

void DocumentControl::setColorOfSelection(Color color) {
    if (EditSelection* sel = this->tab->getXournal()->getSelection(); sel != nullptr) {
        undoRedo->addUndoAction(sel->setColor(color));
    }

    if (TextEditor* edit = getTextEditor(); edit != nullptr) {
        // Todo move into selection
        edit->setColor(color);
    }
}

void DocumentControl::updateSettingsChanged(const ChangedSettings& changes) {
    this->getSidebar()->saveSize();
    this->getDocumentTab()->updateScrollbarSidebarPosition();

    this->enableAutosave(getSettings()->isAutosaveEnabled());

    if (changes.forceSidebarLayout) {
        this->getSidebar()->layout();
    }

    this->updateWindowTitle();
    auto* xournal = this->getDocumentTab()->getXournal();

    xournal->getHandRecognition()->reload();

    if (changes.borderColorChanged) {
        xournal->forceUpdatePagenumbers();
    }

    if (changes.needLayout) {
        xournal->layoutPages();
        getDocumentTab()->getLayout()->scrollRelative(changes.scrollOffset.x, changes.scrollOffset.y);

        if (changes.unlimitedScrollingChanged) {
            const int xUnlimited = static_cast<int>(getDocumentTab()->getLayout()->getVisibleRect().width);
            const int yUnlimited = static_cast<int>(getDocumentTab()->getLayout()->getVisibleRect().height);
            if (getSettings()->getUnlimitedScrolling()) {
                getDocumentTab()->getLayout()->scrollRelative(xUnlimited, yUnlimited);
            } else {
                getDocumentTab()->getLayout()->scrollRelative(-xUnlimited, -yUnlimited);
            }
        }
    }
}

void DocumentControl::replaceDocument(std::unique_ptr<Document> doc, int scrollToPage) {
    this->closeDocument();

    fs::path filepath = doc->getFilepath();

    this->doc->lock();
    *this->doc = *doc;
    this->doc->unlock();

    this->fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);

    setEmergencyDocument(this->doc.get());

    // Set folder as last save path, so the next save will be at the current document location
    // This is important because of the new .xopp format, where Xournal .xoj handled as import, not as file to load
    if (!filepath.empty()) {
        getSettings()->setLastSavePath(filepath.parent_path());
    }

    fileLoaded(scrollToPage);
}

void DocumentControl::fileLoaded(int scrollToPage) {
    this->doc->lock_shared();
    auto filepath = this->doc->getEvMetadataFilename();
    this->doc->unlock_shared();

    double targetZoom = -1.;

    if (!filepath.empty()) {
        RecentManager::addRecentFileFilename(filepath);
        auto md = MetadataManager::getForFile(filepath);
        if (md) {
            if (scrollToPage == -1) {
                scrollToPage = md->page;
            }
            targetZoom = md->zoom;
        }
        if (scrollToPage >= 0) {
            getScrollHandler()->scrollToPage(as_unsigned(scrollToPage));
        }
    }
    if (targetZoom > 0. && !getSettings()->getForceZoomToFitOnLoad()) {
        zoom->setZoom(this->getMainControl()->getZoomBounds().convertFromZoomReal(targetZoom));
    } else {
        zoom->setZoomFitMode(true);
    }

    updateWindowTitle();
    tab->getXournal()->forceUpdatePagenumbers();
    getMainControl()->getCursor()->updateCursor();
    getMainControl()->updatePageActions(getCurrentPageNo(), this->doc->getPageCount());
}

void DocumentControl::print() {
    this->doc->lock_shared();
    PrintHandler::print(this->doc.get(), getCurrentPageNo(), this->getMainControl()->getGtkWindow());
    this->doc->unlock_shared();
}

void DocumentControl::updateWindowTitle() {
    std::string title{};  ///< Actually a UTF-8 string

    this->doc->lock_shared();
    const fs::path& refPath = doc->getFilepath().empty() ? doc->getPdfFilepath() : doc->getFilepath();
    if (refPath.empty()) {
        title = _("Unsaved Document");
    } else {
        if (getSettings()->isPageNumberInTitlebarShown()) {
            title = "[" + std::to_string(getCurrentPageNo() + 1) + "/" + std::to_string(doc->getPageCount()) + "]  ";
        }
        if (undoRedo->isChanged()) {
            title += "*";
        }

        if (getSettings()->isFilepathInTitlebarShown()) {
            title += std::string("[") + char_cast(refPath.parent_path().u8string().c_str()) + "] - ";
        }
        title += char_cast(refPath.filename().u8string());
    }
    this->doc->unlock_shared();

    title += " - Xournal++";

    this->getMainControl()->updateWindowTitle(xoj::util::utf8(title).str());
}

static fs::path createExportFilePath(Document* doc, const Settings* settings) {
    doc->lock_shared();
    fs::path suggestedPath = doc->createSaveFoldername(settings->getLastSavePath());
    suggestedPath /=
            doc->createSaveFilename(Document::PDF, settings->getDefaultSaveName(), settings->getDefaultPdfExportName());
    doc->unlock_shared();
    return suggestedPath;
}

void DocumentControl::exportAsPdf() {
    this->clearSelectionEndText();

    auto suggestedPath = createExportFilePath(this->doc.get(), this->getSettings());

    auto callback = [ctrl = this](std::optional<fs::path> out) {
        if (out) {
            ctrl->getMainControl()->schedule<PdfExportJob, JOB_PRIORITY_NONE>(std::move(*out));
        }
    };

    xoj::SaveExportDialog::showExportFileDialog(this->getMainControl()->getGtkWindow(), this->getSettings(),
                                                std::move(suggestedPath), xoj::FileTypes::PDF, std::move(callback));
}

static const xoj::FileType& getFileType(ExportFormat fmt) {
    switch (fmt) {
        case EXPORT_GRAPHICS_PNG:
            return xoj::FileTypes::PNG;
        case EXPORT_GRAPHICS_SVG:
            return xoj::FileTypes::SVG;
        case EXPORT_XOJ:
            return xoj::FileTypes::XOJ;
        case EXPORT_GRAPHICS_PDF:
        default:
            return xoj::FileTypes::PDF;
    }
}

void DocumentControl::exportAs() {
    this->clearSelectionEndText();

    // First open the ExportDialog selecting the format and various parameters
    xoj::popup::PopupWindowWrapper<xoj::popup::ExportDialog> popup(
            this->getMainControl()->getGladeSearchPath(), this->getCurrentPageNo() + 1,
            this->getDocument()->getPageCount(), !this->getDocument()->getPdfFilepath().empty(),
            [ctrl = this](std::unique_ptr<ExportParameters> params) {
                auto suggestedPath = createExportFilePath(ctrl->doc.get(), ctrl->getSettings());
                const auto& filetype = getFileType(params->format);

                // Then open a file chooser dialog to pick where to export
                xoj::SaveExportDialog::showExportFileDialog(
                        ctrl->getMainControl()->getGtkWindow(), ctrl->getSettings(), std::move(suggestedPath), filetype,
                        [ctrl, params = std::move(params)](std::optional<fs::path> p) mutable {
                            if (p) {
                                ctrl->getMainControl()->schedule<CustomExportJob, JOB_PRIORITY_NONE>(
                                        std::move(p.value()), std::move(params));
                            }
                        });
            });
    popup.show(this->getMainControl()->getGtkWindow());
}

void DocumentControl::save() {
    saveImpl(false, [](bool) {});
}

void DocumentControl::saveAs() {
    saveImpl(true, [](bool) {});
}

void DocumentControl::saveImpl(bool saveAs, std::function<void(bool)> callback) {
    // clear selection before saving
    clearSelectionEndText();

    this->doc->lock_shared();
    fs::path filepath = this->doc->getFilepath();
    this->doc->unlock_shared();

    auto doSave = [ctrl = this, cb = std::move(callback)]() {
        // clear selection before saving
        ctrl->clearSelectionEndText();

        ctrl->getMainControl()->schedule<SaveJob, JOB_PRIORITY_URGENT>(std::move(cb));
    };

    if (saveAs || filepath.empty()) {
        // No need to backup the old saved file, as there is none
        this->doc->lock_shared();
        this->doc->setCreateBackupOnSave(false);
        auto suggestedPath = this->doc->createSaveFoldername(this->getSettings()->getLastSavePath());
        suggestedPath /= this->doc->createSaveFilename(Document::XOPP, this->getSettings()->getDefaultSaveName());
        this->doc->unlock_shared();
        xoj::SaveExportDialog::showSaveFileDialog(this->getMainControl()->getGtkWindow(), getSettings(),
                                                  std::move(suggestedPath),
                                                  [doSave = std::move(doSave), ctrl = this](std::optional<fs::path> p) {
                                                      if (p && !p->empty()) {
                                                          ctrl->getSettings()->setLastSavePath(p->parent_path());
                                                          ctrl->doc->lock();
                                                          ctrl->doc->setFilepath(std::move(p.value()));
                                                          ctrl->doc->unlock();
                                                          doSave();
                                                      }
                                                  });
    } else {
        doSave();
    }
}

void DocumentControl::resetSavedStatus() {
    this->doc->lock_shared();
    auto filepath = this->doc->getFilepath();
    this->doc->unlock_shared();

    this->undoRedo->documentSaved();
    RecentManager::addRecentFileFilename(filepath);
    this->updateWindowTitle();
}

void DocumentControl::close(std::function<void(bool)> callback, const bool allowDestroy, const bool allowCancel,
                            const bool forceClose) {
    clearSelectionEndText();

    doc->lock_shared();
    auto const& file = doc->getEvMetadataFilename();
    doc->unlock_shared();
    metadata->storeMetadata(file, static_cast<int>(getCurrentPageNo()), zoom->getZoomReal());
    metadata->documentChanged();

    resetGeometryTool();

    bool safeToClose = forceClose || !undoRedo->isChanged();
    if (!safeToClose) {
        fs::path path = doc->getFilepath();
        const bool fileRemoved = !path.empty() && !fs::exists(path);
        const auto message = fileRemoved ? _("Document file was removed.") : _("This document is not saved yet.");
        const bool saveAs = fileRemoved || path.empty();
        const auto saveLabel = saveAs ? _("Save As...") : _("Save");

        enum { SAVE = 1, DISCARD, CANCEL };
        std::vector<XojMsgBox::Button> buttons = {{saveLabel, SAVE}, {_("Discard"), DISCARD}};
        if (allowCancel) {
            buttons.emplace_back(_("Cancel"), CANCEL);
        }
        XojMsgBox::askQuestion(getMainControl()->getGtkWindow(), message, std::string(), std::move(buttons),
                               [ctrl = this, saveAs, allowDestroy, callback = std::move(callback)](int response) {
                                   auto execAfter = [allowDestroy, ctrl, cb = std::move(callback)](bool saved) {
                                       if (saved && allowDestroy) {
                                           ctrl->closeDocument();
                                       }
                                       cb(saved);
                                   };
                                   if (response == SAVE) {
                                       ctrl->saveImpl(saveAs, std::move(execAfter));
                                       return;
                                   }
                                   bool safeToClose = response == DISCARD;
                                   execAfter(safeToClose);
                               });
    } else {
        if (allowDestroy) {
            this->closeDocument();
        }
        callback(true);
    }
}

void DocumentControl::closeDocument() {
    this->undoRedo->clearContents();

    this->getMainControl()->removeJobsFor(this);

    this->doc->lock();
    this->doc->clearDocument();
    this->doc->unlock();

    this->fireDocumentChanged(DOCUMENT_CHANGE_CLEARED);

    if (navHistory) {
        navHistory->reset();
    }

    this->undoRedoChanged();
}

void DocumentControl::clipboardPasteText(std::string text) {
    auto t = std::make_unique<Text>();
    t->setText(std::move(text));
    t->setFont(getSettings()->getFont());
    const auto& tool = getMainControl()->getToolHandler()->getTool(TOOL_TEXT);
    t->setColor(tool.getColor());
    t->setAlignment(tool.getTextAlignment());
    t->setJustify(tool.getTextJustify());

    clipboardPaste(std::move(t));
}

void DocumentControl::clipboardPasteImage(GdkPixbuf* img) {
    auto image = std::make_unique<Image>();
    xoj::util::GObjectSPtr<GdkPixbuf> pixbuf(gdk_pixbuf_apply_embedded_orientation(img), xoj::util::adopt);

    image->setImage(pixbuf.get());

    auto zoom100 = this->getMainControl()->getZoomBounds().value100percent;

    auto width = static_cast<double>(gdk_pixbuf_get_width(pixbuf.get())) / zoom100;
    auto height = static_cast<double>(gdk_pixbuf_get_height(pixbuf.get())) / zoom100;

    auto pageNr = getCurrentPageNo();
    if (pageNr == npos) {
        return;
    }

    this->doc->lock_shared();
    PageRef page = this->doc->getPage(pageNr);
    auto pageWidth = page->getWidth();
    auto pageHeight = page->getHeight();
    page.reset();  // No need to hold a ref to the page anymore
    this->doc->unlock_shared();

    // Size: 3/4 of the page size
    pageWidth = pageWidth * 3.0 / 4.0;
    pageHeight = pageHeight * 3.0 / 4.0;


    double scaling = 1;

    if (width > pageWidth) {
        scaling = pageWidth / width;
    }
    if (height > pageHeight) {
        scaling = std::min(scaling, pageHeight / height);
    }

    scaling /= zoom100;

    image->setTransformation(xoj::util::Matrix::SCALING(scaling, scaling));

    clipboardPaste(std::move(image));
}

void DocumentControl::clipboardPaste(ElementPtr e) {
    double x = 0;
    double y = 0;
    auto pageNr = getCurrentPageNo();
    if (pageNr == npos) {
        return;
    }

    XojPageView* view = tab->getXournal()->getViewFor(pageNr);
    if (view == nullptr) {
        return;
    }

    this->doc->lock_shared();
    PageRef page = this->doc->getPage(pageNr);
    Layer* layer = page->getSelectedLayer();
    this->doc->unlock_shared();

    tab->getXournal()->getPasteTarget(x, y);

    const auto& box = e->getBoundingBox();

    x = std::max(0.0, x - box.width / 2);
    y = std::max(0.0, y - box.height / 2);

    e->move(x, y);

    undoRedo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, e.get()));
    auto sel = SelectionFactory::createFromFloatingElement(this, page, layer, view, std::move(e));

    tab->getXournal()->setSelection(sel.release());
}

void DocumentControl::clipboardPasteXournal(ObjectInputStream& in) {
    auto pNr = getCurrentPageNo();
    if (pNr == npos && tab != nullptr) {
        return;
    }

    this->doc->lock();
    PageRef page = this->doc->getPage(pNr);
    Layer* layer = page->getSelectedLayer();

    XojPageView* view = tab->getXournal()->getViewFor(pNr);

    if (!view || !page) {
        this->doc->unlock();
        return;
    }

    auto selection = std::make_unique<EditSelection>(this, page, page->getSelectedLayer(), view);
    this->doc->unlock();

    try {
        ElementPtr element;
        std::string version = in.readString();
        if (version != PROJECT_STRING) {
            g_warning("Paste from Xournal Version %s to Xournal Version %s", version.c_str(), PROJECT_STRING);
        }

        selection->readSerialized(in);

        // document lock not needed anymore, because we don't change the document, we only change the selection

        int count = in.readInt();
        auto pasteAddUndoAction = std::make_unique<AddUndoAction>(page, false);
        // this will undo a group of elements that are inserted

        for (int i = 0; i < count; i++) {
            std::string name = in.getNextObjectName();
            element.reset();

            if (name == "Stroke") {
                element = std::make_unique<Stroke>();
            } else if (name == "Image") {
                element = std::make_unique<Image>();
            } else if (name == "TexImage") {
                element = std::make_unique<TexImage>();
            } else if (name == "Text") {
                element = std::make_unique<Text>();
            } else if (name == "Link") {
                element = std::make_unique<Link>();
            } else {
                throw InputStreamException(FS(FORMAT_STR("Get unknown object {1}") % name), __FILE__, __LINE__);
            }

            element->readSerialized(in);

            pasteAddUndoAction->addElement(layer, element.get(), layer->indexOf(element.get()));
            // Todo: unique_ptr
            selection->addElement(std::move(element), std::numeric_limits<Element::Index>::max());
        }
        undoRedo->addUndoAction(std::move(pasteAddUndoAction));

        double x = 0;
        double y = 0;
        // calculate x/y of paste target, see clipboardPaste(Element* e)
        tab->getXournal()->getPasteTarget(x, y);

        x = std::max(0.0, x - selection->getWidth() / 2);
        y = std::max(0.0, y - selection->getHeight() / 2);

        // calculate difference between current selection position and destination
        auto dx = x - selection->getXOnView();
        auto dy = y - selection->getYOnView();

        selection->moveSelection(dx, dy);
        // update all Elements (same procedure as moving a element selection by hand and releasing the mouse button)
        selection->mouseUp();

        tab->getXournal()->setSelection(selection.release());
    } catch (const std::exception& e) {
        g_warning("could not paste, Exception occurred: %s", e.what());
        Stacktrace::printStacktrace();
    }
}

auto DocumentControl::clipboardGetSelection() const -> EditSelection* { return tab->getXournal()->getSelection(); }

void DocumentControl::moveSelectionToLayer(size_t layerNo) {
    PageRef currentP = getCurrentPage();
    if (layerNo >= currentP->getLayerCount()) {
        return;
    }
    auto selection = tab->getXournal()->getSelection();
    if (!selection) {
        return;
    }

    auto* oldLayer = currentP->getSelectedLayer();
    auto* newLayer = currentP->getLayers().at(layerNo);
    auto moveSelUndo = std::make_unique<MoveSelectionToLayerUndoAction>(currentP, getLayerController(), oldLayer,
                                                                        currentP->getSelectedLayerId() - 1, layerNo);
    for (const auto* e: selection->getElementsView()) {
        moveSelUndo->addElement(newLayer, e, newLayer->indexOf(e));
    }
    undoRedo->addUndoAction(std::move(moveSelUndo));

    getLayerController()->switchToLay(layerNo + 1, /*hideShow=*/false, /*clearSelection=*/false);
}

void DocumentControl::deleteSelection() {
    if (tab) {
        tab->getXournal()->deleteSelection();
    }
}

void DocumentControl::clearSelection() {
    if (this->tab) {
        this->tab->getXournal()->clearSelection();
        this->getMainControl()->getWindow()->getPdfToolbox()->hide();
    }
}

void DocumentControl::addChangedDocumentListener(DocumentListener* dl) { this->changedDocumentListeners.push_back(dl); }

void DocumentControl::removeChangedDocumentListener(DocumentListener* dl) { this->changedDocumentListeners.remove(dl); }

void DocumentControl::setFillToSelection(int penfill, int highlighterfill) {
    xoj_assert(this->tab);
    if (auto* sel = this->tab->getXournal()->getSelection(); sel != nullptr) {
        undoRedo->addUndoAction(UndoActionPtr(sel->setFill(penfill, highlighterfill)));
    }
}

bool DocumentControl::setLineStyleOfSelection(const LineStyle& style) {
    if (this->tab && this->tab->getXournal()->getSelection()) {
        undoRedo->addUndoAction(this->tab->getXournal()->getSelection()->setLineStyle(style));
        return true;
    }
    return false;
}

void DocumentControl::setThicknessToSelection(double penthickness, double highlighterthickness,
                                              double eraserthickness) {
    xoj_assert(this->tab);
    if (auto* sel = this->tab->getXournal()->getSelection(); sel != nullptr) {
        undoRedo->addUndoAction(UndoActionPtr(sel->setSize(penthickness, highlighterthickness, eraserthickness)));
    }
}

void DocumentControl::setFontToTextAndSelection(const XojFont& font) {
    xoj_assert(this->tab);
    if (EditSelection* sel = this->tab->getXournal()->getSelection(); sel) {
        undoRedo->addUndoAction(UndoActionPtr(sel->setFont(font)));
    }

    if (TextEditor* editor = getTextEditor(); editor) {
        editor->setFont(font);
    }
}

auto DocumentControl::getUndoRedoHandler() const -> UndoRedoHandler* { return this->undoRedo.get(); }
auto DocumentControl::getZoomControl() const -> ZoomControl* { return this->zoom.get(); }
auto DocumentControl::getDocument() const -> Document* { return this->doc.get(); }
auto DocumentControl::getDocumentTab() const -> DocumentTab* { return this->tab; }
void DocumentControl::setDocumentTab(DocumentTab* tab) { this->tab = tab; }
auto DocumentControl::getMainControl() const -> MainControl* { return mainControl; }
auto DocumentControl::getSettings() const -> Settings* { return mainControl->getSettings(); }
auto DocumentControl::getToolHandler() const -> ToolHandler* { return mainControl->getToolHandler(); }


auto DocumentControl::getTextEditor() -> TextEditor* {
    if (this->tab) {
        return this->tab->getXournal()->getTextEditor();
    }
    return nullptr;
}

auto DocumentControl::getScrollHandler() const -> ScrollHandler* { return this->scrollHandler.get(); }

auto DocumentControl::getSidebar() const -> Sidebar* { return this->tab->getSidebar(); }

auto DocumentControl::getSearchBar() const -> SearchBar* { return this->tab->getSearchBar(); }

auto DocumentControl::getPageBackgroundChangeController() const -> PageBackgroundChangeController* {
    return this->pageBackgroundChangeController.get();
}

auto DocumentControl::getLayerController() const -> LayerController* { return this->layerController.get(); }

void DocumentControl::undo() { UndoRedoController::undo(this); }

void DocumentControl::redo() {
    this->clearSelectionEndText();
    UndoRedoController::redo(this);
}
