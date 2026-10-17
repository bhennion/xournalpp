/*
 * Xournal++
 *
 * Output streams for writing
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>       // for string
#include <string_view>  // for string_view

#include <zlib.h>  // for gzFile

#include "util/raii/GObjectSPtr.h"

#include "filesystem.h"  // for path

class OutputStream {
public:
    OutputStream();
    virtual ~OutputStream();

public:
    bool write(const char* str);  ///< Return true on success
    bool write(const std::string_view sv);  ///< Return true on success
    bool write(const std::u8string_view sv);  ///< Return true on success

    virtual bool write(const char* data, size_t len) = 0;  ///< Return true on success
    virtual bool close() = 0;  ///< Return true on success
};

class GLibBasedOutputStream: public OutputStream {
public:
    GLibBasedOutputStream();
    ~GLibBasedOutputStream() override;

public:
    bool write(const char* data, size_t len) override;  ///< Return true on success
    bool close() override;  ///< Return true on success

    const std::string& getLastError() const;

protected:
    xoj::util::GObjectSPtr<GOutputStream> stream;

    std::string error;
};

class GzOutputStream: public GLibBasedOutputStream {
public:
    explicit GzOutputStream(xoj::FilePath file);
    ~GzOutputStream() override;
private:
    xoj::FilePath file;
};

class FileOutputStream: public GLibBasedOutputStream {
public:
    explicit FileOutputStream(xoj::FilePath file);
    ~FileOutputStream() override;
private:
    xoj::FilePath file;
};
