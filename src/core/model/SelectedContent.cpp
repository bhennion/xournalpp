#include "SelectedContent.h"

#include <string>

#include "model/geometry/Transformation.h"
#include "undo/ArrangeUndoAction.h"    // for ArrangeUndoAction
#include "undo/ColorUndoAction.h"      // for ColorUndoAction
#include "undo/DeleteUndoAction.h"     // for DeleteUndoAction
#include "undo/FillUndoAction.h"       // for FillUndoAction
#include "undo/FontUndoAction.h"       // for FontUndoAction
#include "undo/InsertUndoAction.h"     // for InsertsUndoAction
#include "undo/LineStyleUndoAction.h"  // for LineStyleUndoAction
#include "undo/MoveUndoAction.h"       // for MoveUndoAction
#include "undo/RotateUndoAction.h"     // for RotateUndoAction
#include "undo/ScaleUndoAction.h"      // for ScaleUndoAction
#include "undo/SizeUndoAction.h"       // for SizeUndoAction
#include "util/Assert.h"
#include "util/i18n.h"
#include "util/serializing/ObjectInputStream.h"
#include "util/serializing/ObjectOutputStream.h"

#include "Image.h"
#include "Layer.h"
#include "Stroke.h"
#include "TexImage.h"
#include "Text.h"

static Position initialPosition(const xoj::util::Rectangle<double>& box) {
    Position p;
    p.halfWidth = 0.5 * box.width;
    p.halfHeight = 0.5 * box.height;
    p.center.x = box.x + p.halfWidth;
    p.center.y = box.y + p.halfHeight;
    p.angle = 0.0;
    return p;
}

SelectedContent::SelectedContent(PageRef page, Layer* layer): page(std::move(page)), layer(layer) {}

SelectedContent::SelectedContent(PageRef page, Layer* layer, InsertionOrder order,
                                 const xoj::util::Rectangle<double>& box,
                                 const xoj::util::Rectangle<double>& snapBounds):
        originalPosition(initialPosition(snapBounds)),
        position(originalPosition),
        horizontalMargin(std::max(snapBounds.x - box.x, box.x + box.width - (snapBounds.x + snapBounds.width))),
        verticalMargin(std::max(snapBounds.y - box.y, box.y + box.height - (snapBounds.y + snapBounds.height))),
        insertionOrder(std::move(order)),
        page(std::move(page)),
        layer(layer) {
    xoj_assert(horizontalMargin >= 0.0);
    xoj_assert(verticalMargin >= 0.0);
}

void SelectedContent::forEachElement(std::function<void(Element*)> f) const {
    for (auto const& [e, _]: this->insertionOrder) {
        f(e.get());
    }
}

auto SelectedContent::drop(bool scaleLineWidth) -> Range {
    Range rg = this->computeCoarseBoundingBox(this->getPosition());

    auto transformation = xoj::geometry::Transformation::fromTo(this->originalPosition, this->position, scaleLineWidth);

    for (auto&& [e, p]: this->insertionOrder) {
        e->applyTransformation(transformation.get());
        xoj_assert(p >= 0);
        this->layer->insertElement(std::move(e), p);
    }
    this->insertionOrder.clear();

    return rg;
}

auto SelectedContent::dropAClone(bool scaleLineWidth) const -> std::pair<UndoActionPtr, Range> {
    auto transformation = xoj::geometry::Transformation::fromTo(this->originalPosition, this->position, scaleLineWidth);

    std::vector<Element*> elts;
    elts.reserve(this->insertionOrder.size());

    for (auto&& [e, p]: this->insertionOrder) {
        auto clone = e->clone();
        clone->applyTransformation(transformation.get());
        elts.emplace_back(clone.get());
        xoj_assert(p >= 0);
        this->layer->insertElement(std::move(clone), p);
    }
    return {std::make_unique<InsertsUndoAction>(page, layer, elts), computeCoarseBoundingBox(this->getPosition())};
}

void SelectedContent::cancel() {
    for (auto&& [e, p]: insertionOrder) {
        xoj_assert(p >= 0);
        this->layer->insertElement(std::move(e), p);
    }
    this->insertionOrder.clear();
}

auto SelectedContent::makeMoveEffective(bool scaleLineWidth) -> InsertionOrder {
    auto transformation = xoj::geometry::Transformation::fromTo(this->originalPosition, this->position, scaleLineWidth);
    for (auto&& [e, _]: this->insertionOrder) {
        e->applyTransformation(transformation.get());
    }
    return std::move(this->insertionOrder);
}

