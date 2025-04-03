#include "MuPdfExport.h"

#ifdef ENABLE_MUPDF

#include <ctime>
#include <sstream>  // for ostringstream, operator<<
#include <vector>   // for vector

#ifndef NDEBUG
#define NDEBUG
#include <mupdf/classes.h>
#undef NDEBUG
#else
#include <mupdf/classes.h>
#endif

#include "model/Document.h"     // for Document
#include "util/Assert.h"        // for xoj_assert
#include "util/Util.h"          // for npos
#include "util/i18n.h"          // for _
#include "util/safe_casts.h"    // for strict_cast
#include "util/serdesstream.h"  // for serdes_stream

#include "config.h"      // for PROJECT_STRING
#include "filesystem.h"  // for path

static constexpr auto XOBJECT_KEY = "XObject";
static constexpr auto CONTENTS_KEY = "Contents";
static constexpr auto FOREGROUND_BASENAME = "XoppForeground";

MuPdfExport::MuPdfExport(Document* doc, ProgressListener* progressListener): HybridPdfExport(doc, progressListener) {}

MuPdfExport::~MuPdfExport() = default;

static void ensureContentIsArrayOfIndirects(mupdf::PdfPage p) {
    if (auto cont = p.pdf_page_contents(); cont.pdf_is_dict()) {
        if (!cont.pdf_is_indirect()) {
            cont = p.doc().pdf_add_object(cont);
        }
        xoj_assert(cont.pdf_is_indirect());
        auto a = p.obj().pdf_dict_put_array(CONTENTS_KEY, 2);
        a.pdf_array_push(cont);
    } else if (cont.pdf_is_array()) {
        for (int i = 0; i < cont.pdf_array_len(); i++) {
            auto c = cont.pdf_array_get(i);
            if (!c.pdf_is_indirect()) {
                c = p.doc().pdf_add_object(c);
                xoj_assert(c.pdf_is_indirect());
                cont.pdf_array_put(i, c);
            }
        }
    } else {
        throw std::logic_error("PDF document page has invalid contents type");
    }
}

static void ensureResourcesAreDictOfIndirects(mupdf::PdfPage p) {
    auto res = p.pdf_page_resources();
    if (!res.pdf_is_dict()) {
        throw std::logic_error("PDF document page has invalid resources type");
    }
    for (int i = 0; i < res.pdf_dict_len(); i++) {
        if (auto r = res.pdf_dict_get_val(i); !r.pdf_is_indirect()) {
            res.pdf_dict_put(res.pdf_dict_get_key(i), p.doc().pdf_add_object(r));
        }
    }
}

/**
 * Duplicates the page p in its document and insert it at insertPosition
 * The resources and contents are linked (and not copied): changing an object on one page will affect the other
 * This does not duplicate the PdfAnnotations
 */
static mupdf::PdfObj duplicatePage(mupdf::PdfPage& source, int insertPosition) {
    auto cont = source.pdf_page_contents();
    xoj_assert(cont.pdf_is_array());  // ensureContentIsArrayOfIndirects() has been called already
    auto clonescontents = cont.pdf_copy_array();
    auto res = source.pdf_page_resources().pdf_copy_dict();

    auto clone = source.doc().pdf_add_page(source.pdf_bound_page(FZ_MEDIA_BOX), 0, res, mupdf::FzBuffer());
    clone.pdf_dict_puts(CONTENTS_KEY, clonescontents);
    source.doc().pdf_insert_page(insertPosition, clone);
    return clone;
}

static void reorderBackgrounds(mupdf::PdfDocument background,
                               const std::vector<HybridPdfExport::OutputPageInfo>& outputPageInfos) {
    size_t count = as_unsigned(background.pdf_count_pages());
    // Count occurrences of background pages: the user might have duplicated or removed pages
    auto occurrences = HybridPdfExport::countOccurrences(count, outputPageInfos);
    // We may need to shuffle the pages around: Remember the original pages addresses.
    std::vector<mupdf::PdfPage> backgroundPages;
    backgroundPages.reserve(count);
    for (int i = 0; i < as_signed(count); i++) {
        auto page = backgroundPages.emplace_back(background.pdf_load_page(i));
        const auto [nbOcc, hasOverlay] = occurrences[as_unsigned(i)];
        if (nbOcc >= 2) {
            // We want indirects (=references) to easily make a shallow clone
            ensureContentIsArrayOfIndirects(page);
            ensureResourcesAreDictOfIndirects(page);
        } else if (hasOverlay) {
            // We want an array to easily add the overlay
            ensureContentIsArrayOfIndirects(page);
        }
    }
    int nbValidPages = 0;
    for (int i = 0; i < as_signed(outputPageInfos.size()); i++) {
        auto n = outputPageInfos[as_unsigned(i)].pdfBackgroundPageNumber;
        if (n != npos) {
            background.pdf_insert_page(nbValidPages, backgroundPages[n].obj());
            if (occurrences[n].number >= 2) {
                // Keep a copy for next time
                duplicatePage(backgroundPages[n], nbValidPages + 1);
                backgroundPages[n] = background.pdf_load_page(nbValidPages + 1);
            }
            nbValidPages++;
            occurrences[n].number--;
        }
    }
    // The backgrounds are in place. Remove the unused backgrounds.
    // Nb: their resources and contents are not removed. It should be handled by garbage collection upon saving.
    background.pdf_delete_page_range(nbValidPages, background.pdf_count_pages());
}

