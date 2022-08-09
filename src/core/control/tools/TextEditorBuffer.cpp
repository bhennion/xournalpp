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
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);
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


/********** Mark computation ************/

static bool isCursorPosition(PangoLogAttr a) { return a.is_cursor_position; }
static bool isWordStart(PangoLogAttr a) { return a.is_word_start; }
static bool isWordEnd(PangoLogAttr a) { return a.is_word_end; }
static bool isParagraphEnd(PangoLogAttr a) { return a.is_mandatory_break; }

auto TextEditorBuffer::getUTF8CharCount() const -> Mark::index_type {
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);
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
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);
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
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);
    assert(utf8Offset <= getUTF8CharCount());
    /*
     * Same as in getMarkFromByteIndex: We have 3 known valid marks: beginMark, endMark and insertionMark.
     * Difference: going backward is now three times more expensive than forward
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
    const char* p = g_utf8_offset_to_pointer(std::next(content.data(), ref.byteOffset),
                                             static_cast<long>(utf8Offset) - static_cast<long>(ref.utf8CharOffset));

    Mark m;
    m.byteOffset = static_cast<Mark::index_type>(std::distance(content.data(), p));
    m.utf8CharOffset = utf8Offset;
    ASSERT_MARK_VALID(m);
    return m;
}

template <typename UnaryOp>
auto TextEditorBuffer::getUTF8OffsetOfNthOccurence(int n, UnaryOp condition) const -> Mark::index_type {
    if (n == 0) {
        return 0;
    }
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);
    int n_attrs = 0;
    const PangoLogAttr* const attrs = pango_layout_get_log_attrs_readonly(layout.get(), &n_attrs);
    const PangoLogAttr* a = std::next(attrs, insertionMark.utf8CharOffset);

    if (n > 0) {
        // Go forward
        auto* const end = std::next(attrs, n_attrs);
        for (; n > 0 && a != end; --n) {
            a = std::find_if(std::next(a), end, condition);
        }
        return static_cast<Mark::index_type>(std::distance(attrs, a));
    } else {
        // Go backwards (n < 0)
        auto rend = std::make_reverse_iterator(attrs);
        auto b = std::make_reverse_iterator(std::next(a));
        for (; n < 0 && b != rend; ++n) {
            b = std::find_if(std::next(b), rend, condition);
        }
        return static_cast<Mark::index_type>(std::distance(b, rend));
    }
}

template <typename UnaryOp>
auto TextEditorBuffer::getMarkOfNthOccurence(int n, UnaryOp condition) const -> Mark {
    return getMarkFromUTF8CharOffset(getUTF8OffsetOfNthOccurence(n, condition));
}

auto TextEditorBuffer::getMarkAtCursorPlusNGraphemes(int n) const -> Mark {
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);
    return getMarkOfNthOccurence(n, isCursorPosition);
}

auto TextEditorBuffer::getMarkAtCursorPlusNWords(int n) const -> Mark {
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);
    return n < 0 ? getMarkOfNthOccurence(n, isWordStart) : getMarkOfNthOccurence(n, isWordEnd);
}

auto TextEditorBuffer::getMarkAtCursorPlusNParagraphs(int n) const -> Mark {
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);

    if (n < 0 && insertionMark.utf8CharOffset != 0) {
        const PangoLogAttr* const attrs = pango_layout_get_log_attrs_readonly(layout.get(), nullptr);
        if (isParagraphEnd(attrs[insertionMark.utf8CharOffset - 1])) {
            // The cursor is at the beginning of a paragraph
            // Ignore the preceding line break
            n--;
        }
    }

    auto offset = getUTF8OffsetOfNthOccurence(n, isParagraphEnd);
    if (offset < insertionMark.utf8CharOffset) {
        // Move past the line break
        offset++;
        assert(offset != insertionMark.utf8CharOffset);
    }
    return getMarkFromUTF8CharOffset(offset);
}

auto TextEditorBuffer::getMarkAtCursorPlusNLines(int n) const -> Mark {
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);

    PangoLayoutLine* line = nullptr;
    {  // Fetch the line
        int lineNb = 0;
        pango_layout_index_to_line_x(layout.get(), static_cast<int>(insertionMark.byteOffset), false, &lineNb, nullptr);
        if (int clamp = std::clamp(lineNb + n, 0, pango_layout_get_line_count(layout.get()) - 1); clamp == lineNb) {
            // No other line to move to in this direction.
            return insertionMark;
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
    return getMarkFromByteIndex(static_cast<Mark::index_type>(index + trailing));
}

auto TextEditorBuffer::getEndOfOverwriteMark() const -> Mark {
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);
    int n_attrs = 0;
    const PangoLogAttr* const attrs = pango_layout_get_log_attrs_readonly(layout.get(), &n_attrs);
    const PangoLogAttr* a = std::next(attrs, insertionMark.utf8CharOffset);
    if (insertionMark.byteOffset == content.length() || a->is_mandatory_break) {
        // End of buffer or end of line. Do not write over anything.
        return insertionMark;
    }
    auto* const end = std::next(attrs, n_attrs);
    assert(a != end);
    a = std::find_if(std::next(a), end, isCursorPosition);
    return getMarkFromUTF8CharOffset(static_cast<Mark::index_type>(std::distance(attrs, a)));
}

TextEditorBuffer::Mark TextEditorBuffer::getEndOfWordMark(TextEditorBuffer::Direction direction) const {
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);
    int n_attrs = 0;
    const PangoLogAttr* const attrs = pango_layout_get_log_attrs_readonly(layout.get(), &n_attrs);
    const PangoLogAttr* a = std::next(attrs, insertionMark.utf8CharOffset);
    std::ptrdiff_t offset = 0;
    if (direction == FORWARDS) {
        auto* const end = std::next(attrs, n_attrs);
        a = std::find_if(a, end, isWordEnd);
        offset = std::distance(attrs, a);
    } else {
        auto rend = std::make_reverse_iterator(attrs);
        auto b = std::make_reverse_iterator(std::next(a));
        b = std::find_if(b, rend, isWordStart);
        offset = std::distance(b, rend);
    }
    assert(offset >= 0);
    return getMarkFromUTF8CharOffset(static_cast<Mark::index_type>(offset));
}

TextEditorBuffer::Mark TextEditorBuffer::getEndOfLineMark(TextEditorBuffer::Direction direction) const {
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);
    int lineNumber = 0;
    pango_layout_index_to_line_x(layout.get(), static_cast<int>(insertionMark.byteOffset), false, &lineNumber, nullptr);
    PangoLayoutLine* line = pango_layout_get_line_readonly(layout.get(), lineNumber);

    auto index = pango_layout_line_get_start_index(line);
    if (direction == FORWARDS) {
        index += pango_layout_line_get_length(line) - 1;  // ??
    }
    return getMarkFromByteIndex(static_cast<Mark::index_type>(index));
}

TextEditorBuffer::Mark TextEditorBuffer::getEndOfParagraphMark(TextEditorBuffer::Direction direction) const {
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);
    int n_attrs = 0;
    const PangoLogAttr* const attrs = pango_layout_get_log_attrs_readonly(layout.get(), &n_attrs);
    const PangoLogAttr* a = std::next(attrs, insertionMark.utf8CharOffset);
    std::ptrdiff_t offset = 0;
    if (direction == FORWARDS) {
        auto* const end = std::next(attrs, n_attrs);
        a = std::find_if(a, end, isParagraphEnd);
        offset = std::distance(attrs, a);
    } else {
        auto rend = std::make_reverse_iterator(attrs);
        auto b = std::make_reverse_iterator(std::next(a));
        b = std::find_if(b, rend, isParagraphEnd);
        offset = std::distance(b, rend);
        if (offset != 0) {
            offset++;  // skip the line break
        }
    }
    assert(offset >= 0);
    return getMarkFromUTF8CharOffset(static_cast<Mark::index_type>(offset));
}

/********** End of Mark computations *************/