auto SelectedContent::getMarginScale(const Position& pos) const -> double {
    return originalPosition.halfHeight != 0.0 ? originalPosition.halfWidth != 0.0 ?
                                                std::max({1., pos.halfWidth / originalPosition.halfWidth,
                                                          pos.halfHeight / originalPosition.halfHeight}) :
                                                std::max(1., pos.halfHeight / originalPosition.halfHeight) :
           originalPosition.halfWidth != 0.0  ? std::max(1., pos.halfWidth / originalPosition.halfWidth) :
                                                1.;
}

auto SelectedContent::computeCoarseBoundingBox(const Position& pos) const -> Range {
    // When rescaling, we need to rescale the margins as well
    double marginScale = getMarginScale(pos);

    double halfWidth = std::abs(pos.halfWidth) + marginScale * horizontalMargin;
    double halfHeight = std::abs(pos.halfHeight) + marginScale * verticalMargin;

    // Calculate new clip region delta due to rotation
    // The computed box contains the rotation of the unrotated box. It may be larger than the actual elements.
    // e.g. if a diagonal line is rotated
    const double cos = std::cos(pos.angle), sin = std::sin(pos.angle);
    halfWidth = std::abs(halfWidth * cos) + std::abs(halfHeight * sin);
    halfHeight = std::abs(halfWidth * sin) + std::abs(halfHeight * cos);

    return Range(pos.center.x - halfWidth, pos.center.y - halfHeight, pos.center.x + halfWidth,
                 pos.center.y + halfHeight);
}

auto SelectedContent::computeUnrotatedBoundingBox() const -> Range {
    // When rescaling, we need to rescale the margins as well
    double marginScale = getMarginScale(position);

    double halfWidth = std::abs(position.halfWidth) + marginScale * horizontalMargin;
    double halfHeight = std::abs(position.halfHeight) + marginScale * verticalMargin;

    return Range(position.center.x - halfWidth, position.center.y - halfHeight, position.center.x + halfWidth,
                 position.center.y + halfHeight);
}

void SelectedContent::serialize(ObjectOutputStream& out) const {
    out.writeObject("SelectedContent");

    out.writeDouble(this->position.center.x);
    out.writeDouble(this->position.center.y);
    out.writeDouble(this->position.halfWidth);
    out.writeDouble(this->position.halfHeight);
    out.writeDouble(this->position.angle);

    out.writeDouble(this->horizontalMargin);
    out.writeDouble(this->verticalMargin);

    out.writeSizeT(this->insertionOrder.size());
    for (const auto& [e, _]: this->insertionOrder) {
        e->serialize(out);
    }

    out.endObject();
}

void SelectedContent::readSerialized(ObjectInputStream& in) {
    in.readObject("EditSelection");

    this->position.center.x = in.readDouble();
    this->position.center.y = in.readDouble();
    this->position.halfWidth = in.readDouble();
    this->position.halfHeight = in.readDouble();
    this->position.angle = in.readDouble();

    this->horizontalMargin = in.readDouble();
    this->verticalMargin = in.readDouble();
    xoj_assert(horizontalMargin >= 0.0);
    xoj_assert(verticalMargin >= 0.0);

    size_t count = in.readSizeT();
    this->insertionOrder.reserve(count);
    xoj_assert(this->insertionOrder.empty());

    for (size_t i = 0; i < count; i++) {
        std::string name = in.getNextObjectName();
        std::unique_ptr<Element> element;

        if (name == "Stroke") {
            element = std::make_unique<Stroke>();
        } else if (name == "Image") {
            element = std::make_unique<Image>();
        } else if (name == "TexImage") {
            element = std::make_unique<TexImage>();
        } else if (name == "Text") {
            element = std::make_unique<Text>();
        } else {
            throw InputStreamException(FS(FORMAT_STR("Get unknown object {1}") % name), __FILE__, __LINE__);
        }

        element->readSerialized(in);
        this->insertionOrder.emplace_back(std::move(element));
    }
    in.endObject();
}

auto SelectedContent::applyTranslation(const xoj::util::Point<double>& newCenter, PageRef newPage,
                                       Layer* newLayer) -> UndoActionPtr {
    auto vector = newCenter - this->position.center;
    if (newPage == nullptr) {
        newPage = this->page;
        newLayer = this->layer;
    }
    xoj_assert(newLayer != nullptr);
    auto undoaction = std::make_unique<MoveUndoAction>(this->layer, this->page,
                                                       insertionOrderToElementRefVector(this->insertionOrder), vector.x,
                                                       vector.y, newLayer, newPage);
    this->page = newPage;
    this->layer = newLayer;
    this->position.moveTo(newCenter);
    return undoaction;
}

