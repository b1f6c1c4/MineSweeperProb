#include "GameMgr.h"
#include "random.h"
#include <assert.h>
#include <iostream>

GameMgr::GameMgr(int width, int height, int totalMines) : m_TotalWidth(width), m_TotalHeight(height), m_TotalMines(totalMines), m_Settled(false), m_Started(true), m_Succeed(false), m_ToOpen(width * height - totalMines), m_Solver(width * height)
{
    m_Blocks.reserve(width * height);

    auto lst = BlockSet();
    lst.reserve(width * height);
    for (auto i = 0; i < width; ++i)
        for (auto j = 0; j < height;++j)
        {
            BlockProperty blk;
            blk.Index = GetIndex(i, j);
            blk.X = i;
            blk.Y = j;
            blk.IsOpen = false;
            blk.IsMine = false;
            blk.Self.push_back(blk.Index);
            blk.Surrounding.reserve(8);
            for (auto di = -1; di <= 1; di++)
                if (i + di >= 0 && i + di < width)
                    for (auto dj = -1; dj <= 1; dj++)
                        if (j + dj >= 0 && j + dj < height)
                            if (di != 0 || dj != 0)
                                blk.Surrounding.push_back(GetIndex(i + di, j + dj));
            lst.push_back(blk.Index);
            m_Blocks.push_back(std::move(blk));
        }
    m_Solver.AddRestrain(lst, totalMines);
}

Solver& GameMgr::GetSolver()
{
    return m_Solver;
}

const Solver& GameMgr::GetSolver() const
{
    return m_Solver;
}

bool GameMgr::IsStarted() const
{
    return m_Started;
}

bool GameMgr::IsSucceed() const
{
    return m_Succeed;
}

double GameMgr::GetBits() const
{
    return m_Solver.GetTotalStates().Log2();
}

const BlockProperty& GameMgr::GetBlockProperty(int x, int y) const
{
    return m_Blocks[GetIndex(x, y)];
}

double GameMgr::GetBlockProbability(int x, int y) const
{
    return m_Solver.GetProbability(GetIndex(x, y));
}

BlockStatus GameMgr::GetBlockStatus(int x, int y) const
{
    return m_Solver.GetBlockStatus(GetIndex(x, y));
}

void GameMgr::OpenBlock(int x, int y)
{
    OpenBlock(GetIndex(x, y));
}

bool GameMgr::SemiAutomaticStep(bool withProb)
{
    if (!m_Started)
        return false;
    m_Solver.Solve(withProb);
    auto flag = false;
    for (auto i = 0; i < m_Blocks.size();++i)
    {
        if (m_Blocks[i].IsOpen || m_Solver.GetBlockStatus(i) != Blank)
            continue;
        OpenBlock(i);
        flag = true;
        if (m_Started)
            continue;
        break;
    }
    return flag && m_Started;
}

bool GameMgr::SemiAutomatic()
{
    if (!m_Started)
        return false;
    while (true)
    {
        while (SemiAutomaticStep(false)) {}
        if (!SemiAutomaticStep(true))
            break;
    }
    return m_Started;
}

void GameMgr::AutomaticStep()
{
    if (!m_Started)
        return;

    m_Solver.Solve(true);
    auto ary = std::vector<int>();
    {
        auto vals = std::vector<double>();
        double best = 1;
        for (auto i = 0; i < m_Blocks.size(); ++i)
            if (!m_Blocks[i].IsOpen && m_Solver.GetBlockStatus(i) == Unknown)
            {
                ary.push_back(i);
                auto p = m_Solver.GetProbability(i);
                vals.push_back(p);
                if (p < best)
                    best = p;
            }

        auto tmp = std::vector<int>();
        for (auto i = 0; i < ary.size(); ++i)
            if (vals[i] <= best)
                tmp.push_back(ary[i]);
        tmp.swap(ary);
    }

    {
        auto vals = std::vector<BigInteger>();
        auto vals2 = std::vector<double>();
        BigInteger best = 1;
        for (auto i = 0; i < ary.size(); ++i)
        {
            int m;
            auto di = m_Solver.DistributionCond(m_Blocks[ary[i]].Surrounding, m_Blocks[ary[i]].Self, 0, m);
            if (di.size() == 0)
                std::cout << "FFFFFFFFFFFFF";
            vals.push_back(di[0]);
            if (di[0] > best)
                best = di[0];

            BigInteger t(0);
            for (auto j = 0; j < di.size(); ++j)
                t += di[j];

            double q = 0;
            for (auto j = 0; j < di.size();++j)
                if (di[j] != 0)
                {
                    auto p = di[j] / t;
                    q += p*log2(p);
                }

            vals2.push_back(q);
        }

        auto tmp = std::vector<int>();
        auto tmpv = std::vector<double>();
        for (auto i = 0; i < ary.size(); ++i)
            if (vals[i] <= best)
            {
                tmp.push_back(ary[i]);
                tmpv.push_back(vals2[i]);
            }
        tmp.swap(ary);
        tmpv.swap(vals2);


        double best2 = 0;
        for (auto i = 0; i < ary.size(); ++i)
            if (vals2[i] > best2)
                best2 = vals2[i];
        for (auto i = 0; i < ary.size(); ++i)
            if (vals2[i] >= best2)
                tmp.push_back(ary[i]);
        tmp.swap(ary);
    }

    auto blk = ary.size() == 1 ? ary[0] : ary[RandomInteger(ary.size())];
    OpenBlock(blk);
}

void GameMgr::Automatic()
{
    while (m_Started)
        if (SemiAutomatic())
            AutomaticStep();
}

int GameMgr::GetIndex(int x, int y) const
{
    return x * m_TotalHeight + y;
}

void GameMgr::SettleMines(int initID)
{
    auto totalMines = m_TotalMines;
    while (totalMines > 0)
    {
        auto id = RandomInteger(m_TotalWidth * m_TotalHeight);
        if (id == initID)
            continue;
        if (m_Blocks[id].IsMine)
            continue;
        m_Blocks[id].IsMine = true;
        totalMines--;
    }
    m_Settled = true;

    std::for_each(m_Blocks.begin(), m_Blocks.end(), [this](BlockProperty &blk)
    {
        if (blk.IsMine)
            return;
        blk.Degree = 0;
        std::for_each(blk.Surrounding.begin(), blk.Surrounding.end(), [&blk,this](int &id)
        {
            if (m_Blocks[id].IsMine)
                ++blk.Degree;
        });
    });
}

void GameMgr::OpenBlock(int id)
{
    if (!m_Started)
        return;
    if (!m_Settled)
        SettleMines(id);

    if (m_Blocks[id].IsOpen)
        return;
    m_Blocks[id].IsOpen = true;

    if (m_Blocks[id].IsMine)
    {
        m_Started = false;
        return;
    }

    m_Solver.AddRestrain(id, false);
    if (m_Blocks[id].Degree == 0)
    {
        std::for_each(m_Blocks[id].Surrounding.begin(), m_Blocks[id].Surrounding.end(), [this](int &blk)
        {
            OpenBlock(blk);
        });
    }
    m_Solver.AddRestrain(m_Blocks[id].Surrounding, m_Blocks[id].Degree);

    if (--m_ToOpen == 0)
    {
        m_Started = false;
        m_Succeed = true;
    }
}
