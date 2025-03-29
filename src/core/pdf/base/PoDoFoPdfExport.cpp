#include "PoDoFoPdfExport.h"

#include <algorithm>  // for copy, min
#include <map>        // for map
#include <memory>     // for __shared_ptr_access
#include <sstream>    // for ostringstream, operator<<
#include <stack>      // for stack
#include <utility>    // for pair, make_pair
#include <vector>     // for vector

#include <cairo-pdf.h>    // for cairo_pdf_surface_set_met...
#include <glib-object.h>  // for g_object_unref
#include <podofo/podofo.h>

#include "control/jobs/ProgressListener.h"  // for ProgressListener
#include "model/Document.h"                 // for Document
#include "model/Layer.h"                    // for Layer
#include "model/LinkDestination.h"          // for LinkDestination, XojLinkDest
#include "model/PageRef.h"                  // for PageRef
#include "model/PageType.h"                 // for PageType
#include "model/XojPage.h"                  // for XojPage
#include "util/Assert.h"                    // for xoj_assert
#include "util/PathUtil.h"
#include "util/Util.h"          // for npos
#include "util/i18n.h"          // for _
#include "util/serdesstream.h"  // for serdes_stream
#include "view/DocumentView.h"  // for DocumentView

#include "config.h"      // for PROJECT_STRING
#include "filesystem.h"  // for path


PoDoFoPdfExport::PoDoFoPdfExport(Document* doc, ProgressListener* progressListener):
        XojCairoPdfExport(doc, progressListener) {}

PoDoFoPdfExport::~PoDoFoPdfExport() = default;

static cairo_status_t writeFun(void* stream, const unsigned char* data, unsigned int length) {
    *static_cast<std::stringstream*>(stream) << std::string_view((char*)data, length);
    return CAIRO_STATUS_SUCCESS;
}

auto PoDoFoPdfExport::startPdf(std::stringstream& stream) -> bool {
    this->surface = cairo_pdf_surface_create_for_stream(writeFun, &stream, 0, 0);
    this->cr = cairo_create(surface);

    configureCairoFontOptions();

    return cairo_surface_status(this->surface) == CAIRO_STATUS_SUCCESS;
}

/**
 * Duplicates the page p in its document and insert it at insertPosition
 * The resources and contents are linked (and not copied): changing an object on one page will affect the other
 * This does not duplicate the PdfAnnotations
 */
static PoDoFo::PdfPage& duplicatePage(PoDoFo::PdfPage& source, unsigned int insertPosition) {
    auto& clone = source.GetDocument().GetPages().CreatePageAt(insertPosition, source.GetRect());
    clone.SetMediaBox(source.GetMediaBox());
    if (auto* sourceContents = source.GetContents(); sourceContents != nullptr) {
        auto& cloneContents = clone.GetOrCreateContents().GetObject();
        if (cloneContents.IsArray()) {
            auto& array = cloneContents.GetArray();
            auto& ob = sourceContents->GetObject();
            if (ob.IsArray()) {
                for (auto&& y: ob.GetArray()) {
                    // Copy references and add others by creating a reference pointing to them
                    array.AddIndirectSafe(y);
                }
            } else if (ob.IsDictionary()) {
                // See source code from PoDoFo::PdfContents::CreateStreamForAppending(): this is supposed to work.
                array.AddIndirect(ob);
            }
        } else {
            // This should never happen: newly created pages in PoDoFo have a PdfArray contents
            throw std::logic_error(std::string(_("Linking content to a page whose content is not a PdfArray: ")) +
                                   cloneContents.GetDataTypeString().data());
        }
    }
    if (auto& resourcesP = source.GetResources(); true) {
        auto& resourcesQDict = clone.GetResources().GetDictionary();
        for (auto&& a: resourcesP.GetDictionary()) {
            resourcesQDict.AddKeyIndirectSafe(a.first, a.second);
        }
    }
    return clone;
}

/**
 * Creates a clone of overlay and adds it on top of background. External resources (e.g. fonts) are not duplicated.
 * The two pages should belong to the same PoDoFo::PdfDocument instance
 */
