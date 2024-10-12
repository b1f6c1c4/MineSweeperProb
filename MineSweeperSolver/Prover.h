#pragma once
#include "BasicSolver.h"
#include "stdafx.h"
#include "GameMgr.h"
#include "BinomialHelper.h"
#include <atomic>
#include <concepts>
#include <limits>
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <variant>
#include <boost/heap/d_ary_heap.hpp>

struct BaseCase;
struct ReportingCase;
struct ForkedCase;
struct SafeCase;
struct ActionCase;
struct UnsafeCase;

using PCase = BaseCase *;
using RCase = ReportingCase *;
using FCase = ForkedCase *;
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
    unsigned Depth;
    int Duplication;

    [[nodiscard]] const GameMgr &Game() { return *ThePGame(); }
    [[nodiscard]] PGame ThePGame();
    BaseCase &Deflate();
    void Deplete() { m_Game = std::monostate{}; }

    virtual PCase Fork() = 0;
    PCase CheckedFork();

    virtual bool IsHolder() const { return false; }
    virtual bool ShallDeflate() const { return false; }

    virtual std::string ToString() const;

#ifdef TRACEBACK
    std::string Traceback;
    void PrintTraceback() const;
#else
#define Traceback ""
#endif

    int LargestModifiedIndex;

protected:
    std::variant<std::monostate, std::string, PGame> m_Game;
};

struct ReportingCase
{
    virtual double GatherDangerAndAssignParent(ActionCase *c) = 0;
    virtual operator PCase() = 0;
    static auto ToPCase(ReportingCase *c) { return c ? c->operator PCase() : nullptr; }
};

struct ForkedCase : BaseCase
{
    ForkedCase(PCase p, PGame g, int id)
        : BaseCase{ p, g }, Id{ id }, m_Degree{}
    {
        LargestModifiedIndex = std::max(LargestModifiedIndex, Id);
    }

    int Id;

    template <bool Ephermeral>
    RCase Fork();

    std::string ToString() const override;

    auto GetDegree() const { return m_Degree; }

protected:
    int m_Degree;
};

struct HolderCase : BaseCase
{
private:
    struct Comparer
    {
        bool operator()(ActionCase *lhs, ActionCase *rhs) const;
    };

    // the 'largest' elem is the top()
    boost::heap::d_ary_heap<ActionCase *,
        boost::heap::arity<4>,
        boost::heap::compare<Comparer>,
        boost::heap::mutable_<true>> m_Heap;

public:
    using BaseCase::BaseCase;

    using handle_t = decltype(m_Heap)::handle_type;

    void AddChildren(ActionCase *v);

    PCase Fork() override { throw std::logic_error{ "Do not call this" }; }

    bool IsHolder() const override { return true; }

    std::string ToString() const override;

    void ReportDanger(ActionCase *self, double v);

    auto GetDanger() const { return Danger; }

protected:
    // protects m_Heap, this->Danger, and all children's Danger
    mutable std::shared_mutex mtx;

    virtual void ReportDangerUp(double v) { }

    // the amount of danger observed at this case
    // initialized to the min prob of mine in all unopened blocks
    // gradually increases
    // range: 0 ~ TotalState
    // protected by mtx
    double Danger;
};

struct ActionCase : ForkedCase
{
    ActionCase(PCase p, PGame g, int id);

    PCase Fork() override;

    std::string ToString() const override;

    void ReportDanger(double v);

    // accumulated danger; protected by parent->mtx
    double Danger;
    HolderCase::handle_t Handle;
};

struct SafeCase : ForkedCase, ReportingCase
{
    SafeCase(PCase p, PGame g, int lmi)
        : ForkedCase{ p, g, g->GetBestBlockList().front() }
    {
        LargestModifiedIndex = std::max(LargestModifiedIndex, lmi);
    }

    PCase Fork() override;

    std::string ToString() const override;

    double GatherDangerAndAssignParent(ActionCase *c) override;
    operator PCase() override { return this; }

private:
    mutable std::shared_mutex m_Mutex;
    std::vector<RCase> m_Children;
};

struct UnsafeCase : HolderCase, ReportingCase
{
    UnsafeCase(PCase p, PGame g, int lmi);

    bool ShallDeflate() const override { return true; }

    PCase Fork() override;

    std::string ToString() const override;

    double GatherDangerAndAssignParent(ActionCase *c) override;
    operator PCase() override { return this; }

protected:
    void ReportDangerUp(double v) override;

private:
    BlockSet m_List;
    BlockSet::iterator m_It;

    std::vector<ActionCase *> m_AdditionalParents;
};
