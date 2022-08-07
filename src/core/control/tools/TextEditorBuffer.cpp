#include "TextEditorBuffer.h"

#include <cassert>
#include <cmath>
#include <string>

#include "model/Text.h"
#include "util/Interval.h"
#include "util/raii/PangoSPtr.h"

#define TEXT_BUFFER_DEBUG

#ifdef TEXT_BUFFER_DEBUG
#define ASSERT_MARK_VALID(m) \
    assert(g_utf8_pointer_to_offset(content.data(), content.data() + m.byteOffset) == m.utf8CharOffset)
#define ASSERT_UTF8_VALID() assert(g_utf8_validate_len(content.data(), content.length(), nullptr));
#else
#define ASSERT_MARK_VALID(m)
#define ASSERT_UTF8_VALID()
#endif

TextEditorBuffer::TextEditorBuffer(const Text& textElement):
    layout(textElement.createPangoLayout()) {}

void TextEditorBuffer::replaceContentImplementation() {
    selectionMark.clear();
    updateLayout();
    insertionMark = endMark();
}

void TextEditorBuffer::updateVirtualCursorAbscissa() {
    assert(layoutUpToDate);
    pango_layout_index_to_line_x(layout.get(), static_cast<int>(insertionMark.byteOffset), false, nullptr,
                                 &virtualCursorAbscissa);
}

auto TextEditorBuffer::hasSelection() const -> bool { return selectionMark && selectionMark != insertionMark; }

auto TextEditorBuffer::clearSelection() -> bool {
    bool res = selectionMark;
    selectionMark.clear();
    return res;
}

void TextEditorBuffer::ensureSelection() {
    if (!clearSelection()) {
        selectionMark = insertionMark;
    }
}

void TextEditorBuffer::updateFont(const Text& textElement) {
    textElement.updatePangoFont(layout.get());
}

void TextEditorBuffer::updateLayoutIfNeedBe() {
    // TODO
}

auto TextEditorBuffer::getUTF8CharCount() const -> Mark::index_type {
    assert(layoutUpToDate);
    return static_cast<Mark::index_type>(pango_layout_get_character_count(layout.get()));
}

auto TextEditorBuffer::endMark() const -> Mark {
    Mark m;
    m.byteOffset = static_cast<Mark::index_type>(content.length());
    m.utf8CharOffset = getUTF8CharCount();
    ASSERT_MARK_VALID(m);
    return m;
}

TextEditorBuffer::Mark TextEditorBuffer::getMarkFromByteIndex(Mark::index_type byteIndex) const {
    assert(byteIndex <= content.length());
    assert(layoutUpToDate);
    const char* p = std::next(content.data(), byteIndex);
    /*
     * We need to find out the utf8 character offset.
     * We have 3 known valid marks: beginMark, endMark and insertionMark.
     * Take the closest as a reference and iterate from there
     */
    Mark ref;
    if (byteIndex <= insertionMark.byteOffset / 2) {
        ref = beginMark();
    } else if (byteIndex <= (insertionMark.byteOffset + content.length()) / 2) {
        ref = insertionMark;
    } else {
        ref = endMark();
    }
    Mark m;
    m.byteOffset = byteIndex;
    m.utf8CharOffset = static_cast<Mark::index_type>(ref.utf8CharOffset +
                                                     g_utf8_pointer_to_offset(content.data() + ref.byteOffset, p));
    ASSERT_MARK_VALID(m);
    return m;
}

TextEditorBuffer::Mark TextEditorBuffer::getMarkFromUTF8CharOffset(Mark::index_type utf8Offset) const {
    assert(layoutUpToDate);
    assert(utf8Offset <= getUTF8CharCount());
    /*
     * Same as in getMarkFromByteIndex: We have 3 known valid marks: beginMark, endMark and insertionMark.
     * Difference: going backward is now more expensive than forward
     * see https://docs.gtk.org/glib/func.utf8_offset_to_pointer.html
     */
    Mark ref;
    if (utf8Offset <= (3 * insertionMark.utf8CharOffset) / 4) {
        ref = beginMark();
    } else if (utf8Offset <= (insertionMark.byteOffset + 3 * getUTF8CharCount()) / 4) {
        ref = insertionMark;
    } else {
        ref = endMark();
    }
    const char* p = g_utf8_offset_to_pointer(content.data() + ref.byteOffset,
                                             static_cast<long>(utf8Offset) - static_cast<long>(ref.utf8CharOffset));

    Mark m;
    m.byteOffset = static_cast<Mark::index_type>(std::distance(content.data(), p));
    m.utf8CharOffset = utf8Offset;
    ASSERT_MARK_VALID(m);
    return m;
}

void TextEditorBuffer::insert(const std::string& s) { insert(std::string_view(s.data(), s.length())); }

