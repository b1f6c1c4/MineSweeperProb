#include "GameMgr.h"
#include "random.h"
#include "BinomialHelper.h"
#include <functional>

template <class T>
static void Largest(std::vector<Block> &bests, std::function<T(Block)> fun);
template <class T>
static void Largest(std::vector<Block> &bests, std::function<const T &(Block)> fun);

GameMgr::GameMgr(int width, int height, int totalMines) : m_TotalWidth(width), m_TotalHeight(height), m_TotalMines(totalMines), m_Settled(false), m_Started(true), m_Succeed(false), m_ToOpen(width * height - totalMines), m_Solver(width * height)
{
    m_Blocks.reserve(width * height);

    auto lst = BlockSet();
    lst.reserve(width * height);
    for (auto i = 0; i < width; ++i)
        for (auto j = 0; j < height; ++j)
        {
            m_Blocks.emplace_back();
            m_BlocksR.emplace_back();
            auto &blk = m_Blocks.back();
            auto &blkR = m_BlocksR.back();
            blk.Index = GetIndex(i, j);
            blk.X = i;
            blk.Y = j;
            blk.IsOpen = false;
            blk.IsMine = false;
            blkR.Self.push_back(blk.Index);
            blkR.Surrounding.reserve(8);
            for (auto di = -1; di <= 1; di++)
                if (i + di >= 0 && i + di < width)
                    for (auto dj = -1; dj <= 1; dj++)
                        if (j + dj >= 0 && j + dj < height)
                            if (di != 0 || dj != 0)
                                blkR.Surrounding.push_back(GetIndex(i + di, j + dj));
            lst.push_back(blk.Index);
        }
    m_Solver.AddRestrain(lst, totalMines);

    m_AllBits = Binomial(width * height, totalMines).Log2();
}

Solver &GameMgr::GetSolver()
{
    return m_Solver;
}

const Solver &GameMgr::GetSolver() const
{
    return m_Solver;
}

int GameMgr::GetTotalWidth() const
{
    return m_TotalWidth;
}

int GameMgr::GetTotalHeight() const
{
    return m_TotalHeight;
}

int GameMgr::GetTotalMines() const
{
    return m_TotalMines;
}

int GameMgr::GetToOpen() const
{
    return m_ToOpen;
}

bool GameMgr::GetStarted() const
{
    return m_Started;
}

bool GameMgr::GetSucceed() const
{
    return m_Succeed;
}

double GameMgr::GetBits() const
{
    return m_Solver.GetTotalStates().Log2();
}

double GameMgr::GetAllBits() const
{
    return m_AllBits;
}

const BlockProperty &GameMgr::GetBlockProperty(int x, int y) const
{
    return m_Blocks[GetIndex(x, y)];
}

const BlockProperty *GameMgr::GetBlockProperties() const
{
    return &*m_Blocks.begin();
}

double GameMgr::GetBlockProbability(int x, int y) const
{
    return m_Solver.GetProbability(GetIndex(x, y));
}

BlockStatus GameMgr::GetInferredStatus(int x, int y) const
{
    return m_Solver.GetBlockStatus(GetIndex(x, y));
}

const Block *GameMgr::GetBestBlocks() const
{
    if (m_Best.empty())
        return nullptr;
    return &*m_Best.begin();
}

int GameMgr::GetBestBlockCount() const
{
    return m_Best.size();
}

const Block *GameMgr::GetPreferredBlocks() const
{
    if (m_Preferred.empty())
        return nullptr;
    return &*m_Preferred.begin();
}

int GameMgr::GetPreferredBlockCount() const
{
    return m_Preferred.size();
}

void GameMgr::OpenBlock(int x, int y)
{
    OpenBlock(GetIndex(x, y));
}

template <class T>
void Largest(std::vector<Block> &bests, std::function<T(int)> fun)
{
    if (bests.size() <= 1)
        return;
    auto newBests = std::vector<Block>();
    newBests.push_back(bests.front());
    auto bestVal = fun(bests.front());
    for (auto i = 1; i < bests.size(); ++i)
    {
        auto p = fun(bests[i]);
        if (bestVal < p)
        {
            bestVal = p;
            newBests.clear();
        }
        if (!(p < bestVal))
            newBests.push_back(bests[i]);
    }
    newBests.swap(bests);
}

template <class T>
void Largest(std::vector<Block> &bests, std::function<const T &(int)> fun)
{
    if (bests.size() <= 1)
        return;
    auto newBests = std::vector<Block>();
    newBests.push_back(bests.front());
    const T *bestVal = &fun(bests.front());
    for (auto i = 1; i < bests.size(); ++i)
    {
        const T *p = &fun(bests[i]);
        if (*bestVal < *p)
        {
            bestVal = p;
            newBests.clear();
        }
        if (!(*p < *bestVal))
            newBests.push_back(bests[i]);
    }
    newBests.swap(bests);
}

