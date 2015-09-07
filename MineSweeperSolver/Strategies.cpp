#include "Strategies.h"
#include "base64.h"

#define MASK(shift) (1 << (shift))
#define MASKL(lng) (MASK((lng)) - 1)
#define MASKH(lng) (~(MASK(8 - (lng)) - 1))

template <class T>
static T ReadBits(std::basic_string<unsigned char> buf, int &cur, int length)
{
    auto buff = new unsigned char[max((length + 7) / 8, sizeof(T))];
    ZeroMemory(buff, max((length + 7) / 8, sizeof(T)));
    auto curb = 0;

    while (curb < length)
    {
        auto to = min(min(length - curb, 8 - curb % 8), 8 - cur % 8);
        buff[curb / 8] |= ((buf[cur / 8] & MASKH(8 - cur % 8)) >> cur % 8 & MASKL(to)) << curb % 8;
        curb += to, cur += to;
    }

    auto res = *reinterpret_cast<T*>(buff);
    delete[] buff;
    return res;
}

template <class T>
static void WriteBits(std::basic_string<unsigned char> &buf, const T &val, int &cur, int length)
{
    auto buff = reinterpret_cast<const unsigned char *>(&val);
    auto curb = 0;

    while (curb < length)
    {
        if (cur / 8 == buf.size())
            buf.push_back(0);
        auto to = min(min(length - curb, 8 - cur % 8), 8 - curb % 8);
        buf[cur / 8] |= ((buff[curb / 8] & MASKH(8 - curb % 8)) >> curb % 8 & MASKL(to)) << cur % 8;
        curb += to, cur += to;
    }
}

Strategy DLL_API ReadStrategy(std::string str)
{
    auto buf = base64_decode(str);
    Strategy st;
    auto cur = 0;

#define R(field, type, length) field = ReadBits<type>(buf, cur, length)

    R(st.Index, int, 9);
    st.InitialPositionSpecified = st.Index == 480;

    R(st.Logic, LogicMethod, 2);
    R(st.HeuristicEnabled, bool, 1);

    size_t sz;
    R(sz, size_t, 3);
    st.DecisionTree.reserve(sz);
    for (auto i = 0; i < sz; i++)
        st.DecisionTree.push_back(ReadBits<HeuristicMethod>(buf, cur, 3));

    R(st.ExhaustCriterion, int, 9);
    R(st.PruningCriterion, int, 12);
    if (st.ExhaustCriterion != 0)
    {
        st.ExhaustEnabled = true;
        st.PruningEnabled = st.PruningCriterion != st.ExhaustCriterion;
    }
    else
    {
        st.ExhaustEnabled = false;
        st.PruningEnabled = false;
    }

    R(sz, size_t, 3);
    st.PruningDecisionTree.reserve(sz);
    for (auto i = 0; i < sz; i++)
        st.PruningDecisionTree.push_back(ReadBits<HeuristicMethod>(buf, cur, 3));

    return st;
}

std::string DLL_API WriteStrategy(const Strategy &st)
{
    std::basic_string<unsigned char> buf;
    auto cur = 0;

#define W(field, type, length) WriteBits<type>(buf, field, cur, length)
    
    W(st.InitialPositionSpecified ? st.Index : 480, int, 9);

    W(st.Logic, LogicMethod, 2);
    W(st.HeuristicEnabled, bool, 1);

    W(st.DecisionTree.size(), size_t, 3);
    for (auto m : st.DecisionTree)
        WriteBits<HeuristicMethod>(buf, m, cur, 3);

    W(st.ExhaustEnabled ? st.ExhaustCriterion : 0, int, 9);
    W(st.PruningEnabled ? st.PruningCriterion : (st.ExhaustEnabled ? st.ExhaustCriterion : 0), int, 12);

    W(st.PruningDecisionTree.size(), size_t, 3);
    for (auto m : st.PruningDecisionTree)
        WriteBits<HeuristicMethod>(buf, m, cur, 3);

    return base64_encode(buf);
}