void TextEditorBuffer::insert(const std::string_view& s) {
    assert(g_utf8_validate_len(s.data(), s.length(), nullptr) && "inserting string with invalid UTF8 encoding");
    content.insert(insertionMark.byteOffset, s);
    insertionMark.byteOffset += s.length();
    insertionMark.utf8CharOffset +=
            static_cast<Mark::index_type>(g_utf8_strlen(s.data(), static_cast<gssize>(s.length())));
    selectionMark.clear();
    layoutUpToDate = false;
    ASSERT_MARK_VALID(insertionMark);
    ASSERT_UTF8_VALID();
}

void TextEditorBuffer::insert(char c) {
    content.insert(insertionMark.byteOffset, 1, c);
    insertionMark.byteOffset++;
    insertionMark.utf8CharOffset++;
    selectionMark.clear();
    layoutUpToDate = false;
    ASSERT_MARK_VALID(insertionMark);
    ASSERT_UTF8_VALID();
}

auto TextEditorBuffer::getEndOfOverwriteMark() const -> Mark {
    assert(layoutUpToDate);
    int n_attrs = 0;
    const PangoLogAttr* const attrs = pango_layout_get_log_attrs_readonly(layout.get(), &n_attrs);
    const PangoLogAttr* a = std::next(attrs, insertionMark.utf8CharOffset);
    if (insertionMark.byteOffset == content.length() || a->is_mandatory_break) {
        // End of buffer or end of line. Do not write over anything.
        return insertionMark;
    }
    auto* const end = std::next(attrs, n_attrs);
    assert(a != end);
    a = std::find_if(std::next(a), end, [](auto attr) { return attr.is_cursor_position; });
    auto distance = std::distance(attrs, a) - insertionMark.utf8CharOffset;
    return getMarkCloseToInsertionMark(distance);
}

void TextEditorBuffer::overwriteNextGraphemeWith(const std::string_view& s) {
    assert(g_utf8_validate_len(s.data(), s.length(), nullptr) && "inserting string with invalid UTF8 encoding");
    updateLayoutIfNeedBe();
    Mark end = getEndOfOverwriteMark();
    content.replace(insertionMark.byteOffset, end.byteOffset - insertionMark.byteOffset, s);
    insertionMark.byteOffset += s.length();
    insertionMark.utf8CharOffset +=
            static_cast<Mark::index_type>(g_utf8_strlen(s.data(), static_cast<gssize>(s.length())));
    selectionMark.clear();
    layoutUpToDate = false;
    ASSERT_MARK_VALID(insertionMark);
    ASSERT_UTF8_VALID();
}

bool TextEditorBuffer::moveInsertionMark(Mark::index_type byteIndex) {
    if (byteIndex != insertionMark.byteOffset) {
        insertionMark = getMarkFromByteIndex(byteIndex);
        return true;
    }
    return false;
}

static inline int double_to_pango(double x) { return static_cast<int>(std::round(x * PANGO_SCALE)); }
static inline double pango_to_double(int x) { return static_cast<double>(x) / PANGO_SCALE; }

bool TextEditorBuffer::moveCursorAt(double x, double y) {
    updateLayoutIfNeedBe();
    int index = 0;
    int trailing = 0;
    pango_layout_xy_to_index(this->layout.get(), double_to_pango(x), double_to_pango(y), &index, &trailing);

    // If the point is in the second half of the grapheme, trailing contains the length of the grapheme in bytes.
    // Otherwise, trailing == 0. So index + trailing is where the cursor should go.
    assert(index + trailing >= 0);
    if (moveInsertionMark(static_cast<Mark::index_type>(index + trailing))) {
        updateVirtualCursorAbscissa();
        return true;
    }
    return false;
}

auto TextEditorBuffer::getMarkCloseToInsertionMark(std::ptrdiff_t distance) const -> Mark {
    if (auto pos = distance + insertionMark.utf8CharOffset; pos <= 0) {
        return beginMark();
    } else if (pos >= getUTF8CharCount()) {
        return endMark();
    } else {
        const char* p = g_utf8_offset_to_pointer(content.data() + insertionMark.byteOffset, distance);
        Mark m;
        m.byteOffset = static_cast<Mark::index_type>(std::distance(content.data(), p));
        m.utf8CharOffset = static_cast<Mark::index_type>(pos);
        ASSERT_MARK_VALID(m);
        return m;
    }
}

