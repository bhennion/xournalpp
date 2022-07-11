/*
 * Xournal++
 *
 * Variant for storing all possible tool handlers and avoid constant reallocation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>      // for size_t
#include <type_traits>  // for bool_constant, false_type
#include <utility>      // for forward
#include <variant>      // for variant

#include "ArrowHandler.h"
#include "CoordinateSystemHandler.h"
#include "EllipseHandler.h"
#include "RectangleHandler.h"
#include "RulerHandler.h"
#include "SplineHandler.h"
#include "StrokeHandler.h"

class InputHandlerVariant final {
public:
    // The following enum must correspond to the list of type of the std::variant below
    enum HandlerType : size_t {
        NONE,
        ARROW_HANDLER,
        COORDINATE_SYSTEM_HANDLER,
        ELLIPSE_HANDLER,
        RECTANGLE_HANDLER,
        RULER_HANDLER,
        SPLINE_HANDLER,
        STROKE_HANDLER
    };

private:
    // The following list of types must correspond to enum HandlerType above
    using variant_type = std::variant<std::monostate,           // NONE
                                      ArrowHandler,             // ARROW_HANDLER
                                      CoordinateSystemHandler,  // COORDINATE_SYSTEM_HANDLER
                                      EllipseHandler,           // ELLIPSE_HANDLER
                                      RectangleHandler,         // RECTANGLE_HANDLER
                                      RulerHandler,             // RULER_HANDLER
                                      SplineHandler,            // SPLINE_HANDLER
                                      StrokeHandler             // STROKE_HANDLER
                                      >;

    using base_type = InputHandler;


    template <class T, class VariantType>
    struct is_variant_derived_of: std::false_type {};

    template <class T, class... Ts>
    struct is_variant_derived_of<T, std::variant<std::monostate, Ts...>>:
            std::bool_constant<(... && std::is_base_of<T, Ts>{})> {};

    static_assert(is_variant_derived_of<base_type, variant_type>::value);

public:
    void reset();

    operator bool() const;

    template <class T, class... Args>
    T& emplace(Args&&... args) {
        static_assert(!std::is_same<T, std::monostate>::value);
        T& object = this->data.emplace<T>(std::forward<Args>(args)...);
        ptr = &object;
        return object;
    }

    auto getType() const -> HandlerType;

    template <class T>
    auto get() -> T* {
        return &std::get<T>(this->data);
    }

    template <class T>
    auto get() const -> const T* {
        return &std::get<T>(this->data);
    }

    template <class T>
    auto get_if() -> T* {
        return std::get_if<T>(&this->data);
    }

    template <class T>
    auto get_if() const -> const T* {
        return std::get_if<T>(&this->data);
    }

    auto get() -> base_type* { return ptr; }
    auto get() const -> const base_type* { return ptr; }

    base_type* operator->() { return ptr; }
    const base_type* operator->() const { return ptr; }


private:
    variant_type data;
    base_type* ptr = nullptr;
};