void TextEditorBuffer::insert(const std::string_view& s) {
    assert(g_utf8_validate_len(s.data(), s.length(), nullptr) && "inserting string with invalid UTF8 encoding");
    if (s.empty()) {
        return;
    }
    content.insert(insertionMark.byteOffset, s);
    insertionMark.byteOffset += s.length();
    insertionMark.utf8CharOffset +=
            static_cast<Mark::index_type>(g_utf8_strlen(s.data(), static_cast<gssize>(s.length())));
    selectionMark.clear();
    layoutStatus = LayoutStatus::NEED_STRING_REFRESH;
    ASSERT_MARK_VALID(insertionMark);
    ASSERT_UTF8_VALID();
}

void TextEditorBuffer::insert(char c) {
    content.insert(insertionMark.byteOffset, 1, c);
    insertionMark.byteOffset++;
    insertionMark.utf8CharOffset++;
    selectionMark.clear();
    layoutStatus = LayoutStatus::NEED_STRING_REFRESH;
    ASSERT_MARK_VALID(insertionMark);
    ASSERT_UTF8_VALID();
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
    layoutStatus = LayoutStatus::NEED_STRING_REFRESH;
    ASSERT_MARK_VALID(insertionMark);
    ASSERT_UTF8_VALID();
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
    Mark::index_type byteIndex = static_cast<Mark::index_type>(index + trailing);
    if (byteIndex == insertionMark.byteOffset) {
        return false;
    }

    insertionMark = getMarkFromByteIndex(byteIndex);
    updateVirtualCursorAbscissa();
    return true;
}

