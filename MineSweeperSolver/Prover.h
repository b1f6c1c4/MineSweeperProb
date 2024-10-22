#pragma once
#include <atomic>
#include <concepts>
#include <forward_list>
#include <limits>
#include <memory>
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

protected:
    std::variant<std::monostate, std::string, PGame> m_Game;
};

struct ReportingCase
{
    void AssignParent(FCase p);
    void ResovleParents(std::set<ACase> &parents, std::queue<SCase> &sc);

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
    SafeCase(PCase p, PGame g, int lmi)
        : ForkedCase{ p, g, g->GetBestBlockList().front() }
    {
        LargestModifiedIndex = std::max(LargestModifiedIndex, lmi);
    }

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
    void ResolveDangerAndReport();

private:
    BlockSet m_List;
    BlockSet::iterator m_It;
};

class CaseRegistry
{
    std::atomic<ACase> m_ActionCases;
    std::atomic<unsigned> m_MaxStep;

    mutable std::shared_mutex m_Mutex;
    unsigned m_MaxDepth;
    std::forward_list<std::atomic<UCase>> m_UnsafeCases;

    void updateMax(std::atomic<unsigned> &v, unsigned d)
    {
        auto old = v.load();
        while (d > old)
            if (v.compare_exchange_weak(old, d))
                break;
        return;
    }

    template <typename T>
        requires std::derived_from<T, BaseCase>
    void push(std::atomic<T *> &atm, T *v)
    {
        auto &rn = *reinterpret_cast<T **>(&v->RegistryNext);
        rn = atm.load(std::memory_order_acquire);
        while (!atm.compare_exchange_weak(rn, v));
    }

    void ForeachActionCases(auto &&fun)
    {
        for (auto ac = m_ActionCases.load(std::memory_order_acquire); ac; ac = reinterpret_cast<ACase>(ac->RegistryNext))
            fun(ac);
    }

    void ForeachUnsafeCases(auto &&fun)
    {
        auto it = [this]{ std::shared_lock lock{ m_Mutex }; return m_UnsafeCases.begin(); }();
        for (; it != m_UnsafeCases.end(); ++it)
            for (auto uc = it->load(std::memory_order_acquire); uc; uc = reinterpret_cast<UCase>(uc->RegistryNext))
                fun(uc);
    }

public:
    void Save(ACase ac);
    void Save(UCase uc);

    // only the dedicated thread can call this
    void ResolveDanger(HCase root);

    auto GetDepth() const
    {
        std::shared_lock lock{ m_Mutex };
        return m_MaxDepth;
    }

    auto GetStep() const { return m_MaxStep.load(std::memory_order_relaxed); }
};