auto SelectedContent::applyRotation(double newAngle) -> UndoActionPtr {
    auto undoaction = std::make_unique<RotateUndoAction>(
            this->page, insertionOrderToElementRefVector(this->insertionOrder), this->position.center.x,
            this->position.center.y, newAngle - this->position.angle);
    this->position.setAngle(newAngle);
    return undoaction;
}

auto SelectedContent::applyScaling(const Position& newPos, xoj::util::Point<double> scalingCenter,
                                   bool restoreLineWidth) -> UndoActionPtr {
    xoj_assert(newPos.angle == this->position.angle);
    double fx = this->position.halfWidth == 0. ? 1. : newPos.halfWidth / this->position.halfWidth;
    double fy = this->position.halfHeight == 0. ? 1. : newPos.halfHeight / this->position.halfHeight;
    auto undoaction = std::make_unique<ScaleUndoAction>(
            this->page, insertionOrderToElementRefVector(this->insertionOrder), scalingCenter.x, scalingCenter.y, fx,
            fy, this->position.angle, restoreLineWidth);
    this->position = newPos;
    return undoaction;
}

auto SelectedContent::setSize(ToolSize size, const double* thicknessPen, const double* thicknessHighlighter,
                              const double* thicknessEraser) -> std::pair<UndoActionPtr, Range> {
    auto undo = std::make_unique<SizeUndoAction>(this->page, this->layer);
    Range range;

    for (auto& [e, _]: this->insertionOrder) {
        if (e->getType() == ELEMENT_STROKE) {
            auto* s = dynamic_cast<Stroke*>(e.get());
            StrokeTool tool = s->getToolType();

            double originalWidth = s->getWidth();

            auto pointCount = s->getPointCount();
            std::vector<double> originalPressure = SizeUndoAction::getPressure(s);

            if (tool == StrokeTool::PEN) {
                s->setWidth(thicknessPen[size]);
            } else if (tool == StrokeTool::HIGHLIGHTER) {
                s->setWidth(thicknessHighlighter[size]);
            } else if (tool == StrokeTool::ERASER) {
                s->setWidth(thicknessEraser[size]);
            }

            // scale the stroke
            double factor = s->getWidth() / originalWidth;
            xoj_assert(factor > 0.0);
            if (factor > 1.0) {
                s->scalePressure(factor);
                range = range.unite(Range(s->boundingRect()));
            } else {
                range = range.unite(Range(s->boundingRect()));
                s->scalePressure(factor);
            }

            // save the new pressure
            std::vector<double> newPressure = SizeUndoAction::getPressure(s);

            undo->addStroke(s, originalWidth, s->getWidth(), originalPressure, newPressure, pointCount);
        }
    }

    if (!range.empty()) {
        xoj_assert(range.isValid());
        return std::make_pair(std::move(undo), std::move(range));
    }

    return {nullptr, Range()};
}

auto SelectedContent::setFill(int alphaPen, int alphaHighligther) -> std::pair<UndoActionPtr, Range> {
    auto undo = std::make_unique<FillUndoAction>(this->page, this->layer);
    Range range;

    for (auto& [e, _]: this->insertionOrder) {
        if (e->getType() == ELEMENT_STROKE) {
            auto* s = dynamic_cast<Stroke*>(e.get());
            StrokeTool tool = s->getToolType();

            int newFill;
            if (tool == StrokeTool::PEN) {
                newFill = alphaPen;
            } else if (tool == StrokeTool::HIGHLIGHTER) {
                newFill = alphaHighligther;
            } else {
                continue;
            }

            if (newFill == s->getFill()) {
                continue;
            }

            auto originalFill = s->getFill();
            s->setFill(newFill);

            undo->addStroke(s, originalFill, newFill);
            range = range.unite(Range(s->boundingRect()));
        }
    }

    if (!range.empty()) {
        xoj_assert(range.isValid());
        return std::make_pair(std::move(undo), std::move(range));
    }

    return {nullptr, Range()};
}

