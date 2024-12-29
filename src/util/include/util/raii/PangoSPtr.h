/*
 * Xournal++
 *
 * RAII wrappers for C library classes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <pango/pango-attributes.h>  // for PangoAttrList, pango_attr_list_ref, pango_attr_list_unref

#include "CLibrariesSPtr.h"
#include "IdentityFunction.h"

namespace xoj::util {
inline namespace raii {

namespace specialization {
class PangoAttrListHandler {
public:
    constexpr static auto ref = pango_attr_list_ref;
    constexpr static auto unref = pango_attr_list_unref;
    constexpr static auto adopt = identity<PangoAttrList>;
};

struct PangoFontDescriptionDeleter {
    inline void operator()(PangoFontDescription* desc) { if (desc) { pango_font_description_free(desc); }}
};
struct PangoAttributeDeleter {
    inline void operator()(PangoAttribute* a) { if (a) { pango_attribute_destroy(a); }}
};
};  // namespace specialization

using PangoAttrListSPtr = CLibrariesSPtr<PangoAttrList, specialization::PangoAttrListHandler>;
using PangoAttributeUPtr = std::unique_ptr<PangoAttribute, specialization::PangoAttributeDeleter>;
using PangoFontDescriptionUPtr = std::unique_ptr<PangoFontDescription, specialization::PangoFontDescriptionDeleter>;
};  // namespace raii
};  // namespace xoj::util
