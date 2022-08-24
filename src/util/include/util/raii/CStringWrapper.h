/*
 * Xournal++
 *
 * C-type strings wrapper
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

namespace xoj::util {

inline namespace raii {
    class CString {
    public:
        CString() = default;
        CString(char* s): data(s) {}
        CString(const CString&) = delete;
        CString(CString&& s): data(std::exchange(s.data, nullptr)) {}
        CString& operator=(const CString&) = delete;
        CString& operator=(CString&& s) {
            g_free(data);
            data = std::exchange(s.data, nullptr);
            return *this;
        }
        CString& operator=(char* s) {
            g_free(data);
            data = s;
            return *this;
        }
        ~CString() { g_free(data); }

        const char* get() { return data; }
        operator bool() { return data; }

        const char& operator[](size_t n) { return data[n]; }

    private:
        char* data = nullptr;
    };
};  // namespace raii
};  // namespace xoj::util