template <typename UnaryOp1, typename UnaryOp2>
auto TextEditorBuffer::getUTF8CharDistanceOfNthOccurence(int n, UnaryOp1& forwardCondition,
                                                         UnaryOp2& backwardCondition) const -> std::ptrdiff_t {
    if (n == 0) {
        return 0;
    }
    assert(layoutUpToDate);
    int n_attrs = 0;
    const PangoLogAttr* const attrs = pango_layout_get_log_attrs_readonly(layout.get(), &n_attrs);
    const PangoLogAttr* a = std::next(attrs, insertionMark.utf8CharOffset);

    if (n > 0) {
        // Go forward
        auto* const end = std::next(attrs, n_attrs);
        for (; n > 0 && a != end; --n) {
            a = std::find_if(std::next(a), end, forwardCondition);
        }
        auto distance = std::distance(attrs, a) - insertionMark.utf8CharOffset;
        assert(distance > 0);
        return distance;
    } else {
        // Go backwards (n < 0)
        auto rend = std::make_reverse_iterator(attrs);
        auto b = std::make_reverse_iterator(std::next(a));
        for (; n < 0 && b != rend; ++n) {
            b = std::find_if(std::next(b), rend, backwardCondition);
        }
        auto distance = std::distance(b, rend) - insertionMark.utf8CharOffset;
        assert(distance < 0);
        return distance;
    }
}

bool TextEditorBuffer::moveCursorByNGraphemes(int n) {
    updateLayoutIfNeedBe();
    auto isCursorPos = [](auto attr) { return attr.is_cursor_position; };
    auto distance = getUTF8CharDistanceOfNthOccurence(n, isCursorPos, isCursorPos);
    if (distance != 0) {
        insertionMark = getMarkCloseToInsertionMark(distance);
        updateVirtualCursorAbscissa();
        return true;
    }
    return false;
}

bool TextEditorBuffer::moveCursorByNWords(int n) {
    updateLayoutIfNeedBe();
    auto isWordStart = [](auto attr) { return attr.is_word_start; };
    auto isWordEnd = [](auto attr) { return attr.is_word_end; };
    auto distance = getUTF8CharDistanceOfNthOccurence(n, isWordEnd, isWordStart);
    if (distance != 0) {
        insertionMark = getMarkCloseToInsertionMark(distance);
        updateVirtualCursorAbscissa();
        return true;
    }
    return false;
}

bool TextEditorBuffer::moveCursorByNParagraphs(int n) {
    updateLayoutIfNeedBe();
    auto isParagraphEnd = [](auto attr) { return attr.is_mandatory_break; };

    if (n < 0 && insertionMark.utf8CharOffset != 0) {
        const PangoLogAttr* const attrs = pango_layout_get_log_attrs_readonly(layout.get(), nullptr);
        if (isParagraphEnd(attrs[insertionMark.utf8CharOffset - 1])) {
            // The cursor is at the beginning of a paragraph
            // Ignore the preceding line break
            n--;
        }
    }

    auto distance = getUTF8CharDistanceOfNthOccurence(n, isParagraphEnd, isParagraphEnd);
    if (distance == 0) {
        return false;
    }
    if (distance < 0) {
        // Move past the line break
        distance++;
        assert(distance != 0);  // Should be covered by the n--; above
    }
    assert(insertionMark.utf8CharOffset + distance >= 0);
    insertionMark = getMarkFromUTF8CharOffset(static_cast<Mark::index_type>(insertionMark.utf8CharOffset + distance));
    updateVirtualCursorAbscissa();
    return true;
}

bool TextEditorBuffer::moveCursorByNLines(int n) {
    updateLayoutIfNeedBe();

    PangoLayoutLine* line = nullptr;
    {  // Fetch the line
        int lineNb = 0;
        pango_layout_index_to_line_x(layout.get(), static_cast<int>(insertionMark.byteOffset), false, &lineNb, nullptr);
        if (int clamp = std::clamp(lineNb + n, 0, pango_layout_get_line_count(layout.get()) - 1); clamp == lineNb) {
            return false;
        } else {
            lineNb = clamp;
        }
        line = pango_layout_get_line_readonly(layout.get(), lineNb);
        assert(line);
    }

    int index = 0;
    int trailing = 0;
    pango_layout_line_x_to_index(line, this->virtualCursorAbscissa, &index, &trailing);
    // If the point is in the second half of the grapheme, trailing contains the length of the grapheme in bytes.
    // Otherwise, trailing == 0. So index + trailing is where the cursor should go.
    assert(index + trailing >= 0);
    return moveInsertionMark(static_cast<Mark::index_type>(index + trailing));
}

bool TextEditorBuffer::moveCursorToLineEnd(Direction direction) {
    updateLayoutIfNeedBe();
    int lineNumber = 0;
    pango_layout_index_to_line_x(layout.get(), static_cast<int>(insertionMark.byteOffset), false, &lineNumber, nullptr);
    PangoLayoutLine* line = pango_layout_get_line_readonly(layout.get(), lineNumber);

    auto index = pango_layout_line_get_start_index(line);
    if (direction == FORWARDS) {
        index += pango_layout_line_get_length(line) - 1;  // ??
    }
    if (moveInsertionMark(static_cast<Mark::index_type>(index))) {
        updateVirtualCursorAbscissa();
        return true;
    }
    return false;
}

