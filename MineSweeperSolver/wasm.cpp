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

void seed() {
    SeedEngine();
}

auto parse(const std::string &str) {
    return parse(str.c_str());
}

void cache(int w, int h, int m) {
    CacheBinomials(w * h, m);
}

EMSCRIPTEN_BINDINGS(mws) {
    function("seed", &seed);
    function("parse", static_cast<Configuration (*)(const std::string &)>(&parse));
    function("cache", static_cast<void (*)(int, int, int)>(&cache));
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
        .value("SEMI", SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability)
        .value("HEUR", SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability | SolvingState::Heuristic)
        .value("AUTO", SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability | SolvingState::Heuristic | SolvingState::Drained)
        ;
    class_<BlockProperty>("BlockProperty")
        .property("index", &BlockProperty::Index)
        .property("x", &BlockProperty::X)
        .property("y", &BlockProperty::Y)
        .property("degree", &BlockProperty::Degree)
        .property("isOpen", &BlockProperty::IsOpen)
        .property("hasMine", &BlockProperty::IsMine)
        ;
    register_vector<Block>("BlockSet");
    register_vector<double>("VectorDouble");
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
        .property("totalWidth", &GameMgr::GetTotalWidth)
        .property("totalHeight", &GameMgr::GetTotalHeight)
        .property("totalMines", &GameMgr::GetTotalMines)
        .property("toOpen", &GameMgr::GetToOpen)
        .property("wrongGuesses", &GameMgr::GetWrongGuesses)
        .property("settled", &GameMgr::GetSettled)
        .property("started", &GameMgr::GetStarted)
        .property("succeed", &GameMgr::GetSucceed)
        .property("bits", &GameMgr::GetBits)
        .property("allBits", &GameMgr::GetAllBits)
        .function("blockPropertyOf", &GameMgr::GetBlockProperty)
        .function("blockProbabilityOf", &GameMgr::GetBlockProbability)
        .function("inferredStatusOf", &GameMgr::GetInferredStatus)
        .property("bestBlocks", &GameMgr::GetBestBlockList)
        .property("preferredBlocks", &GameMgr::GetPreferredBlockList)
        .property("bestProbabilityList", &GameMgr::GetBestProbabilityList)
        ;
}
