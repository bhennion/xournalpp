#include <cstddef>  // for size_t
#include <limits>   // for numeric_limits
#include <memory>   // for make_unique, __sha...
#include <numeric>  // for transform_reduce
#include <utility>

#include "control/Control.h"                 // for Control
#include "model/Document.h"                  // for Document
#include "model/Element.h"                   // for Element::Index
#include "model/ElementInsertionPosition.h"  // for ElementInsertionPosition
#include "model/Layer.h"                     // for Layer
#include "model/XojPage.h"                   // for XojPage
#include "util/Assert.h"                     // for xoj_assert
#include "util/Range.h"                      // for Range

#include "EditSelection.h"

namespace SelectionFactory {
/// @return Bounds and SnappingBounds
static auto computeBoxes(const InsertionOrder& elts) -> std::pair<Range, Range> {
    return std::transform_reduce(
            elts.begin(), elts.end(), std::pair<Range, Range>(),
            [](auto&& p, auto&& q) {
                return std::pair<Range, Range>(p.first.unite(q.first), p.second.unite(q.second));
            },
            [](auto&& e) { return std::make_pair(Range(e.e->boundingRect()), Range(e.e->getSnappedBounds())); });
}

auto createFromFloatingElement(Control* ctrl, const PageRef& page, Layer* layer, XojPageView* view,
                               ElementPtr eOwn) -> std::unique_ptr<EditSelection> {
    auto* e = eOwn.get();  // Order of parameter evaluation is unspecified, eOwn.get() must be evaluated before moving
    InsertionOrder i{1};
    i[0] = InsertionPosition{std::move(eOwn)};
    return std::make_unique<EditSelection>(ctrl, std::move(i), page, layer, view, Range(e->boundingRect()),
                                           Range(e->getSnappedBounds()));
}

auto createFromFloatingElements(Control* ctrl, const PageRef& page, Layer* layer, XojPageView* view,
                                InsertionOrder elts) -> std::pair<std::unique_ptr<EditSelection>, Range> {
    xoj_assert(std::is_sorted(elts.begin(), elts.end()));
    auto [bounds, snappingBounds] = computeBoxes(elts);
    return std::make_pair(
            std::make_unique<EditSelection>(ctrl, std::move(elts), page, layer, view, bounds, snappingBounds), bounds);
}

auto createFromElementOnActiveLayer(Control* ctrl, const PageRef& page, XojPageView* view, Element* e,
                                    Element::Index pos) -> std::unique_ptr<EditSelection> {
    Document* doc = ctrl->getDocument();
    Layer* layer = nullptr;

    InsertionOrder i(1);
    i[0] = [&] {
        std::lock_guard lock(*doc);  // lock scope
        layer = page->getSelectedLayer();
        return layer->removeElementAt(e, pos);
    }();
    page->fireElementChanged(e);
    return std::make_unique<EditSelection>(ctrl, std::move(i), page, layer, view, Range(e->boundingRect()),
                                           Range(e->getSnappedBounds()));
}

auto createFromElementsOnActiveLayer(Control* ctrl, const PageRef& page, XojPageView* view,
                                     InsertionOrderRef elts) -> std::unique_ptr<EditSelection> {
    xoj_assert(std::is_sorted(elts.begin(), elts.end()));
    Document* doc = ctrl->getDocument();
    Layer* layer = nullptr;
    auto ownedElts = [&] {
        std::lock_guard lock(*doc);  // lock scope
        layer = page->getSelectedLayer();
        return layer->removeElementsAt(elts);
    }();

    auto [bounds, snappingBounds] = computeBoxes(ownedElts);
    page->fireRangeChanged(bounds);

    return std::make_unique<EditSelection>(ctrl, std::move(ownedElts), page, layer, view, bounds, snappingBounds);
}

auto addElementFromActiveLayer(Control* ctrl, EditSelection* base, Element* e,
                               Element::Index pos) -> std::unique_ptr<EditSelection> {
    Document* doc = ctrl->getDocument();
    Layer* layer = base->getSourceLayer();
    auto ownedElem = [&] {
        std::lock_guard lock(*doc);  // lock scope
        return layer->removeElementAt(e, pos);
    }();
    pos = ownedElem.pos;
    const PageRef& page = base->getSourcePage();
    page->fireElementChanged(e);

    InsertionOrder elts = base->makeMoveEffective();
    xoj_assert(!elts.empty());
    xoj_assert(std::is_sorted(elts.begin(), elts.end()));
    /**
     * To sort out the proper Element::Index of the added element *e,  we need to imagine elts were added to the layer,
     * so that the index may need to be increased.
     * Explicitly, we need to insert (e, pos + n) at position n so that the resulting vector is still sorted. Figuring
     * out the value of n requires our own binary search (std::lower_bound won't work).
     */
    auto begin = elts.begin(), first = begin, last = elts.end();
    while (first != last) {
        auto it = std::next(first, std::distance(first, last) / 2);
        if (it->pos <= pos + std::distance(begin, it)) {
            first = std::next(it);
        } else {
            last = it;
        }
    }
    ownedElem.pos += std::distance(begin, first);
    elts.insert(first, std::move(ownedElem));
    xoj_assert(std::is_sorted(elts.begin(), elts.end()));

    auto [bounds, snappingBounds] = computeBoxes(elts);

    return std::make_unique<EditSelection>(ctrl, std::move(elts), page, layer, base->getView(), bounds, snappingBounds);
}

auto addElementsFromActiveLayer(Control* ctrl, EditSelection* base,
                                const InsertionOrderRef& newElts) -> std::unique_ptr<EditSelection> {
    xoj_assert(std::is_sorted(newElts.begin(), newElts.end()));
    Document* doc = ctrl->getDocument();
    Layer* layer = base->getSourceLayer();
    auto ownedElts = [&] {  // lock scope
        std::lock_guard lock(*doc);
        return layer->removeElementsAt(newElts);
    }();

    auto [bounds, snappingBounds] = computeBoxes(ownedElts);
    const PageRef& page = base->getSourcePage();
    page->fireRangeChanged(bounds);

    InsertionOrder oldElts = base->makeMoveEffective();
    xoj_assert(std::is_sorted(oldElts.begin(), oldElts.end()));
    auto [oldBounds, oldSnappingBounds] = computeBoxes(oldElts);

    InsertionOrder newSelection;
    newSelection.reserve(oldElts.size() + newElts.size());
    /**
     * To sort out the proper Element::Indices, we need to imagine oldElts were added back to the layer, so that some of
     * newElts' would see their indices increase. A simple std::merge won't do. See comment in addElementFromActiveLayer
     */
    auto oldIt = oldElts.begin(), oldEnd = oldElts.end();
    std::ptrdiff_t shift = 0;  // number of elements from oldElts that have been added to newSelection

    for (auto newIt = ownedElts.begin(), newEnd = ownedElts.end(); newIt != newEnd;) {
        if (oldIt == oldEnd) {
            xoj_assert(shift == static_cast<std::ptrdiff_t>(oldElts.size()));
            for (; newIt != newEnd; ++newIt) {
                newSelection.emplace_back(std::move(newIt->e), newIt->pos + shift);
            }
            break;
        }

        if (oldIt->pos < newIt->pos + shift) {
            newSelection.emplace_back(std::move(*oldIt));
            ++oldIt;
            ++shift;
        } else {
            newSelection.emplace_back(std::move(newIt->e), newIt->pos + shift);
            ++newIt;
        }
    }
    std::move(oldIt, oldEnd, std::back_inserter(newSelection));
    xoj_assert(newSelection.size() == oldElts.size() + newElts.size());
    xoj_assert(std::is_sorted(newSelection.begin(), newSelection.end()));
    return std::make_unique<EditSelection>(ctrl, std::move(newSelection), page, layer, base->getView(),
                                           bounds.unite(oldBounds), snappingBounds.unite(oldSnappingBounds));
}
};  // namespace SelectionFactory
