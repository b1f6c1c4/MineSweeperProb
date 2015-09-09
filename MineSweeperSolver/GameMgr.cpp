#include "GameMgr.h"
#include "random.h"
#include "BinomialHelper.h"
#include "Drainer.h"
#include <iostream>

GameMgr::GameMgr(int width, int height, int totalMines) : m_TotalWidth(width), m_TotalHeight(height), m_TotalMines(totalMines), m_Settled(false), m_Started(true), m_Succeed(false), m_ToOpen(width * height - totalMines), m_Solver(nullptr), m_Drainer(nullptr)
{
    m_Solver = new Solver(width * height);

    m_Blocks.reserve(width * height);
    m_BlocksR.reserve(width * height);

    BlockSet lst;
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
            blkR.reserve(8);
            for (auto di = -1; di <= 1; ++di)
                if (i + di >= 0 && i + di < width)
                    for (auto dj = -1; dj <= 1; ++dj)
                        if (j + dj >= 0 && j + dj < height)
                            if (di != 0 || dj != 0)
                                blkR.push_back(GetIndex(i + di, j + dj));
            lst.push_back(blk.Index);
        }
    m_Solver->AddRestrain(lst, totalMines);

    m_AllBits = log2(Binomial(width * height, totalMines));
}

GameMgr::GameMgr(std::istream &sr) : m_TotalWidth(0), m_TotalHeight(0), m_TotalMines(0), m_Settled(false), m_Started(true), m_Succeed(false), m_ToOpen(0), m_Solver(nullptr), m_Drainer(nullptr)
{
#define READ(val) sr.read(reinterpret_cast<char *>(&(val)), sizeof(val));
    READ(m_TotalWidth);
    READ(m_TotalHeight);
    READ(m_TotalMines);
    READ(m_Settled);
    READ(m_Started);
    READ(m_ToOpen);

    m_Solver = new Solver(m_TotalWidth * m_TotalHeight);

    m_Blocks.reserve(m_TotalWidth * m_TotalHeight);
    m_BlocksR.reserve(m_TotalWidth * m_TotalHeight);

    auto lst = BlockSet();
    lst.reserve(m_TotalWidth * m_TotalHeight);
    for (auto i = 0; i < m_TotalWidth; ++i)
        for (auto j = 0; j < m_TotalHeight; ++j)
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
            blkR.reserve(8);
            for (auto di = -1; di <= 1; ++di)
                if (i + di >= 0 && i + di < m_TotalWidth)
                    for (auto dj = -1; dj <= 1; ++dj)
                        if (j + dj >= 0 && j + dj < m_TotalHeight)
                            if (di != 0 || dj != 0)
                                blkR.push_back(GetIndex(i + di, j + dj));
            lst.push_back(blk.Index);
        }
    m_Solver->AddRestrain(lst, m_TotalMines);

    if (m_Settled)
    {
        for (auto &blk : m_Blocks)
        {
            READ(blk.Degree);
            READ(blk.IsOpen);
            READ(blk.IsMine);
            if (blk.IsOpen && !blk.IsMine)
                m_Solver->AddRestrain(blk.Index, false);
        }

        for (auto &blk : m_Blocks)
            if (blk.IsOpen && !blk.IsMine && blk.Degree != 0)
                m_Solver->AddRestrain(m_BlocksR[blk.Index], blk.Degree);
    }

    CacheBinomials(m_TotalWidth * m_TotalHeight, m_TotalMines);
    m_AllBits = log2(Binomial(m_TotalWidth * m_TotalHeight, m_TotalMines));
}

GameMgr::~GameMgr()
{
    if (m_Solver != nullptr)
    {
        delete m_Solver;
        m_Solver = nullptr;
    }
    if (m_Drainer != nullptr)
    {
        delete m_Drainer;
        m_Drainer = nullptr;
    }
}

Solver &GameMgr::GetSolver()
{
    return *m_Solver;
}

const Solver &GameMgr::GetSolver() const
{
    return *m_Solver;
}

const Drainer *GameMgr::GetDrainer() const
{
    return m_Drainer;
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
    return log2(m_Solver->GetTotalStates());
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
    return m_Solver->GetProbability(GetIndex(x, y));
}

BlockStatus GameMgr::GetInferredStatus(int x, int y) const
{
    return m_Solver->GetBlockStatus(GetIndex(x, y));
}

const Block *GameMgr::GetBestBlocks() const
{
    if (m_Best.empty())
        return nullptr;
    return &*m_Best.begin();
}

size_t GameMgr::GetBestBlockCount() const
{
    return m_Best.size();
}

const Block *GameMgr::GetPreferredBlocks() const
{
    if (m_Preferred.empty())
        return nullptr;
    return &*m_Preferred.begin();
}

size_t GameMgr::GetPreferredBlockCount() const
{
    return m_Preferred.size();
}

