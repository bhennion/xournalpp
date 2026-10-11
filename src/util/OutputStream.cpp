#include "util/OutputStream.h"

#include <cassert>
#include <cerrno>
#include <cstring>      // for strlen
#include <string_view>  // for string_view
#include <utility>      // for move

#include "util/i18n.h"  // for FS, _F
#include "util/raii/CStringWrapper.h"
#include "util/safe_casts.h"

OutputStream::OutputStream() = default;

OutputStream::~OutputStream() = default;

void OutputStream::write(const char* str) { write(str, std::strlen(str)); }

void OutputStream::write(const std::string_view sv) { write(sv.data(), sv.length()); }

void OutputStream::write(const std::u8string_view sv) { write(char_cast(sv.data()), sv.length()); }

////////////////////////////////////////////////////////
/// GzOutputStream /////////////////////////////////////
////////////////////////////////////////////////////////

GzOutputStream::GzOutputStream(xoj::FilePath file): file(std::move(file)) {
    g_debug("opening file: %s", xoj::util::OwnedCString::assumeOwnership(g_file_get_uri(this->file.get())).get());

    GError* err = nullptr;
    xoj::util::GObjectSPtr<GFileOutputStream> filestream(
            g_file_replace(this->file.get(), nullptr, false, G_FILE_CREATE_REPLACE_DESTINATION, nullptr, &err),
            xoj::util::adopt);
    if (err) {
        this->error = FS(_F("Error opening file: \"{1}\"") %
                         xoj::util::OwnedCString::assumeOwnership(g_file_get_uri(this->file.get())).get());
        this->error = this->error + "\n" + err->message;
        g_warning("failed to open stream: %s", error.c_str());
        g_error_free(err);
        return;
    }

    xoj::util::GObjectSPtr<GConverter> converter(G_CONVERTER(g_zlib_compressor_new(G_ZLIB_COMPRESSOR_FORMAT_GZIP, -1)),
                                                 xoj::util::adopt);
    this->compressionstream.reset(g_converter_output_stream_new(G_OUTPUT_STREAM(filestream.get()), converter.get()),
                                  xoj::util::adopt);
    g_debug("Opened streams %p/%p", this->compressionstream.get(), filestream.get());
}

GzOutputStream::~GzOutputStream() { this->close(); }

auto GzOutputStream::getLastError() const -> const std::string& { return this->error; }

void GzOutputStream::write(const char* data, size_t len) {
    xoj_assert(len != 0 && this->compressionstream);
    GError* err = nullptr;
    gsize written = 0;
    if (!g_output_stream_write_all(this->compressionstream.get(), data, len, &written, nullptr, &err)) {
        this->error = FS(_F("Error writing to file: \"{1}\"") %
                         xoj::util::OwnedCString::assumeOwnership(g_file_get_uri(this->file.get())).get());
        this->error = this->error + "\n" + err->message;
        g_warning("Error writing bytes: %s", err->message);
        g_error_free(err);
    } else {
        g_debug("wrote %zu bytes", len);
    }
}

void GzOutputStream::close() {
    if (!this->compressionstream) {
        return;
    }
    g_debug("closing stream %p", this->compressionstream.get());
    GError* err = nullptr;
    if (!g_output_stream_close(G_OUTPUT_STREAM(this->compressionstream.get()), nullptr, &err)) {
        this->error = FS(_F("Error closing file: \"{1}\"") %
                         xoj::util::OwnedCString::assumeOwnership(g_file_get_uri(this->file.get())).get());
        this->error = this->error + "\n" + err->message;
        g_warning("Error closing stream: %s", err->message);
        g_error_free(err);
    } else {
        g_debug("closed stream %p", this->compressionstream.get());
    }
    this->compressionstream.reset();
    g_debug("and destroyed it");
}
