#pragma once
#include "BasicSolver.h"
#include "stdafx.h"
#include "GameMgr.h"
#include "BinomialHelper.h"
#include <atomic>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <variant>
#include <boost/heap/fibonacci_heap.hpp>

struct BaseCase;
using PCase = BaseCase *;
using PGame = std::shared_ptr<GameMgr>;

#ifndef NDEBUG
#define TRACEBACK
#endif

struct BaseCase
{
    BaseCase(PCase p, PGame game);
    explicit BaseCase(PCase p);
    virtual ~BaseCase();

    PCase parent;
    double TotalStates;
    unsigned Depth;
    int Duplication;

    [[nodiscard]] GameMgr &Game() { return *ThePGame(); }
    [[nodiscard]] PGame ThePGame();
    BaseCase &Deflate();
    virtual void Deplete() { m_Game = std::monostate{}; }

    virtual PCase Fork() = 0;

    virtual std::string ToString() const;

#ifdef TRACEBACK
    std::string Traceback;
#else
#define Traceback ""
#endif

protected:
    std::variant<std::monostate, std::string, PGame> m_Game;
};

struct ForkedCase : BaseCase
{
    ForkedCase(PCase p, PGame g, int id)
        : BaseCase{ p, g }, Id{ id }, m_Degree{} { }

    int Id;

    template <bool Ephermeral>
    PCase Fork();

    std::string ToString() const override;

    auto GetDegree() const { return m_Degree; }

protected:
    int m_Degree;
};

struct ActionCase;

struct HolderCase : BaseCase
{
private:
    mutable std::mutex mtx; // protects m_Heap and all children's Danger

    struct Comparer
    {
        bool operator()(ActionCase *lhs, ActionCase *rhs) const;
    };

    // the 'largest' elem is the top()
    boost::heap::fibonacci_heap<ActionCase *, boost::heap::compare<Comparer>> m_Heap;

public:
    using BaseCase::BaseCase;

    using handle_t = decltype(m_Heap)::handle_type;
    using value_t = decltype(*m_Heap.ordered_begin());

    void AddChildren(ActionCase *v);

    PCase Fork() override { throw std::logic_error{ "Do not call this" }; }

    std::string ToString() const override;

    void ReportDanger(ActionCase *self, double v);

    auto GetDanger() const { return Danger; }

protected:
    // the amount of danger observed at this case
    // initialized to the min prob of mine in all unopened blocks
    // gradually increases
    // range: 0 ~ TotalState
    // protected by mtx
    double Danger;
};

struct ActionCase : ForkedCase
{
    ActionCase(PCase p, PGame g, int id)
        : ForkedCase{ p, g, id },
          Danger{ Game().GetBlockProbability(id) * TotalStates } { }

#ifdef NDEBUG
    virtual void Deplete() override
    {
        if (m_Degree)
            BaseCase::Deplete();
        else
            delete this;
    }
#endif

    PCase Fork() override;

    std::string ToString() const override;

    void ReportDanger(double v);

    // accumulated danger; protected by parent->mtx
    double Danger;
    HolderCase::handle_t Handle;
};

struct SafeCase : ForkedCase
{
    SafeCase(PCase p, PGame g)
        : ForkedCase{ p, g, g->GetBestBlockList().front() } { }

#ifdef NDEBUG
    virtual void Deplete() override { delete this; }
#endif

    PCase Fork() override { return ForkedCase::Fork<true>(); }

    std::string ToString() const override;
};

struct UnsafeCase : HolderCase
{
    UnsafeCase(PCase p, PGame g)
        : HolderCase{ p, g },
          m_It{ Game().GetPreferredBlockList().begin() }
    {
        Duplication = g->GetPreferredBlockCount();
        ReportDanger(nullptr, Game().GetMinProbability() * TotalStates);
    }

    PCase Fork() override;

    std::string ToString() const override;

private:
    BlockSet::const_iterator m_It;
};
