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

bool OutputStream::write(const char* str) { return write(str, std::strlen(str)); }

bool OutputStream::write(const std::string_view sv) { return write(sv.data(), sv.length()); }

bool OutputStream::write(const std::u8string_view sv) { return write(char_cast(sv.data()), sv.length()); }

/// GLibBasedOutputStream ////

GLibBasedOutputStream::GLibBasedOutputStream() = default;

GLibBasedOutputStream::~GLibBasedOutputStream() { this->close(); }

auto GLibBasedOutputStream::getLastError() const -> const std::string& { return this->error; }

bool GLibBasedOutputStream::write(const char* data, size_t len) {
    xoj_assert(len != 0 && this->stream);
    GError* err = nullptr;
    gsize written = 0;
    if (!g_output_stream_write_all(this->stream.get(), data, len, &written, nullptr, &err)) {
        this->error = _("Error writing to GLibBasedOutputStream");
        this->error = this->error + "\n" + err->message;
        g_warning("Error writing bytes: %s", err->message);
        g_error_free(err);
        return false;
    }
    return true;
}

bool GLibBasedOutputStream::close() {
    if (!this->stream) {
        return true;
    }
    GError* err = nullptr;
    if (!g_output_stream_close(this->stream.get(), nullptr, &err)) {
        this->error = _("Error closing GLibBasedOutputStream");
        this->error = this->error + "\n" + err->message;
        g_warning("Error closing stream: %s", err->message);
        g_error_free(err);
        this->stream.reset();
        return false;
    }
    this->stream.reset();
    return true;
}

/// GzOutputStream ////

GzOutputStream::GzOutputStream(xoj::FilePath file): file(std::move(file)) {
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
    this->stream.reset(g_converter_output_stream_new(G_OUTPUT_STREAM(filestream.get()), converter.get()),
                                  xoj::util::adopt);
}

GzOutputStream::~GzOutputStream() = default;

///// FileOutputStream /////

FileOutputStream::FileOutputStream(xoj::FilePath file): file(std::move(file)) {
    GError* err = nullptr;
    this->stream.reset(G_OUTPUT_STREAM(g_file_replace(this->file.get(), nullptr, false, G_FILE_CREATE_REPLACE_DESTINATION, nullptr, &err)),
                                                         xoj::util::adopt);
    if (err) {
        this->error = FS(_F("Error opening file: \"{1}\"") %
        xoj::util::OwnedCString::assumeOwnership(g_file_get_uri(this->file.get())).get());
        this->error = this->error + "\n" + err->message;
        g_warning("failed to open stream: %s", error.c_str());
        g_error_free(err);
        return;
    }
}

FileOutputStream::~FileOutputStream() = default;
