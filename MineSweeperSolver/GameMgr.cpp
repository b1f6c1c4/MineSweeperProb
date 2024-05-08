#include "GameMgr.h"
#include "random.h"
#include "BinomialHelper.h"
#include "Drainer.h"
#include <iostream>
#include <utility>

GameMgr::GameMgr(int width, int height, int totalMines, bool isSNR, Strategy strategy, bool allowWrongGuess) : BasicStrategy(std::move(strategy)), m_IsExternal(false), m_AllowWrongGuess(allowWrongGuess), m_TotalWidth(width), m_TotalHeight(height), m_TotalMines(totalMines), m_IsSNR(isSNR), m_Settled(false), m_Started(true), m_Succeed(false), m_ToOpen(width * height - totalMines), m_WrongGuesses(0), m_Solver(nullptr), m_Drainer(nullptr), m_LastProbe(-1)
{
    if (BasicStrategy.Logic == LogicMethod::Single || BasicStrategy.Logic == LogicMethod::Double)
        m_Solver = std::make_unique<Solver>(m_TotalWidth * m_TotalHeight);
    else
        m_Solver = std::make_unique<Solver>(m_TotalWidth * m_TotalHeight, m_TotalMines);
    GenerateBlocksR();
    m_AllBits = log2(Binomial(m_TotalWidth * m_TotalHeight, m_TotalMines));
}

GameMgr::GameMgr(int width, int height, int totalMines, Strategy strategy) : BasicStrategy(std::move(strategy)), m_IsExternal(true), m_AllowWrongGuess(false), m_TotalWidth(width), m_TotalHeight(height), m_TotalMines(totalMines), m_IsSNR(false), m_Settled(true), m_Started(true), m_Succeed(false), m_ToOpen(-1), m_WrongGuesses(0), m_Solver(nullptr), m_Drainer(nullptr), m_LastProbe(-1)
{
    if (BasicStrategy.Logic == LogicMethod::Single || BasicStrategy.Logic == LogicMethod::Double || m_TotalMines == -1)
        m_Solver = std::make_unique<Solver>(m_TotalWidth * m_TotalHeight);
    else
        m_Solver = std::make_unique<Solver>(m_TotalWidth * m_TotalHeight, m_TotalMines);
    GenerateBlocksR();
    if (m_TotalMines == -1)
        m_AllBits = m_TotalWidth * m_TotalHeight;
    else
        m_AllBits = log2(Binomial(m_TotalWidth * m_TotalHeight, m_TotalMines));
}