static void overlayPage(PoDoFo::PdfPage& overlay, PoDoFo::PdfPage& background) {
    auto xboj = overlay.GetDocument().CreateXObjectForm(overlay.GetMediaBox());
    xboj->FillFromPage(overlay);
    PoDoFo::PdfPainter painter;
    painter.SetCanvas(background);
    painter.DrawXObject(*xboj, 0, 0);
}

/**
 * Modifies output in place as follows:
 * Takes the pages of output as backgrounds and pages of overlay as layers to add to the backgrounds, according to the
 * overlay to background index provided
 *
 * Assumes  overlay.GetPages().GetCount() == overlayToBackgroundIndex.size()
 *
 * The function may throw (either from PoDoFo itself, or if the values in overlayToBackgroundIndex are not either npos
 * or smaller than output.GetPages().GetCount())
 *
 * Afterward, you will have output.GetPages().GetCount() == overlay.GetPages().GetCount()
 */
static void mergeBackgroundsAndOverlays(PoDoFo::PdfMemDocument& output, const PoDoFo::PdfDocument& overlay,
                                        const std::vector<size_t>& overlayToBackgroundIndex) {
    xoj_assert(overlay.GetPages().GetCount() == overlayToBackgroundIndex.size());
    auto& outputPages = output.GetPages();
    auto count = outputPages.GetCount();

    // We may need to shuffle the pages around: Remember the original pages addresses.
    std::vector<PoDoFo::PdfPage*> backgroundPages;
    backgroundPages.reserve(count);
    for (decltype(count) i = 0; i < count; i++) {
        backgroundPages.emplace_back(&outputPages.GetPageAt(i));
    }

    // Append all overlay pages: because PDF pages can share resources, this handles copying all the data and resources
    // into the new document without unnecessarily duplicating data.
    outputPages.AppendDocumentPages(overlay);

    // Count occurrences of background pages: the user might have duplicated or removed pages
    std::vector<size_t> occurrences(count, 0);
    for (auto n: overlayToBackgroundIndex) {
        if (n != npos) {
            if (n >= count) {
                throw std::out_of_range(_("PDF page number is out of range"));
            }
            occurrences[n]++;
        }
    }

    auto nextOverlayPageIndex = count;

    // Remove background pages that no longer appear
    for (size_t n = 0; n < count; n++) {
        if (occurrences[n] == 0) {
            outputPages.RemovePageAt(backgroundPages[n]->GetIndex());
            backgroundPages[n] = nullptr;
            nextOverlayPageIndex--;
        }
    }
    // Check that the overlay pages indeed start at nextOverlayPageIndex
    xoj_assert([&]() {
        auto it = std::find_if(backgroundPages.rbegin(), backgroundPages.rend(), [](auto* p) { return p != nullptr; });
        return it == backgroundPages.rend() || (*it)->GetIndex() == nextOverlayPageIndex - 1;
    }());

    /*
     * In order to keep the pdf small by avoiding unnecessarily duplicating data (e.g. fonts), we edit the document in
     * place. This results in the following somewhat complicated loop.
     * Beware the user might have duplicated/removed/moved pages...
     */
    for (decltype(count) i = 0; i < overlayToBackgroundIndex.size(); i++) {
        /* The invariants of this loop are: at the beginning of the scope
         *      * Pages with index [0,i) are finished (background + overlay)
         *      * Pages with index [i, nextOverlayPageIndex) are raw background pages (no overlay yet)
         *      * Pages with index [nextOverlayPageIndex, end) are overlays, not yet painted over their background
         *
         * Warning: (nextOverlayPageIndex - i == end - nextOverlayPageIndex) may not be true because backgrounds can be
         * used for several overlays and some overlays may have no background at all.
         */
        xoj_assert(i < nextOverlayPageIndex);
        if (auto n = overlayToBackgroundIndex[i]; n == npos) {
            // The xopp page has no pdf background: simply move the overlay into place
            outputPages.GetPageAt(nextOverlayPageIndex).MoveTo(i);
            nextOverlayPageIndex++;
        } else {
            xoj_assert(n < count);
            xoj_assert(occurrences[n] != 0);
            xoj_assert(backgroundPages[n] != nullptr);
            PoDoFo::PdfPage& bg = *backgroundPages[n];
            if (occurrences[n] > 1) {
                // There are other occurrences of this PDF background - We need to keep a clean clone
                backgroundPages[n] = &duplicatePage(bg, i + 1);
                // We added a page before the remaining overlay pages
                nextOverlayPageIndex++;
            } else {
                // This background will not be used again
                backgroundPages[n] = nullptr;
            }
            bg.MoveTo(i);
            overlayPage(outputPages.GetPageAt(nextOverlayPageIndex), bg);
            outputPages.RemovePageAt(nextOverlayPageIndex);  // Remove copy of the overlay page
            occurrences[n]--;                                // We consumed one occurrence of the background
        }
    }
    xoj_assert(std::all_of(occurrences.begin(), occurrences.end(), [](size_t o) { return o == 0; }));
    xoj_assert(std::all_of(backgroundPages.begin(), backgroundPages.end(), [](auto* p) { return p == nullptr; }));
    xoj_assert(nextOverlayPageIndex == overlayToBackgroundIndex.size());
    xoj_assert(outputPages.GetCount() == overlayToBackgroundIndex.size());

    output.CollectGarbage();  // Remove any unused resource
}

