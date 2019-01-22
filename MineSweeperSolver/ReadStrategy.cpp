#include "ReadStrategy.h"
#include <sstream>

bool ReadStrategy(const std::string &hsh, Strategy &st, size_t width, size_t height)
{
    int indexI, indexJ;

    std::stringstream ss(hsh);

#define CHK(s) if (ch != s) return false
#define NXT(s) do { if (!(ss >> ch)) return false; CHK(s); } while (false)
#define GET(ch) if (!(ss >> ch)) ch = '\0'

    char ch;
    GET(ch);
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

    GET(ch);
    if (ch == '@')
    {
        NXT('[');
        st.InitialPositionSpecified = true;
        if (!(ss >> indexI))
            return false;
        NXT(',');
        if (!(ss >> indexJ))
            return false;
        NXT(']');

        st.Index = (indexI - 1) + (indexJ - 1) * width;

        GET(ch);
    }
    CHK('-');

    st.HeuristicEnabled = true;
    GET(ch);
    if (ch == 'N')
        NXT('H');
    else if (ch == 'P')
    {
        GET(ch);
        if (ch == 'u')
        {
            NXT('r');
            NXT('e');
            st.HeuristicEnabled = false;
        }
        if (st.HeuristicEnabled)
            st.DecisionTree.push_back(HeuristicMethod::MinMineProb);
    }
    while (ch != '-' && ch != '\0')
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
        GET(ch);
    }

    if (ch == '\0')
    {
        st.ExhaustEnabled = false;
        st.ExhaustCriterion = 0;
        st.PruningEnabled = false;
        st.PruningCriterion = 0;
        return true;
    }

    GET(ch);
    if (ch != 'D')
        return false;
    st.ExhaustEnabled = true;
    if (!(ss >> st.ExhaustCriterion))
        return false;

    GET(ch);
    if (ch == '\0')
        return true;

    CHK('-');
    NXT('P');

    st.PruningEnabled = true;
    if (!(ss >> st.PruningCriterion))
        return false;

    return true;
}
