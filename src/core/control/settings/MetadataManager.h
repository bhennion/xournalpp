/*
 * Xournal++
 *
 * Last opened files with all settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <mutex>  // for mutex
#include <optional>
#include <vector>  // for vector

#include <glib.h>  // for gint64

#include "filesystem.h"  // for path


class MetadataEntry {
public:
    MetadataEntry();

public:
    xoj::FilePath metadataFile;
    xoj::FilePath path;
    double zoom;
    int page;
    gint64 time;
};

class MetadataManager {
public:
    MetadataManager() = default;
    virtual ~MetadataManager();

public:
    /**
     * Get the metadata for a file
     */
    static std::optional<MetadataEntry> getForFile(xoj::FilePath const& file);

    /**
     * Store the current data into metadata
     */
    void storeMetadata(xoj::FilePath const& file, int page, double zoom);

    /**
     * Document was closed, a new document was opened etc.
     */
    void documentChanged();

private:
    /**
     * Delete an old metadata file
     */
    static void deleteMetadataFile(xoj::FilePath const& path);

    /**
     * Parse a single metadata file
     */
    static std::optional<MetadataEntry> loadMetadataFile(xoj::FilePath const& path);

    /**
     * Store metadata to file
     */
    static void storeMetadata(const MetadataEntry& m);

    static void writeMetadataToFile(const MetadataEntry& m, const xoj::FilePath& file);

private:
    /**
     * Load the metadata list (sorted)
     */
    static std::vector<MetadataEntry> loadList();

private:
    std::mutex mutex;
    std::unique_ptr<MetadataEntry> metadata;

    friend class Metadata_testRead_Test;
    friend class Metadata_testWriteReadCycle_Test;
};
