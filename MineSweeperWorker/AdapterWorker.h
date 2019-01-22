#pragma once
#include "stdafx.h"
#include "../../MineSweeperProb/MineSweeperSolver/GameMgr.h"
#include "../../MineSweeperProb/MineSweeperSolver/BinomialHelper.h"

class AdapterWorker
{
public:
    virtual ~AdapterWorker() { }

    Configuration Config;

    NO_COPY(AdapterWorker);
    NO_MOVE(AdapterWorker);

protected:
    AdapterWorker() { }

    size_t Run() const;
    void Cache() const;
};

inline size_t AdapterWorker::Run() const
{
    auto imme = true;
    GameMgr mgr(Config.Width, Config.Height, Config.TotalMines, Config, !imme);
    mgr.Automatic();
    if (imme)
        return mgr.GetSucceed() ? 0 : 1;
    return mgr.GetWrongGuesses();
}

inline void AdapterWorker::Cache() const
{
    CacheBinomials(Config.Width * Config.Height, Config.TotalMines);
}
