#include "AdapterWorker.h"
#include "../MineSweeperSolver/GameMgr.h"
#include "../MineSweeperSolver/BinomialHelper.h"
#include "../MineSweeperSolver/random.h"

AdapterWorker::AdapterWorker() : m_Random(ProperlySeededRandomEngine<std::mt19937>()) { }

size_t AdapterWorker::Run()
{
    int total;
    if (Config.IsTotalMine)
        total = Config.TotalMines;
    else
    {
        std::binomial_distribution<> dist(Config.Width * Config.Height - 1, Config.Probability);
        total = dist(m_Random);
    }

    GameMgr mgr(Config.Width, Config.Height, total, Config, Config.Slack);
    mgr.Automatic();
    if (!Config.Slack)
        return mgr.GetSucceed() ? 0 : 1;
    return mgr.GetWrongGuesses();
}

void AdapterWorker::Cache() const
{
    CacheBinomials(Config.Width * Config.Height, Config.TotalMines);
}
