#pragma once
#define BOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS
#include <atomic>
#include <boost/thread/pthread/shared_mutex.hpp>
#include <concepts>
#include <condition_variable>
#include <forward_list>
#include <limits>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <shared_mutex>
#include <stdexcept>
#include <variant>
#include "BasicSolver.h"
#include "stdafx.h"
#include "GameMgr.h"
#include "BinomialHelper.h"

struct BaseCase;
struct ReportingCase;
struct ForkedCase;
struct HolderCase;
struct SafeCase;
struct ActionCase;
struct UnsafeCase;
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

class Trie
{
public:
    struct node_t
    {
        std::mutex mtx;
        std::atomic<RCase> p;
        // degree: 9 if unopened
        std::array<std::atomic<node_t *>, 10zu> next;
    };

private:
    node_t root;
    std::atomic<size_t> cnt;
    node_t *ensure(node_t *ptr, int d);

public:
    void Dispose();

    node_t *find(FCase c, int special = -1);

    [[nodiscard]] auto size() const
    {
        return cnt.load(std::memory_order_relaxed) * sizeof(node_t) + sizeof(Trie);
    }
};

using node_t = Trie::node_t;

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

    int LargestModifiedIndex;

    void *RegistryNext;

protected:
    std::variant<std::monostate, std::string, PGame> m_Game;
};

struct ReportingCase
{
    void AssignParent(FCase p);

    virtual operator PCase() = 0;
    const BaseCase *GetAnyParent() const;

protected:
    mutable std::mutex ParentsMtx;
    std::set<ACase> AllParents;
};

struct ForkedCase : BaseCase
{
    ForkedCase(PCase p, PGame g, int id)
        : BaseCase{ p, g }, Id{ id }, m_Degree{}
    {
        LargestModifiedIndex = std::max(LargestModifiedIndex, Id);
    }

    int Id;

    virtual RCase Fork();

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

    RCase Fork() override;

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
    SafeCase(PCase p, PGame g, int lmi);

    std::string ToString() const override;

    operator PCase() override { return this; }

#ifdef TRACEBACK
    using ReportingCase::GetAnyParent;
#endif
};

struct UnsafeCase : HolderCase, ReportingCase
{
    UnsafeCase(PCase p, PGame g, int lmi);

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

class CaseRegistry
{
    std::atomic<ACase> m_ActionCases;

    // some statistics, never locked
    std::atomic<unsigned> m_MaxStep;
    std::atomic<size_t> m_D0, m_D1;
    // count number of outstanding cases
    std::atomic<size_t> m_ACases, m_SCases, m_UCases;

    // rlocked by anything below
    // wlocked by m_MaxDepth, m_Completed change
    mutable boost::upgrade_mutex m_Mutex;
    unsigned m_MaxDepth;
    bool m_Completed;

    // it is guaranteed that (whenever rlocked by m_Mutex)
    //   m_D0SafeCases <=> Depth == m_MaxDepth - 1
    //   m_D1SafeCases <=> Depth == m_MaxDepth
    //   m_D1UnsafeCases <=> Depth == m_MaxDepth
    std::atomic<SCase> m_D0SafeCases, m_D1SafeCases;
    std::atomic<UCase> m_D0UnsafeCases, m_D1UnsafeCases;
    // m_UnsafeCases.front() <=> Depth == m_MaxDepth - 1
    std::forward_list<std::atomic<UCase>> m_UnsafeCases;

    // wait for a stage change (m_MaxDepth or m_Completed)
    boost::condition_variable_any m_CVStage;
    // wait for m_Completed
    boost::condition_variable_any m_CVCompletion;

    // you must hold rlock of m_Mutex before calling this!
    void updateMax(std::atomic<unsigned> &v, unsigned d);

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
            (v.next ? *v.next : v.front) = x;
            v.next = reinterpret_cast<T **>(&x->RegistryNext);
            return v;
        }

        // you must hold rlock of m_Mutex before calling this!
        friend void operator>>(ThreadLocalList<T> &&v, std::atomic<T *> &atm)
        {
            if (!v.next) return;
            *v.next = atm.load(std::memory_order_acquire);
            while (!atm.compare_exchange_weak(*v.next, v.front));
            v.front = nullptr, v.next = nullptr;
        }
    };

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

    using TLLS = ThreadLocalList<SafeCase>;
    using TLLU = ThreadLocalList<UnsafeCase>;

    struct TLL
    {
        TLLS scs;
        TLLU ucs;
    };

    // you must hold rlock of m_Mutex before calling this!
    void Enqueue(RCase c, TLL &rcs);

    // you must hold rlock of m_Mutex before calling this!
    void Process(ACase uc, TLL &rcs);
    void Process(SCase sc, TLL &rcs);
    void Process(UCase uc, TLL &rcs);
    void WriteReport();

    HCase root;
    void ResolveDangerImpl();

public:
    CaseRegistry(HCase root, int id);
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