bool TextEditorBuffer::moveInsertionMark(Mark mark) {
    if (mark != insertionMark) {
        insertionMark = mark;
        updateVirtualCursorAbscissa();
        if (!isCursorInPreeditString()) {
            // If we move away from the preedit string, commit it
            preeditStringStart.clear();
        }
        return true;
    }
    return false;
}

bool TextEditorBuffer::moveCursorByNGraphemes(int n) {
    updateLayoutIfNeedBe();
    return moveInsertionMark(getMarkAtCursorPlusNGraphemes(n));
}

bool TextEditorBuffer::moveCursorByNWords(int n) {
    updateLayoutIfNeedBe();
    return moveInsertionMark(getMarkAtCursorPlusNWords(n));
}

bool TextEditorBuffer::moveCursorByNParagraphs(int n) {
    updateLayoutIfNeedBe();
    return moveInsertionMark(getMarkAtCursorPlusNParagraphs(n));
}

bool TextEditorBuffer::moveCursorByNLines(int n) {
    updateLayoutIfNeedBe();
    Mark m = getMarkAtCursorPlusNLines(n);
    // Do not rely on `moveInsertionMark()`, so that we do not update `virtualCursorAbscissa`.
    if (m != insertionMark) {
        insertionMark = m;
        if (!isCursorInPreeditString()) {
            preeditStringStart.clear();
        }
        return true;
    }
    return false;
}

bool TextEditorBuffer::moveCursorToWordEnd(Direction direction) {
    updateLayoutIfNeedBe();
    return moveInsertionMark(getEndOfWordMark(direction));
}

bool TextEditorBuffer::moveCursorToLineEnd(Direction direction) {
    updateLayoutIfNeedBe();
    return moveInsertionMark(getEndOfLineMark(direction));
}

bool TextEditorBuffer::moveCursorToParagraphEnd(Direction direction) {
    updateLayoutIfNeedBe();
    return moveInsertionMark(getEndOfParagraphMark(direction));
}

bool TextEditorBuffer::moveCursorToBufferEnd(Direction direction) {
    updateLayoutIfNeedBe();  // for endMark
    return moveInsertionMark(direction == FORWARDS ? endMark() : beginMark());
}

