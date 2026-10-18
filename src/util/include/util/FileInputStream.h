/*
 * Xournal++
 *
 * Input stream for a gzipped file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "util/GzInputStream.h"  // for GLibBasedInputStream

#include "filesystem.h"  // for path


namespace xoj::util {

class FileInputStream final: public GLibBasedInputStream {
public:
    FileInputStream();
    ~FileInputStream() override;

    [[nodiscard]] bool canSeek () const;
    bool seek(std::ptrdiff_t offset, int whence);
    [[nodiscard]] std::ptrdiff_t tell();
    struct Info {
        size_t mtime;  ///< in seconds since EPOCH
        size_t size;   ///< in bytes
    };
    Info getFileInfo() const;
    void open(const xoj::FilePath& filepath);

private:
    xoj::FilePath file;
};

}  // namespace xoj::util
