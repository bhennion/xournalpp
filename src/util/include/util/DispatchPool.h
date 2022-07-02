/*
 * Xournal++
 *
 * Template class for a dispatch pool
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

namespace xoj::util {
template <class listener_type>
class DispatchPool {
public:
    DispatchPool() = default;
    DispatchPool(DispatchPool&&) = default;
    DispatchPool(const DispatchPool&) = default;
    DispatchPool& operator=(DispatchPool&&) = default;
    DispatchPool& operator=(const DispatchPool&) = default;

    template <typename... Args>
    void dispatch(Args&&... args) const {
        for (auto* v: this->pool) { v->on(std::forward<Args>(args)...); }
    }

    void add(listener_type* v) {
        assert(v != nullptr && "Adding nullptr listener");
        assert(std::find(this->pool.begin(), this->pool.end(), v) == this->pool.end() && "Listener is already listed");
        this->pool.emplace_back(v);
    }

    void remove(listener_type* v) {
        assert(v != nullptr && "Adding nullptr listener");
        auto it = std::find(this->pool.begin(), this->pool.end(), v);
        if (it != this->pool.end()) {
            this->pool.erase(it);
        }
    }
private:
    std::vector<listener_type*> pool;
};
};  // namespace xoj::util