bool TextEditorBuffer::deleteUntilMark(TextEditorBuffer::Mark mark) {
    /*
     * Can there be a selection at this point? If so, should we delete it as well?
     * In the following implementation, we assume yes and yes.
     */
    Interval<Mark> interval = Interval<Mark>::getInterval(mark, insertionMark);
    if (hasSelection()) {
        interval.envelop(selectionMark);
        selectionMark.clear();
    }
    if (interval.min == interval.max) {
        // Nothing to delete
        return false;
    }
    if (!isMarkInPreeditString(interval.min) || !isMarkInPreeditString(interval.max)) {
        // Commit the preedit string if we modify anything outside of it.
        preeditStringStart.clear();
    }
    content.erase(interval.min.byteOffset, interval.max.byteOffset - interval.min.byteOffset);
    ASSERT_UTF8_VALID();
    layoutStatus = LayoutStatus::NEED_STRING_REFRESH;
    moveInsertionMark(interval.min);
    return true;
}

bool TextEditorBuffer::deleteNGraphemesFromCursor(int n) {
    updateLayoutIfNeedBe();
    return deleteUntilMark(getMarkAtCursorPlusNGraphemes(n));
}

bool TextEditorBuffer::deleteNWordsFromCursor(int n) {
    updateLayoutIfNeedBe();
    return deleteUntilMark(getMarkAtCursorPlusNWords(n));
}

bool TextEditorBuffer::deleteNLinesFromCursor(int n) {
    updateLayoutIfNeedBe();
    return deleteUntilMark(getMarkAtCursorPlusNLines(n));
}

bool TextEditorBuffer::deleteNParagraphsFromCursor(int n) {
    updateLayoutIfNeedBe();
    return deleteUntilMark(getMarkAtCursorPlusNParagraphs(n));
}

bool TextEditorBuffer::deleteFromCursorUntilWordEnd(TextEditorBuffer::Direction direction) {
    updateLayoutIfNeedBe();
    return deleteUntilMark(getEndOfWordMark(direction));
}

bool TextEditorBuffer::deleteFromCursorUntilLineEnd(TextEditorBuffer::Direction direction) {
    updateLayoutIfNeedBe();
    return deleteUntilMark(getEndOfLineMark(direction));
}

bool TextEditorBuffer::deleteFromCursorUntilParagraphEnd(TextEditorBuffer::Direction direction) {
    updateLayoutIfNeedBe();
    return deleteUntilMark(getEndOfParagraphMark(direction));
}

bool TextEditorBuffer::deleteWhiteSpacesAroundCursor() {
    updateLayoutIfNeedBe();
    int n_attrs = 0;
    const PangoLogAttr* const attrs = pango_layout_get_log_attrs_readonly(layout.get(), &n_attrs);
    const PangoLogAttr* a = std::next(attrs, insertionMark.utf8CharOffset);

    auto isNotWhitespace = [](PangoLogAttr attr) { return !attr.is_white; };
    auto rend = std::reverse_iterator(attrs);
    auto b = std::find_if(std::reverse_iterator(a), rend, isNotWhitespace);
    const PangoLogAttr* firstWhiteSpace = b == rend ? attrs : &(b[-1]);

    const PangoLogAttr* firstNotWhiteSpace = std::find_if(a, std::next(attrs, n_attrs), isNotWhitespace);
    Mark end = getMarkFromUTF8CharOffset(static_cast<Mark::index_type>(std::distance(attrs, firstNotWhiteSpace)));
    bool moved = moveInsertionMark(
            getMarkFromUTF8CharOffset(static_cast<Mark::index_type>(std::distance(attrs, firstWhiteSpace))));
    bool deleted = deleteUntilMark(end);
    assert(deleted || !moved);
    return deleted;
}

bool TextEditorBuffer::deleteSelection() {
    bool ret = deleteUntilMark(selectionMark);
    selectionMark.clear();
    return ret;
}

