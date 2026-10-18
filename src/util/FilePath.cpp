/*
 * Xournal++
 *
 * Paths to resources
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "filesystem.h"

#include "util/PathUtil.h"
#include "util/StringUtils.h"

std::u8string xoj::FilePath::filename() const {
    auto p = Util::GFilename::assumeOwnerhip(g_file_get_basename(this->get())).toPath();
    return p ? p->u8string() : u8"";
}

std::string xoj::FilePath::extension() const {
    auto p = Util::GFilename::assumeOwnerhip(g_file_get_basename(this->get())).toPath();
    return p ? p->extension() : ""; }
