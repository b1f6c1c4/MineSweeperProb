#pragma once
#define BOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS
#include <atomic>
#include <boost/thread/pthread/shared_mutex.hpp>
#include <concepts>
#include <condition_variable>
#include <forward_list>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <variant>
#include "BasicSolver.h"
#include "stdafx.h"
#include "GameMgr.h"

struct BaseCase;
struct ReportingCase;
struct ForkedCase;
struct HolderCase;
struct SafeCase;
struct ActionCase;
struct UnsafeCase;
class HSPQ;
class CaseRegistry;

using PCase = BaseCase *;
using RCase = ReportingCase *;
using ACase = ActionCase *;
using FCase = ForkedCase *;
using HCase = HolderCase *;
using SCase = SafeCase *;
using UCase = UnsafeCase *;
using PGame = std::shared_ptr<GameMgr>;

#ifndef NDEBUG
#ifndef TRACEBACK
#define TRACEBACK
#endif
#endif

constexpr __uint128_t operator""_ulll(const char *x)
{
    __uint128_t y{};
    for (auto i = 0zu; x[i] != '\0'; i++)
    {
        y *= 10u;
        y += x[i] - '0';
    }
    return y;
}

class HashCache
{
    static constexpr __uint128_t Modulo{
        // a prime p s.t. 10*(p-1)+9 < 2^128 so no overflow happens
        34028236692093846346337460743176821023_ulll };
    static_assert(Modulo * 10u + 1226u == 0u, "Modulo wrong");
    // a primitive root for Z/pZ above
    static constexpr __uint128_t Base{ 10u };
    static std::vector<__uint128_t> cache;
public:
    static void ensure(size_t sz);
    // n=0 for blank, n=1..9 for opened
    static void set(__uint128_t &v, int id, unsigned n);
};

struct BaseCase
{
    BaseCase(PCase p, PGame game);
    explicit BaseCase(PCase p);
    virtual ~BaseCase();

    double TotalStates;
    // Depth: number of opened blocks
    // Step: number of actions
    unsigned Depth, Step;

    [[nodiscard]] const GameMgr &Game() { return *ThePGame(); }
    [[nodiscard]] PGame ThePGame();

    // Convert game to string to reduce memory footprint
    void Deflate();
    // Discard game information
    void Deplete() { m_Game = std::monostate{}; }

    virtual bool IsHolder() const { return false; }
    virtual bool IsAction() const { return false; }

    virtual std::string ToString() const;

#ifdef TRACEBACK
    std::string Traceback;
    void PrintTraceback() const;
    virtual const BaseCase *GetAnyParent() const { return nullptr; }
#else
#define Traceback ""
#endif

    __uint128_t Hash;

    void *RegistryNext;

protected:
    std::variant<std::monostate, std::string, PGame> m_Game;
};

struct ReportingCase
{
    virtual ~ReportingCase() = default;

    void AssignParent(FCase p);

    virtual operator PCase() = 0;
    const BaseCase *GetAnyParent() const;

    virtual __uint128_t Hash() { return operator PCase()->Hash; };
    virtual double TotalStates() { return operator PCase()->TotalStates; };

    void Dismiss()
    {
        if (!Dismissed.test_and_set(std::memory_order_acquire)) {
            operator PCase()->Deplete();
        }
    }

    [[nodiscard]] operator bool()
    {
        return !Dismissed.test(std::memory_order_relaxed);
    }

protected:
    mutable std::mutex ParentsMtx;
    std::set<ACase> AllParents;

    std::atomic_flag Dismissed;
};

struct IgnorableCase : ReportingCase
{
    operator PCase() override { return nullptr; }
    __uint128_t Hash() override { return m_Hash; };
    double TotalStates() override { return 0; };

    explicit IgnorableCase(__uint128_t h)
        : m_Hash{ h }
    {
        Dismissed.test_and_set(std::memory_order_relaxed);
    }
private:
    __uint128_t m_Hash;
};

struct ForkedCase : BaseCase
{
    ForkedCase(PCase p, PGame g, int id)
        : BaseCase{ p, g }, Id{ id }, m_Degree{} { }

    int Id;

    size_t Fork(HSPQ &registry);

    std::string ToString() const override;

    auto GetDegree() const { return m_Degree; }

protected:
    int m_Degree;
};

struct HolderCase : BaseCase
{
    HolderCase(PCase p, PGame game, double d)
        : BaseCase{ p, game }, Danger{ d } { }

    bool IsHolder() const override { return true; }

    std::string ToString() const override;

    // if Child == nullptr, Danger is the unsafe's intrinsic danger
    // if Child != nullptr, Danger is min(Danger of children)
    // only modifiable by the dedicated thread
    double Danger;

    // only the dedicated thread can call this
    void ResolveDanger();