GameMgr::GameMgr(std::istream &sr, Strategy strategy) : BasicStrategy(std::move(strategy)), m_IsExternal(false), m_AllowWrongGuess(false), m_TotalWidth(0), m_TotalHeight(0), m_TotalMines(0), m_IsSNR(false), m_Settled(false), m_Started(true), m_Succeed(false), m_ToOpen(0), m_WrongGuesses(0), m_Solver(nullptr), m_Drainer(nullptr), m_LastProbe(0)
{
#define READ(val) sr.read(reinterpret_cast<char *>(&(val)), sizeof(val))
    READ(m_IsExternal);
    READ(m_AllowWrongGuess);
    READ(m_TotalWidth);
    READ(m_TotalHeight);
    READ(m_TotalMines);
    READ(m_IsSNR);
    READ(m_Settled);
    READ(m_Started);
    READ(m_ToOpen);
    READ(m_WrongGuesses);
    READ(m_LastProbe);

    if (BasicStrategy.Logic == LogicMethod::Single || BasicStrategy.Logic == LogicMethod::Double || !m_TotalMines)
        m_Solver = std::make_unique<Solver>(m_TotalWidth * m_TotalHeight);
    else
        m_Solver = std::make_unique<Solver>(m_TotalWidth * m_TotalHeight, m_TotalMines);

    GenerateBlocksR();

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
    return &*m_Drainer;
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

int GameMgr::GetWrongGuesses() const
{
    return m_WrongGuesses;
}

bool GameMgr::GetSettled() const
{
    return m_Settled;
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

int GameMgr::GetLastProbe() const
{
    return m_LastProbe;
}

double GameMgr::GetMinProbability() const
{
    double m = 1;
    if (m_Drainer)
        for (auto p : m_Drainer->GetBestProbabilityList())
            m = MIN(m, p);
    else
        for (auto &blk : m_Blocks)
            if (m_Solver->GetBlockStatus(blk.Index) == BlockStatus::Unknown)
            {
                auto p = m_Solver->GetProbability(blk.Index);
                m = MIN(m, p);
            }
    return m;
}

double GameMgr::GetMaxProbability() const
{
    double m = 0;
    if (m_Drainer)
        for (auto p : m_Drainer->GetBestProbabilityList())
            m = MAX(m, p);
    else
        for (auto &blk : m_Blocks)
            if (m_Solver->GetBlockStatus(blk.Index) == BlockStatus::Unknown)
            {
                auto p = m_Solver->GetProbability(blk.Index);
                m = MAX(m, p);
            }
    return m;
}

const BlockProperty &GameMgr::GetBlockProperty(int x, int y) const
{
    return m_Blocks[GetIndex(x, y)];
}

const BlockProperty &GameMgr::SetBlockDegree(int x, int y, int degree)
{
    if (!m_IsExternal)
        throw std::runtime_error("only external games can be modified");
    auto id = GetIndex(x, y);
    auto &b = m_Blocks[id];
    b.Degree = degree;
    b.IsOpen = true;
    m_Solver->AddRestrain(m_BlocksR[id], degree);
    m_Solver->AddRestrain(id, false);
    return b;
}

const BlockProperty &GameMgr::SetBlockMine(int x, int y, bool mined)
{
    if (!m_IsExternal)
        throw std::runtime_error("only external games can be modified");
    auto id = GetIndex(x, y);
    auto &b = m_Blocks[id];
    b.IsOpen = true;
    b.IsMine = mined;
    b.Degree = -1;
    m_Solver->AddRestrain(id, mined);
    return b;
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

const BlockSet &GameMgr::GetBestBlockList() const
{
    return m_Best;
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

const BlockSet &GameMgr::GetPreferredBlockList() const
{
    return m_Preferred;
}

const std::vector<double> &GameMgr::GetBestProbabilityList() const
{
    static std::vector<double> empty{};
    if (!m_Drainer)
        return empty;
    return m_Drainer->GetBestProbabilityList();
}

void GameMgr::Solve(SolvingState maxDepth, bool shortcut)
{
    if (!m_Started)
        return;

    m_Best.clear();
    m_Preferred.clear();

    m_Solver->Solve(maxDepth & (SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability), shortcut);
#ifndef NDEBUG
    if (m_Solver->GetTotalStates() == 0)
        throw std::runtime_error("total states = 0");
#endif

#ifndef NDEBUG
    if (!m_IsExternal)
    {
        for (auto i = 0; i < m_Blocks.size(); ++i)
            switch (m_Solver->GetBlockStatus(i))
            {
            case BlockStatus::Mine:
                if (!m_Blocks[i].IsMine)
                    throw std::runtime_error("mine is not mine");
                break;
            case BlockStatus::Blank:
                if (m_Blocks[i].IsMine)
                    throw std::runtime_error("blank is not blank");
                break;
            case BlockStatus::Unknown:
            default:
                break;
            }
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
#ifndef NDEBUG
	if (shortcut)
		for (auto i = 0; i < m_Blocks.size(); ++i)
			if (!m_Blocks[i].IsOpen && m_Solver->GetBlockStatus(i) == BlockStatus::Blank)
                throw std::runtime_error("not open but blank");
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

#define LARGEST(exp) Largest(m_Preferred, [this](Block blk) -> double { return exp; } )

    for (auto heu : BasicStrategy.DecisionTree)
        switch (heu)
        {
        case HeuristicMethod::MinMineProb:
            LARGEST(-m_Solver->GetProbability(blk));
            break;
        case HeuristicMethod::MaxZeroProb:
            LARGEST(m_Solver->ZeroCondQ(m_BlocksR[blk], blk) * (1 - m_Solver->GetProbability(blk)));
            break;
        case HeuristicMethod::MaxZerosProb:
            LARGEST(m_Solver->ZerosCondQ(m_BlocksR[blk], blk) * (1 - m_Solver->GetProbability(blk)));
            break;
        case HeuristicMethod::MaxZerosExp:
            LARGEST(m_Solver->ZerosECondQ(m_BlocksR[blk], blk) * (1 - m_Solver->GetProbability(blk)));
            break;
        case HeuristicMethod::MaxQuantityExp:
            LARGEST(m_Solver->QuantityCondQ(m_BlocksR[blk], blk));
            break;
        case HeuristicMethod::MinFrontierDist:
            LARGEST(-FrontierDist(blk));
            break;
        case HeuristicMethod::MaxUpperBound:
            LARGEST(m_Solver->UpperBoundCondQ(m_BlocksR[blk], blk) * (1 - m_Solver->GetProbability(blk)));
            break;
        default:
            break;
        }
}

void GameMgr::OpenOptimalBlocks()
{
    if (m_IsExternal)
        throw std::runtime_error("external games cannot be automated");

    if (!m_Best.empty())
    {
        for (auto b : m_Best)
        {
            OpenBlockImpl(m_LastProbe = b);
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
    OpenBlockImpl(m_LastProbe = blk);
}

bool GameMgr::SemiAutomaticStep(SolvingState maxDepth, bool single)
{
    if (m_IsExternal)
        throw std::runtime_error("external games cannot be automated");

    if (!m_Started)
        return false;

    if (m_Solver->CanOpenForSure == 0)
        m_Solver->Solve(maxDepth, true);
    if (m_Solver->CanOpenForSure == 0)
#ifndef NDEBUG
    {
        for (auto i = 0; i < m_Blocks.size(); ++i)
            if (!m_Blocks[i].IsOpen && m_Solver->GetBlockStatus(i) == BlockStatus::Blank)
                throw std::runtime_error("not open but blank");
        return false;
    }
#else
        return false;
#endif
#ifndef NDEBUG
    auto flag = false;
#endif
    for (auto i = 0; i < m_Blocks.size(); ++i)
    {
        if (m_Blocks[i].IsOpen || m_Solver->GetBlockStatus(i) != BlockStatus::Blank)
            continue;
        OpenBlockImpl(m_LastProbe = i);
#ifndef NDEBUG
        flag = true;
#endif
        if (single)
            break;
        if (m_Started)
            continue;
        break;
    }
    ASSERT(flag);
    return m_Started;
}

bool GameMgr::SemiAutomatic(SolvingState maxDepth)
{
    if (m_IsExternal)
        throw std::runtime_error("external games cannot be automated");

    if (!m_Started)
        return false;
    while (SemiAutomaticStep(maxDepth, false)) { }
    return m_Started;
}

void GameMgr::AutomaticStep(SolvingState maxDepth)
{
    if (m_IsExternal)
        throw std::runtime_error("external games cannot be automated");

    if (!m_Started)
        return;

    if (!m_Settled && BasicStrategy.InitialPositionSpecified)
    {
        OpenBlockImpl(m_LastProbe = BasicStrategy.Index);
        return;
    }

    Solve(maxDepth, true);
    OpenOptimalBlocks();
}

void GameMgr::Automatic(bool drain)
{
    if (m_IsExternal)
        throw std::runtime_error("external games cannot be automated");

    SolvingState st;
    switch (BasicStrategy.Logic)
    {
    case LogicMethod::Passive:
        st = SolvingState::Stale;
        break;
    case LogicMethod::Single:
    case LogicMethod::SingleExtended:
        st = SolvingState::Reduce;
        break;
    case LogicMethod::Double:
    case LogicMethod::DoubleExtended:
        st = SolvingState::Reduce | SolvingState::Overlap;
        break;
    case LogicMethod::Full:
        st = SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability;
        break;
    default:
        throw std::runtime_error("logic not supported");
    }

    if (!m_Settled)
        if (BasicStrategy.InitialPositionSpecified)
            OpenBlockImpl(m_LastProbe = BasicStrategy.Index);
        else if (!BasicStrategy.HeuristicEnabled)
            OpenBlockImpl(m_LastProbe = RandomInteger(m_Blocks.size()));

    if (st == SolvingState::Stale) // Passive Logic
    {
        if (!BasicStrategy.HeuristicEnabled)
        {
            m_Started = false;
            return;
        }

        while (m_Started)
        {
            // All open blocks are preferred
            for (auto i = 0; i < m_Blocks.size(); ++i)
            {
                if (m_Blocks[i].IsOpen)
                    continue;
                m_Preferred.push_back(i);
            }
            OpenOptimalBlocks();
        }
    }
    else // Anything more than passive
        while (m_Started)
        {
            SemiAutomatic(st);

            if (!BasicStrategy.HeuristicEnabled)
            {
                m_Started = false;
                break;
            }

            if (drain)
                AutomaticStep(st | SolvingState::Heuristic | SolvingState::Drained);
            else
                AutomaticStep(st | SolvingState::Heuristic);
        }
}

void GameMgr::EnableDrainer()
{
    if (m_Drainer != nullptr)
        return;
    SemiAutomatic(SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability);
    m_Drainer = std::make_unique<Drainer>(*this);
    Solve(SolvingState::Probability | SolvingState::Drained, false);
}

void GameMgr::Save(std::ostream &sw) const
{
#define WRITE(val) sw.write(reinterpret_cast<const char *>(&(val)), sizeof(val))
    WRITE(m_IsExternal);
    WRITE(m_AllowWrongGuess);
    WRITE(m_TotalWidth);
    WRITE(m_TotalHeight);
    WRITE(m_TotalMines);
    WRITE(m_IsSNR);
    WRITE(m_Settled);
    WRITE(m_Started);
    WRITE(m_ToOpen);
    WRITE(m_WrongGuesses);
    WRITE(m_LastProbe);
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
again:
        auto id = RandomInteger(m_TotalWidth * m_TotalHeight);
        if (id == initID)
            goto again;
        if (m_IsSNR) {
            for (auto &blk : m_BlocksR[initID])
                if (id == blk)
                    goto again;
        }
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

void GameMgr::GenerateBlocksR()
{
    m_Blocks.reserve(m_TotalWidth * m_TotalHeight);
    m_BlocksR.reserve(m_TotalWidth * m_TotalHeight);

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
        }
}

bool GameMgr::OpenBlock(int x, int y)
{
    auto id = GetIndex(x, y);
    if (!m_Started || m_Settled && m_Blocks[id].IsOpen)
        return false;

    OpenBlockImpl(m_LastProbe = id);
    return true;
}

void GameMgr::OpenBlockImpl(int id)
{
    if (!m_Started)
        return;
    if (!m_Settled)
        SettleMines(id);

    if (m_Blocks[id].IsOpen)
        return;
    m_Blocks[id].IsOpen = true;

    if (m_Blocks[id].IsMine)
        if (m_AllowWrongGuess)
        {
            m_WrongGuesses++;
            m_Solver->AddRestrain(id, true);
            return;
        }
        else
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
            OpenBlockImpl(blk);
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
    auto d = MAX(m_TotalWidth, m_TotalHeight);
    for (auto b : m_Blocks)
    {
        if (!b.IsOpen)
            continue;
        auto v = MAX(abs(b.X - bt.X), abs(b.Y - bt.Y));
        if (v < d)
            d = v;
    }
    return d;
}