bool PoDoFoPdfExport::overlayAndSave(const fs::path& saveDestination, std::istream& overlaystream,
                                     const std::vector<size_t>& overlayToBackgroundMap) {
    try {
        PoDoFo::PdfMemDocument overlay;
        overlay.Load(std::make_shared<PoDoFo::StandardStreamDevice>(overlaystream));

        PoDoFo::PdfMemDocument output;
        output.Load(doc->getPdfFilepath().u8string());

        mergeBackgroundsAndOverlays(output, overlay, overlayToBackgroundMap);

        output.GetMetadata().SetTitle(PoDoFo::PdfString(doc->getFilepath().filename().u8string()));
        output.GetMetadata().SetCreator(PoDoFo::PdfString(PROJECT_STRING));

        output.Save(saveDestination.u8string());
    } catch (const std::exception& e) {
        this->lastError = _("Error with overlay or final export:");
        this->lastError += std::string("\n") + e.what();
        return false;
    }
    return true;
}

auto PoDoFoPdfExport::createPdf(fs::path const& file, const PageRangeVector& range, bool progressiveMode) -> bool {
    if (progressiveMode || exportBackground == EXPORT_BACKGROUND_NONE) {
        // For progressive mode or without any background, cairo export seems enough.
        return XojCairoPdfExport::createPdf(file, range, progressiveMode);
    }
    if (range.empty()) {
        this->lastError = _("No pages to export!");
        return false;
    }

    // Export the annotations to a PDF stream via cairo
    auto stream = serdes_stream<std::stringstream>();

    if (!startPdf(stream)) {
        this->lastError = _("Failed to initialize PDF Cairo surface");
        this->lastError += "\nCairo error: ";
        this->lastError += cairo_status_to_string(cairo_surface_status(this->surface));
        return false;
    }

    size_t count = 0;
    for (const auto& e: range) {
        xoj_assert(e.last >= e.first);  // Ok, when the PageRangeVector was the result of parsing
        count += e.last - e.first + 1;  // Not accurate, if e.last is > doc->getPageCount()
    }

    if (this->progressListener) {
        this->progressListener->setMaximumState(count);
    }

    size_t c = 0;
    std::vector<size_t> overlayToBackgroundIndex;
    overlayToBackgroundIndex.reserve(count);
    for (const auto& e: range) {
        auto max = std::min(e.last, doc->getPageCount() - 1);  // Should be e.last for parsed PageRangeVector
        for (size_t i = e.first; i <= max; i++) {
            exportPage(i, false);
            overlayToBackgroundIndex.emplace_back(doc->getPage(i)->getPdfPageNr());

            if (this->progressListener) {
                this->progressListener->setCurrentState(++c);
            }
        }
    }

    if (!endPdf()) {
        return false;
    }

    return overlayAndSave(file, stream, overlayToBackgroundIndex);
}

auto PoDoFoPdfExport::createPdf(fs::path const& file, bool progressiveMode) -> bool {
    PageRangeVector range = {{0, doc->getPageCount() - 1}};
    return createPdf(file, range, progressiveMode);
}
