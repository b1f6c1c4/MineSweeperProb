#include "ReadStrategy.h"
#include <sstream>

bool ReadStrategy(const std::string &hsh, Strategy &st, size_t width, size_t height)
{
    int indexI, indexJ;

    std::stringstream ss(hsh);

#define CHK(s) if (ch != s) return false
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
        return false;
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

        st.Index = (indexI - 1) + (indexJ - 1) * width;

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

        ss >> ch;
    }

    return true;
}
