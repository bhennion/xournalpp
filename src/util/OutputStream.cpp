#include "util/OutputStream.h"

#include <cassert>
#include <cerrno>
#include <cstring>      // for strlen
#include <string_view>  // for string_view
#include <utility>      // for move

#include "util/GzUtil.h"  // for GzUtil
#include "util/i18n.h"    // for FS, _F
#include "util/safe_casts.h"
#include "util/raii/CStringWrapper.h"

OutputStream::OutputStream() = default;

OutputStream::~OutputStream() = default;

void OutputStream::write(const char* str) { write(str, std::strlen(str)); }

void OutputStream::write(const std::string_view sv) { write(sv.data(), sv.length()); }

void OutputStream::write(const std::u8string_view sv) { write(char_cast(sv.data()), sv.length()); }

////////////////////////////////////////////////////////
/// GzOutputStream /////////////////////////////////////
////////////////////////////////////////////////////////

GzOutputStream::GzOutputStream(xoj::util::GFileSPtr file): file(std::move(file)) {
    g_debug("opening file: %s", xoj::util::OwnedCString::assumeOwnership(g_file_get_uri(this->file.get())).get());

    GError* err = nullptr;
    this->str.reset(g_file_replace (this->file.get(), nullptr, false, G_FILE_CREATE_REPLACE_DESTINATION, nullptr,
        &err), xoj::util::adopt);
    if (err) {
        this->error = FS(_F("Error opening file: \"{1}\"") % xoj::util::OwnedCString::assumeOwnership(g_file_get_uri(this->file.get())).get());
        this->error = this->error + "\n" + err->message;
        g_warning("failed to open stream: %s", error.c_str());
        g_error_free(err);
    } else {
        g_debug("Opened stream %p", this->str.get());
    }
}

GzOutputStream::~GzOutputStream() {
    this->close();
}

auto GzOutputStream::getLastError() const -> const std::string& { return this->error; }

void GzOutputStream::write(const char* data, size_t len) {
    xoj_assert(len != 0 && this->str);
    // std::vector<unsigned char> compressedData(compressBound(len));
    // unsigned long s = compressedData.size();
    // auto ret = compress(compressedData.data(), &s, (const unsigned char*)data, len);
    // if (ret != Z_OK) {
    //     this->error = "Compression failed: ";
    //     this->error += ret == Z_MEM_ERROR ? "Z_MEM_ERROR" : ret == Z_BUF_ERROR ? "Z_BUF_ERROR" : "Unknown error";
    //     return;
    // }
    GError* err = nullptr;
    gsize written = 0;
    if (!g_output_stream_write_all(G_OUTPUT_STREAM(this->str.get()), data, len, &written, nullptr, &err)) {
        this->error = FS(_F("Error writing to file: \"{1}\"") % g_file_get_uri(this->file.get()));
        this->error = this->error + "\n" + err->message;
        g_warning("Error writing bytes: %s", err->message);
        g_error_free(err);
    } else {
        g_debug("wrote %zu bytes", len);
    }
}

void GzOutputStream::close() {
    if (!this->str) {
        return;
    }
    g_debug("closing stream %p", this->str.get());
    GError* err = nullptr;
    if (!g_output_stream_close (G_OUTPUT_STREAM(this->str.get()), nullptr, &err)) {
        this->error = FS(_F("Error closing file: \"{1}\"") % g_file_get_uri(this->file.get()));
        this->error = this->error + "\n" + err->message;
        g_debug("Error closing stream: %s", err->message);
        g_error_free(err);
    } else {
        g_debug("closed stream %p", this->str.get());
    }
    this->str.reset();
    g_debug("and destroyed it");
}
