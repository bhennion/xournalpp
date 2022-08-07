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
    void updateLayoutIfNeedBe();
    
    /**
     * @brief Inserts at the insertion mark
     */
    void insert(const std::string& s);
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
        operator bool() { byteOffset != npos; }
        bool operator==(const Mark& m) { return byteOffset == m.byteOffset; }
        bool operator!=(const Mark& m) { return !(*this == m); }
    };
    static inline constexpr Mark beginMark() { return {0, 0}; }
    Mark endMark() const;
    Mark::index_type getUTF8CharCount() const;

    Mark getMarkFromByteIndex(Mark::index_type byteIndex) const;
    Mark getMarkFromUTF8CharOffset(Mark::index_type utf8Offset) const;

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
    bool moveCursorToLineEnd(Direction direction);
    bool moveCursorToParagraphEnd(Direction direction);
    bool moveCursorToBufferEnd(Direction direction);

    /**
     * @brief Get a clone of the paragraph in which the cursor is. The IM preedit string is omitted.
     */
    std::string getCursorSurroundings() const;

    Range getCursorBoundingBox(bool overwriting) const;
    Range getSelectionBoundingBox() const;
    Range getTextBoundingBox() const;

private:

    std::string content;
    Mark insertionMark = {0, 0};
    Mark selectionMark = {Mark::npos, Mark::npos};

    xoj::util::GSPtr<PangoLayout> layout;
    bool layoutUpToDate = false;

    // InputMethod preedit data
    xoj::util::PangoAttrListSPtr preeditAttrList;
    std::string preeditString;
    int preeditCursor;
    
    /**
     * @brief Coordinate of the virtual cursor, in Pango coordinates.
     * (The virtual cursor is used when moving the cursor vertically (e.g. pressing up arrow), to get a good "vertical
     * move" feeling, even if we pass by a shorter line)
     */
    int virtualCursorAbscissa = 0;

private:
    template <class UnaryOp>
    size_t utf8_find_if(size_t pos, UnaryOp condition) const;

    template <class UnaryOp>
    size_t utf8_rfind_if(size_t pos, UnaryOp condition) const;

    const char* utf8_next_safe(const char* p, std::ptrdiff_t n = 1) const;

    void replaceContentImplementation();

    void updateVirtualCursorAbscissa();

    /**
     * @return Whether the mark moved or not
     */
    bool moveInsertionMark(Mark::index_type byteIndex);

    /**
     * @brief Get a mark (supposedly in the vicinity of insertionMark
     * @param d Direction (compared to insertionMark). The direction determines an end END of the string towards which
     * we are going. This end END = rend() if d == BACKWARD, or END = end() if d == FORWARD.
     * @param distance Distance (in terms of utf8 characters offsets) between the targeted mark and END.
     * WARNING: The sign of `distance` follows the convention for reversed_iterators if d == BACKWARDS.
     *          So distance <= 0 means we are past END.
     */
    Mark getMarkCloseToInsertionMark(std::ptrdiff_t distance) const;

    template <typename UnaryOp1, typename UnaryOp2>
    std::ptrdiff_t getUTF8CharDistanceOfNthOccurence(int n, UnaryOp1& forwardCondition,
                                                     UnaryOp2& backwardCondition) const;

    Mark getEndOfOverwriteMark() const;
};
