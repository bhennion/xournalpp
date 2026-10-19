/*
 * Xournal++
 *
 * File types: mimetype/extension
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "i18n.h"

namespace xoj {
struct FileType {
    const char* extension;
    const char* mimetype;
    const char* description;
};

/// The filetypes we may save or export to, in SaveFileDialogs
namespace FileTypes {
// Use N_ for the descriptions so it is send to translators (but still constexpr)
constexpr FileType PDF = {"pdf", "application/pdf", N_("PDF files")};
constexpr FileType XOJ = {"xoj", "application/x-xojpp", N_("Xournal files")};
constexpr FileType XOPP = {"xopp", "application/x-xopp", N_("Xournal++ files")};
constexpr FileType XOPT = {"xopt", "application/x-xopt", N_("Xournal++ template")};
constexpr FileType SVG = {"svg", "image/svg+xml", N_("SVG graphics")};
constexpr FileType PNG = {"png", "image/png", N_("PNG graphics")};
constexpr FileType ZIP = {"zip", "application/zip", N_("ZIP archive")};
};  // namespace FileTypes
};
