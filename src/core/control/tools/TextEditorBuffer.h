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

#include <string>

#include "util/raii/GObjectSPtr.h"
#include "util/raii/PangoSPtr.h"

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
     * @brief Start selecting at cursor position
     */
    void startSelection();
    
    void updateFont(const Text& textElement);
    void updateLayoutIfNeedBe();
    
    /**
     * @brief Inserts at the insertion mark
     */
    void insert(const std::string& s);
    void insert(const std::string_view& s);
    void insert(char c);
    
    void moveInsertionMark(size_t mark);
    void moveCursorAt(double x, double y);
    void moveCursorByNGraphemes(int n);
    void moveCursorByNWords(int n);
    void moveCursorByNLines(int n);
    void moveCursorByNParagraphs(int n);
    

private:

    std::string content;
    size_t insertionMark = 0;
    size_t selectionMark = npos;
    
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
    
    static constexpr size_t npos = std::string::npos;
    
private:
    template <class UnaryOp>
    size_t utf8_find_if(size_t pos, UnaryOp condition) const;

    template <class UnaryOp>
    size_t utf8_rfind_if(size_t pos, UnaryOp condition) const;

    std::string::iterator utf8_next(std::string::iterator it, std::ptrdiff_t n = 1) const;
    
    void replaceContentImplementation();
};
