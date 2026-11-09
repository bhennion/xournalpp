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
#include <optional>

#include "util/raii/GObjectSPtr.h"
#include "util/raii/CStringWrapper.h"

// enable fs
namespace fs = std::filesystem;  // NOLINT(misc-unused-alias-decls)

namespace xoj {
    struct RelativePath: public std::u8string {
        explicit RelativePath(const std::u8string& s): std::u8string(s) {}
        explicit RelativePath(std::u8string&& s): std::u8string(std::move(s)) {}
    };
    struct IRI : std::u8string {};
    struct AbsolutePath : xoj::util::GFileSPtr {
        template<typename... Args>
        AbsolutePath(Args... args): xoj::util::GFileSPtr(args...) {}

        inline bool empty() const { return this->get() == nullptr; }
        inline AbsolutePath clone() const { return !empty() ? AbsolutePath(g_file_dup(this->get()), xoj::util::adopt) : AbsolutePath(); }
        /// Like URI but with UTF8 character not escaped (e.g. for displaying)
        inline xoj::util::OwnedCString iri() const { return xoj::util::OwnedCString::assumeOwnership(g_file_get_parse_name(this->get())); }
        inline xoj::util::OwnedCString uri() const { return xoj::util::OwnedCString::assumeOwnership(g_file_get_uri(this->get())); }
        inline AbsolutePath parent_path() const { return empty() ? AbsolutePath() : AbsolutePath(g_file_get_parent(this->get()), xoj::util::adopt); }
        std::string extension() const;
        std::u8string filename() const;
        AbsolutePath& operator/=(const RelativePath& s);
        AbsolutePath operator/(const RelativePath& s);
        RelativePath getAsRelativeTo(const AbsolutePath& base) const;
    };
    namespace fst {
        inline bool exists(const AbsolutePath& p) { return p && g_file_query_exists(p.get(), nullptr); }
        inline bool equivalent(const AbsolutePath& p, const AbsolutePath& q) { return p && q && g_file_equal(p.get(),q.get()); }
        inline bool is_directory(const AbsolutePath& p) { return p && g_file_query_file_type(p.get(), G_FILE_QUERY_INFO_NONE, nullptr) == G_FILE_TYPE_DIRECTORY; }
        void remove (const AbsolutePath& p);
        AbsolutePath current_path();

        std::optional<std::string> dump_content(const xoj::AbsolutePath& path, bool showErrorToUser = true);
    }
}
