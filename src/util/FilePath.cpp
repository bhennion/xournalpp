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
#include "util/i18n.h"
#include "util/XojMsgBox.h"

std::u8string xoj::AbsolutePath::filename() const {
    auto p = Util::GFilename::assumeOwnerhip(g_file_get_basename(this->get())).toPath();
    return p ? p->u8string() : u8"";
}

std::string xoj::AbsolutePath::extension() const {
    auto p = Util::GFilename::assumeOwnerhip(g_file_get_basename(this->get())).toPath();
    return p ? p->extension() : "";
}

xoj::AbsolutePath xoj::AbsolutePath::operator/(const RelativePath& s) {
    if (empty()) {
        return AbsolutePath(g_file_parse_name(char_cast(s.c_str())), xoj::util::adopt);
    }
    GError* err;
    AbsolutePath newpath(g_file_get_child_for_display_name(this->get(), char_cast(s.c_str()), &err), xoj::util::adopt);
    if (err) {
        auto msg = std::string("Unable to concatenate paths\n") + err->message;
        g_error_free(err);
        throw std::runtime_error{msg};
    }
    return newpath;
}

xoj::AbsolutePath& xoj::AbsolutePath::operator/=(const RelativePath& s) {
    return *this = this->operator/(s);
}

xoj::RelativePath xoj::AbsolutePath::getAsRelativeTo(const AbsolutePath& base) const {
    return xoj::RelativePath(Util::GFilename::assumeOwnerhip(g_file_get_relative_path(this->get(), base.get())).toUTF8().value_or(u8""));
}

void xoj::fst::remove(const xoj::AbsolutePath& p) {
    GError* err = nullptr;
    g_file_delete(p.get(), nullptr, &err);
    if (err) {
        auto msg = std::string("Unable to delete file ") + p.iri().get() + "\n" + err->message;
        g_error_free(err);
        throw std::runtime_error{msg};
    }
}

auto xoj::fst::dump_content(xoj::AbsolutePath const& path, bool showErrorToUser)
-> std::optional<std::string> {
    GError* err = nullptr;
    xoj::util::GObjectSPtr<GBytes> data(g_file_load_bytes(path.get(), nullptr, nullptr, &err), xoj::util::adopt);
    if (err) {
        if (showErrorToUser) {
            const auto msg = FS(_F("Error reading \"{1}\":\n{2}") % path.iri().get() % err->message);
            XojMsgBox::showErrorToUser(nullptr, msg);
        }
        g_error_free(err);
        return std::nullopt;
    }
    xoj_assert(data);
    return std::string(static_cast<const char*>(g_bytes_get_data(data.get(), nullptr)), g_bytes_get_size(data.get()));
}

auto xoj::fst::current_path() -> xoj::AbsolutePath {
    return xoj::AbsolutePath(g_file_new_for_path("."), xoj::util::adopt);
}