bool TextEditorBuffer::moveCursorToParagraphEnd(TextEditorBuffer::Direction direction) {
    // TODO
}

bool TextEditorBuffer::moveCursorToBufferEnd(TextEditorBuffer::Direction direction) {
    updateLayoutIfNeedBe();  // for endMark
    Mark m = direction == FORWARDS ? endMark() : beginMark();
    if (m.byteOffset != insertionMark.byteOffset) {
        insertionMark = m;
        updateVirtualCursorAbscissa();
        return true;
    }
    return false;
}

std::string TextEditorBuffer::getCursorSurroundings() const {}

Range TextEditorBuffer::getCursorBoundingBox(bool overwriting) const {
    assert(layoutUpToDate);
    Mark::index_type byteIndex = insertionMark.byteOffset;
    if (!preeditString.empty()) {
        byteIndex += preeditCursor;
    }
    PangoRectangle rect = {0};
    if (overwriting) {
        pango_layout_index_to_pos(this->layout.get(), static_cast<int>(byteIndex), &rect);
        // Be aware that rect.width could be negative here (if writing right to left).
    } else {
        pango_layout_get_cursor_pos(this->layout.get(), static_cast<int>(byteIndex), &rect, nullptr);
    }
    Range cursorBox(pango_to_double(rect.x), pango_to_double(rect.y));
    cursorBox.addPoint(pango_to_double(rect.x + rect.width), pango_to_double(rect.y + rect.height));
    return cursorBox;
}

Range TextEditorBuffer::getSelectionBoundingBox() const {
    assert(layoutUpToDate);
    if (!hasSelection()) {
        return Range();
    }
    int firstByteIndex = static_cast<int>(std::min(insertionMark.byteOffset, selectionMark.byteOffset));
    int lastByteIndex = static_cast<int>(std::max(insertionMark.byteOffset, selectionMark.byteOffset));

    int lineNb1 = 0;
    int x1 = 0;
    pango_layout_index_to_line_x(layout.get(), firstByteIndex, false, &lineNb1, &x1);
    int lineNb2 = 0;
    int x2 = 0;
    pango_layout_index_to_line_x(layout.get(), lastByteIndex, false, &lineNb2, &x2);

    auto getLineInkRect = [layout = this->layout.get()](int n) {
        PangoLayoutLine* line = pango_layout_get_line_readonly(layout, n);
        PangoRectangle r = {0};
        pango_layout_line_get_extents(line, &r, nullptr);
        return r;
    };

    if (lineNb1 == lineNb2) {
        PangoRectangle r = getLineInkRect(lineNb1);
        Range res(pango_to_double(x1), pango_to_double(r.y));
        res.addPoint(pango_to_double(x2), pango_to_double(r.y + r.height));
        return res;
    }

    Interval<int> xRange = Interval<int>::getInterval(x1, x2);
    auto addRectToXRange = [&xRange](const PangoRectangle& r) {
        xRange.envelop(r.x);
        xRange.envelop(r.x + r.width);
    };
    PangoRectangle r1 = getLineInkRect(lineNb1);
    PangoRectangle r2 = getLineInkRect(lineNb2);
    Interval<int> yRange = Interval<int>::getInterval(r1.y, r2.y + r2.height);
    addRectToXRange(r1);
    addRectToXRange(r2);

    for (int n = lineNb1 + 1; n < lineNb2; ++n) {
        addRectToXRange(getLineInkRect(n));
    }

    return Range(pango_to_double(xRange.min), pango_to_double(yRange.min), pango_to_double(xRange.max),
                 pango_to_double(yRange.max));
}

Range TextEditorBuffer::getTextBoundingBox() const {
    assert(layoutUpToDate);

    int w = 0;
    int h = 0;
    pango_layout_get_size(this->layout.get(), &w, &h);
    double width = (static_cast<double>(w)) / PANGO_SCALE;
    double height = (static_cast<double>(h)) / PANGO_SCALE;

    return Range(0., 0., width, height);
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

auto TextEditorBuffer::utf8_next_safe(const char* p, std::ptrdiff_t n) const -> const char* {
    if (n > 0) {
        const char* end = content.data() + content.length();
        assert(p <= end);
        for (; n && p != end ; p = g_utf8_next_char(p), --n) {}
    } else {
        const char* begin = content.data();
        assert(p >= begin);
        for (; n && p != begin ; p = g_utf8_prev_char(p), ++n) {}
    }
    return p;
}
