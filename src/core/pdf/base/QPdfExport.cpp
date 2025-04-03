#include "QPdfExport.h"

#ifdef ENABLE_MUPDF

#include <algorithm>
#include <ctime>
#include <sstream>  // for ostringstream, operator<<
#include <vector>   // for vector

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFPageObjectHelper.hh>

#include "model/Document.h"   // for Document
#include "util/Assert.h"      // for xoj_assert
#include "util/Util.h"        // for npos
#include "util/i18n.h"        // for _
#include "util/safe_casts.h"  // for strict_cast

#include "config.h"      // for PROJECT_STRING
#include "filesystem.h"  // for path

QPdfExport::QPdfExport(Document* doc, ProgressListener* progressListener): HybridPdfExport(doc, progressListener) {}

QPdfExport::~QPdfExport() = default;

// static void ensureContentIsArrayOfIndirects(QPDFPageObjectHelper& p) {
//     if (auto cont = p.pdf_page_contents(); cont.pdf_is_dict()) {
//         if (!cont.pdf_is_indirect()) {
//             cont = p.doc().pdf_add_object(cont);
//         }
//         xoj_assert(cont.pdf_is_indirect());
//         auto a = p.obj().pdf_dict_put_array(CONTENTS_KEY, 2);
//         a.pdf_array_push(cont);
//     } else if (cont.pdf_is_array()) {
//         for (int i = 0; i < cont.pdf_array_len(); i++) {
//             auto c = cont.pdf_array_get(i);
//             if (!c.pdf_is_indirect()) {
//                 c = p.doc().pdf_add_object(c);
//                 xoj_assert(c.pdf_is_indirect());
//                 cont.pdf_array_put(i, c);
//             }
//         }
//     } else {
//         throw std::logic_error("PDF document page has invalid contents type");
//     }
// }
//
// static void ensureResourcesAreDictOfIndirects(mupdf::PdfPage p) {
//     auto res = p.pdf_page_resources();
//     if (!res.pdf_is_dict()) {
//         throw std::logic_error("PDF document page has invalid resources type");
//     }
//     for (int i = 0; i < res.pdf_dict_len(); i++) {
//         if (auto r = res.pdf_dict_get_val(i); !r.pdf_is_indirect()) {
//             res.pdf_dict_put(res.pdf_dict_get_key(i), p.doc().pdf_add_object(r));
//         }
//     }
// }

/**
 * Duplicates the page p in its document and insert it at insertPosition
 * The resources and contents are linked (and not copied): changing an object on one page will affect the other
 * This does not duplicate the PdfAnnotations
 */
// static mupdf::PdfObj duplicatePage(mupdf::PdfPage& source, int insertPosition) {
//     auto cont = source.pdf_page_contents();
//     xoj_assert(cont.pdf_is_array());  // ensureContentIsArrayOfIndirects() has been called already
//     auto clonescontents = cont.pdf_copy_array();
//     auto res = source.pdf_page_resources().pdf_copy_dict();
//
//     auto clone = source.doc().pdf_add_page(source.pdf_bound_page(FZ_MEDIA_BOX), 0, res, mupdf::FzBuffer());
//     clone.pdf_dict_puts(CONTENTS_KEY, clonescontents);
//     source.doc().pdf_insert_page(insertPosition, clone);
//     return clone;
// }

static void reorderBackgrounds(QPDF& background, const std::vector<HybridPdfExport::OutputPageInfo>& outputPageInfos) {
    auto pageHelper = QPDFPageDocumentHelper(background);
    // We may need to shuffle the pages around: Remember the original pages addresses.
    auto backgroundPages = pageHelper.getAllPages();
    auto count = backgroundPages.size();
    // Count occurrences of background pages: the user might have duplicated or removed pages
    auto occurrences = HybridPdfExport::countOccurrences(count, outputPageInfos);

    // for (size_t i = 0; i < count; i++) {
    //     const auto& occ = occurrences[i];
    //     if (occ.hasOverlay) {
    //         // We want an array to easily add the overlay
    //         ensureContentIsArrayOfIndirects(page);
    //         if (occ.number >= 2) {
    //             // We want indirects (=references) to easily make a shallow clone
    //             ensureResourcesAreDictOfIndirects(page);
    //         }
    //     }
    // }
    size_t nbValidPages = 0;
    std::optional<QPDFPageObjectHelper> lastPageSet;
    for (size_t i = 0; i < outputPageInfos.size(); i++) {
        auto n = outputPageInfos[i].pdfBackgroundPageNumber;
        if (n != npos) {
            const auto& ps = background.getAllPages();
            xoj_assert(nbValidPages < ps.size());
            if (!ps[nbValidPages].isSameObjectAs(backgroundPages[n])) {
                if (nbValidPages > 0) {
                    auto prevPage = ps[nbValidPages - 1];
                    pageHelper.removePage(backgroundPages[n]);
                    pageHelper.addPageAt(backgroundPages[n], false, prevPage);
                } else {
                    pageHelper.removePage(backgroundPages[n]);
                    pageHelper.addPage(backgroundPages[n], true);
                }
            }
            if (occurrences[n].number >= 2) {
                // Keep a copy for next time
                backgroundPages[n].getObjectHandle().makeResourcesIndirect(background);
                backgroundPages[n] = backgroundPages[n].shallowCopyPage();
                pageHelper.addPage(backgroundPages[n], false);  // Append the copy
                xoj_assert(background.getAllPages().back().isSameObjectAs(backgroundPages[n]));
            }
            nbValidPages++;
            occurrences[n].number--;
        }
    }

    // The backgrounds are in place. Remove the unused backgrounds.
    auto pages = pageHelper.getAllPages();
    for (auto it = std::next(pages.begin(), as_signed(nbValidPages)); it != pages.end(); it++) {
        pageHelper.removePage(*it);
    }
    xoj_assert(as_signed(background.getAllPages().size()) ==
               std::count_if(outputPageInfos.begin(), outputPageInfos.end(),
                             [](auto&& a) { return a.pdfBackgroundPageNumber != npos; }));
}

