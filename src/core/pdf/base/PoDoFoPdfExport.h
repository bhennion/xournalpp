/*
 * Xournal++
 *
 * PDF Document Export Abstraction Interface - use cairo for the annotations and overlay them on the original PDF using
 * PoDoFo to avoid information loss
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include <cairo.h>    // for CAIRO_VERSION, CAIRO_VERSION...
#include <gtk/gtk.h>  // for GtkTreeModel

#include "control/jobs/BaseExportJob.h"  // for ExportBackgroundType, EXPORT...
#include "util/ElementRange.h"           // for PageRangeVector

#include "XojCairoPdfExport.h"  // for XojPdfExport
#include "filesystem.h"         // for path

class Document;
class ProgressListener;

class PoDoFoPdfExport: public XojCairoPdfExport {
public:
    PoDoFoPdfExport(Document* doc, ProgressListener* progressListener);
    ~PoDoFoPdfExport() override;

public:
    bool createPdf(fs::path const& file, bool progressiveMode) override;
    bool createPdf(fs::path const& file, const PageRangeVector& range, bool progressiveMode) override;

private:
    bool startPdf(std::stringstream& stream);
    bool overlayAndSave(const fs::path& saveDestination, std::istream& overlaystream,
                        const std::vector<size_t>& overlayToBackgroundMap);
};
