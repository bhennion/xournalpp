#include "TextEditorBuffer.h"

#include <cassert>
#include <string>

#include "model/Text.h"
#include "util/raii/PangoSPtr.h"
#include <cmath>

TextEditorBuffer::TextEditorBuffer(const Text& textElement):
    layout(textElement.createPangoLayout()) {}

void TextEditorBuffer::replaceContentImplementation() {
    selectionMark = npos;
    insertionMark = content.length();
    layoutUpToDate = false;
}

auto TextEditorBuffer::hasSelection() const -> bool {
    return selectionMark != npos && selectionMark != insertionMark;
}

auto TextEditorBuffer::clearSelection() -> bool {
    return std::exchange(selectionMark, npos) != npos;
}

void TextEditorBuffer::startSelection() {
    selectionMark = insertionMark;
}

void TextEditorBuffer::updateFont(const Text& textElement) {
    textElement.updatePangoFont(layout.get());
}

void TextEditorBuffer::updateLayoutIfNeedBe() {
    // TODO
}

void TextEditorBuffer::insert(const std::string& s) {
    content.insert(insertionMark, s);
    insertionMark += s.length();
    selectionMark = npos;
    layoutUpToDate = false;
}

void TextEditorBuffer::insert(const std::string_view& s) {
    content.insert(insertionMark, s);
    insertionMark += s.length();
    selectionMark = npos;
    layoutUpToDate = false;
}

void TextEditorBuffer::insert(char c) {
    content.insert(insertionMark++, 1, c);
    selectionMark = npos;
    layoutUpToDate = false;
}

void TextEditorBuffer::moveInsertionMark(size_t mark) {
    assert(mark <= content.length());
    insertionMark = mark;
}

static inline int double_to_pango(double x) { return static_cast<int>(std::round(x * PANGO_SCALE)); }

void TextEditorBuffer::moveCursorAt(double x, double y) {
    int index = 0;
    int trailing = 0;
    pango_layout_xy_to_index(this->layout.get(), double_to_pango(x), double_to_pango(y), &index, &trailing);

    // If the point is in the second half of the grapheme, trailing contains the length of the grapheme in bytes.
    // Otherwise, trailing == 0. So index + trailing is where the cursor should go.
    moveInsertionMark(static_cast<size_t>(index + trailing));
}

void TextEditorBuffer::moveCursorByNGraphemes(int n) {
    updateLayoutIfNeedBe();
    int n_attrs = 0;
    const PangoLogAttr* attr = pango_layout_get_log_attrs_readonly(layout.get(), &n_attrs);
    size_t mark = insertionMark;
    auto isCursorPos = [attr](const char* p){return };
    while (n > 0) {
        utf8_find_if(mark, )
    }
}


[[maybe_unused]] static constexpr bool is_valid_utf8(const char* p) {
    return g_unichar_isdefined(g_utf8_get_char_validated(p, -1));
}

template <class UnaryOp>
auto TextEditorBuffer::utf8_find_if(size_t pos, UnaryOp condition) const -> size_t {
    const char* p = content.data() + pos;
    const char* end = content.data() + content.length();
    assert(is_valid_utf8(p));
    for (; p != end && !condition(p) ; p = g_utf8_next_char(p)) {}
    return p == end ? std::string::npos : static_cast<size_t>(p - content.data());
}

template <class UnaryOp>
auto TextEditorBuffer::utf8_rfind_if(size_t pos, UnaryOp condition) const -> size_t {
    const char* p = content.data() + pos;
    const char* begin = content.data();
    assert(is_valid_utf8(p));
    for (; p != begin && !condition(p) ; p = g_utf8_prev_char(p)) {}
    if (p == begin) {
        return condition(begin) ? 0U : std::string::npos;
    }
    return static_cast<size_t>(p - content.data());
}

auto TextEditorBuffer::utf8_next(std::string::iterator it, std::ptrdiff_t n = 1) const -> std::string::iterator {
    const char* p = &*it;
    if (n > 0) {
        const char* end = content.data() + content.length();
        assert(p <= end);
        for (; n && p != end ; p = g_utf8_next_char(p), --n) {}
    } else {
        const char* begin = content.data();
        assert(p >= begin);
        for (; n && p != begin ; p = g_utf8_prev_char(p), ++n) {}
    }
    return std::next(it, p - &*it);
}
