#pragma once
#include "BasicSolver.h"
#include "stdafx.h"
#include "GameMgr.h"
#include "BinomialHelper.h"
#include <memory>
#include <variant>

struct BaseCase;
using PCase = BaseCase *;
using PGame = std::shared_ptr<GameMgr>;

struct BaseCase
{
    BaseCase(PCase p, PGame game);
    explicit BaseCase(PCase p);
    virtual ~BaseCase() = default;

    PCase parent;
    double TotalStates;
    int Duplication;

    [[nodiscard]] GameMgr &Game() { return *PGame(); }
    [[nodiscard]] PGame PGame();
    BaseCase &Deflate();

    virtual PCase Fork();

private:
    std::variant<std::string, ::PGame> m_Game;
};

struct ForkedCase : BaseCase
{
    ForkedCase(PCase p, ::PGame g, int id)
        : BaseCase{ p, g }, Id{ id }, m_Degree{} { }

    int Id;

    PCase Fork() override;

private:
    int m_Degree;
};

struct ActionCase : ForkedCase
{
};

struct SafeCase : ForkedCase
{
    SafeCase(PCase p, ::PGame g)
        : ForkedCase{ p, g, g->GetBestBlockList().front() } { }
};

struct UnsafeCase : BaseCase
{
    UnsafeCase(PCase p, ::PGame g)
        : BaseCase{ p, g }
    {
        Duplication = g->GetPreferredBlockCount();
    }

    PCase Fork() override;

    std::map<int, ActionCase *> Actions;

private:
    BlockSet::const_iterator m_It;
};
