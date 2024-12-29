/*
 * Xournal++
 *
 * A text element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <list>    // for list
#include <string>  // for string
#include <vector>  // for vector

#include <pango/pango.h>

#include "model/Element.h"
#include "util/raii/GObjectSPtr.h"
#include "util/raii/PangoSPtr.h"  // for PangoAttrListSPtr

#include "AudioElement.h"  // for AudioElement
#include "Font.h"          // for XojFont

class Element;
class ObjectInputStream;
class ObjectOutputStream;
class XojPdfRectangle;

enum class TextAlignment { LEFT, CENTER, RIGHT, JUSTIFIED };

class Text: public AudioElement {
public:
    Text();
    ~Text() override;

public:
    void setFont(const XojFont& font);
    XojFont& getFont();
    const XojFont& getFont() const;
    double getFontSize() const;       // same result as getFont()->getSize(), but const
    std::string getFontName() const;  // same result as getFont()->getName(), but const

    const std::string& getText() const;
    void setText(std::string text);

    void setWidth(double width);
    void setHeight(double height);

    void setInEditing(bool inEditing);
    bool isInEditing() const;

    xoj::util::GObjectSPtr<PangoLayout> createPangoLayout() const;
    void updatePangoFont(PangoLayout* layout) const;

    void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) override;
    void rotate(double x0, double y0, double th) override;

    bool rescaleOnlyAspectRatio() const override;

    void setAlignment(TextAlignment align);
    TextAlignment getAlignment() const;

    const xoj::util::PangoAttrListSPtr& getAttributeList() const;

    /**
     * Replaces the current attribute list with the given one
     * Important: Does not make a copy of the data.
     */
    void replaceAttributes(xoj::util::PangoAttrListSPtr attributes);

    void setColor(Color c) override;

    auto cloneText() const -> std::unique_ptr<Text>;
    auto clone() const -> ElementPtr override;

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

protected:
    void calcSize() const override;
    void updateSnapping() const;

public:
    std::vector<XojPdfRectangle> findText(const std::string& search) const;

private:
    XojFont font;

    std::string text;

    TextAlignment alignment = TextAlignment::LEFT;
    xoj::util::PangoAttrListSPtr attributes;

    bool inEditing = false;
};