bool TextEditorBuffer::deleteUponBackspace() {
    if (insertionMark == beginMark()) {
        return false;
    }
    updateLayoutIfNeedBe();
    int n_attrs = 0;
    const PangoLogAttr* const attrs = pango_layout_get_log_attrs_readonly(layout.get(), &n_attrs);
    const PangoLogAttr* afterGraphemeLog = std::next(attrs, insertionMark.utf8CharOffset);
    auto rend = std::reverse_iterator(attrs);
    auto b = std::find_if(std::reverse_iterator(afterGraphemeLog), rend, isCursorPosition);
    const PangoLogAttr* beforeGraphemeLog = b == rend ? attrs : &*b;
    Mark beforeGrapheme =
            getMarkFromUTF8CharOffset(static_cast<Mark::index_type>(std::distance(attrs, beforeGraphemeLog)));
    std::string_view grapheme(std::next(content.data(), beforeGrapheme.byteOffset),
                              insertionMark.byteOffset - beforeGrapheme.byteOffset);

    if (afterGraphemeLog->backspace_deletes_character) {
        // The previous grapheme combines characters. Remove only the last character.
        gchar* normalizedGrapheme =
                g_utf8_normalize(grapheme.data(), static_cast<long>(grapheme.length()), G_NORMALIZE_DEFAULT);
        auto length = g_utf8_strlen(normalizedGrapheme, -1);
        assert(length > 0);
        auto endByteIndex = static_cast<size_t>(
                std::distance(normalizedGrapheme, g_utf8_offset_to_pointer(normalizedGrapheme, length - 1)));
        std::string_view truncatedGrapheme(normalizedGrapheme, endByteIndex);
        content.replace(beforeGrapheme.byteOffset, grapheme.length(), truncatedGrapheme);
        g_free(normalizedGrapheme);
    } else {
        content.erase(beforeGrapheme.byteOffset, grapheme.length());
    }
    return true;
}

Range TextEditorBuffer::getCursorBoundingBox(bool overwriting) const {
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);
    Mark::index_type byteIndex = insertionMark.byteOffset;
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
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);
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

    // TODO Rethink this using line direction

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
    assert(layoutStatus == LayoutStatus::UP_TO_DATE);

    // Todo: this is not suitable for repaints because of italic fonts getting out of the logical bounding box.
    // Get the ink bounding box?

    int w = 0;
    int h = 0;
    pango_layout_get_size(this->layout.get(), &w, &h);
    double width = (static_cast<double>(w)) / PANGO_SCALE;
    double height = (static_cast<double>(h)) / PANGO_SCALE;

    return Range(0., 0., width, height);
}

void TextEditorBuffer::setPreeditData(std::string_view s, PangoAttrList* attrs, Mark::index_type cursorPos) {
    std::string_view preeditStr(content.c_str() + insertionMark.byteOffset, preeditStringByteCount);
    if (preeditStr != s) {
        content.replace(insertionMark.byteOffset, preeditStringByteCount, s);
        preeditStringByteCount = static_cast<Mark::index_type>(s.length());
        layoutStatus = LayoutStatus::NEED_STRING_REFRESH;
        ASSERT_UTF8_VALID();
    } else if (layoutStatus == LayoutStatus::UP_TO_DATE && !pango_attr_list_equal(attrs, preeditAttrList.get())){
        layoutStatus = LayoutStatus::NEED_ATTRIBUTE_REFRESH;
    }
    preeditAttrList.reset(attrs);  // Take ownership of attrs in all cases!
    if (insertionMark.utf8CharOffset != preeditStringStart.utf8CharOffset + cursorPos) {
        moveInsertionMark(getMarkFromUTF8CharOffset(preeditStringStart.utf8CharOffset + cursorPos));
    }
}

auto TextEditorBuffer::getCursorSurroundings() const -> Surroundings {
    Surroundings res;
    std::string& s = res.text;

    Mark parBegin = getEndOfParagraphMark(BACKWARDS);
    Mark parEnd = getEndOfParagraphMark(FORWARDS);

    if (preeditStringByteCount == 0) {
        s = std::string_view(content.c_str() + parBegin.byteOffset, parEnd.byteOffset - parBegin.byteOffset);
    } else {
        s.reserve(parEnd.byteOffset - parBegin.byteOffset - preeditStringByteCount);
        s = std::string_view(content.c_str() + parBegin.byteOffset, insertionMark.byteOffset - parBegin.byteOffset);
        s += std::string_view(content.c_str() + insertionMark.byteOffset + preeditStringByteCount, parEnd.byteOffset - (insertionMark.byteOffset + preeditStringByteCount));
    }

    res.cursorByteIndex = static_cast<int>(insertionMark.byteOffset - parBegin.byteOffset);
    res.selectionByteIndex = res.cursorByteIndex;
    if (hasSelection()) {
        auto clamped = std::clamp(selectionMark.byteOffset, parBegin.byteOffset, parEnd.byteOffset);
        res.selectionByteIndex = static_cast<int>(clamped - parBegin.byteOffset);
    }

    return res;
}

