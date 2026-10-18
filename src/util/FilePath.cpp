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

#include "filesystem.h"

#include <string>

#include "util/PathUtil.h"

std::u8string xoj::FilePath::filename() const {
    auto p = Util::GFilename::assumeOwnerhip(g_file_get_basename(this->get())).toPath();
    return p ? p->u8string() : u8"";
}

std::string xoj::FilePath::extension() const {
    auto p = Util::GFilename::assumeOwnerhip(g_file_get_basename(this->get())).toPath();
    return p ? p->extension() : "";
}

xoj::FilePath& xoj::FilePath::operator/=(const std::u8string& s) {
    GError* err;
    FilePath newpath(g_file_get_child_for_display_name(this->get(), char_cast(s.c_str()), &err), xoj::util::adopt);
    if (err) {
        auto msg = std::string("Unable to concatenate paths\n") + err->message;
        g_error_free(err);
        throw std::runtime_error{msg};
    }
    return *this = std::move(newpath);
}
