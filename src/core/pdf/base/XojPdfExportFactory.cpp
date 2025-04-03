#include "XojPdfExportFactory.h"

#include "model/Document.h"

#include "MuPdfExport.h"
#include "PoDoFoPdfExport.h"
#include "QPdfExport.h"
#include "XojCairoPdfExport.h"  // for XojCairoPdfExport

class XojPdfExport;

XojPdfExportFactory::XojPdfExportFactory() = default;

XojPdfExportFactory::~XojPdfExportFactory() = default;

auto XojPdfExportFactory::createExport(Document* doc, ProgressListener* listener, bool podofo, bool mupdf, bool qpdf)
        -> std::unique_ptr<XojPdfExport> {
    if (!doc->getPdfFilepath().empty()) {
#ifdef ENABLE_PODOFO
        if (podofo) {
            return std::make_unique<PoDoFoPdfExport>(doc, listener);
        }
#endif
#ifdef ENABLE_MUPDF
        if (mupdf) {
            return std::make_unique<MuPdfExport>(doc, listener);
        }
#endif
#ifdef ENABLE_QPDF
        if (qpdf) {
            return std::make_unique<QPdfExport>(doc, listener);
        }
#endif
    }
    return std::make_unique<XojCairoPdfExport>(doc, listener);
}
