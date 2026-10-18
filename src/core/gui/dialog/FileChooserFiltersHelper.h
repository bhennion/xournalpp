/*
 * Xournal++
 *
 * Helper functions to add filters to GtkFileChooserDialogs
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>

#include <gtk/gtk.h>

#include "util/i18n.h"
#include "util/FileTypes.h"

namespace xoj{
void addFilterForFile(GtkFileChooser* fc, const FileType& f);

inline void addFilterPdf(GtkFileChooser* fc) { addFilterForFile(fc, FileTypes::PDF); }
inline void addFilterXoj(GtkFileChooser* fc) { addFilterForFile(fc, FileTypes::XOJ); }
inline void addFilterXopp(GtkFileChooser* fc) { addFilterForFile(fc, FileTypes::XOPP); }
inline void addFilterXopt(GtkFileChooser* fc) { addFilterForFile(fc, FileTypes::XOPT); }
inline void addFilterSvg(GtkFileChooser* fc) { addFilterForFile(fc, FileTypes::SVG); }
inline void addFilterPng(GtkFileChooser* fc) { addFilterForFile(fc, FileTypes::PNG); }

void addFilterAllFiles(GtkFileChooser* fc);
void addFilterSupported(GtkFileChooser* fc);
void addFilterImages(GtkFileChooser* fc);  ///< All images supported by GdkPixbuf
void addFilterTex(GtkFileChooser* fc);
};  // namespace xoj
