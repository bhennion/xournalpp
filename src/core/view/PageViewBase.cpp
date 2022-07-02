#include "PageViewBase.h"

using namespace xoj::view;

void PageViewBase::registerToPool(const std::shared_ptr<pool_type>& pool) {
    if (auto p = this->pool.lock()) {
        p->unregister(this);
    }
    this->pool = pool;
    if (auto p = this->pool.lock()) {
        p->register(this);
    }
}

void xoj::view::PageViewBase::unregisterFromPool() {
    if (auto p = this->pool.lock()) {
        p->unregister(this);
    }
    this->pool.reset();
}

auto xoj::view::PageViewBase::getPool() const -> std::shared_ptr<pool_type> {
    return this->pool.lock();
}
