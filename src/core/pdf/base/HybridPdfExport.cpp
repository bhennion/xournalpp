#include "HybridPdfExport.h"

#include <algorithm>  // for min
#include <sstream>    // for stringstream
#include <vector>     // for vector

#include <cairo-pdf.h>  // for cairo_pdf_surface_create_for_stream

#include "control/jobs/ProgressListener.h"  // for ProgressListener
#include "model/Document.h"                 // for Document
#include "model/PageRef.h"                  // for PageRef
#include "model/XojPage.h"                  // for XojPage
#include "util/Assert.h"                    // for xoj_assert
#include "util/i18n.h"                      // for _
#include "util/serdesstream.h"              // for serdes_stream

#include "filesystem.h"  // for path


HybridPdfExport::HybridPdfExport(Document* doc, ProgressListener* progressListener):
        XojCairoPdfExport(doc, progressListener) {}

HybridPdfExport::~HybridPdfExport() = default;

static cairo_status_t writeFun(void* stream, const unsigned char* data, unsigned int length) {
    *static_cast<std::stringstream*>(stream) << std::string_view((char*)data, length);
    return CAIRO_STATUS_SUCCESS;
}

auto HybridPdfExport::startPdf(std::stringstream& stream) -> bool {
    this->surface = cairo_pdf_surface_create_for_stream(writeFun, &stream, 0, 0);
    this->cr = cairo_create(surface);

    configureCairoFontOptions();

    return cairo_surface_status(this->surface) == CAIRO_STATUS_SUCCESS;
}

auto HybridPdfExport::createPdf(fs::path const& file, const PageRangeVector& range, bool progressiveMode) -> bool {
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

auto HybridPdfExport::createPdf(fs::path const& file, bool progressiveMode) -> bool {
    PageRangeVector range = {{0, doc->getPageCount() - 1}};
    return createPdf(file, range, progressiveMode);
}
