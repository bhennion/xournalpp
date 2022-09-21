/*
 * Xournal++
 *
 * Variant for storing derived classes of the same base
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>      // for size_t
#include <type_traits>  // for bool_constant
#include <utility>      // for forward
#include <variant>      // for variant

template <typename I, class B, class... Ts>
class SameBaseVariant {
private:
    using variant_type = std::variant<std::monostate, Ts...>;
    using base_type = B;
    using index_type = I;

    static_assert(std::bool_constant<(... && std::is_base_of<base_type, Ts>{})>::value);

public:
    void reset() {
        this->data.template emplace<std::monostate>();
        this->ptr = nullptr;
    }

    operator bool() const { return this->data.index() != 0; }

    template <class T, class... Args>
    T& emplace(Args&&... args) {
        static_assert(!std::is_same<T, std::monostate>::value);
        T& object = this->data.template emplace<T>(std::forward<Args>(args)...);
        ptr = &object;
        return object;
    }

    auto getType() const -> index_type { return static_cast<index_type>(this->data.index()); }

    template <class T>
    auto get() -> T* {
        static_assert(!std::is_same<T, std::monostate>::value);
        return &std::get<T>(this->data);
    }

    template <class T>
    auto get() const -> const T* {
        static_assert(!std::is_same<T, std::monostate>::value);
        return &std::get<T>(this->data);
    }

    template <class T>
    auto get_if() -> T* {
        static_assert(!std::is_same<T, std::monostate>::value);
        return std::get_if<T>(&this->data);
    }

    template <class T>
    auto get_if() const -> const T* {
        static_assert(!std::is_same<T, std::monostate>::value);
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
