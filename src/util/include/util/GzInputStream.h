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

#include "util/InputStream.h"  // for InputStream

#include "filesystem.h"  // for path


namespace xoj::util {

class GLibBasedInputStream: public InputStream {
public:
    GLibBasedInputStream();
    ~GLibBasedInputStream() override;

public:
    ptrdiff_t read(char* buffer, size_t len) override;
    void close() override;

protected:
    xoj::util::GObjectSPtr<GInputStream> stream;
};

class GzInputStream final: public GLibBasedInputStream {
public:
    explicit GzInputStream(const xoj::FilePath& filepath);
    ~GzInputStream() override;

    bool hasValidHeader() const;

private:
    xoj::FilePath file;
};
}  // namespace xoj::util