    friend class CaseRegistry;

protected:
    // only callable by the single thread calling Fork()
    void AddChildren(ACase v);

    std::atomic<ACase> Child;
};

struct ActionCase : ForkedCase
{
    ActionCase(HCase p, PGame g, int id);

    // call this once before forking; if true no forking necessary
    [[nodiscard]] bool PrepareFork();

    bool IsAction() const override { return true; }

    std::string ToString() const override;

    const double IntrinsicDanger;

    // accumulated danger
    // only the dedicated thread can access it
    double Danger;

#ifdef TRACEBACK
    HCase Parent;
    const BaseCase *GetAnyParent() const override { return Parent; }
#endif

    ACase Sibling;

    void ResetDanger() { Danger = IntrinsicDanger; }
};

struct SafeCase : ForkedCase, ReportingCase
{
    SafeCase(PCase p, PGame g);

    std::string ToString() const override;

    operator PCase() override { return this; }

#ifdef TRACEBACK
    using ReportingCase::GetAnyParent;
#endif
};

struct UnsafeCase : HolderCase, ReportingCase
{
    UnsafeCase(PCase p, PGame g);

    void ResolveParents();

    ACase Fork();

    // before ResolveParents(): DO NOT CALL
    // after ResolveParents(): thread-safe
    void ReportDanger();

    std::string ToString() const override;

    operator PCase() override { return this; }

#ifdef TRACEBACK
    using ReportingCase::GetAnyParent;
#endif

private:
    BlockSet m_List;
    BlockSet::iterator m_It;
    std::vector<ACase> m_CachedParents;
};

class HSPQ
{
    const size_t m_MaxOccupied, m_ArraySize;
    // Assuming no hash collision!
    std::atomic<RCase> *m_Array;

    struct Comparer
    {
        bool operator()(RCase lhs, RCase rhs) const
        {
            return lhs->operator PCase()->TotalStates
                > rhs->operator PCase()->TotalStates;
        }
    };

    mutable std::mutex m_Mtx;
    size_t m_Occupied;
    double m_Threshold;
    std::vector<RCase> m_Queue;

public:
    // it must holds that mo < as
    HSPQ(size_t mo, size_t as)
        : m_MaxOccupied{ mo }, m_ArraySize{ as },
          m_Array{ new std::atomic<RCase>[as] },
          m_Occupied{}, m_Threshold{ -1.0 } { }
    ~HSPQ() { if (m_Array) delete [] m_Array; }

    [[nodiscard]] RCase Find(__uint128_t hash)
    {
        auto h0 = hash % m_ArraySize;
        for (auto h = h0; ; h++)
        {
            if (h == m_ArraySize) h = 0u;
            auto v = m_Array[h].load(std::memory_order_acquire);
            if (!v)
                return nullptr;
            if (hash == v->Hash())
                return v;
        }
    }

    // obj could be: SafeCase, UnsafeCase, or IgnorableCase
    // returns the old value
    RCase Emplace(RCase obj)
    {
        if (obj->TotalStates() <= m_Threshold)
            return nullptr;
        {
            std::lock_guard lock{ m_Mtx };
            if (m_Occupied >= m_MaxOccupied)
            {
                auto p = m_Queue.front();
                std::pop_heap(m_Queue.begin(), m_Queue.end(), Comparer{});
                m_Threshold = p->TotalStates();
                p->Dismiss();
                m_Queue.back() = obj;
                std::push_heap(m_Queue.begin(), m_Queue.end(), Comparer{});
            }
            else
            {
                m_Occupied++;
                m_Queue.push_back(obj);
                std::push_heap(m_Queue.begin(), m_Queue.end(), Comparer{});
            }
        }
        auto hash = obj->Hash();
        auto h0 = hash % m_ArraySize;
        for (auto h = h0; ; h++) {
            if (h == m_ArraySize) h = 0u;
            auto v = m_Array[h].load(std::memory_order_acquire);
        again:
            if (v && v->Hash() == hash)
                return v;
            if (!v)
            {
                if (m_Array[h].compare_exchange_weak(v, obj,
                            std::memory_order_acq_rel,
                            std::memory_order_acquire))
                    return nullptr;
                goto again;
            }
        }
    }

    // the below are not thread-safe
    void Clear() {
        std::memset(m_Array, 0, m_ArraySize * sizeof(m_Array[0]));
        m_Threshold = -1.0;
        m_Queue = {};
    }

    [[nodiscard]] auto begin() const { return m_Queue.begin(); }
    [[nodiscard]] auto end() const { return m_Queue.end(); }

    [[nodiscard]] double Utilization() const
    {
        std::lock_guard lock{ m_Mtx };
        return static_cast<double>(m_Occupied) / m_ArraySize;
    }
};

