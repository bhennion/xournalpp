#pragma once

#include <string_view>  // for string_view

#include <gtk/gtk.h>

#include "util/glib_casts.h"


namespace xoj::util::signal {
/**
 * Valid specializations of Signals should contain
 *      * A type called callback_type
 *      * Either
 *          * a function `static constexpr const char* name() { return "the signal identifier string"; }`
 *          * a function `static constexpr bool matches(const char* str)` returning true if this specialization should
 *            be used for the signal corresponding to the given parameter string. (See e.g. Signals<GObject, 0> for
 *            "notify::some_property_name" signals)
 *
 * For each class, there should be one specialization per signal Signals<MyGObject, 0>, Signals<MyGObject, 1> and so on.
 * The indices must start at 0 and be consecutive. See gtk-signals.h for examples
 */
template <class GtkObj, size_t N>
struct Signals {};
}  // namespace xoj::util::signal

#include "gtk-signals.h"  // for specializations of Signals

namespace xoj::util::signal {

/// Specialization for GObject::notify signal class
template <>
struct Signals<GObject, 0> {
    static constexpr bool matches(const char* str) { return std::string_view(str, 8) == "notify::"; }
    using callback_type = void(GObject*, GParamSpec*, gpointer user_data);
};

inline namespace details {
template <typename signal, typename U = void>
struct matches {
    template <typename str>
    static constexpr bool match() {
        return std::string_view(signal::name()) == std::string_view(str::str());
    }
};

template <typename signal>
struct matches<signal, std::void_t<decltype(signal::matches)>> {
    template <typename str>
    static constexpr bool match() {
        return signal::matches(str::str());
    }
};


template <class T, typename sig, size_t N>
struct signal_trait;

template <class T, typename sig, size_t N, typename U = void>
struct get_next_callback {
    using type = std::false_type;
};
template <class T, typename sig, size_t N>
struct get_next_callback<T, sig, N, std::void_t<typename Signals<T, N + 1>::callback_type>> {
    using type = typename signal_trait<T, sig, N + 1>::callback_type;
};

template <class T, typename sig, size_t N = 0>
struct signal_trait {
    using callback_type =
            std::conditional_t<matches<Signals<T, N>>::template match<sig>(), typename Signals<T, N>::callback_type,
                               typename get_next_callback<T, sig, N>::type>;
};

#ifndef NDEBUG
struct testsigone {
    constexpr static const char* str() { return "enable-debugging"; }
};
static_assert(std::is_same_v<typename signal_trait<GtkWindow, testsigone>::callback_type,
                             gboolean(GtkWindow*, gboolean toggle, gpointer user_data)>);

struct testsigtwo {
    constexpr static const char* str() { return "focus"; }
};
static_assert(std::is_same_v<typename signal_trait<GtkWidget, testsigtwo>::callback_type,
                             gboolean(GtkWidget*, GtkDirectionType direction, gpointer user_data)>);

struct testsigmenu {
    constexpr static const char* str() { return "activate"; }
};
static_assert(std::is_same_v<typename signal_trait<GtkMenuItem, testsigmenu>::callback_type,
                             void(GtkMenuItem*, gpointer user_data)>);

struct testsigthree {
    constexpr static const char* str() { return "notify::foobar"; }
};
static_assert(std::is_same_v<typename signal_trait<GObject, testsigthree>::callback_type,
                             void(GObject*, GParamSpec*, gpointer user_data)>);
#endif
}  // namespace details

template <typename DataType, void (*deleter)(DataType*, GClosure*)>
void closure_notify(gpointer data, GClosure* closure) {
    deleter(static_cast<DataType*>(data), closure);
}

template <typename sig, auto cb, auto deleter, typename DataType, typename GtkObj>
gulong connect(GtkObj* self, DataType data) {
    static_assert(std::is_same_v<std::remove_pointer_t<decltype(wrap_v<cb>)>,
                                 typename signal_trait<GtkObj, sig>::callback_type>);
    using LastArg = typename xoj::util::detail::FunctionTraits<decltype(cb)>::LastArg;
    static_assert(std::is_pointer_v<LastArg>);
    static_assert(std::is_same_v<DataType, std::nullptr_t> ||
                  (std::is_pointer_v<DataType> &&
                   (std::is_same_v<LastArg, DataType> || std::is_base_of_v<LastArg, DataType>)));
    constexpr auto del = std::is_null_pointer_v<DataType> || (deleter == nullptr) ?
                                 nullptr :
                                 closure_notify<std::remove_pointer_t<DataType>, deleter>;
    // printf("connecting %s object to signal %s with cb %p and deleter %p\n", typeid(GtkObj).name(), sig::str(),
    // wrap_v<cb>, del);
    return g_signal_connect_data(self, sig::str(), G_CALLBACK(wrap_v<cb>), data, del, GConnectFlags(0));
}
template <typename sig, auto cb, typename GtkObj, typename DataType>
gulong connect_object(GtkObj* self, DataType data) {
    static_assert(std::is_same_v<std::remove_pointer_t<decltype(wrap_v<cb>)>,
                                 typename signal_trait<GtkObj, sig>::callback_type>);
    using LastArg = typename xoj::util::detail::FunctionTraits<decltype(cb)>::LastArg;
    static_assert(std::is_pointer_v<LastArg>);
    static_assert(std::is_same_v<DataType, std::nullptr_t> ||
                  (std::is_pointer_v<DataType> &&
                   (std::is_same_v<LastArg, DataType> || std::is_base_of_v<LastArg, DataType>)));
    return g_signal_connect_object(self, sig::str(), G_CALLBACK(wrap_v<cb>), data, GConnectFlags(0));
}

#define xoj_signal_connect(self, sig, cb, data)                                       \
    [&]() {                                                                           \
        constexpr auto callback = cb;                                                 \
        struct signalname {                                                           \
            static constexpr const char* str() { return sig; }                        \
        };                                                                            \
        return xoj::util::signal::connect<signalname, callback, nullptr>(self, data); \
    }()

#define xoj_signal_connect_data(self, sig, cb, data, deleter)                     \
    [&]() {                                                                       \
        constexpr auto callback = cb;                                             \
        constexpr auto del = deleter;                                             \
        struct signalname {                                                       \
            static constexpr const char* str() { return sig; }                    \
        };                                                                        \
        return xoj::util::signal::connect<signalname, callback, del>(self, data); \
    }()

#define xoj_signal_connect_object(self, sig, cb, data)                              \
    [&]() {                                                                         \
        constexpr auto callback = cb;                                               \
        struct signalname {                                                         \
            static constexpr const char* str() { return sig; }                      \
        };                                                                          \
        return xoj::util::signal::connect_object<signalname, callback>(self, data); \
    }()
}  // namespace xoj::util::signal
