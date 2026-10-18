#include "util/GzInputStream.h"

#include <cerrno>     // for errno
#include <cstring>    // for strerror
#include <stdexcept>  // for runtime_rrror
#include <string>     // for string

#include "util/Assert.h"  // for xoj_assert
#include "util/i18n.h"    // for FS, _F
#include "util/safe_casts.h"

#include "filesystem.h"  // for path


namespace xoj::util {

GLibBasedInputStream::GLibBasedInputStream() = default;
GLibBasedInputStream::~GLibBasedInputStream() {
    this->close();
}

static void throwGError(std::string msg, GError* err) {
    msg = msg + "\n" + err->message;
    g_error_free(err);
    throw std::runtime_error{msg};
}

auto GLibBasedInputStream::read(char* buffer, size_t len) -> ptrdiff_t {
    xoj_assert(len != 0 && this->stream);
    GError* err = nullptr;
    auto bytesread = g_input_stream_read(this->stream.get(), buffer, len, nullptr, &err);
    if (err) {
        throwGError(_("Error reading from GLibBasedOutputStream"), err);
    }
    xoj_assert(bytesread >= 0);
    return bytesread;
}

void GLibBasedInputStream::close() {
    if (!this->stream) {
        return;
    }
    GError* err = nullptr;
    if (!g_input_stream_close(this->stream.get(), nullptr, &err)) {
        throwGError(_("Error closing GLibBasedInputStream"), err);
    }
    this->stream.reset();
}

GzInputStream::GzInputStream(const xoj::FilePath& filepath): file(filepath) {
    GError* err = nullptr;
    this->stream.reset(
        G_INPUT_STREAM(g_file_read(this->file.get(), nullptr, &err)),
                                                        xoj::util::adopt);
    if (err) {
        throwGError(FS(_F("Error opening file: \"{1}\"") % this->file.iri().get()), err);
    }

    xoj::util::GObjectSPtr<GConverter> converter(G_CONVERTER(g_zlib_decompressor_new(G_ZLIB_COMPRESSOR_FORMAT_GZIP)),
                                                 xoj::util::adopt);

    // Let try the converter on some data, to make sure the file is compressed
    if (g_seekable_can_seek(G_SEEKABLE(this->stream.get()))) {
        auto startPos = g_seekable_tell(G_SEEKABLE(this->stream.get()));
        constexpr size_t BUFFER_SIZE = 128;
        char in[BUFFER_SIZE];
        auto s = this->read(in, BUFFER_SIZE);
        // Rewind
        if (GError* err = nullptr; !g_seekable_seek(G_SEEKABLE(this->stream.get()), startPos, G_SEEK_SET, nullptr, &err)) {
            throwGError(FS(_F("Could not seek in file: \"{1}\"") % this->file.iri().get()), err);
        }
        char out[BUFFER_SIZE];
        gsize sizes[2];
        if (g_converter_convert(converter.get(), in, static_cast<gsize>(s), out, BUFFER_SIZE, G_CONVERTER_NO_FLAGS, sizes, sizes + 1, nullptr) == G_CONVERTER_ERROR) {
            // The converter is not suitable for this file. Let's try no compression
            return;
        }
        g_converter_reset(converter.get());  // The g_converter_input_stream_new will read the header once again
    } else {
        g_message("File stream \"%s\" is not seekable: unable to test the compression", this->file.iri().get());
    }

    // Put the decompressor between the filestream and us
    this->stream.reset(g_converter_input_stream_new(this->stream.get(), converter.get()),
                       xoj::util::adopt);
}

bool GzInputStream::hasValidHeader() const {
    auto* fi = g_zlib_decompressor_get_file_info(G_ZLIB_DECOMPRESSOR(g_converter_input_stream_get_converter(G_CONVERTER_INPUT_STREAM(this->stream.get()))));
    return fi;
}

GzInputStream::~GzInputStream() = default;
}  // namespace xoj::util
