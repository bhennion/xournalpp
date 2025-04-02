#include "PoDoFoPdfExport.h"

#ifdef ENABLE_PODOFO

#include <algorithm>  // for any_of, find_if, all_of
#include <sstream>    // for stringstream
#include <vector>     // for vector

#include <podofo/podofo.h>

#include "model/Document.h"     // for Document
#include "util/Assert.h"        // for xoj_assert
#include "util/Util.h"          // for npos
#include "util/i18n.h"          // for _
#include "util/serdesstream.h"  // for serdes_stream

#include "config.h"      // for PROJECT_STRING
#include "filesystem.h"  // for path


PoDoFoPdfExport::PoDoFoPdfExport(Document* doc, ProgressListener* progressListener):
        HybridPdfExport(doc, progressListener) {}

PoDoFoPdfExport::~PoDoFoPdfExport() = default;

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
 * Assumes  overlay.GetPages().GetCount() == outputPageInfos.size()
 *
 * The function may throw (either from PoDoFo itself, or if the values in outputPageInfos are not either npos
 * or smaller than output.GetPages().GetCount())
 *
 * Afterward, you will have output.GetPages().GetCount() == overlay.GetPages().GetCount()
 */
static void mergeBackgroundsAndOverlays(PoDoFo::PdfMemDocument& output, const PoDoFo::PdfDocument& overlay,
                                        const std::vector<HybridPdfExport::OutputPageInfo>& outputPageInfos) {
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
    // (At this time, PoDoFo does not implement something like mupdf's graft map)
    outputPages.AppendDocumentPages(overlay);

    // Count occurrences of background pages: the user might have duplicated or removed pages
    auto occurrences = HybridPdfExport::countOccurrences(count, outputPageInfos);

    auto nextOverlayPageIndex = count;

    // Remove background pages that no longer appear
    for (size_t n = 0; n < count; n++) {
        if (occurrences[n].number == 0) {
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
    for (decltype(count) i = 0; i < outputPageInfos.size(); i++) {
        /* The invariants of this loop are: at the beginning of the scope
         *      * Pages with index [0,i) are finished (background + overlay)
         *      * Pages with index [i, nextOverlayPageIndex) are raw background pages (no overlay yet)
         *      * Pages with index [nextOverlayPageIndex, end) are overlays, not yet painted over their background
         *
         * Warning: (nextOverlayPageIndex - i == end - nextOverlayPageIndex) may not be true because backgrounds can be
         * used for several overlays and some overlays may have no background at all.
         */
        xoj_assert(i < nextOverlayPageIndex);
        auto [hasOverlay, n] = outputPageInfos[i];
        if (n == npos) {
            xoj_assert(hasOverlay);
            // The xopp page has no pdf background: simply move the "overlay" into place
            outputPages.GetPageAt(nextOverlayPageIndex).MoveTo(i);
            nextOverlayPageIndex++;
        } else {
            xoj_assert(n < count);
            xoj_assert(occurrences[n].number != 0);
            xoj_assert(backgroundPages[n] != nullptr);
            PoDoFo::PdfPage& bg = *backgroundPages[n];
            if (occurrences[n].number > 1) {
                // There are other occurrences of this PDF background - We need to keep a clean clone
                backgroundPages[n] = &duplicatePage(bg, i + 1);
                // We added a page before the remaining overlay pages
                nextOverlayPageIndex++;
            } else {
                // This background will not be used again. Only for xoj_assert
                backgroundPages[n] = nullptr;
            }
            bg.MoveTo(i);
            if (hasOverlay) {
                overlayPage(outputPages.GetPageAt(nextOverlayPageIndex), bg);
                outputPages.RemovePageAt(nextOverlayPageIndex);  // Remove copy of the overlay page
            }
            occurrences[n].number--;  // We consumed one occurrence of the background
        }
    }
    xoj_assert(std::all_of(occurrences.begin(), occurrences.end(), [](const auto& o) { return o.number == 0; }));
    xoj_assert(std::all_of(backgroundPages.begin(), backgroundPages.end(), [](auto* p) { return p == nullptr; }));
    xoj_assert(nextOverlayPageIndex == outputPageInfos.size());
    xoj_assert(outputPages.GetCount() == outputPageInfos.size());

    output.CollectGarbage();  // Remove any unused resource
}

bool PoDoFoPdfExport::overlayAndSave(const fs::path& saveDestination, std::stringstream& overlaystream,
                                     const std::vector<OutputPageInfo>& outputPageInfos) {
    try {
        PoDoFo::PdfMemDocument output;
        output.Load(doc->getPdfFilepath().u8string());

        PoDoFo::PdfMemDocument overlay;
        // In case no pages had an overlay, the stream contains a single empty page. We do not want that page.
        if (std::any_of(outputPageInfos.begin(), outputPageInfos.end(), [](auto&& i) { return i.hasOverlay; })) {
            overlay.Load(std::make_shared<PoDoFo::StandardStreamDevice>(static_cast<std::istream&>(overlaystream)));
        }

        mergeBackgroundsAndOverlays(output, overlay, outputPageInfos);

        output.GetMetadata().SetTitle(PoDoFo::PdfString(doc->getFilepath().filename().u8string()));
        output.GetMetadata().SetCreator(PoDoFo::PdfString(std::string(PROJECT_STRING) + " PoDoFo exporter"));

        output.Save(saveDestination.u8string());
    } catch (const std::exception& e) {
        this->lastError = _("Error with overlay or final export:");
        this->lastError += std::string("\n") + e.what();
        return false;
    }
    return true;
}
#endif
