#include "BackgroundImage.h"

#include <string>   // for string
#include <utility>  // for move

#include <glib-object.h>  // for g_object_unref

#include "util/Stacktrace.h"  // for Stacktrace
#include "util/StringUtils.h"

/*
 * The contents of a background image
 *
 * Internal impl object, dont move this to an external header/source file due this is the best way to reduce code
 * bloat and increase encapsulation. This object is only used in this source scope and is a RAII Container for the
 * GdkPixbuf*
 * No xournal memory leak tests necessary, because we use smart ptrs to ensure memory correctness
 */

struct BackgroundImage::Content {
    Content(xoj::AbsolutePath path, GError** error):
            path(std::move(path)) {
                xoj::util::GObjectSPtr<GInputStream> str(G_INPUT_STREAM(g_file_read(this->path.get(), nullptr, error)), xoj::util::adopt);
                if (error && *error) {
                    return;
                }
                this->pixbuf = gdk_pixbuf_new_from_stream(str.get(), nullptr, error);
                g_input_stream_close(str.get(), nullptr, nullptr);
            }

    Content(GInputStream* stream, xoj::AbsolutePath path, GError** error):
            path(std::move(path)), pixbuf(gdk_pixbuf_new_from_stream(stream, nullptr, error)) {}

    ~Content() {
        if (this->pixbuf) {
            g_object_unref(this->pixbuf);
            this->pixbuf = nullptr;
        }
    };

    Content(const Content&) = delete;
    Content(Content&&) = default;
    auto operator=(const Content&) -> Content& = delete;
    auto operator=(Content&&) -> Content& = default;

    xoj::AbsolutePath path;
    GdkPixbuf* pixbuf = nullptr;
    int pageId = -1;
    bool attach = false;
};

void BackgroundImage::free() { this->img.reset(); }

void BackgroundImage::loadFile(xoj::AbsolutePath const& path, GError** error) {
    this->img = std::make_shared<Content>(path, error);
}

void BackgroundImage::loadFile(GInputStream* stream, xoj::AbsolutePath const& path, GError** error) {
    this->img = std::make_shared<Content>(stream, path, error);
}

auto BackgroundImage::getCloneId() const -> int { return this->img ? this->img->pageId : -1; }

void BackgroundImage::setCloneId(int id) {
    if (this->img) {
        this->img->pageId = id;
    }
}

void BackgroundImage::clearSaveState() { this->setCloneId(-1); }

auto BackgroundImage::getFilepath() const -> xoj::AbsolutePath { return this->img ? this->img->path : xoj::AbsolutePath{}; }

void BackgroundImage::setFilepath(xoj::AbsolutePath path) {
    if (this->img) {
        this->img->path = std::move(path);
    }
}

auto BackgroundImage::isAttached() const -> bool { return this->img ? this->img->attach : false; }

void BackgroundImage::setAttach(bool attach) {
    if (!this->img) {
        g_warning("BackgroundImage::setAttach: please load first an image before call setAttach!");
        Stacktrace::printStacktrace();
        return;
    }
    this->img->attach = attach;
}

auto BackgroundImage::getPixbuf() -> GdkPixbuf* { return this->img ? this->img->pixbuf : nullptr; }
auto BackgroundImage::getPixbuf() const -> const GdkPixbuf* { return this->img ? this->img->pixbuf : nullptr; }

auto BackgroundImage::isEmpty() const -> bool { return !this->img; }