void GameMgr::Solve(bool withProb, bool withPref)
{
    if (!m_Started)
        return;

    m_Best.clear();
    m_Preferred.clear();

    m_Solver.Solve(true, withProb);

#define DEBUG
#ifdef DEBUG
    for (auto i = 0; i < m_Blocks.size(); ++i)
        switch (m_Solver.GetBlockStatus(i))
        {
        case BlockStatus::Mine: 
            if (!m_Blocks[i].IsMine)
                throw;
            break;
        case BlockStatus::Blank:
            if (m_Blocks[i].IsMine)
                throw;
            break;
        case BlockStatus::Unknown:
        default: 
            break;
        }
#endif

    for (auto i = 0; i < m_Blocks.size(); ++i)
        if (!m_Blocks[i].IsOpen && m_Solver.GetBlockStatus(i) == BlockStatus::Blank)
            m_Best.push_back(i);

    if (!m_Best.empty())
        return;

    if (!withProb)
        return;
    
    for (auto i = 0; i < m_Blocks.size(); ++i)
    {
        if (m_Blocks[i].IsOpen || m_Solver.GetBlockStatus(i) != BlockStatus::Unknown)
            continue;
        m_Preferred.push_back(i);
    }

    Largest(m_Preferred, std::function<double(Block)>([this](Block blk) { return -m_Solver.GetProbability(blk); }));

    if (!withPref)
        return;

    Largest(m_Preferred, std::function<const BigInteger &(Block)>([this](Block blk)->const BigInteger & { return m_Solver.ZeroCondQ(m_BlocksR[blk].Surrounding, blk); }));

    Largest(m_Preferred, std::function<double(Block)>([this](Block blk)
    {
        int m;
        auto di = m_Solver.DistributionCondQ(m_BlocksR[blk].Surrounding, blk, m);

        BigInteger t(0);
        for (auto j = 0; j < di.size(); ++j)
            t += di[j];

        double q = 0;
        for (auto j = 0; j < di.size(); ++j)
            if (di[j] != 0)
            {
                auto p = di[j] / t;
                q += -p * log2(p);
            }

        return q;
    }));
}

void GameMgr::OpenOptimalBlocks()
{
    if (!m_Best.empty())
    {
        for (auto i = 0; i < m_Best.size(); ++i)
        {
            OpenBlock(m_Best[i]);
            if (!m_Started)
                break;
        }
        m_Best.clear();
        return;
    }

    if (m_Preferred.empty())
        return;

    auto blk = m_Preferred.size() == 1 ? m_Preferred[0] : m_Preferred[RandomInteger(m_Preferred.size())];
    m_Preferred.clear();
    OpenBlock(blk);
}

bool GameMgr::SemiAutomaticStep(bool withOverlap, bool withProb)
{
    if (!m_Started)
        return false;
    m_Solver.Solve(withOverlap, withProb);
    auto flag = false;
    for (auto i = 0; i < m_Blocks.size(); ++i)
    {
        if (m_Blocks[i].IsOpen || m_Solver.GetBlockStatus(i) != BlockStatus::Blank)
            continue;
        OpenBlock(i);
        flag = true;
        if (m_Started)
            continue;
        break;
    }
    return flag && m_Started;
}

bool GameMgr::SemiAutomatic(bool withProb)
{
    if (!m_Started)
        return false;
    while (true)
    {
        while (SemiAutomaticStep(false, false)) {}
        if (SemiAutomaticStep(true, false))
            continue;
        if (!withProb || !SemiAutomaticStep(false, true))
            break;
    }
    return m_Started;
}

void GameMgr::AutomaticStep()
{
    if (!m_Started)
        return;

    Solve(true, true);
    OpenOptimalBlocks();
}

void GameMgr::Automatic()
{
    while (m_Started)
        if (SemiAutomatic(true))
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

    for (auto i = 0; i < m_Blocks.size(); ++i)
    {
        if (m_Blocks[i].IsMine)
            continue;
        m_Blocks[i].Degree = 0;
        for (auto &id : m_BlocksR[i].Surrounding)
            if (m_Blocks[id].IsMine)
                ++m_Blocks[i].Degree;
    }
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
        for (auto &blk : m_BlocksR[id].Surrounding)
            OpenBlock(blk);
    }
    m_Solver.AddRestrain(m_BlocksR[id].Surrounding, m_Blocks[id].Degree);

    if (--m_ToOpen == 0)
    {
        m_Started = false;
        m_Succeed = true;
    }
}