auto SelectedContent::setFont(const XojFont& font) -> std::pair<UndoActionPtr, Range> {
    auto undo = std::make_unique<FontUndoAction>(this->page, this->layer);
    Range range;

    for (auto& [e, _]: this->insertionOrder) {
        if (e->getType() == ELEMENT_TEXT) {
            Text* t = dynamic_cast<Text*>(e.get());
            undo->addStroke(t, t->getFont(), font);
            range = range.unite(Range(t->boundingRect()));
            t->setFont(font);
            range = range.unite(Range(t->boundingRect()));
        }
    }

    if (!range.empty()) {
        xoj_assert(range.isValid());
        return std::make_pair(std::move(undo), std::move(range));
    }

    return {nullptr, Range()};
}

auto SelectedContent::setLineStyle(LineStyle style) -> std::pair<UndoActionPtr, Range> {
    auto undo = std::make_unique<LineStyleUndoAction>(this->page, this->layer);
    Range range;

    for (auto& [e, _]: this->insertionOrder) {
        if (e->getType() == ELEMENT_STROKE) {
            auto s = static_cast<Stroke*>(e.get());
            auto lastLineStyle = s->getLineStyle();
            if (lastLineStyle == style) {
                continue;
            }
            s->setLineStyle(style);
            undo->addStroke(s, lastLineStyle, s->getLineStyle());
            range = range.unite(Range(s->boundingRect()));
        }
    }

    if (!range.empty()) {
        xoj_assert(range.isValid());
        return std::make_pair(std::move(undo), std::move(range));
    }

    return {nullptr, Range()};
}

auto SelectedContent::setColor(Color color) -> std::pair<UndoActionPtr, Range> {
    auto undo = std::make_unique<ColorUndoAction>(this->page, this->layer);
    Range range;

    for (auto& [e, _]: this->insertionOrder) {
        if (e->getType() == ELEMENT_TEXT || e->getType() == ELEMENT_STROKE) {
            auto lastColor = e->getColor();
            e->setColor(color);
            undo->addStroke(e.get(), lastColor, e->getColor());
            range = range.unite(Range(e->boundingRect()));
        }
    }

    if (!range.empty()) {
        xoj_assert(range.isValid());
        return std::make_pair(std::move(undo), std::move(range));
    }

    return {nullptr, Range()};
}

auto SelectedContent::deleteContent() -> std::pair<UndoActionPtr, Range> {
    auto undo = std::make_unique<DeleteUndoAction>(page, false);

    for (auto& [e, pos]: this->insertionOrder) {
        undo->addElement(layer, std::move(e), pos);
    }
    this->insertionOrder.clear();
    return {std::move(undo), this->computeCoarseBoundingBox(this->position)};
}

auto SelectedContent::rearrangeInsertionOrder(const SelectionOrderChange change) -> std::pair<UndoActionPtr, Range> {
    auto oldOrd = refInsertionOrder(this->insertionOrder);
    std::string desc = _("Arrange");
    switch (change) {
        case SelectionOrderChange::BringToFront:
            for (auto& [_, i]: this->insertionOrder) {
                i = std::numeric_limits<Element::Index>::max();
            }
            break;
        case SelectionOrderChange::BringForward:
            // Set indices of elements to range from [max(indices) + 1, max(indices) + 1 + num elements)
            if (!this->insertionOrder.empty()) {
                Element::Index i = this->insertionOrder.back().pos + 1;
                for (auto& [_, pos]: this->insertionOrder) {
                    pos = i++;
                }
            }
            desc = _("Bring forward");
            break;
        case SelectionOrderChange::SendBackward:
            // Set indices of elements to range from [min(indices) - 1, min(indices) + num elements - 1)
            if (!this->insertionOrder.empty()) {
                Element::Index i = this->insertionOrder.front().pos;
                i = i > 0 ? i - 1 : 0;
                for (auto& [_, pos]: this->insertionOrder) {
                    pos = i++;
                }
            }
            desc = _("Send backward");
            break;
        case SelectionOrderChange::SendToBack:
            Element::Index i = 0;
            for (auto& [_, pos]: this->insertionOrder) {
                pos = i++;
            }
            desc = _("Send to back");
            break;
    }

    auto newOrd = refInsertionOrder(this->insertionOrder);

    return {std::make_unique<ArrangeUndoAction>(page, layer, desc, std::move(oldOrd), std::move(newOrd)),
            computeCoarseBoundingBox(this->position)};
}

auto SelectedContent::getElements() const -> InsertionOrderElementsView {
    return InsertionOrderElementsView(insertionOrder);
}