template <typename T>
    requires std::derived_from<T, BaseCase>
class ThreadLocalList
{
    T *front, **next;

public:
    ThreadLocalList() : front{}, next{} { }
    explicit ThreadLocalList(T *v)
        : front{ v },
          next{ reinterpret_cast<T **>(&v->RegistryNext) } { }

    friend auto &operator<<(ThreadLocalList<T> &v, T *x)
    {
        (v ? *v.next : v.front) = x;
        v.next = reinterpret_cast<T **>(&x->RegistryNext);
        return v;
    }

    friend void operator>>(ThreadLocalList<T> &&v, ThreadLocalList<T> &d)
    {
        if (!v) return;
        *v.next = d.front;
        d.front = v.front;
        d.next = v.next;
        v.front = nullptr, v.next = nullptr;
    }

    friend void operator>>(ThreadLocalList<T> &&v, std::atomic<T *> &atm)
    {
        if (!v.next) return;
        *v.next = atm.load(std::memory_order_acquire);
        while (!atm.compare_exchange_weak(*v.next, v.front));
        v.front = nullptr, v.next = nullptr;
    }

    [[nodiscard]] operator bool() const { return next; }
};

class CaseRegistry
{
    std::atomic<ACase> m_ActionCases;

    // some statistics, never locked
    std::atomic<unsigned> m_MaxStep;
    std::atomic<size_t> m_D0, m_D1;
    // count number of outstanding cases
    std::atomic<size_t> m_ACases, m_SCases, m_UCases;
    // count memory footprint
    std::atomic<size_t> m_AMem, m_SMem, m_UMem;

    // rlocked by anything below
    // wlocked by m_MaxDepth, m_Completed change
    mutable boost::upgrade_mutex m_Mutex;
    unsigned m_MaxDepth;
    bool m_Completed;

    // it is guaranteed that (whenever rlocked by m_Mutex)
    //   m_D0SafeCases <=> Depth == m_MaxDepth - 1
    //   m_D0UnsafeCases <=> Depth == m_MaxDepth - 1
    std::atomic<SCase> m_D0SafeCases;
    std::atomic<UCase> m_D0UnsafeCases;
    // m_UnsafeCases.front() <=> Depth == m_MaxDepth - 1
    std::forward_list<std::atomic<UCase>> m_UnsafeCases;

    // a thread-safe registry for all D1 cases
    // requires wlock to use its non-thread-safe functions
    HSPQ &m_D1Registry;

    // wait for a stage change (m_MaxDepth or m_Completed)
    boost::condition_variable_any m_CVStage;
    // wait for m_Completed
    boost::condition_variable_any m_CVCompletion;

    // you must hold rlock of m_Mutex before calling this!
    void updateMax(std::atomic<unsigned> &v, unsigned d);

    // you must hold rlock of m_Mutex before calling this!
    template <typename T>
        requires std::derived_from<T, BaseCase>
    auto pop(std::atomic<T *> &atm)
    {
        auto c = atm.load(std::memory_order_acquire);
        if (!c) return c;
        while (!atm.compare_exchange_weak(c, reinterpret_cast<T *>(c->RegistryNext), std::memory_order_acquire))
            if (!c) { return c; }
        c->RegistryNext = nullptr;
        return c;
    }

    // you must hold rlock of m_Mutex before calling this!
    template <typename T>
        requires std::derived_from<T, BaseCase>
    void foreach(std::atomic<T *> &atm, auto &&fun)
    {
        for (auto ptr = atm.load(std::memory_order_acquire); ptr;)
        {
            auto nxt = reinterpret_cast<T *>(ptr->RegistryNext);
            fun(ptr);
            ptr = nxt;
        }
    }

    // you must hold rlock of m_Mutex before calling this!
    void foreachUnsafeCases(auto &&fun)
    {
        for (auto &atm : m_UnsafeCases)
            foreach(atm, fun);
    }

    // you must hold rlock of m_Mutex before calling this!
    void Process(ACase uc);
    void Process(SCase sc);
    void Process(UCase uc);
    void WriteReport();

    // you must hold wlock of m_Mutex before calling this!
    void ShiftD1R();

    HCase root;
    void ResolveDangerImpl();

public:
    CaseRegistry(HCase root, int id, HSPQ &reg);
    void Dispose();

    // worker thread entry
    void Process();

    template <typename T>
    bool WriteReport(T &&t)
    {
        boost::shared_lock lock{ m_Mutex };
        m_CVStage.wait_for(lock, t);
        WriteReport();
        return !m_Completed;
    }

    template <typename T>
    bool Wait(T &&t)
    {
        boost::shared_lock lock{ m_Mutex };
        m_CVCompletion.wait_for(lock, t);
        return !m_Completed;
    }

    // anyone can call this at any time
    void ResolveDanger();
};
