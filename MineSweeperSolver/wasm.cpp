#include "stdafx.h"
#include "BinomialHelper.h"
#include "GameMgr.h"
#include "random.h"
#include "Drainer.h"
#include "facade.hpp"
#include <fstream>

#ifndef __EMSCRIPTEN__

#define DLL_API

#else // __EMSCRIPTEN__

#include <emscripten/bind.h>
using namespace emscripten;

#endif // __EMSCRIPTEN__

int main() { }

auto parse(const std::string &str) {
    return parse(str.c_str());
}

void cache(int w, int h, int m) {
    CacheBinomials(w * h, m);
}

EMSCRIPTEN_BINDINGS(mws) {
    class_<Strategy>("Strategy")
        .property("initialPositionSpecified", &Strategy::InitialPositionSpecified)
        .property("index", &Strategy::Index)
        .property("logic", &Strategy::Logic)
        .property("heuristicEnabled", &Strategy::HeuristicEnabled)
        .property("decisionTree", &Strategy::DecisionTree)
        .property("exhaustEnabled", &Strategy::ExhaustEnabled)
        .property("pruningEnabled", &Strategy::PruningEnabled)
        .property("exhaustCriterion", &Strategy::ExhaustCriterion)
        .property("pruningCriterion", &Strategy::PruningCriterion)
        .property("pruningDecisionTree", &Strategy::PruningDecisionTree)
        ;
    class_<Configuration, base<Strategy>>("Configuration")
        .property("width", &Configuration::Width)
        .property("height", &Configuration::Height)
        .property("totalMines", &Configuration::TotalMines)
        .property("isSNR", &Configuration::IsSNR)
        ;
    function("parse", static_cast<Configuration (*)(const std::string &)>(&parse));
    function("cache", static_cast<void (*)(int, int, int)>(&cache));
    enum_<BlockStatus>("BlockStatus")
        .value("UNKNOWN", BlockStatus::Unknown)
        .value("MINE", BlockStatus::Mine)
        .value("BLANK", BlockStatus::Blank)
        ;
    enum_<SolvingState>("SolvingState")
        .value("STALE", SolvingState::Stale)
        .value("REDUCE", SolvingState::Reduce)
        .value("OVERLAP", SolvingState::Overlap)
        .value("PROBABILITY", SolvingState::Probability)
        .value("HEURISTIC", SolvingState::Heuristic)
        .value("DRAINED", SolvingState::Drained)
        .value("AUTOMATIC", SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability | SolvingState::Heuristic)
        ;
    class_<GameMgr>("GameMgr")
        .constructor<int, int, int, bool, Strategy, bool>()
        .function("openBlock", static_cast<void (GameMgr::*)(int, int)>(&GameMgr::OpenBlock))
        .function("solve", &GameMgr::Solve)
        .function("semiAutomaticStep", &GameMgr::SemiAutomaticStep)
        .function("semiAutomatic", &GameMgr::SemiAutomatic)
        .function("automaticStep", &GameMgr::AutomaticStep)
        .function("automatic", &GameMgr::Automatic)
        .function("enableDrainer", &GameMgr::EnableDrainer)
        .function("openOptimalBlocks", &GameMgr::OpenOptimalBlocks)
        ;
}

/*
struct GameStatus
{
    int TotalWidth, TotalHeight, TotalBlocks, TotalMines;
    bool Started, Succeed;
    double Bits, AllBits;
    int ToOpen, WrongGuesses;

    const BlockProperty *BlockProperties;
    const BlockStatus *InferredStatus;
    const double *Probabilities;
    const double *DrainProbabilities;

    int BestBlockCount;
    const Block *BestBlocks;
    int PreferredBlockCount;
    const Block *PreferredBlocks;
};

extern "C" DLL_API GameMgr *CreateGameMgrFromFile(const wchar_t *filename)
{
    std::ifstream sr(filename, std::ios::binary);
    auto mgr = new GameMgr(sr);
    sr.close();
    return mgr;
}

extern "C" DLL_API void SaveGameMgrToFile(GameMgr *mgr, const wchar_t *filename)
{
    std::ofstream sw(filename, std::ios::binary);
    mgr->Save(sw);
    sw.close();
}

extern "C" DLL_API GameStatus *GetGameStatus(GameMgr *mgr)
{
    auto st = new GameStatus;

#define ST(name) st->name = mgr->Get##name();
    ST(TotalWidth);
    ST(TotalHeight);
    ST(TotalMines);
    ST(Started);
    ST(Succeed);
    ST(Bits);
    ST(AllBits);
    ST(ToOpen);
    ST(WrongGuesses);
    ST(BlockProperties);
    ST(BestBlockCount);
    ST(BestBlocks);
    ST(PreferredBlockCount);
    ST(PreferredBlocks);

    st->TotalBlocks = st->TotalWidth * st->TotalHeight;
    st->InferredStatus = mgr->GetSolver().GetBlockStatuses();
    st->Probabilities = mgr->GetSolver().GetProbabilities();
    if (mgr->GetDrainer() != nullptr)
#include "stdafx.h"
#include "GameMgr.h"
#include "random.h"
#include "Drainer.h"
#include <fstream>
        st->DrainProbabilities = mgr->GetDrainer()->GetBestProbabilities();
    else
        st->DrainProbabilities = nullptr;

    return st;
}

extern "C" DLL_API void ReleaseGameStatus(GameStatus *status)
{
    if (status != nullptr)
        delete status;
}

*/
