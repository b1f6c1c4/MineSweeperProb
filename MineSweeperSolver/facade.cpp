#include "facade.hpp"

#include <sstream>

#include "GameMgr.h"
#include "BinomialHelper.h"

Configuration parse(const char *hsh)
{
    Configuration cfg;
    Strategy &st = cfg;

    int indexI, indexJ;

    std::stringstream ss(hsh);

#define FAIL throw std::runtime_error("format error")
#define CHK(s) if (ch != (s)) FAIL
#define NXT(s) do { ss >> ch; CHK(s); } while (false)

    char ch;
    ss >> ch;
    switch (ch)
    {
        case 'P':
            st.Logic = LogicMethod::Passive;
            break;
        case 'S':
            st.Logic = LogicMethod::Single;
            break;
        case 'D':
            st.Logic = LogicMethod::Double;
            break;
        case 'F':
            st.Logic = LogicMethod::Full;
            break;
        default:
            FAIL;
    }
    NXT('L');

    st.InitialPositionSpecified = false;
    st.Index = -1;

    ss >> ch;
    if (ch == '@')
    {
        NXT('[');
        st.InitialPositionSpecified = true;
        ss >> indexI;
        NXT(',');
        ss >> indexJ;
        NXT(']');

        ss >> ch;
    }
    CHK('-');

    st.HeuristicEnabled = true;
    ss >> ch;
    if (ch == 'N')
        NXT('H');
    else if (ch == 'P')
    {
        ss >> ch;
        if (ch == 'u')
        {
            NXT('r');
            NXT('e');
            st.HeuristicEnabled = false;
        }
        if (st.HeuristicEnabled)
            st.DecisionTree.push_back(HeuristicMethod::MinMineProb);
    }
    while (ch != '-')
    {
        switch (ch)
        {
            case 'P':
                st.DecisionTree.push_back(HeuristicMethod::MinMineProb);
                break;
            case 'Z':
                st.DecisionTree.push_back(HeuristicMethod::MaxZeroProb);
                break;
            case 'S':
                st.DecisionTree.push_back(HeuristicMethod::MaxZerosProb);
                break;
            case 'E':
                st.DecisionTree.push_back(HeuristicMethod::MaxZerosExp);
                break;
            case 'Q':
                st.DecisionTree.push_back(HeuristicMethod::MaxQuantityExp);
                break;
            case 'F':
                st.DecisionTree.push_back(HeuristicMethod::MinFrontierDist);
                break;
            case 'U':
                st.DecisionTree.push_back(HeuristicMethod::MaxUpperBound);
                break;
            default:
                break;
        }
        ss >> ch;
    }

    st.ExhaustEnabled = false;
    st.ExhaustCriterion = 0;
    st.PruningEnabled = false;
    st.PruningCriterion = 0;
    ss >> ch;
    if (ch == 'D')
    {
        st.ExhaustEnabled = true;
        ss >> st.ExhaustCriterion;

        NXT('-');

        ss >> ch;
    }
    ss.putback(ch);

    ss >> cfg.Width;
    NXT('-');
    ss >> cfg.Height;
    NXT('-');
    NXT('T');
    ss >> cfg.TotalMines;
    NXT('-');
    NXT('S');
    ss >> ch;
    if (ch == 'F') {
        NXT('A');
        NXT('R');
        cfg.IsSNR = false;
    } else if (ch == 'N') {
        NXT('R');
        cfg.IsSNR = true;
    } else {
        FAIL;
    }

    if (st.InitialPositionSpecified)
        st.Index = (indexI - 1) + (indexJ - 1) * cfg.Height;

    return cfg;
}

bool run(const Configuration &Config)
{
    GameMgr mgr(Config.Width, Config.Height, Config.TotalMines, Config.IsSNR, Config, false);
    mgr.Automatic();
    return mgr.GetSucceed();
}

void cache(const Configuration &Config)
{
    CacheBinomials(Config.Width * Config.Height, Config.TotalMines);
}
