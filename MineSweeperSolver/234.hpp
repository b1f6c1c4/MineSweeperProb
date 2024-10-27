#pragma once
#include <atomic>
#include <functional>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <boost/thread/upgrade_mutex.hpp>

// Important limitations:
// 1. Insert-only
// 2. Both K and V are immutable once inserted
// 3. K must support <, ==, !=
// 4. There must be a special value, I, of K s.t. == works
template <typename K, typename V, K I = K{}>
     requires {
         std::is_trivial_v<K>;
         std::is_copy_constructible_v<V>;
         { K a, b; a < b } -> bool;
         { K a, b; a == b } -> bool;
         { K a, b; a != b } -> bool;
     }
class Lockfree234
{
    struct kvp_t
    {
        K first;
        mutable V second;
    };
    static_assert(std::is_trivial_v<kvp_t>, "kvp_t is not trivial");

    struct node_t;
    using ap_t = std::atomic<node_t *>;

    template <kvp_t node_t::*MPtr>
    struct H {};

    enum state_t : unsigned char
    {
        NON_LEAF = 0,
        PROMOTE_P = 1,
        PROMOTE_Q = 2,
        PROMOTE_R = 4,
        AB = 8,
        ABC = 16 | 8,
        LEAF = 128,
    };

#define LX(atm)  unref((atm)).load(std::memory_order_relaxed)
#define LR(atm)  unref((atm)).load(std::memory_order_release)
#define LA(atm)  unref((atm)).load(std::memory_order_acquire)
#define LAR(atm) unref((atm)).load(std::memory_order_acq_rel)
#define SX(atm, v)  unref((atm)).store((v), std::memory_order_relaxed)
#define SR(atm, v)  unref((atm)).store((v), std::memory_order_release)
#define SA(atm, v)  unref((atm)).store((v), std::memory_order_acquire)
#define SAR(atm, v) unref((atm)).store((v), std::memory_order_acq_rel)
#define CASR(atm, x, v)  unref((atm)).compare_exchange_weak((x), (v), std::memory_order_release)
#define CASAR(atm, x, v) unref((atm)).compare_exchange_weak((x), (v), std::memory_order_acq_rel)

    struct alignas(64) node_t
    {
        kvp_t a, b, c;
        std::atomic<state_t> st;
        union {
            boost::upgrade_mutex mtx; // when (st & LEAF)
            struct { // when !(st & LEAF)
                ap_t p, q, r, s;
            };
        };

#define E kvp_t{I}
        explicit node_t(const kvp_t &kvp)
            : a(kvp), b(E), c(E), st{ LEAF }, mtx{} { }
        explicit node_t(const kvp_t &kvp, node_t *pp, node_t *qq)
            : a(kvp), b(E), c(E), st{ NON_LEAF }, p{pp}, q{qq} { }
        template <kvp_t node_t::*MPtr>
        explicit node_t(const node_t *base, H<MPtr>)
            : a(base->*MPtr), b(E), c(E),
              st{ NON_LEAF },
              p{base->*H<MPtr>::left},
              q{base->*H<MPtr>::right}, r{}, s{} { }
#undef E

        [[nodiscard]] auto Is2() const { return b.first == I; }
        [[nodiscard]] auto Is4() const { return c.first != I; }

        void Dispose()
        {
            if (LX(st) & LEAF) return;
            if (p) p->Dispose(), delete p;
            if (q) q->Dispose(), delete q;
            if (r) r->Dispose(), delete r;
            if (s) s->Dispose(), delete s;
        }
    };
    ap_t root;

    std::atomic<size_t> height;

    template <> struct H<&node_t::a>
    { static constexpr node_t *node_t::* left = &node_t::p, *node_t::*right = &node_t::q; };
    template <> struct H<&node_t::b>
    { static constexpr node_t *node_t::* left = &node_t::q, *node_t::*right = &node_t::r; };
    template <> struct H<&node_t::c>
    { static constexpr node_t *node_t::* left = &node_t::r, *node_t::*right = &node_t::s; };

    static constexpr auto &unref(ap_t &atm) { return atm; }
    static constexpr auto &unref(ap_t *atm) { return *atm; }

public:

    kvp_t find(K k) const
    {
        if (k == I) return {};
        auto ptr = LA(root);
        if (!ptr) return {};
    next:
        if (k < ptr->a.first)
        {
            if (ptr->IsLeaf()) return {};
            ptr = LA(ptr->p);
            goto next;
        }
        if (k == ptr->a.first) return &ptr->a;
        if (ptr->b.first == I || k < ptr->b.first)
        {
            if (ptr->IsLeaf()) return {};
            ptr = LA(ptr->q);
            goto next;
        }
        if (k == ptr->b.first) return &ptr->b;
        if (ptr->c.first == I || k < ptr->c.first)
        {
            if (ptr->IsLeaf()) return {};
            ptr = LA(ptr->r);
            goto next;
        }
        if (k == ptr->c.first) return &ptr->c;
        // k > ptr->c.first
        {
            if (ptr->IsLeaf()) return {};
            ptr = LA(ptr->r);
            goto next;
        }
    }

    kvp_t try_emplace(K k, V v)
    {
        if (k == I)
            throw std::logic_error{ "You cannot insert an invalid key" };

        node_t *parent{}, *ptr{ LA(root) };
        if (!ptr) // empty root insert
        {
            auto tmp = new node_t({ k, v });
            while (!CASAR(grand, ptr, tmp))
                if (ptr)
                {
                    // root changed, proceed to normal loop
                    // root must not be empty, as we are insert-only
                    delete tmp;
                    goto next;
                }
            // root change complete
            return tmp->a;
        }
    next:
        if (k == ptr->a.first) return &ptr->a;
        if (k == ptr->b.first) return &ptr->b;
        if (k == ptr->c.first) return &ptr->c;
        if (ptr->Is4())
        {
            if (!parent) // grand == &root, ptr == root
            {
                auto tmp = new node_t(ptr->b,
                        new node_t(ptr, &node_t::a),
                        new node_t(ptr, &node_t::c));
                if (!CASAR(grand, ptr, tmp))
                {
                    // root changed, start over
                    // root must not be empty, as we are insert-only
                    delete tmp->p;
                    delete tmp->q;
                    delete tmp;
                    goto next;
                }
                delete ptr;
                height.fetch_add(1, std::memory_order_relaxed);
                parent = tmp;
                if (k < tmp->a.first)
                    ptr = tmp->p;
                else
                    ptr = tmp->q;
                goto down;
            }
            if (parent->Is4())
                throw std::logic_error{ "4-node's parent must not be 4-node" };
#define RETURN_PARENT(x) \
    { \
        res = &node_t::x; \
        ptr->unlock(); \
        ptr = parent; \
        parent = nullptr; \
        goto finally; \
    }
            node_t *alt_parent{}, *alt_ptr{};
            if (parent->Is2())
            {
                if (parent->a.first < ptr->b.first) // parent->q == ptr
                {
                    if (k == parent->b.first) { return &parent->b };
                    alt_ptr = new node_t(
                    parent->r = new node_t(ptr, H<&node_t::c>{});
                    ptr->c.first = I;
                    parent->b = std::move(ptr->b);
                    ptr->b.first = I;
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
    down:
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
