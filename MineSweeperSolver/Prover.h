#pragma once
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
    node_t *find(FCase c, int special = -1);

    [[nodiscard]] auto size() const { return cnt.load(std::memory_order_relaxed); }
};

using node_t = Trie::node_t;

struct BaseCase
{
    BaseCase(PCase p, PGame game);
    explicit BaseCase(PCase p);
    virtual ~BaseCase();

    PCase parent;
    double TotalStates;
    // Depth: number of opened blocks
    // Step: number of actions
    unsigned Depth, Step;
    int Duplication;

    [[nodiscard]] const GameMgr &Game() { return *ThePGame(); }
    [[nodiscard]] PGame ThePGame();
    BaseCase &Deflate();
    void Deplete() { m_Game = std::monostate{}; }

    virtual PCase Fork() = 0;
    PCase CheckedFork();

    virtual bool IsHolder() const { return false; }
    virtual bool IsAction() const { return false; }
    virtual bool ShallDeflate() const { return false; }

    virtual std::string ToString() const;

#ifdef TRACEBACK
    std::string Traceback;
    void PrintTraceback() const;
#else
#define Traceback ""
#endif

    int LargestModifiedIndex;

    void *RegistryNext;
    void *QueueNext;

protected:
    std::variant<std::monostate, std::string, PGame> m_Game;
};

struct ReportingCase
{
    void AssignParent(FCase p);
    void ResolveParents(std::set<ACase> &parents, std::queue<SCase> &sc);

    virtual operator PCase() = 0;
    static auto ToPCase(RCase c) { return c ? c->operator PCase() : nullptr; }

    std::shared_mutex ParentsMtx;
    std::vector<FCase> AdditionalParents;
};

struct ForkedCase : BaseCase
{
    ForkedCase(PCase p, PGame g, int id)
        : BaseCase{ p, g }, Id{ id }, m_Degree{}
    {
        ++Depth;
        LargestModifiedIndex = std::max(LargestModifiedIndex, Id);
    }

    int Id;

    PCase Fork() override;

    std::string ToString() const override;

    auto GetDegree() const { return m_Degree; }

protected:
    int m_Degree;
};

struct HolderCase : BaseCase
{
    HolderCase(PCase p, PGame game, double d)
        : BaseCase{ p, game }, Danger{ d } { }

    // only callable by the single thread calling Fork()
    void AddChildren(ACase v);

    PCase Fork() override { throw std::logic_error{ "Do not call this" }; }

    bool IsHolder() const override { return true; }

    std::string ToString() const override;

    std::atomic<ACase> Child;

    // if Child == nullptr, Danger is the unsafe's intrinsic danger
    // if Child != nullptr, Danger is min(Danger of children)
    // only modifiable by the dedicated thread
    double Danger;

    // only the dedicated thread can call this
    void ResolveDanger();
};

struct ActionCase : ForkedCase
{
    ActionCase(PCase p, PGame g, int id);

    PCase Fork() override;

    bool IsAction() const override { return true; }

    std::string ToString() const override;

    const double IntrinsicDanger;

    // accumulated danger
    // only the dedicated thread can access it
    double Danger;

    ACase Sibling;

    void ResetDanger() { Danger = IntrinsicDanger; }
};

struct SafeCase : ForkedCase, ReportingCase
{
    SafeCase(PCase p, PGame g, int lmi);

    std::string ToString() const override;

    operator PCase() override { return this; }
};

struct UnsafeCase : HolderCase, ReportingCase
{
    UnsafeCase(PCase p, PGame g, int lmi);

    bool ShallDeflate() const override { return true; }

    PCase Fork() override;

    std::string ToString() const override;

    operator PCase() override { return this; }

    // only the dedicated thread can call this
    void ResolveDangerAndReport(bool materialize);

private:
    BlockSet m_List;
    BlockSet::iterator m_It;
    std::atomic<bool> m_IsMaterialized;
};

class CaseRegistry
{
    std::atomic<ACase> m_ActionCases;

    // some statistics, never locked
    std::atomic<unsigned> m_MaxStep;
    std::atomic<size_t> m_Processed, m_Pending;
    // count number of outstanding cases
    std::atomic<size_t> m_ACases, m_SCases, m_UCases;
    std::atomic<size_t> m_ReapedSCases;

    // rlocked by anything below
    // wlocked by m_MaxDepth, m_Reaping, m_Completed change
    mutable boost::upgrade_mutex m_Mutex;

    // a depth has two stages: forking and reaping
    //
    // when m_Reaping == false:
    //   m_PendingD0Cases ===Fork()>>>  m_PendingD1Cases
    //                                  m_D1SafeCases
    //                                  m_UnsafeCases.front()
    //
    // when m_Reaping == true:
    //   m_D0SafeCases    ===>>>  delete
    //
    unsigned m_MaxDepth;
    bool m_Reaping, m_Completed;

