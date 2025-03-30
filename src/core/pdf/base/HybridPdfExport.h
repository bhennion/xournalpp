/*
 * Xournal++
 *
 * PDF Document Export Abstraction Interface - uses cairo for the annotations and overlay them on the original PDF using
 * another PDF library
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cstddef>  // for size_t
#include <sstream>

#include "util/ElementRange.h"  // for PageRangeVector

#include "XojCairoPdfExport.h"  // for XojPdfExport
#include "filesystem.h"         // for path

class Document;
class ProgressListener;

class HybridPdfExport: public XojCairoPdfExport {
public:
    HybridPdfExport(Document* doc, ProgressListener* progressListener);
    ~HybridPdfExport() override;

public:
    bool createPdf(fs::path const& file, bool progressiveMode) override;
    bool createPdf(fs::path const& file, const PageRangeVector& range, bool progressiveMode) override;

private:
    bool startPdf(std::stringstream& stream);
    virtual bool overlayAndSave(const fs::path& saveDestination, std::stringstream& overlaystream,
                                const std::vector<size_t>& overlayToBackgroundMap) = 0;
};
