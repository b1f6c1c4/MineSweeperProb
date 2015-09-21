#include "Strategies.h"
#include <sstream>

DLL_API Strategy ReadStrategy(std::string str)
{
    std::stringstream ss(str);

    Strategy st;
    char ch;
    ss >> ch;
    switch (ch)
    {
    case 'N':
        st.Logic = LogicMethod::None;
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
        throw;
    }
    ss >> ch;
    if (ch != 'L')
        throw;
    ss >> ch;
    if (ch == '@')
    {
        ss >> ch;
        if (ch != '[')
            throw;
        ss >> st.Index;
        ss >> ch;
        if (ch != ']')
            throw;
        ss >> ch;
    }
    if (ch != '-')
        throw;

    st.HeuristicEnabled = true;
    ss >> ch;
    if (ch == 'N')
    {
        ss >> ch;
        if (ch != 'H')
            throw;
    }
    else if (ch == 'P')
    {
        if (!ss.eof())
        {
            ss >> ch;
            if (ch == 'u')
            {
                ss >> ch;
                if (ch != 'r')
                    throw;
                ss >> ch;
                if (ch != 'e')
                    throw;
                st.HeuristicEnabled = false;
            }
        }
        else
            ch = '-';
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
        if (!ss.eof())
            ss >> ch;
        else
            ch = '-';
    }

    st.ExhaustEnabled = false;
    if (ss.eof())
        return st;

    ss >> ch;
    if (ch != 'D')
        throw;

    st.ExhaustEnabled = true;
    ss >> st.ExhaustCriterion;

    st.PruningEnabled = false;
    if (ss.eof())
        return st;

    ss >> ch;
    if (ch != '-')
        throw;
    ss >> ch;
    if (ch != 'P')
        throw;

    st.PruningEnabled = true;
    ss >> st.PruningCriterion;

    ss >> ch;
    if (ch != '-')
        throw;

    while (!ss.eof())
    {
        ss >> ch;
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
    }

    return st;
}

DLL_API std::string WriteStrategy(const Strategy &st)
{
    std::stringstream ss;
    
    switch (st.Logic)
    {
    case LogicMethod::None:
        ss << "NL";
        break;
    case LogicMethod::Single:
        ss << "SL";
        break;
    case LogicMethod::Double:
        ss << "DL";
        break;
    case LogicMethod::Full:
        ss << "FL";
        break;
    default:
        ss << "XL";
        break;
    }

    if (st.InitialPositionSpecified)
        ss << "@[" << st.Index << "]";

    ss << "-";

    if (st.HeuristicEnabled)
        if (st.DecisionTree.empty())
            ss << "NH";
        else
            for (auto m : st.DecisionTree)
                switch (m)
                {
                case HeuristicMethod::None: break;
                case HeuristicMethod::MinMineProb: 
                    ss << "P";
                    break;
                case HeuristicMethod::MaxZeroProb:
                    ss << "Z";
                    break;
                case HeuristicMethod::MaxZerosProb:
                    ss << "S";
                    break;
                case HeuristicMethod::MaxZerosExp:
                    ss << "E";
                    break;
                case HeuristicMethod::MaxQuantityExp:
                    ss << "Q";
                    break;
                case HeuristicMethod::MinFrontierDist:
                    ss << "F";
                    break;
                case HeuristicMethod::MaxUpperBound:
                    ss << "U";
                    break;
                default:
                    ss << "X";
                    break;
                }

    if (st.ExhaustEnabled)
    {
        ss << "-D" << st.ExhaustCriterion;
        if (st.PruningEnabled)
        {
            ss << "-P" << st.PruningCriterion << "-";
            for (auto m : st.PruningDecisionTree)
                switch (m)
                {
                case HeuristicMethod::None: break;
                case HeuristicMethod::MinMineProb:
                    ss << "P";
                    break;
                case HeuristicMethod::MaxZeroProb:
                    ss << "Z";
                    break;
                case HeuristicMethod::MaxZerosProb:
                    ss << "S";
                    break;
                case HeuristicMethod::MaxZerosExp:
                    ss << "E";
                    break;
                case HeuristicMethod::MaxQuantityExp:
                    ss << "Q";
                    break;
                case HeuristicMethod::MinFrontierDist:
                    ss << "F";
                    break;
                case HeuristicMethod::MaxUpperBound:
                    ss << "U";
                    break;
                default:
                    ss << "X";
                    break;
                }
        }
    }

    return ss.str();
}
