/*
 * Xournal++
 *
 * Template class for a dispatch pool and listeners
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

namespace xoj::util {

/**
 * @brief Generic template for dispatchers.
 *
 * The listeners must implement functions
 *      void on(Arg&&...);
 * that the dispatcher will call through dispatch().
 */
template <class ListenerT>
class DispatchPool final {
public:
    using listener_type = ListenerT;

    template <typename... Args>
    void dispatch(Args&&... args) const {
        for (auto* v: this->pool) {
            v->on(std::forward<Args>(args)...);
        }
    }

    void add(listener_type* v) {
        assert(v != nullptr && "Adding nullptr listener");
        assert(std::find(this->pool.begin(), this->pool.end(), v) == this->pool.end() && "Listener is already listed");
        this->pool.emplace_back(v);
    }

    void remove(listener_type* v) {
        assert(v != nullptr && "Removing nullptr listener");
        auto it = std::find(this->pool.begin(), this->pool.end(), v);
        if (it != this->pool.end()) {
            this->pool.erase(it);
        }
    }

    [[nodiscard]] bool empty() const { return pool.empty(); }

private:
    std::vector<listener_type*> pool;
};

/**
 * @brief CRTP-style class for listener
 * Usage:
 *  class A: Listener<A> {
 *      (virtual) ~A() { unregisterFromPool(); }
 *      void on(...);  // Signal receiver
 *  };
 *
 * WARNING: Always unregisterFromPool() in derived class destructor.
 *
 * For listening to several type of dispatchers do:
 *     class A : public Listener<A> {...};  // virtual void on(...); in here
 *     class B : public Listener<B> {...};  // virtual void on(...); in here - types must differ from the ones in A
 *     class C : public A, public B {...};  // overload everything here
 */
template <class T>
class Listener {
private:
    // Keep the constructor private: only T can inherit Listener<T> (for the static_cast<T*>(this) below).
    Listener() = default;
    friend T;

    using pool_type = xoj::util::DispatchPool<T>;

public:
    /**
     * @brief Register to a new dispatch pool. Unregisters from any pool the listener was previously registered to.
     */
    void registerToPool(const std::shared_ptr<pool_type>& newpool) {
        if (auto p = this->pool.lock()) {
            p->remove(static_cast<T*>(this));
        }
        newpool->add(static_cast<T*>(this));
        this->pool = newpool;
    }

    /**
     * @brief Unregisters from the current pool (if any).
     */
    void unregisterFromPool() {
        if (auto p = this->pool.lock()) {
            p->remove(static_cast<T*>(this));
        }
        this->pool.reset();
    }

    [[nodiscard]] auto getPool() const -> std::shared_ptr<pool_type> { return this->pool.lock(); }

private:
    std::weak_ptr<pool_type> pool;
};
};  // namespace xoj::util