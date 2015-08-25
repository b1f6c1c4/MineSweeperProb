#include "GameMgr.h"
#include "random.h"

GameMgr::GameMgr(int width, int height, int totalMines) : m_TotalWidth(width), m_TotalHeight(height), m_TotalMines(totalMines), m_Settled(false), m_Started(true), m_Succeed(false), m_ToOpen(width * height - totalMines), m_Solver(width * height)
{
    m_Blocks.reserve(width*height);

    for (auto i = 0; i < width; ++i)
        for (auto j = 0; j < height;++j)
        {
            BlockProperty blk;
            blk.Index = GetIndex(i, j);
            blk.X = i;
            blk.Y = j;
            blk.IsOpen = false;
            blk.IsMine = false;
            blk.Surrounding.reserve(8);
            for (auto di = -1; di <= 1; di++)
                if (i + di >= 0 && i + di < width)
                    for (auto dj = -1; dj <= 1; dj++)
                        if (j + dj >= 0 && j + dj < height)
                            if (di != 0 || dj != 0)
                                blk.Surrounding.push_back(GetIndex(i + di, j + dj));
        }
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

int GameMgr::GetIndex(int x, int y) const
{
    return x*m_TotalHeight + y;
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