void TextEditorBuffer::deleteUTF8CharsAroundPreedit(int start, int nUTF8Chars) {
    if (start >= 0 || (start < 0 && nUTF8Chars <= std::abs(start)) || preeditStringByteCount == 0) {
        // All removed characters are in a single contiguous section of string
        auto p = utf8_next_safe(std::next(content.c_str(), insertionMark.byteOffset), start);
        auto q = utf8_next_safe(p, nUTF8Chars);
        insertionMark.byteOffset = static_cast<Mark::index_type>(std::distance(content.c_str(), p));
        content.erase(static_cast<size_t>(std::distance(content.c_str(), p)), static_cast<size_t>(std::distance(p, q)));
    } else {
        // Half before, half after
        // Need to erase two non-contiguous sections of the string. Two calls to erase() would copy the string twice.
        std::string s;
        std::swap(s, content);
        auto p = utf8_next_safe(std::next(s.c_str(), insertionMark.byteOffset), start);
        auto q = utf8_next_safe(std::next(s.c_str(), insertionMark.byteOffset + preeditStringByteCount), start + nUTF8Chars);
        content.reserve(s.length() - static_cast<size_t>(nUTF8Chars));  // Suboptimal size
        // Copy the first portion of the string
        content += std::string_view(s.c_str(), static_cast<size_t>(std::distance(s.c_str(), p) - 1));
        // Copy the preedit string back in place
        content += std::string_view(std::next(s.c_str() + insertionMark.byteOffset), preeditStringByteCount);
        // Copy the tail
        content += std::string_view(q, s.length() - static_cast<size_t>(std::distance(s.c_str(), q)));
        insertionMark.byteOffset = static_cast<Mark::index_type>(std::distance(s.c_str(), p));
    }
    insertionMark.utf8CharOffset = static_cast<Mark::index_type>(start + static_cast<std::ptrdiff_t>(insertionMark.utf8CharOffset));
    ASSERT_MARK_VALID(insertionMark);
    ASSERT_UTF8_VALID();
}

bool TextEditorBuffer::isMarkInPreeditString(Mark mark) const {
    return preeditStringStart && preeditStringStart.byteOffset <= mark.byteOffset &&
           preeditStringStart.byteOffset + preeditStringByteCount >= mark.byteOffset;
}

bool TextEditorBuffer::isCursorInPreeditString() const { return isMarkInPreeditString(insertionMark); }

void TextEditorBuffer::selectWordAtCursor() {
    selectionMark = getEndOfWordMark(BACKWARDS);
    insertionMark = getEndOfWordMark(FORWARDS);
}

void TextEditorBuffer::selectLineOfCursor() {
    selectionMark = getEndOfLineMark(BACKWARDS);
    insertionMark = getEndOfLineMark(FORWARDS);
}

void TextEditorBuffer::selectAll() {
    selectionMark = beginMark();
    insertionMark = endMark();
}

// auto TextEditorBuffer::getSelection() const -> std::string_view {
//     if (hasSelection()) {
//         Interval<Mark> sel = Interval<Mark>::getInterval(insertionMark, selectionMark);
//         return std::string_view(std::next(content.data(), sel.min.byteOffset), sel.max.byteOffset -
//         sel.max.byteOffset);
//     }
//     return std::string_view();
// }
//
// auto getContent...

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
        assert(p == end || is_valid_utf8(p));
        for (; n && p != end ; p = g_utf8_next_char(p), --n) {}
    } else {
        const char* begin = content.data();
        assert(p >= begin);
        assert(is_valid_utf8(p));
        for (; n && p != begin ; p = g_utf8_prev_char(p), ++n) {}
    }
    return p;
}
