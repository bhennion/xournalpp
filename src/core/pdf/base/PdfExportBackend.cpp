#include "PdfExportBackend.h"

#include "util/i18n.h"

#include "config-features.h"

ExportBackend ExportBackend::fromString(const char* str) {
    return str != nullptr ? fromString(std::string_view(str)) : ExportBackend(ExportBackend::DEFAULT);
}

ExportBackend ExportBackend::fromString(std::string_view str) {
    if (str == "cairo") {
        return ExportBackend::CAIRO;
    }
#ifdef ENABLE_QPDF
    if (str == "qpdf") {
        return ExportBackend::QPDF;
    }
#endif
#ifdef ENABLE_PODOFO
    if (str == "podofo") {
        return ExportBackend::PODOFO;
    }
#endif
#ifdef ENABLE_MUPDF
    if (str == "mupdf") {
        return ExportBackend::MUPDF;
    }
#endif
    if (str != DEFAULT_ID_STRING && !str.empty()) {
        g_warning("%s", (_F("Unknown pdf backend: {1}. Available backends are: {2}. Using default backend.") % str %
                         listAvailableBackends())
                                .c_str());
    }
    return ExportBackend::DEFAULT;
}

const char* ExportBackend::listAvailableBackends() {
    static const char* availablePdfExportBackends = "cairo"
#ifdef ENABLE_QPDF
                                                    " qpdf"
#endif
#ifdef ENABLE_MUPDF
                                                    " mupdf"
#endif
#ifdef ENABLE_PODOFO
                                                    " podofo"
#endif
            ;
    return availablePdfExportBackends;
}

std::vector<std::pair<const char*, const char*>> ExportBackend::getPrettyNamesOfAvailableBackends() {
    std::vector<std::pair<const char*, const char*>> res;
    res.emplace_back(DEFAULT_ID_STRING, _("Default"));
    res.emplace_back("cairo", "Cairo");
#ifdef ENABLE_QPDF
    res.emplace_back("qpdf", "QPDF");
#endif
#ifdef ENABLE_MUPDF
    res.emplace_back("mupdf", "mupdf");
#endif
#ifdef ENABLE_PODOFO
    res.emplace_back("podofo", "PoDoFo");
#endif
    return res;
}
