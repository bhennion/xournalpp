#include "util/FileInputStream.h"

#include <cerrno>     // for errno
#include <cstring>    // for strerror
#include <stdexcept>  // for runtime_rrror
#include <string>     // for string

#include "util/Assert.h"  // for xoj_assert
#include "util/i18n.h"    // for FS, _F
#include "util/safe_casts.h"

#include "filesystem.h"  // for path


namespace xoj::util {

FileInputStream::FileInputStream() = default;

void FileInputStream::open(const xoj::FilePath& filepath) {
    this->file = filepath;
    GError* err = nullptr;
    this->stream.reset(
        G_INPUT_STREAM(g_file_read(this->file.get(), nullptr, &err)),
                                                        xoj::util::adopt);
    if (err) {
        auto error = FS(_F("Error opening file: \"{1}\"") % this->file.iri().get());
        error = error + "\n" + err->message;
        g_error_free(err);
        throw std::runtime_error{error};
    }
}

FileInputStream::~FileInputStream() = default;

bool FileInputStream::canSeek() const {
    return this->stream && g_seekable_can_seek(G_SEEKABLE(this->stream.get()));
}

static GSeekType seektype(int whence) {
    switch (whence) {
        case SEEK_CUR: return G_SEEK_CUR;
        case SEEK_END: return G_SEEK_END;
        case SEEK_SET:
        default:
            return G_SEEK_SET;
    }
}

bool FileInputStream::seek(std::ptrdiff_t offset, int whence) {
    return this->stream && g_seekable_seek(G_SEEKABLE(this->stream.get()), offset, seektype(whence), nullptr, nullptr);
}

std::ptrdiff_t FileInputStream::tell() {
    return this->stream ? g_seekable_tell(G_SEEKABLE(this->stream.get())) : 0;
}

auto FileInputStream::getFileInfo() const -> Info {
    GError* err = nullptr;
    auto* fi = g_file_input_stream_query_info(G_FILE_INPUT_STREAM(this->stream.get()), G_FILE_ATTRIBUTE_TIME_MODIFIED "," G_FILE_ATTRIBUTE_STANDARD_SIZE, nullptr, &err );
    if (err) {
        auto error = FS(_F("Error querying file info: \"{1}\"") % this->file.iri().get());
        error = error + "\n" + err->message;
        g_error_free(err);
        throw std::runtime_error{error};
    }
    Info info;
    info.mtime = g_file_info_get_attribute_uint64(fi, G_FILE_ATTRIBUTE_TIME_MODIFIED);
    info.size = g_file_info_get_attribute_uint64(fi, G_FILE_ATTRIBUTE_STANDARD_SIZE);

    return info;
}

}  // namespace xoj::util
