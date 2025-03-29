#include "XojPdfExportFactory.h"

#include "model/Document.h"

#include "PoDoFoPdfExport.h"
#include "XojCairoPdfExport.h"  // for XojCairoPdfExport

class XojPdfExport;

XojPdfExportFactory::XojPdfExportFactory() = default;

XojPdfExportFactory::~XojPdfExportFactory() = default;

auto XojPdfExportFactory::createExport(Document* doc, ProgressListener* listener) -> std::unique_ptr<XojPdfExport> {
    if (!doc->getPdfFilepath().empty()) {
        return std::make_unique<PoDoFoPdfExport>(doc, listener);
    } else {
        return std::make_unique<XojCairoPdfExport>(doc, listener);
    }
}
