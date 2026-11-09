/*
 * Xournal++
 *
 * A background image of a page
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for shared_ptr

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gio/gio.h>                // for GInputStream
#include <glib.h>                   // for GError

#include "filesystem.h"  // for path

struct BackgroundImage {
    friend bool operator==(const BackgroundImage& lhs, const BackgroundImage& rhs) = default;

    void free();

    void loadFile(xoj::AbsolutePath const& filepath, GError** error);
    void loadFile(GInputStream* stream, xoj::AbsolutePath const& filepath, GError** error);

    int getCloneId() const;
    void setCloneId(int id);
    void clearSaveState();

    xoj::AbsolutePath getFilepath() const;
    void setFilepath(xoj::AbsolutePath filepath);

    bool isAttached() const;
    void setAttach(bool attach);

    GdkPixbuf* getPixbuf();
    const GdkPixbuf* getPixbuf() const;

    bool isEmpty() const;

private:
    struct Content;

    std::shared_ptr<Content> img;
};
