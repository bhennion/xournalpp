/*
 * Xournal++
 *
 * Contiguous and pango-aware UTF_8 textbuffer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <limits>
#include <string>

#include "util/raii/GObjectSPtr.h"
#include "util/raii/PangoSPtr.h"

class Range;
class Text;

class TextEditorBuffer final {
public:
    TextEditorBuffer(const Text& textElement);

    template <class... Args>
    void replaceContent(Args&&... args) {
        content.operator=(std::forward<Args>(args)...);
        replaceContentImplementation();
    }

    bool hasSelection() const;

    /**
     * @return Whether there was a selection before or not
     */
    bool clearSelection();

    /**
     * @brief Start selecting at cursor position if a selection is not already going on
     */
    void ensureSelection();

    void updateFont(const Text& textElement);
    
    /**
     * @brief Inserts at the insertion mark
     */
    void insert(const std::string_view& s);
    void insert(char c);

    /**
     *
     */
    void overwriteNextGraphemeWith(const std::string_view& s);

    struct Mark {
        using index_type = unsigned int;
        index_type byteOffset;
        index_type utf8CharOffset;
        static constexpr index_type npos = std::numeric_limits<index_type>::max();
        void clear() { byteOffset = npos; }
        operator bool() const { return byteOffset != npos; }
        bool operator==(const Mark& m) const { return byteOffset == m.byteOffset; }
        bool operator!=(const Mark& m) const { return !(*this == m); }
        bool operator<(const Mark& m) const { return byteOffset < m.byteOffset; }
        bool operator>(const Mark& m) const { return m < *this; }
        bool operator<=(const Mark& m) const { return !(*this > m); }
        bool operator>=(const Mark& m) const { return !(*this < m); }
    };

    /**
     * @brief Moves the insertion mark to the position best fitting the given coordinates
     *        The coordinates are relative to the text box
     * @return Whether the cursor moved or not
     */
    bool moveCursorAt(double x, double y);

    bool moveCursorByNGraphemes(int n);
    bool moveCursorByNWords(int n);
    bool moveCursorByNLines(int n);
    bool moveCursorByNParagraphs(int n);

    enum Direction { FORWARDS, BACKWARDS };
    bool moveCursorToWordEnd(Direction direction);
    bool moveCursorToLineEnd(Direction direction);
    bool moveCursorToParagraphEnd(Direction direction);
    bool moveCursorToBufferEnd(Direction direction);

    /**
     * @return Whether text was deleted or not
     */
    bool deleteNGraphemesFromCursor(int n);
    bool deleteNWordsFromCursor(int n);
    bool deleteNLinesFromCursor(int n);
    bool deleteNParagraphsFromCursor(int n);

    bool deleteFromCursorUntilWordEnd(Direction direction);
    bool deleteFromCursorUntilLineEnd(Direction direction);
    bool deleteFromCursorUntilParagraphEnd(Direction direction);

    bool deleteWhiteSpacesAroundCursor();
    bool deleteSelection();
    bool deleteUponBackspace();  // Upon backspace, we may need to delete a combining character rather than a grapheme


    Range getCursorBoundingBox(bool overwriting) const;
    Range getSelectionBoundingBox() const;
    Range getTextBoundingBox() const;

    void setPreeditData(std::string_view s, PangoAttrList* attrs, Mark::index_type cursorPos);

    /**
     * @brief Get a clone of the paragraph in which the cursor is. The IM preedit string is omitted.
     * @return The cloned paragraph, the position of the insertion point, and that of the selection end.
     * NB: The position of the selection end will only become useful after passing to GTK 4.2
     *     see https://docs.gtk.org/gtk4/method.IMContext.set_surrounding_with_selection.html
     */
    struct Surroundings { std::string text; int cursorByteIndex; int selectionByteIndex; };
    auto getCursorSurroundings() const -> Surroundings;

    /**
     * @brief Delete from the buffer characters around the preedit string
     * @param offset of the first character to be removed. The offset is expressed in UTF8 characters count, relatively
     * to the insertion mark.
     * @param nUTF8Chars Number of UTF8 characters to remove
     */
    void deleteUTF8CharsAroundPreedit(int start, int nUTF8Chars);

    bool isCursorInPreeditString() const;

    void selectWordAtCursor();
    void selectLineOfCursor();
    void selectAll();

    //     std::string_view getSelection() const;
    //     const std::string& getContent() const;

private:

    std::string content;
    Mark insertionMark = {0, 0};
    Mark selectionMark = {Mark::npos, Mark::npos};

    xoj::util::GSPtr<PangoLayout> layout;
    enum class LayoutStatus {UP_TO_DATE, NEED_ATTRIBUTE_REFRESH, NEED_STRING_REFRESH};
    LayoutStatus layoutStatus = LayoutStatus::NEED_STRING_REFRESH;

    // InputMethod preedit data
    xoj::util::PangoAttrListSPtr preeditAttrList;
    Mark preeditStringStart = {Mark::npos, Mark::npos};
    Mark::index_type preeditStringByteCount = 0;


    /**
     * @brief Coordinate of the virtual cursor, in Pango coordinates.
     * (The virtual cursor is used when moving the cursor vertically (e.g. pressing up arrow), to get a good "vertical
     * move" feeling, even if we pass by a shorter line)
     */
    int virtualCursorAbscissa = 0;

private:
    void updateLayoutIfNeedBe();

    template <class UnaryOp>
    size_t utf8_find_if(size_t pos, UnaryOp condition) const;

    template <class UnaryOp>
    size_t utf8_rfind_if(size_t pos, UnaryOp condition) const;

    const char* utf8_next_safe(const char* p, std::ptrdiff_t n = 1) const;

    void replaceContentImplementation();

    void updateVirtualCursorAbscissa();


    static inline constexpr Mark beginMark() { return {0, 0}; }
    Mark endMark() const;
    Mark::index_type getUTF8CharCount() const;

    Mark getMarkFromByteIndex(Mark::index_type byteIndex) const;
    Mark getMarkFromUTF8CharOffset(Mark::index_type utf8Offset) const;

    template <typename UnaryOp>
    Mark::index_type getUTF8OffsetOfNthOccurence(int n, UnaryOp condition) const;
    template <typename UnaryOp>
    Mark getMarkOfNthOccurence(int n, UnaryOp condition) const;

    Mark getMarkAtCursorPlusNGraphemes(int n) const;
    Mark getMarkAtCursorPlusNWords(int n) const;
    Mark getMarkAtCursorPlusNLines(int n) const;
    Mark getMarkAtCursorPlusNParagraphs(int n) const;

    Mark getEndOfOverwriteMark() const;
    Mark getEndOfWordMark(Direction direction) const;
    Mark getEndOfParagraphMark(Direction direction) const;
    Mark getEndOfLineMark(Direction direction) const;

    /**
     * @return Whether the mark moved or not
     */
    bool moveInsertionMark(Mark mark);

    /**
     * @return Whether something was deleted or not
     */
    bool deleteUntilMark(Mark mark);

    bool isMarkInPreeditString(Mark mark) const;
};
