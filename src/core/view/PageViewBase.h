/*
 * Xournal++
 *
 * Template class for a pool of views, for repaint/rerender event dispatching
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>

class Range;

namespace xoj::util {
template<typename T> class Rectangle;

template<typename listener_type> class DispatchPool;
};

namespace xoj::view {

class PageViewBase {
public:
    PageViewBase() = default;
    PageViewBase(PageViewBase&&) = default;
    PageViewBase(const PageViewBase&) = default;
    PageViewBase& operator=(PageViewBase&&) = default;
    PageViewBase& operator=(const PageViewBase&) = default;
    
    using pool_type = xoj::util::DispatchPool<PageViewBase>;
    
    struct RENDER_t {};
    struct RENDER_PAGE_t {};
    virtual void on(RENDER_t, const xoj::util::Rectangle<double>&) {}
    virtual void on(RENDER_t, const Range&) {}
    virtual void on(RENDER_PAGE_t) {}
    
    struct PAINT_t {};
    struct PAINT_PAGE_t {};
    virtual void on(PAINT_t, const xoj::util::Rectangle<double>&) {}
    virtual void on(PAINT_t, const Range&) {}
    virtual void on(PAINT_PAGE_t) {}
    
    void registerToPool(const std::shared_ptr<pool_type>&);
    void unregisterFromPool();
    std::shared_ptr<pool_type> getPool() const;

private:
    std::weak_ptr<pool_type> pool;
};

using PageViewPool = PageViewBase::pool_type;
using PageViewPoolRef = std::shared_ptr<PageViewPool>;

inline constexpr PageViewBase::RENDER_t RENDER_REQUEST;
inline constexpr PageViewBase::PAINT_t PAINT_REQUEST;
inline constexpr PageViewBase::RENDER_PAGE_t PAGE_RENDER_REQUEST;
inline constexpr PageViewBase::PAINT_PAGE_t PAGE_PAINT_REQUEST;


};  // namespace xoj::view
