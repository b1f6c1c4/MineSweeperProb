#pragma once
#include <atomic>
#include <functional>
#include <optional>
#include <stdexcept>
#include <utility>

template <typename K, typename V,
     typename Comparer = std::less<K>,
     K I = K{}>
class Concurrent234
{
    using kvp_t = std::pair<K, V>;

    struct node_t;

    template <kvp_t node_t::*MPtr>
    struct H {};

    struct alignas(64) node_t
    {
        kvp_t    a,  b,  c;
        node_t *p, *q, *r, *s{};
        mutable std::atomic_flag mtx; // true if being modified

#define E std::make_pair<K, V>(I, V{})
        node_t() : a(E), b(E), c(E), p{}, q{}, r{}, s{} { }
        template <kvp_t node_t::*MPtr>
        explicit node_t(node_t *base, H<MPtr> h)
            : a(std::move(base->*MPtr)), b(E), c(E),
              p{[&]{
                  auto tmp = base->*decltype(h)::left;
                  base->*decltype(h)::left = nullptr;
                  return tmp;
              }()},
              q{[&]{
                  auto tmp = base->*decltype(h)::right;
                  base->*decltype(h)::right = nullptr;
                  return tmp;
              }()},
              r{}, s{} { base->*MPtr = E; }
#undef E

        void lock() const { while (mtx.test_and_set(std::memory_order_acquire)); }
        void unlock() const { mtx.clear(std::memory_order_release); }

        [[nodiscard]] auto Is2() const { return b.first == I; }
        [[nodiscard]] auto Is4() const { return c.first != I; }
        // 2-3-4 tree is always full
        [[nodiscard]] auto IsLeaf() const { return !p; }

        void Dispose()
        {
            if (p) p->Dispose(), delete p;
            if (q) q->Dispose(), delete q;
            if (r) r->Dispose(), delete r;
            if (s) s->Dispose(), delete s;
        }
    } root;

    std::atomic<size_t> height;

    template <> struct H<&node_t::a>
    { static constexpr node_t *node_t::* left = &node_t::p, *node_t::*right = &node_t::q; };
    template <> struct H<&node_t::b>
    { static constexpr node_t *node_t::* left = &node_t::q, *node_t::*right = &node_t::r; };
    template <> struct H<&node_t::c>
    { static constexpr node_t *node_t::* left = &node_t::r, *node_t::*right = &node_t::s; };

    class result_t
    {
        const node_t *node;
    public:
        result_t(node_t *ptr, kvp_t node_t::*mptr)
            : node{ ptr }, first{ (ptr->*mptr).first }, second{ (ptr->*mptr).second } { };
        ~result_t() { if (node) node->unlock(); }
        const K first;
        V &second;
    };

public:
    std::optional<result_t> find(K k)
    {
        if (k == I) return {};
        node_t *parent{}, *ptr{ &root };
        kvp_t node_t::*res{};
    next:
        ptr->lock();
        if (k < ptr->a.first)
        {
            if (ptr->IsLeaf())
                goto finally;
            if (parent) parent->unlock();
            parent = ptr;
            ptr = ptr->p;
            goto next;
        }
        if (k == ptr->a.first) { res = &node_t::a; goto finally; }
        if (ptr->b.first == I || k < ptr->b.first)
        {
            if (ptr->IsLeaf())
                goto finally;
            if (parent) parent->unlock();
            parent = ptr;
            ptr = ptr->q;
            goto next;
        }
        if (k == ptr->b.first) { res = &node_t::b; goto finally; }
        if (ptr->c.first == I || k < ptr->c.first)
        {
            if (ptr->IsLeaf())
                goto finally;
            if (parent) parent->unlock();
            parent = ptr;
            ptr = ptr->r;
            goto next;
        }
        if (k == ptr->c.first) { res = &node_t::c; goto finally; }
        // k > ptr->c.first
        {
            if (ptr->IsLeaf())
                goto finally;
            if (parent) parent->unlock();
            parent = ptr;
            ptr = ptr->r;
            goto next;
        }

    finally:
        if (parent) parent->unlock();
        if (!res)
        {
            ptr->unlock();
            return {};
        }
        return result_t{ ptr, res };
    }

