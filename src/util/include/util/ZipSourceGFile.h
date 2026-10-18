/*
 * Xournal++
 *
 * A zip_source implementation for reading/writing a GFile
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <zip.h>

#include "util/FileInputStream.h"
#include "filesystem.h"  // for path


namespace xoj::util {

/// Opens a GFile and make it a zip_source for libzip
class ZipSourceGFile {
public:
    ZipSourceGFile();
    ~ZipSourceGFile();

public:
    inline zip_t* get() { return zip; }
    inline operator bool() const { return zip != nullptr; }
    void close();
    void open(xoj::FilePath file);

protected:
    /// The stream interface with libzip
    static zip_int64_t zipCallback(void* userdata, void *data, zip_uint64_t len, zip_source_cmd_t cmd);

    FileInputStream stream;

public:
    zip_error_t ze;
    zip_t* zip = nullptr;
};
}  // namespace xoj::util
