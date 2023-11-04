/*
 * Xournal++
 *
 * A factory to construct EditSelection instances
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr

#include "model/Element.h"                   // for Element, Element::Index
#include "model/ElementInsertionPosition.h"  // for InsertionOrder
#include "model/PageRef.h"                   // for PageRef

class Layer;
class EditSelection;
class Control;
class XojPageView;

namespace SelectionFactory {
auto createFromFloatingElement(Control* ctrl, const PageRef& page, Layer* layer, XojPageView* view,
                               ElementPtr e) -> std::unique_ptr<EditSelection>;
auto createFromFloatingElements(Control* ctrl, const PageRef& page, Layer* layer, XojPageView* view,
                                InsertionOrder elts)  //
        -> std::pair<std::unique_ptr<EditSelection>, Range>;
auto createFromElementOnActiveLayer(Control* ctrl, const PageRef& page, XojPageView* view, Element* e,
                                    Element::Index pos = Element::InvalidIndex)  //
        -> std::unique_ptr<EditSelection>;
auto createFromElementsOnActiveLayer(Control* ctrl, const PageRef& page, XojPageView* view,
                                     InsertionOrderRef elts) -> std::unique_ptr<EditSelection>;
/**
 * @brief Creates a new instance containing base->getElements() and *e. The content of *base is cleared but *base is not
 * destroyed.
 */
auto addElementFromActiveLayer(Control* ctrl, EditSelection* base, Element* e,
                               Element::Index pos) -> std::unique_ptr<EditSelection>;
/**
 * @brief Creates a new instance containing base->getElements() and the content of elts. The content of *base is cleared
 * but *base is not destroyed.
 */
auto addElementsFromActiveLayer(Control* ctrl, EditSelection* base,
                                const InsertionOrderRef& elts) -> std::unique_ptr<EditSelection>;
};  // namespace SelectionFactory
