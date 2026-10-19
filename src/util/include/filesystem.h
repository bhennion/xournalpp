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

#include <filesystem>  // IWYU pragma: export

#include "util/raii/GObjectSPtr.h"
#include "util/raii/CStringWrapper.h"

// enable fs
namespace fs = std::filesystem;  // NOLINT(misc-unused-alias-decls)

namespace xoj {
    struct FilePath : xoj::util::GFileSPtr {
        template<typename... Args>
        FilePath(Args... args): xoj::util::GFileSPtr(args...) {}

        inline bool empty() const { return this->get() == nullptr; }
        inline FilePath clone() const { return !empty() ? FilePath(g_file_dup(this->get()), xoj::util::adopt) : FilePath(); }
        /// Like URI but with UTF8 character not escaped (e.g. for displaying)
        inline xoj::util::OwnedCString iri() const { return xoj::util::OwnedCString::assumeOwnership(g_file_get_parse_name(this->get())); }
        inline xoj::util::OwnedCString uri() const { return xoj::util::OwnedCString::assumeOwnership(g_file_get_uri(this->get())); }
        inline FilePath parent_path() const { return empty() ? FilePath() : FilePath(g_file_get_parent(this->get()), xoj::util::adopt); }
        std::string extension() const;
        std::u8string filename() const;
        FilePath& operator/=(const std::u8string& s);
        FilePath operator/(const std::u8string& s);
    };
    namespace fst {
        inline bool exists(const FilePath& p) { return p && g_file_query_exists(p.get(), nullptr); }
        inline bool equivalent(const FilePath& p, const FilePath& q) { return p && q && g_file_equal(p.get(),q.get()); }
        inline bool is_directory(const FilePath& p) { return p && g_file_query_file_type(p.get(), G_FILE_QUERY_INFO_NONE, nullptr) == G_FILE_TYPE_DIRECTORY; }
        void remove (const FilePath& p);
    }
}