bool MuPdfExport::overlayAndSave(const fs::path& saveDestination, std::stringstream& overlaystream,
                                 const std::vector<OutputPageInfo>& outputPageInfos) {
    try {
        auto overlaydata = overlaystream.str();
        mupdf::FzStream fzstream((const unsigned char*)overlaydata.data(), overlaydata.length());
        mupdf::PdfDocument overlay(fzstream);

        mupdf::PdfDocument background(
                doc->getPdfFilepath().u8string());  // TODO: this is passed to open() - UTF8 is ok?

        reorderBackgrounds(background, outputPageInfos);

        // Prepare xobjects representing the pages in the overlay document
        std::vector<mupdf::PdfObj> overlaysAsXObjects;
        overlaysAsXObjects.reserve(outputPageInfos.size());
        auto pageAsXObject = [&overlay](int pageNb) {
            mupdf::FzMatrix identityMatrix;
            auto p = overlay.pdf_load_page(pageNb);
            return overlay.pdf_new_xobject(p.pdf_bound_page(FZ_MEDIA_BOX), identityMatrix, p.pdf_page_resources(),
                                           p.pdf_page_contents().pdf_load_stream());
        };
        for (int n = 0, overlayPagesPrepared = 0; n < as_signed(outputPageInfos.size()); n++) {
            if (outputPageInfos[as_unsigned(n)].hasOverlay) {
                overlaysAsXObjects.emplace_back(pageAsXObject(overlayPagesPrepared++));
            }
        }

        auto graftMap = background.pdf_new_graft_map();  // Use a graft map to minimize data duplication

        for (size_t n = 0, overlayPagesConsumed = 0; n < outputPageInfos.size(); n++) {
            auto [hasOverlay, bgIndex] = outputPageInfos[n];
            if (hasOverlay) {
                if (bgIndex != npos) {
                    auto page = background.pdf_load_page(strict_cast<int>(n));
                    auto resources = page.pdf_page_resources();
                    auto xobjects = resources.pdf_dict_gets(XOBJECT_KEY);
                    if (!xobjects) {
                        xobjects = resources.pdf_dict_puts_dict(XOBJECT_KEY, 1);
                    } else if (xobjects.pdf_is_indirect()) {
                        // We need to copy the XObject dict so other copies of this page may have a different overlay
                        xobjects = xobjects.pdf_copy_dict();
                        page.pdf_page_resources().pdf_dict_puts(XOBJECT_KEY, xobjects);
                    }

                    // Find the first available XObject name
                    int firstAvailable = 0;
                    mupdf::PdfObj name;
                    do {
                        name = mupdf::PdfObj(
                                (std::string(FOREGROUND_BASENAME) + std::to_string(firstAvailable++)).data());
                    } while (
                            xobjects.pdf_dict_get(static_cast<const mupdf::PdfObj&>(name)));  // cast to avoid ambiguity

                    xobjects.pdf_dict_put(name,
                                          graftMap.pdf_graft_mapped_object(overlaysAsXObjects[overlayPagesConsumed]));

                    auto contents = page.pdf_page_contents();
                    xoj_assert(contents.pdf_is_array());
                    auto section = std::string("q\n1 0 0 1 0 0 cm\n/") + name.pdf_to_name() + " Do\nQ\n";
                    auto buffer = mupdf::FzBuffer::fz_new_buffer_from_copied_data((const unsigned char*)section.data(),
                                                                                  section.length());
                    mupdf::PdfObj empty;
                    contents.pdf_array_push(background.pdf_add_stream(buffer, empty, false));
                } else {
                    // Simply insert the overlay as a page
                    graftMap.pdf_graft_mapped_page(strict_cast<int>(n), overlay,
                                                   strict_cast<int>(overlayPagesConsumed));
                }
                overlayPagesConsumed++;
            }
        }

        background.super().fz_set_metadata(FZ_META_INFO_TITLE, doc->getFilepath().filename().u8string().data());
        background.super().fz_set_metadata(FZ_META_INFO_CREATOR,
                                           (std::string(PROJECT_STRING) + " muPDF exporter").data());

        std::time_t now = std::time(nullptr);
        auto* time = std::gmtime(&now);
        char buf[30];
        if (strftime(buf, 30, "D:%Y%m%d%H%M%SZ", time)) {  // See PDF 1.7 specs - section 7.9.4
            background.super().fz_set_metadata(FZ_META_INFO_MODIFICATIONDATE, buf);
        }

        mupdf::PdfWriteOptions opts;
        opts.do_garbage = 3;  // As much garbage collection as possible
        // Set other flags??
        background.pdf_save_document(saveDestination.u8string().data(), opts);
    } catch (const std::exception& e) {
        this->lastError = _("Error with overlay or final export:");
        this->lastError += std::string("\n") + e.what();
        return false;
    }
    return true;
}
#endif
