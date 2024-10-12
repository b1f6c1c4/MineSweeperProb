#include "Util.h"

nlohmann::json to_json(const Configuration &cfg)
{
    nlohmann::json j;
    j["game"]["width"] = cfg.Width;
    j["game"]["height"] = cfg.Height;
    j["game"]["mines"] = cfg.TotalMines;
    j["game"]["snr"] = cfg.IsSNR;
    j["strategy"]["logic"] = cfg.Logic;
    if (!cfg.InitialPositionSpecified)
        j["strategy"]["initial"] = nullptr;
    else
        j["strategy"]["initial"] = { { "x", cfg.Index % cfg.Width + 1 },
                                     { "y", cfg.Index / cfg.Width + 1 } };
    if (!cfg.HeuristicEnabled)
        j["strategy"]["heuristic"] = "Pure";
    else {
        j["strategy"]["heuristic"] = to_string(cfg.DecisionTree);
    }
    if (!cfg.ExhaustEnabled)
        j["strategy"]["exhaust"] = 0;
    else
        j["strategy"]["exhaust"] = cfg.ExhaustCriterion;
    if (!cfg.PruningEnabled)
        j["strategy"]["pruning"] = 0;
    else
        j["strategy"]["pruning"] = cfg.PruningCriterion;
    return j;
}

std::string to_string(const std::vector<HeuristicMethod> &dt) {
    if (dt.empty())
        return "NH";
    std::string str;
    for (auto m: dt)
        switch (m) {
            case HeuristicMethod::None:
                str.push_back(' ');
                break;
            case HeuristicMethod::MinMineProb:
                str.push_back('P');
                break;
            case HeuristicMethod::MaxZeroProb:
                str.push_back('Z');
                break;
            case HeuristicMethod::MaxZerosProb:
                str.push_back('S');
                break;
            case HeuristicMethod::MaxZerosExp:
                str.push_back('E');
                break;
            case HeuristicMethod::MaxQuantityExp:
                str.push_back('Q');
                break;
            case HeuristicMethod::MinFrontierDist:
                str.push_back('F');
                break;
            case HeuristicMethod::MaxUpperBound:
                str.push_back('U');
                break;
            case HeuristicMethod::Relevant2:
                str.push_back('2');
                break;
        }
    return str;
}

NanoTimer::NanoTimer()
{
    clock_gettime(CLOCK_MONOTONIC, &begin);
}

void NanoTimer::stop()
{
    clock_gettime(CLOCK_MONOTONIC, &end);
}

double NanoTimer::seconds() const
{
    return static_cast<double>(end.tv_sec - begin.tv_sec)
        + static_cast<double>(end.tv_nsec - begin.tv_nsec) * 1e-9;
}