    // it is guaranteed that (whenever rlocked by m_Mutex)
    // when m_Reaping == false:
    //   m_D0SafeCases <=> Depth == m_MaxDepth - 1
    //     [[not changing]]
    //   m_D1SafeCases <=> Depth == m_MaxDepth
    //     [[being created]]
    // when m_Reaping == true:
    //   m_D0SafeCases <=> Depth == m_MaxDepth - 2
    //     [[being removed]]
    //   m_D1SafeCases <=> Depth == m_MaxDepth - 1
    //     [[not changing]]
    std::atomic<SCase> m_D0SafeCases, m_D1SafeCases;

    // m_unsafeCases.front() <=> Depth == m_MaxDepth
    std::forward_list<std::atomic<UCase>> m_UnsafeCases;

    // it is guaranteed that (whenever rlocked by m_Mutex)
    // m_PendingD0Cases <=> Depth == m_MaxDepth - 1
    //    only decreasing
    // m_PendingD1Cases <=> Depth == m_MaxDepth
    //    only increasing
    std::atomic<PCase> m_PendingD0Cases, m_PendingD1Cases;

    // locked for ResolveDanger
    // always rlock m_Mutex first then m_MutexRDaRS
    std::mutex m_MutexRDaRS;

    // wait for a stage change
    boost::condition_variable_any m_CVStage;
    // wait for completion
    boost::condition_variable_any m_CVCompletion;

    // you must hold rlock of m_Mutex before calling this!
    void updateMax(std::atomic<unsigned> &v, unsigned d)
    {
        auto old = v.load();
        while (d > old)
            if (v.compare_exchange_weak(old, d))
                break;
        return;
    }

    // you must hold rlock of m_Mutex before calling this!
    template <typename T>
        requires std::derived_from<T, BaseCase>
    void push(std::atomic<T *> &atm, T *v)
    {
        constexpr auto MPtr = std::is_same_v<T, BaseCase> ? &BaseCase::QueueNext : &BaseCase::RegistryNext;
        auto &rn = *reinterpret_cast<T **>(&v->*MPtr);
        rn = atm.load(std::memory_order_acquire);
        while (!atm.compare_exchange_weak(rn, v));
    }

    // you must hold rlock of m_Mutex before calling this!
    // m_Borrowed is increased by one iff succeeded
    template <typename T>
        requires std::derived_from<T, BaseCase>
    auto pop(std::atomic<T *> &atm)
    {
        constexpr auto MPtr = std::is_same_v<T, BaseCase> ? &BaseCase::QueueNext : &BaseCase::RegistryNext;
        auto c = atm.load(std::memory_order_acquire);
        if (!c) return c;
        ++m_Borrowed;
        while (atm.compare_exchange_weak(c, reinterpret_cast<T *>(c->*MPtr), std::memory_order_acquire))
            if (!c) { --m_Borrowed; return c; }
        c->*MPtr = nullptr;
        return c;
    }

    // you must hold rlock of m_Mutex before calling this!
    template <typename T>
        requires std::derived_from<T, BaseCase>
    void foreach(std::atomic<T *> &atm, auto &&fun)
    {
        for (auto ptr = atm.load(std::memory_order_acquire); ptr; ptr = reinterpret_cast<T *>(ptr->RegistryNext))
            fun(ptr);
    }

    // you must hold rlock of m_Mutex before calling this!
    void foreachUnsafeCases(auto &&fun)
    {
        auto it = [this]{ boost::shared_lock lock{ m_Mutex }; return m_UnsafeCases.begin(); }();
        for (; it != m_UnsafeCases.end(); ++it)
            foreach(*it, fun);
    }

    // you must hold rlock of m_Mutex before calling this!
    void Fork(PCase p);
    void Enqueue(PCase p);

public:
    // only indirectly called from CaseRegistry::Fork(PCase)
    void Save(ACase ac);
    void Save(SCase ac);
    void Save(UCase uc);

    // worker thread entry
    void Process();

    template <typename T>
    bool Resolve(T &&t)
    {
        std::shared_lock lock{ m_Mutex };
        m_CVCompletion.wait_for(lock, t);
        return !done();
    }

    template <typename T>
    bool Wait(T &&t)
    {
        std::shared_lock lock{ m_Mutex };
        m_CVCompletion.wait_for(lock, t);
        return !done();
    }

    // anyone can call this
    void ResolveDanger(HCase root);

    auto GetDepth() const
    {
        std::shared_lock lock{ m_Mutex };
        return m_MaxDepth;
    }

    auto GetStep() const { return m_MaxStep.load(std::memory_order_relaxed); }

    auto GetCases() const
    {
        return std::make_tuple(
                m_ACases.load(std::memory_order_relaxed),
                m_SCases.load(std::memory_order_relaxed),
                m_UCases.load(std::memory_order_relaxed)
                );
    }

    auto GetPending() { return m_Pending.load(std::memory_order_relaxed); }
};
