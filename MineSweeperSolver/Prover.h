#pragma once
#include "stdafx.h"
#include "GameMgr.h"
#include "BinomialHelper.h"
#include <memory>

struct BaseCase
{
    explicit BaseCase(std::unique_ptr<GameMgr> &&game);
    explicit BaseCase(std::shared_ptr<BaseCase> p);

    std::shared_ptr<BaseCase> parent;
    double LogProbability;
    int Duplication;

    [[nodiscard]] GameMgr &Game();
    BaseCase &Deflate();

private:
    std::unique_ptr<GameMgr> m_Game;
    std::string m_SavedGame;
};

using PCase = std::shared_ptr<BaseCase>;

struct ActionCase : BaseCase
{
    ActionCase(std::shared_ptr<BaseCase> p, int id)
        : BaseCase{ p }, ActionId{ id } { }

    int ActionId;

    auto &Dist()
    {
        return Game().GetDistInfo(ActionId);
    }
};

struct SafeCase : BaseCase
{
    SafeCase(std::shared_ptr<BaseCase> p, double lp)
        : BaseCase{ p }
    {
        LogProbability += lp;
    }

    auto &Dist()
    {
        return Game().GetDistInfo(Game().GetBestBlockList().front());
    }
};

struct UnsafeCase : BaseCase
{
    UnsafeCase(std::shared_ptr<BaseCase> p, double lp)
        : BaseCase{ p }
    {
        LogProbability += lp;
    }

    std::map<int, std::shared_ptr<ActionCase>> Actions;
};