    template <typename ... Args>
    result_t try_emplace(K k, Args &&... args)
    {
        if (k == I)
            throw std::logic_error{ "You cannot insert an invalid key" };
        node_t *parent{}, *ptr{ &root };
        kvp_t node_t::*res{};
    next:
        ptr->lock();
        if (k == ptr->a.first) { res = &node_t::a; goto finally; }
        if (k == ptr->b.first) { res = &node_t::b; goto finally; }
        if (k == ptr->c.first) { res = &node_t::c; goto finally; }
        if (ptr->Is4())
        {
            if (!parent) // ptr == &root
            {
                height.fetch_add(1, std::memory_order_relaxed);
                ptr->p = new node_t(ptr, H<&node_t::a>{});
                ptr->q = new node_t(ptr, H<&node_t::c>{});
                ptr->a = std::move(ptr->b);
                ptr->b.first = I;
                parent = ptr;
                if (k < ptr->a.first)
                    ptr = ptr->p;
                else
                    ptr = ptr->q;
            }
            else if (parent->Is4())
                throw std::logic_error{ "4-node's parent must not be 4-node" };
#define RETURN_PARENT(x) \
    { \
        res = &node_t::x; \
        ptr->unlock(); \
        ptr = parent; \
        parent = nullptr; \
        goto finally; \
    }
            else if (parent->Is2())
            {
                if (parent->a.first < ptr->b.first) // parent->q == ptr
                {
                    parent->r = new node_t(ptr, H<&node_t::c>{});
                    ptr->c.first = I;
                    parent->b = std::move(ptr->b);
                    ptr->b.first = I;
                    if (k == parent->b.first) RETURN_PARENT(b);
                    if (k > parent->b.first)
                    {
                        ptr->unlock();
                        (ptr = parent->r)->lock();
                    }
                }
                else // parent.p == ptr
                {
                    parent->r = parent->q;
                    parent->q = new node_t(ptr, H<&node_t::c>{});
                    ptr->c.first = I;
                    parent->b = std::move(parent->a);
                    parent->a = std::move(ptr->b);
                    ptr->b.first = I;
                    if (k == parent->a.first) RETURN_PARENT(a);
                    if (k > parent->a.first)
                    {
                        ptr->unlock();
                        (ptr = parent->q)->lock();
                    }
                }
            }
            else // parent is 3-node (i.e. parent->c.first == I)
            {
                if (parent->b.first < ptr->b.first) // parent->r == ptr
                {
                    parent->s = new node_t(ptr, H<&node_t::c>{});
                    ptr->c.first = I;
                    parent->c = std::move(ptr->b);
                    ptr->b.first = I;
                    if (k == parent->c.first) RETURN_PARENT(c);
                    if (k > parent->c.first)
                    {
                        ptr->unlock();
                        (ptr = parent->s)->lock();
                    }
                }
                else if (parent->a.first > ptr->b.first) // parent->p == ptr
                {
                    parent->s = parent->r;
                    parent->r = parent->q;
                    parent->q = new node_t(ptr, H<&node_t::c>{});
                    ptr->c.first = I;
                    parent->c = std::move(parent->b);
                    parent->b = std::move(parent->a);
                    parent->a = std::move(ptr->b);
                    ptr->b.first = I;
                    if (k == parent->a.first) RETURN_PARENT(a);
                    if (k > parent->a.first)
                    {
                        ptr->unlock();
                        (ptr = parent->q)->lock();
                    }
                }
                else // parent->q == ptr
                {
                    parent->s = parent->r;
                    parent->r = new node_t(ptr, H<&node_t::c>{});
                    ptr->c.first = I;
                    parent->c = std::move(parent->b);
                    parent->b = std::move(ptr->b);
                    ptr->b.first = I;
                    if (k == parent->b.first) RETURN_PARENT(b);
                    if (k > parent->b.first)
                    {
                        ptr->unlock();
                        (ptr = parent->r)->lock();
                    }
                }
            }
        }
        // here we've ensured that ptr->c.first == I
#define RETURN_MAKE(x) do { \
        ptr->x = std::make_pair(k, std::forward<Args>(args)...); \
        res = &node_t::x; \
        goto finally; \
    } while (false)
        if (ptr->a.first == I)
        {
            height.fetch_add(1, std::memory_order_relaxed);
            RETURN_MAKE(a);
        }
        if (k < ptr->a.first)
        {
            if (ptr->IsLeaf())
            {
                ptr->c = std::move(ptr->b);
                ptr->b = std::move(ptr->a);
                RETURN_MAKE(a);
            }
            if (parent) parent->unlock();
            parent = ptr;
            ptr = ptr->p;
            goto next;
        }
        if (k == ptr->a.first) { res = &node_t::a; goto finally; }
        if (ptr->b.first == I || k < ptr->b.first)
        {
            if (ptr->IsLeaf())
            {
                ptr->c = std::move(ptr->b);
                RETURN_MAKE(b);
            }
            if (parent) parent->unlock();
            parent = ptr;
            ptr = ptr->q;
            goto next;
        }
        if (k == ptr->b.first) { res = &node_t::b; goto finally; }
        // k > ptr->b.first
        {
            if (ptr->IsLeaf())
                RETURN_MAKE(c);
            if (parent) parent->unlock();
            parent = ptr;
            ptr = ptr->r;
            goto next;
        }

    finally:
        if (parent) parent->unlock();
        return { ptr, res };
    }

    [[nodiscard]] auto get_height() const
    {
        return height.load(std::memory_order_relaxed);
    }

    // thread-unsafe!!
    void clear()
    {
        root.Dispose();
        height.store(0, std::memory_order_relaxed);
        root.a.first = I;
    }

    // thread-unsafe!!
    // guaranteed to be in order!
    // lbe is exclusive
    void foreach(K lbe, auto &&fun)
    {
        if (root.a.first == I) return;
        auto started = false;
        [&](this auto &&self, node_t *ptr){
            if (!ptr) return;
            if (!started) // search for lbe
            {
                if (lbe < ptr->a.first) goto p;
                if (lbe == ptr->a.first) { started = true; goto q; }
                if (ptr->b.first == I || lbe < ptr->b.first) goto q;
                if (lbe == ptr->b.first) { started = true; goto r; }
                if (ptr->c.first == I || lbe < ptr->c.first) goto r;
                if (lbe == ptr->c.first) { started = true; goto s; }
                /* lb > ptr->c.first */ goto s;
            }
        p:
            self(ptr->p);
            fun(ptr->a.first, ptr->a.second);
        q:
            self(ptr->q);
            if (ptr->b.first == I) return;
            fun(ptr->b.first, ptr->b.second);
        r:
            self(ptr->r);
            if (ptr->c.first == I) return;
            fun(ptr->c.first, ptr->c.second);
        s:
            self(ptr->s);
        }(&root);
    }
};
