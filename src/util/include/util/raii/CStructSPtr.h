/*
 * Xournal++
 *
 * Smart pointers for C library structures (i.e. without ref/unref functions)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <utility>

#include <gtk/gtk.h>

namespace xoj::util {

inline namespace raii {
#define XOJ_SPTR_GENERATOR(class_name, type, deletion_func)  \
    namespace detail {                                           \
    struct class_name##Delete {                                  \
        auto operator()(type* ptr) const -> void {               \
            if (ptr) {                                           \
                deletion_func(ptr);                              \
            }                                                    \
        }                                                        \
    };                                                           \
    } /* namespace detail */                                     \
    using class_name##SPtr = std::unique_ptr<type, detail::class_name##Delete>

#define XOJ_SPTR_GENERATOR_TYPE(type, deletion_func) XOJ_SPTR_GENERATOR(type, type, deletion_func)

XOJ_SPTR_GENERATOR(CString, gchar, g_free);

};  // namespace raii
};  // namespace xoj::util