void GameMgr::OpenBlock(int x, int y)
{
    OpenBlock(GetIndex(x, y));
}

void Largest(BlockSet &bests, std::function<int(int)> fun)
{
    if (bests.size() <= 1)
        return;
    BlockSet newBests;
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
        if (bestVal <= p)
            newBests.push_back(bests[i]);
    }
    newBests.swap(bests);
}
void Largest(BlockSet &bests, std::function<double(int)> fun)
{
    if (bests.size() <= 1)
        return;
    BlockSet newBests;
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
        if (bestVal - abs(bestVal) * 1E-8 <= p)
            newBests.push_back(bests[i]);
    }
    newBests.swap(bests);
}

void GameMgr::Solve(SolvingState maxDepth, bool shortcut)
{
    if (!m_Started)
        return;

    m_Best.clear();
    m_Preferred.clear();

    m_Solver->Solve(maxDepth & (SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability), shortcut);
#ifdef _DEBUG
    if (m_Solver->GetTotalStates() == 0)
        throw;
#endif

#ifdef _DEBUG
    for (auto i = 0; i < m_Blocks.size(); ++i)
        switch (m_Solver->GetBlockStatus(i))
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

    if (m_Solver->CanOpenForSure != 0)
    {
        ASSERT(m_Solver->CanOpenForSure >= 0);
        for (auto i = 0; i < m_Blocks.size(); ++i)
            if (!m_Blocks[i].IsOpen && m_Solver->GetBlockStatus(i) == BlockStatus::Blank)
                m_Best.push_back(i);
        ASSERT(!m_Best.empty());
        return;
    }
#ifdef _DEBUG
	if (shortcut)
		for (auto i = 0; i < m_Blocks.size(); ++i)
			if (!m_Blocks[i].IsOpen && m_Solver->GetBlockStatus(i) == BlockStatus::Blank)
				throw;
#endif

    if (!BasicStrategy.HeuristicEnabled ||
        (maxDepth & SolvingState::Heuristic) == SolvingState::Stale &&
        (maxDepth & SolvingState::Drained) == SolvingState::Stale)
        return;

    if ((maxDepth & SolvingState::Drained) == SolvingState::Drained && BasicStrategy.ExhaustEnabled)
        if (m_Drainer == nullptr && m_Solver->GetTotalStates() <= (BasicStrategy.PruningEnabled ? BasicStrategy.PruningCriterion : BasicStrategy.ExhaustCriterion) &&
            (m_Solver->GetTotalStates() > 2 || m_ToOpen > 1))
        {
            EnableDrainer();
            return;
        }

    if (m_Drainer != nullptr)
    {
        m_Drainer->Update();
        if ((maxDepth & SolvingState::Drained) == SolvingState::Drained)
            m_Preferred = m_Drainer->GetBestBlocks();
    }

    if (!BasicStrategy.HeuristicEnabled)
        return;

    if (m_Preferred.empty())
        for (auto i = 0; i < m_Blocks.size(); ++i)
        {
            if (m_Blocks[i].IsOpen || m_Solver->GetBlockStatus(i) != BlockStatus::Unknown)
                continue;
            m_Preferred.push_back(i);
        }

#define LARGEST(exp) Largest(m_Preferred, std::function<double(Block)>([this](Block blk) { return exp; } ))

    for (auto heu : BasicStrategy.DecisionTree)
        switch (heu)
        {
        case HeuristicMethod::MinMineProb:
            LARGEST(-m_Solver->GetProbability(blk));
            break;
        case HeuristicMethod::MaxZeroProb:
            LARGEST(m_Solver->ZeroCondQ(m_BlocksR[blk], blk));
            break;
        case HeuristicMethod::MaxZerosProb:
            LARGEST(m_Solver->ZerosCondQ(m_BlocksR[blk], blk));
            break;
        case HeuristicMethod::MaxZerosExp:
            LARGEST(m_Solver->ZerosECondQ(m_BlocksR[blk], blk));
            break;
        case HeuristicMethod::MaxQuantityExp:
            LARGEST(m_Solver->QuantityCondQ(m_BlocksR[blk], blk));
            break;
        case HeuristicMethod::MinFrontierDist:
            LARGEST(-FrontierDist(blk));
            break;
        default:
            break;
        }
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

bool GameMgr::SemiAutomaticStep(SolvingState maxDepth)
{
    if (!m_Started)
        return false;
    
    if (m_Solver->CanOpenForSure == 0)
        m_Solver->Solve(maxDepth, true);
    if (m_Solver->CanOpenForSure == 0)
#ifdef _DEBUG
    {
        for (auto i = 0; i < m_Blocks.size(); ++i)
            if (!m_Blocks[i].IsOpen && m_Solver->GetBlockStatus(i) == BlockStatus::Blank)
                throw;
        return false;
    }
#else
        return false;
#endif
#ifdef _DEBUG
    auto flag = false;
#endif
    for (auto i = 0; i < m_Blocks.size(); ++i)
    {
        if (m_Blocks[i].IsOpen || m_Solver->GetBlockStatus(i) != BlockStatus::Blank)
            continue;
        OpenBlock(i);
#ifdef _DEBUG
        flag = true;
#endif
        if (m_Started)
            continue;
        break;
    }
    ASSERT(flag);
    return m_Started;
}

bool GameMgr::SemiAutomatic(SolvingState maxDepth)
{
    if (!m_Started)
        return false;
    while (SemiAutomaticStep(maxDepth)) { }
    return m_Started;
}

void GameMgr::AutomaticStep(SolvingState maxDepth)
{
    if (!m_Started)
        return;

    Solve(maxDepth, true);
    OpenOptimalBlocks();
}

void GameMgr::Automatic()
{
    SolvingState st;
    switch (BasicStrategy.Logic)
    {
    case LogicMethod::None: 
        st = SolvingState::Stale;
        break;
    case LogicMethod::Single:
        st = SolvingState::Reduce;
        break;
    case LogicMethod::Double:
        st = SolvingState::Reduce | SolvingState::Overlap;
        break;
    case LogicMethod::Full: 
        st = SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability;
        break;
    default:
        throw;
    }

    if (!m_Settled)
        if (BasicStrategy.InitialPositionSpecified)
            OpenBlock(BasicStrategy.Index);
        else if (!BasicStrategy.HeuristicEnabled)
            OpenBlock(RandomInteger(m_Blocks.size()));

    if (st == SolvingState::Stale)
    {
        if (!BasicStrategy.HeuristicEnabled)
        {
            m_Started = false;
            return;
        }

        while (m_Started)
        {
            for (auto i = 0; i < m_Blocks.size(); ++i)
            {
                if (m_Blocks[i].IsOpen)
                    continue;
                m_Preferred.push_back(i);
            }
            OpenOptimalBlocks();
        }
    }
    else
        while (m_Started)
        {
            SemiAutomatic(st);

            if (!BasicStrategy.HeuristicEnabled)
            {
                m_Started = false;
                break;
            }

            AutomaticStep(st | SolvingState::Heuristic | SolvingState::Drained);
        }
}

void GameMgr::EnableDrainer()
{
    if (m_Drainer != nullptr)
        return;
    SemiAutomatic(SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability);
    m_Drainer = new Drainer(*this);
    Solve(SolvingState::Probability | SolvingState::Drained, false);
}

void GameMgr::Save(std::ostream &sw) const
{
#define WRITE(val) sw.write(reinterpret_cast<const char *>(&(val)), sizeof(val));
    WRITE(m_TotalWidth);
    WRITE(m_TotalHeight);
    WRITE(m_TotalMines);
    WRITE(m_Settled);
    WRITE(m_Started);
    WRITE(m_ToOpen);
    if (m_Settled)
        for (auto &blk : m_Blocks)
        {
            WRITE(blk.Degree);
            WRITE(blk.IsOpen);
            WRITE(blk.IsMine);
        }
}

int GameMgr::GetIndex(int x, int y) const
{
    return x * m_TotalHeight + y;
}

void GameMgr::SettleMines(int initID)
{
    for (auto totalMines = m_TotalMines; totalMines > 0;)
    {
        auto id = RandomInteger(m_TotalWidth * m_TotalHeight);
        if (id == initID)
            continue;
        if (m_Blocks[id].IsMine)
            continue;
        m_Blocks[id].IsMine = true;
        --totalMines;
    }
    m_Settled = true;

    for (auto i = 0; i < m_Blocks.size(); ++i)
    {
        if (m_Blocks[i].IsMine)
            continue;
        m_Blocks[i].Degree = 0;
        for (auto &id : m_BlocksR[i])
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

    if (m_Solver->GetBlockStatus(id) == BlockStatus::Blank)
        --m_Solver->CanOpenForSure;
    ASSERT(m_Solver->CanOpenForSure >= 0);
    m_Solver->AddRestrain(id, false);
    if (m_Blocks[id].Degree == 0)
    {
        for (auto &blk : m_BlocksR[id])
            OpenBlock(blk);
    }
    m_Solver->AddRestrain(m_BlocksR[id], m_Blocks[id].Degree);

    if (--m_ToOpen == 0)
    {
        m_Started = false;
        m_Succeed = true;
    }
}

int GameMgr::FrontierDist(Block blk) const
{
    auto &bt = m_Blocks[blk];
    auto d = max(m_TotalWidth, m_TotalHeight);
    for (auto b : m_Blocks)
    {
        if (!b.IsOpen)
            continue;
        auto v = max(abs(b.X - bt.X), abs(b.Y - bt.Y));
        if (v < d)
            d = v;
    }
    return d;
}