bool QPdfExport::overlayAndSave(const fs::path& saveDestination, std::stringstream& overlaystream,
                                const std::vector<OutputPageInfo>& outputPageInfos) {
    try {
        auto overlaydata = overlaystream.str();  // Should probably be kept alive until we are done (not sure)
        QPDF overlay;
        overlay.processMemoryFile("overlay", overlaydata.data(), overlaydata.length());

        QPDF background;
        background.processFile(doc->getPdfFilepath().u8string().c_str());  // TODO: UTF8 is ok?

        reorderBackgrounds(background, outputPageInfos);

        // Prepare xobjects representing the pages in the overlay document
        std::vector<QPDFObjectHandle> overlaysAsXObjects;
        auto overlayPages = QPDFPageDocumentHelper(overlay).getAllPages();
        overlaysAsXObjects.reserve(overlayPages.size());
        for (auto&& p: overlayPages) {
            overlaysAsXObjects.emplace_back(p.getFormXObjectForPage());
        }

        auto outputPages = QPDFPageDocumentHelper(background).getAllPages();
        for (size_t n = 0, overlayPagesConsumed = 0; n < outputPageInfos.size(); n++) {
            auto [hasOverlay, bgIndex] = outputPageInfos[n];
            if (hasOverlay) {
                if (bgIndex != npos) {
                    auto page = QPDFPageObjectHelper(background.getAllPages()[n]);
                    // See qpdf/examples/pdf-overlay-page.cc

                    // Find a unique resource name for the new form XObject
                    QPDFObjectHandle resources = page.getAttribute("/Resources", true);
                    int min_suffix = 1;
                    std::string name = resources.getUniqueResourceName("/Fx", min_suffix);

                    auto localXObj = background.copyForeignObject(overlaysAsXObjects[overlayPagesConsumed]);

                    // Generate content to place the form XObject centered within destination page's trim box.
                    QPDFMatrix m;
                    std::string content =
                            page.placeFormXObject(localXObj, name, page.getMediaBox().getArrayAsRectangle(), m);
                    if (!content.empty()) {
                        // Append the content to the page's content. Surround the original content with q...Q to
                        // the new content from the page's original content.
                        resources.mergeResources("<< /XObject << >> >>"_qpdf);
                        resources.getKey("/XObject").replaceKey(name, localXObj);
                        page.addPageContents(background.newStream("q\n"), true);
                        page.addPageContents(background.newStream("\nQ\n" + content), false);
                    }
                } else {
                    // Simply insert the overlay as a page
                    if (n == 0) {
                        QPDFPageDocumentHelper(background).addPage(overlayPages[overlayPagesConsumed], true);
                    } else {
                        QPDFPageDocumentHelper(background)
                                .addPageAt(overlayPages[overlayPagesConsumed], false, background.getAllPages()[n - 1]);
                    }
                }
                overlayPagesConsumed++;
            }
        }

        // background.super().fz_set_metadata(FZ_META_INFO_TITLE, doc->getFilepath().filename().u8string().data());
        // background.super().fz_set_metadata(FZ_META_INFO_CREATOR,
        //                                    (std::string(PROJECT_STRING) + " muPDF exporter").data());
        //
        // std::time_t now = std::time(nullptr);
        // auto* time = std::gmtime(&now);
        // char buf[30];
        // if (strftime(buf, 30, "D:%Y%m%d%H%M%SZ", time)) {  // See PDF 1.7 specs - section 7.9.4
        //     background.super().fz_set_metadata(FZ_META_INFO_MODIFICATIONDATE, buf);
        // }
        //
        // mupdf::PdfWriteOptions opts;
        // opts.do_garbage = 3;  // As much garbage collection as possible
        // // Set other flags??

        QPDFWriter writer(background, saveDestination.u8string().data());  // is UTF8 ok?
        writer.write();
    } catch (const std::exception& e) {
        this->lastError = _("Error with overlay or final export:");
        this->lastError += std::string("\n") + e.what();
        return false;
    }
    return true;
}
#endif
