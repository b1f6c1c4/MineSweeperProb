#include "Solver.h"
#include <algorithm>
#include "BinomialHelper.h"

#ifdef _DEBUG
#define ASSERT(val) if (!(val)) throw
#else
#define ASSERT(val)
#endif

#define ZEROQ(val) (abs(val) < 1E-3)

#define M(x, y) matrix[(x) * height + (y)]

static size_t Hash(const BlockSet &set);

Solver::Solver(size_t count) : CanOpenForSure(0), m_State(SolvingState::Stale), m_Manager(count, BlockStatus::Unknown), m_Probability(count), m_TotalStates(NAN), m_Pairs_Temp(nullptr), m_Pairs_Temp_Size(0)
{
    m_BlockSets.emplace_back(count);
    auto &lst = m_BlockSets.back();
    for (auto i = 0; i < count; ++i)
        lst[i] = i;
    m_SetIDs.resize(count, 0);
    m_Matrix.emplace_back();
}

Solver::Solver(const Solver &other) : CanOpenForSure(other.CanOpenForSure), m_State(other.m_State), m_Manager(other.m_Manager), m_BlockSets(other.m_BlockSets), m_SetIDs(other.m_SetIDs), m_Matrix(other.m_Matrix), m_MatrixAugment(other.m_MatrixAugment), m_Minors(other.m_Minors), m_Solutions(other.m_Solutions), m_Probability(other.m_Probability), m_TotalStates(other.m_TotalStates), m_Pairs_Temp(nullptr), m_Pairs_Temp_Size(0) { }

Solver::~Solver()
{
    ClearDistCondQCache();
    if (m_Pairs_Temp != nullptr)
        delete[] m_Pairs_Temp;
}

BlockStatus Solver::GetBlockStatus(Block block) const
{
    return m_Manager[block];
}

const BlockStatus *Solver::GetBlockStatuses() const
{
    return &*m_Manager.begin();
}

double Solver::GetProbability(Block block) const
{
    return m_Probability[block];
}

const double *Solver::GetProbabilities() const
{
    return &*m_Probability.begin();
}

double Solver::GetTotalStates() const
{
    return m_TotalStates;
}

void Solver::AddRestrain(Block blk, bool isMine)
{
    if (m_Manager[blk] == BlockStatus::Unknown)
    {
        m_Manager[blk] = isMine ? BlockStatus::Mine : BlockStatus::Blank;
        m_State = SolvingState::Stale;
        return;
    }
    m_State &= SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability;
    if (m_SetIDs[blk] >= 0)
        ReduceBlockSet(m_SetIDs[blk]);
    if (m_Manager[blk] == BlockStatus::Blank && isMine)
        throw;
    if (m_Manager[blk] == BlockStatus::Mine && !isMine)
        throw;
}

void Solver::AddRestrain(const BlockSet &set, int mines)
{

    auto dMines = 0;
    auto &bin = m_IntersectionCounts_Temp;
    GetIntersectionCounts(set, bin, dMines);

    if (mines == dMines)
    {
        for (auto blk : set)
            if (m_Manager[blk] == BlockStatus::Unknown)
            {
                m_Manager[blk] = BlockStatus::Blank;
                ++CanOpenForSure;
            }
        for (auto blk : set)
            if (m_SetIDs[blk] >= 0)
                ReduceBlockSet(m_SetIDs[blk]);
        return;
    }
    {
        auto t = 0;
        for (auto v : bin)
            t += v;
        if (mines == t + dMines)
        {
            for (auto blk : set)
                if (m_Manager[blk] == BlockStatus::Unknown)
                    m_Manager[blk] = BlockStatus::Mine;
            for (auto blk : set)
                if (m_SetIDs[blk] >= 0)
                    ReduceBlockSet(m_SetIDs[blk]);
            return;
        }
    }

    for (auto &containers : m_Matrix)
        containers.push_back(CONT_ZERO);

    for (auto col = 0; col < bin.size(); ++col)
    {
        if (bin[col] == 0)
            continue;

        if (bin[col] == m_BlockSets[col].size())
        {
            SB(m_Matrix[CNT(col)].back(), SHF(col));
            continue;
        }

        m_BlockSets.emplace_back();
        auto &it = m_BlockSets.back();
        it.reserve(bin[col]);

        auto p = 0, q = 0;
        for (; p < set.size() && q < m_BlockSets[col].size();)
        {
            if (set[p] < m_BlockSets[col][q])
                ++p;
            else if (set[p] > m_BlockSets[col][q])
                ++q;
            else
            {
                it.push_back(set[p]);
                m_BlockSets[col].erase(m_BlockSets[col].begin() + q);
                m_SetIDs[set[p++]] = m_BlockSets.size() - 1;
            }
        }

        if (CONTS(m_BlockSets.size()) > m_Matrix.size())
        {
            ASSERT(CONTS(m_BlockSets.size()) == m_Matrix.size() + 1);
            m_Matrix.emplace_back(m_Matrix.front().size(), CONT_ZERO);
        }
        for (auto i = 0; i < m_Matrix[CNT(col)].size() - 1; ++i)
            if (NZ(m_Matrix[CNT(col)][i], SHF(col)))
                SB(m_Matrix[CNT(m_BlockSets.size() - 1)][i], SHF(m_BlockSets.size() - 1));
        SB(m_Matrix[CNT(m_BlockSets.size() - 1)].back(), SHF(m_BlockSets.size() - 1));
    }

    m_MatrixAugment.push_back(mines - dMines);
    m_State = SolvingState::Stale;
}

void Solver::Solve(SolvingState maxDepth, bool shortcut)
{
    if ((m_State & maxDepth) == maxDepth)
        return;

    if (shortcut && CanOpenForSure > 0)
        return;

    m_Solutions.clear();
    ClearDistCondQCache();

    while (true)
    {
        while ((m_State & SolvingState::Reduce) == SolvingState::Stale)
        {
            ReduceRestrains();
            if (shortcut && CanOpenForSure > 0)
                return;
        }
        if ((maxDepth & SolvingState::Overlap) == SolvingState::Stale)
            break;
        SimpleOverlapAll();
        if (shortcut && CanOpenForSure > 0)
            return;
        if ((m_State & SolvingState::Overlap) == SolvingState::Overlap)
            break;
    }
    MergeSets();

    if ((maxDepth & SolvingState::Probability) == SolvingState::Stale)
        return;

    if ((m_State & SolvingState::Probability) == SolvingState::Probability)
        return;

    m_State |= SolvingState::Probability;

    if (m_BlockSets.empty())
    {
        m_TotalStates = double(1);

        m_Solutions.emplace_back();
        ProcessSolutions();
        return;
    }

    auto width = m_BlockSets.size() + 1;
    auto height = m_Matrix.front().size();
    auto matrix = new double[width * height];
    for (auto col = 0; col < width - 1; ++col)
        for (auto row = 0; row < height; ++row)
            if (NZ(m_Matrix[CNT(col)][row], SHF(col)))
                M(col, row) = 1;
            else
                M(col, row) = 0;
    for (auto row = 0; row < height; ++row)
        M(m_BlockSets.size(), row) = m_MatrixAugment[row];
    Gauss(matrix, width, height);

    if (!m_Minors.empty() &&
        m_Minors.back() == m_BlockSets.size())
        m_Minors.pop_back();
    else
    {
        delete[] matrix;
        m_TotalStates = double(0);
        return;
    }

    EnumerateSolutions(matrix, width, height);
    delete[] matrix;

    if (m_Solutions.empty())
    {
        m_TotalStates = double(0);
        return;
    }

    ProcessSolutions();
}

double Solver::ZeroCondQ(const BlockSet &set, Block blk)
{
    DistCondQParameters par(m_SetIDs[blk], 0);
    int dMines;
    GetIntersectionCounts(set, par.Sets1, dMines);
    par.Hash();
    for (auto b : par.Sets1)
        par.Length += b;
    return ZCondQ(std::move(par)).m_Result.front();
}

double Solver::ZerosCondQ(const BlockSet &set, Block blk)
{
    DistCondQParameters par(m_SetIDs[blk], 0);
    int dMines;
    GetIntersectionCounts(set, par.Sets1, dMines);
    par.Hash();
    for (auto b : par.Sets1)
        par.Length += b;
    return ZsCondQ(std::move(par)).m_Probability;
}

double Solver::ZerosECondQ(const BlockSet& set, Block blk)
{
    DistCondQParameters par(m_SetIDs[blk], 0);
    int dMines;
    GetIntersectionCounts(set, par.Sets1, dMines);
    par.Hash();
    for (auto b : par.Sets1)
        par.Length += b;
    return ZsCondQ(std::move(par)).m_Expectation;
}

const std::vector<double> &Solver::DistributionCondQ(const BlockSet &set, Block blk, int &min)
{
    DistCondQParameters par(m_SetIDs[blk], 0);
    int dMines;
    GetIntersectionCounts(set, par.Sets1, dMines);
    par.Hash();
    for (auto b : par.Sets1)
        par.Length += b;
    min = dMines;
    return DistCondQ(std::move(par)).m_Result;
}

double Solver::QuantityCondQ(const BlockSet &set, Block blk)
{
    DistCondQParameters par(m_SetIDs[blk], 0);
    int dMines;
    GetIntersectionCounts(set, par.Sets1, dMines);
    par.Hash();
    for (auto b : par.Sets1)
        par.Length += b;
    auto &di = DistCondQ(std::move(par));

    double q = 0;
    for (auto j = 0; j < di.m_Result.size(); ++j)
        if (di.m_Result[j] != 0)
        {
            auto p = di.m_Result[j] / di.m_TotalStates;
            q += -p * log2(p);
        }

    return q;
}

void Solver::DropColumn(int col)
{
#ifdef _DEBUG
    for (auto blk : m_BlockSets[col])
        ASSERT(m_SetIDs[blk] != col);
#endif
    auto last = m_BlockSets.size() - 1;
    if (col != last)
        m_BlockSets.back().swap(m_BlockSets[col]);
    m_BlockSets.pop_back();
    if (col != last)
    {
        for (auto blk : m_BlockSets[col])
        {
            ASSERT(m_Manager[blk] != BlockStatus::Unknown || m_SetIDs[blk] == last);
            m_SetIDs[blk] = col;
        }
        for (auto j = 0; j < m_Matrix[CNT(last)].size(); ++j)
            if (NZ(m_Matrix[CNT(last)][j], SHF(last)))
                SB(m_Matrix[CNT(col)][j], SHF(col));
            else
                CB(m_Matrix[CNT(col)][j], SHF(col));
    }
    for (auto j = 0; j < m_Matrix[CNT(last)].size(); ++j)
        CB(m_Matrix[CNT(last)][j], SHF(last));
    if (last % CONT_SIZE == 0)
        m_Matrix.pop_back();
}

void Solver::DropRow(int row)
{
    if (row != m_MatrixAugment.size() - 1)
    {
        m_MatrixAugment[row] = m_MatrixAugment.back();
        m_MatrixAugment.pop_back();
        for (auto &containers : m_Matrix)
        {
            containers[row] = containers.back();
            containers.pop_back();
        }
    }
    else
    {
        m_MatrixAugment.pop_back();
        for (auto &containers : m_Matrix)
            containers.pop_back();
    }
}

bool Solver::ReduceBlockSet(int col)
{
    auto dMines = 0;
    auto &setN = m_Reduce_Temp;
    setN.clear();
    for (auto it = m_BlockSets[col].begin(); it != m_BlockSets[col].end(); ++it)
    {
        switch (m_Manager[*it])
        {
        case BlockStatus::Mine:
            ++dMines;
            m_SetIDs[*it] = -1;
            break;
        case BlockStatus::Blank:
            m_SetIDs[*it] = -2;
            break;
        case BlockStatus::Unknown:
            ASSERT(m_SetIDs[*it] == col);
            setN.push_back(*it);
            continue;
        default:
            ASSERT(false);
        }
    }
    setN.swap(m_BlockSets[col]);
    if (dMines != 0)
    {
        for (auto j = 0; j < m_Matrix[CNT(col)].size(); ++j)
            if (NZ(m_Matrix[CNT(col)][j], SHF(col)))
            {
                m_MatrixAugment[j] -= dMines;
                ASSERT(m_MatrixAugment[j] >= 0);
            }
        m_State = SolvingState::Stale;
    }
    if (m_BlockSets[col].empty())
    {
        DropColumn(col);
        m_State = SolvingState::Stale;
        return true;
    }
    return false;
}

bool Solver::ReduceRestrainBlank(int row)
{
    ASSERT(m_MatrixAugment[row] >= 0);
    if (m_MatrixAugment[row] != 0)
        return false;

    for (auto col = 0; col < m_BlockSets.size(); ++col)
    {
        if (Z(m_Matrix[CNT(col)][row], SHF(col)))
            continue;

        for (auto blk : m_BlockSets[col])
        {
            ASSERT(m_Manager[blk] == BlockStatus::Blank || m_SetIDs[blk] == col);
            m_SetIDs[blk] = -2;
            if (m_Manager[blk] == BlockStatus::Blank)
                continue;
            ASSERT(m_Manager[blk] == BlockStatus::Unknown);
            m_Manager[blk] = BlockStatus::Blank;
            ++CanOpenForSure;
            m_State = SolvingState::Stale;
        }
        DropColumn(col--);
    }
    DropRow(row);
    return true;
}

bool Solver::ReduceRestrainMine(int row)
{
    auto &sum = m_ReduceCount_Temp;
    ASSERT(m_MatrixAugment[row] <= sum[row]);
    if (m_MatrixAugment[row] != sum[row])
        return false;

    for (auto col = 0; col < m_BlockSets.size(); ++col)
    {
        if (Z(m_Matrix[CNT(col)][row], SHF(col)))
            continue;

        for (auto j = 0; j < m_Matrix[CNT(col)].size(); ++j)
            if (NZ(m_Matrix[CNT(col)][j], SHF(col)))
            {
                m_MatrixAugment[j] -= m_BlockSets[col].size();
                sum[j] -= m_BlockSets[col].size();
                ASSERT(m_MatrixAugment[j] >= 0);
            }
        for (auto blk : m_BlockSets[col])
        {
            ASSERT(m_Manager[blk] == BlockStatus::Mine || m_SetIDs[blk] == col);
            m_SetIDs[blk] = -1;
            if (m_Manager[blk] == BlockStatus::Mine)
                continue;
            ASSERT(m_Manager[blk] == BlockStatus::Unknown);
            m_Manager[blk] = BlockStatus::Mine;
            m_State = SolvingState::Stale;
        }
        DropColumn(col--);
    }
    if (row != sum.size() - 1)
        sum[row] = sum.back();
    sum.pop_back();
    DropRow(row);
    return true;
}

void Solver::MergeSets()
{
    std::multimap<size_t, int> hash;
    for (auto i = 0; i < m_BlockSets.size(); ++i)
    {
        size_t h = 5381;
        for (auto v : m_Matrix[CNT(i)])
            h = (h << 5) + h + B(v, SHF(i));
        auto itp = hash.equal_range(h);
        auto flag = true;
        for (auto it = itp.first; it != itp.second; ++it)
        {
            for (auto j = 0; j < m_Matrix[CNT(i)].size(); ++j)
                if (B(m_Matrix[CNT(i)][j], SHF(i)) != B(m_Matrix[CNT(it->second)][j], SHF(it->second)))
                {
                    flag = false;
                    break;
                }
            if (flag)
            {
                m_BlockSets[it->second].reserve(m_BlockSets[it->second].size() + m_BlockSets[i].size());
                for (auto &blk : m_BlockSets[i])
                    m_BlockSets[it->second].push_back(blk);
                sort(m_BlockSets[it->second].begin(), m_BlockSets[it->second].end());
                for (auto &blk : m_BlockSets[i])
                {
                    ASSERT(m_Manager[blk] != BlockStatus::Unknown || m_SetIDs[blk] == i);
                    m_SetIDs[blk] = it->second;
                }
                DropColumn(i);

                --i;
                break;
            }
        }
        if (flag)
            hash.insert(std::make_pair(h, i));
    }
}

void Solver::ReduceRestrains()
{
    m_State |= SolvingState::Reduce;

    for (auto col = 0; col < m_BlockSets.size(); ++col)
        if (ReduceBlockSet(col))
            --col;

    for (auto row = 0; row < m_MatrixAugment.size(); ++row)
        if (ReduceRestrainBlank(row))
            --row;

    auto &sum = m_ReduceCount_Temp;
    sum.clear();
    sum.resize(m_Matrix.front().size(), CONT_ZERO);
    for (auto cnt = 0; cnt < m_Matrix.size(); ++cnt)
        for (auto i = 0; i < m_Matrix[cnt].size(); ++i)
        {
            auto v = m_Matrix[cnt][i];
            for (auto shift = 0; shift < (cnt == m_Matrix.size() - 1 ? SHF(m_BlockSets.size()) : CONT_SIZE); ++shift, v >>= 1)
                if (NZ(v, 0))
                    sum[i] += m_BlockSets[cnt * CONT_SIZE + shift].size();
                else if (v == 0)
                    break;
        }

    for (auto row = 0; row < m_MatrixAugment.size(); ++row)
        if (ReduceRestrainMine(row))
            --row;
}

void Solver::SimpleOverlapAll()
{
    m_State |= SolvingState::Overlap;

    auto d = m_MatrixAugment.size();
    auto sz = (d - 1) * d / 2;
    if (m_Pairs_Temp != nullptr && m_Pairs_Temp_Size < sz)
    {
        delete[] m_Pairs_Temp;
        m_Pairs_Temp = nullptr;
    }
    if (m_Pairs_Temp == nullptr)
    {
        m_Pairs_Temp_Size = sz;
        m_Pairs_Temp = new bool[sz];
        ZeroMemory(m_Pairs_Temp, sizeof(bool) * m_Pairs_Temp_Size);
    }

    for (auto cnt = 0; cnt < m_Matrix.size(); ++cnt)
    {
        auto id = 0;
        for (auto p = 0; p < d - 1; ++p)
            for (auto q = p + 1; q < d; ++q, ++id)
                if (!m_Pairs_Temp[id])
                {
                    auto a = m_Matrix[cnt][p], b = m_Matrix[cnt][q];
                    if ((cnt == m_Matrix.size() - 1 ? LB(a & b, SHF(m_BlockSets.size())) : a & b) != CONT_ZERO)
                    {
                        if (SimpleOverlap(p, q))
                        {
                            m_State = SolvingState::Stale;
                            return;
                        }
                        
                        m_Pairs_Temp[id] = true;
                    }
                }
        ASSERT(id == sz);
    }
}

bool Solver::SimpleOverlap(int r1, int r2)
{
    auto &exceptA = m_OverlapA_Temp, &exceptB = m_OverlapB_Temp, &intersection = m_OverlapC_Temp;
    exceptA.clear() , exceptA.reserve(m_Matrix.size());
    exceptB.clear() , exceptB.reserve(m_Matrix.size());
    intersection.clear() , intersection.reserve(m_Matrix.size());


    for (auto &containers : m_Matrix)
    {
        exceptA.push_back(containers[r1] & ~containers[r2]);
        exceptB.push_back(~containers[r1] & containers[r2]);
        intersection.push_back(containers[r1] & containers[r2]);
    }

    typedef std::pair<int, int> Iv;

    auto sum = [this](const std::vector<Container> &lst)-> Iv
        {
            Iv iv(0, 0);
            for (auto cnt = 0; cnt < lst.size(); ++cnt)
            {
                auto v = lst[cnt];
                for (auto shift = 0; shift < (cnt == lst.size() - 1 ? SHF(m_BlockSets.size()) : CONT_SIZE); ++shift, v >>= 1)
                    if (NZ(v, 0))
                        iv.second += m_BlockSets[cnt * CONT_SIZE + shift].size();
                    else if (v == 0)
                        break;
            }
            return iv;
        };

    auto subs = [](Iv i1, Iv i2)-> Iv
        {
            return Iv(i1.first - i2.second, i1.second - i2.first);
        };
    auto ints = [](Iv i1, Iv i2)-> Iv
        {
            return Iv(max(i1.first, i2.first), min(i1.second, i2.second));
        };

    auto ivAC = Iv(m_MatrixAugment[r1], m_MatrixAugment[r1]), ivBC = Iv(m_MatrixAugment[r2], m_MatrixAugment[r2]);
    auto ivA0 = sum(exceptA), ivB0 = sum(exceptB), ivC0 = sum(intersection);
    auto ivA = ivA0, ivB = ivB0, ivC = ivC0;
    ivA = ints(ivA, subs(ivAC, ivC));
    ivB = ints(ivB, subs(ivBC, ivC));
    ivC = ints(ivC, ints(subs(ivAC, ivA), subs(ivBC, ivB)));
    ivA = ints(ivA, subs(ivAC, ivC));
    ivB = ints(ivB, subs(ivBC, ivC));
    ivC = ints(ivC, ints(subs(ivAC, ivA), subs(ivBC, ivB)));

    auto proc = [this](const std::vector<Container> &lst, const Iv &iv0,const Iv &iv)
        {
            if (lst.empty())
                return;
            if (iv0.second == iv.first)
                for (auto cnt = 0; cnt < lst.size(); ++cnt)
                {
                    auto v = lst[cnt];
                    for (auto shift = 0; shift < (cnt == lst.size() - 1 ? SHF(m_BlockSets.size()) : CONT_SIZE); ++shift, v >>= 1)
                        if (NZ(v, 0))
                        {
                            for (const auto &blk : m_BlockSets[cnt * CONT_SIZE + shift])
                            {
                                if (m_Manager[blk] == BlockStatus::Mine)
                                    continue;
                                ASSERT(m_Manager[blk] == BlockStatus::Unknown);
                                m_Manager[blk] = BlockStatus::Mine;
                                m_State = SolvingState::Stale;
                            }
                        }
                        else if (v == 0)
                            break;
                }
            else if (iv0.first == iv.second)
                for (auto cnt = 0; cnt < lst.size(); ++cnt)
                {
                    auto v = lst[cnt];
                    for (auto shift = 0; shift < (cnt == lst.size() - 1 ? SHF(m_BlockSets.size()) : CONT_SIZE); ++shift, v >>= 1)
                        if (NZ(v, 0))
                        {
                            for (const auto &blk : m_BlockSets[cnt * CONT_SIZE + shift])
                            {
                                if (m_Manager[blk] == BlockStatus::Blank)
                                    continue;
                                ASSERT(m_Manager[blk] == BlockStatus::Unknown);
                                m_Manager[blk] = BlockStatus::Blank;
                                ++CanOpenForSure;
                                m_State = SolvingState::Stale;
                            }
                        }
                        else if (v == 0)
                            break;
                }
        };

    proc(exceptA, ivA0, ivA);
    proc(exceptB, ivB0, ivB);
    proc(intersection, ivC0, ivC);

    return false;
}

void Solver::Gauss(double *matrix, size_t width, size_t height)
{
    m_Minors.clear();
    auto major = 0;
    for (auto col = 0; col < width; ++col)
    {
        int biasRow;
        for (biasRow = major; biasRow < height; ++biasRow)
            if (abs(M(col, biasRow)) >= 1E-8)
                break;
        if (biasRow >= height)
        {
            m_Minors.push_back(col);
            continue;
        }

        auto &vec = m_GaussVec_Temp;
        vec.clear();
        vec.resize(height);
        auto theBiasInv = 1 / M(col, biasRow);
        for (auto row = 0; row < height; ++row)
        {
            if (row != biasRow)
                vec[row] = -M(col, row) * theBiasInv;
#ifdef _DEBUG
            if (row == major)
                M(col, row) = 1;
            else
                M(col, row) = 0;
#endif
        }
        if (major == biasRow)
            for (auto co = col + 1; co < width; ++co)
            {
                auto bias = M(co, biasRow);
                if (!ZEROQ(bias))
                    for (auto row = 0; row < height; ++row)
                        if (row == major)
                            M(co, row) *= theBiasInv;
                        else
                            M(co, row) += vec[row] * bias;
            }
        else
            for (auto co = col + 1; co < width; ++co)
            {
                auto swap = M(co, major);
                auto bias = M(co, biasRow);
                if (ZEROQ(bias))
                    M(co, major) = 0, M(co, biasRow) = swap;
                else
                    for (auto row = 0; row < height; ++row)
                        if (row == major)
                            M(co, row) = bias * theBiasInv;
                        else if (row == biasRow)
                            M(co, row) = swap + vec[major] * bias;
                        else
                            M(co, row) += vec[row] * bias;
            }
        ++major;
    }
}

void Solver::EnumerateSolutions(const double *matrix, size_t width, size_t height)
{
    auto n = m_BlockSets.size();

    if (m_Minors.empty())
    {
        m_Solutions.emplace_back();
        auto &lst = m_Solutions.back().Dist;
        lst.reserve(n);
        for (auto row = 0; row < height; ++row)
            lst.push_back(static_cast<int>(round(M(width - 1, row))));
        return;
    }

    auto mR = n - m_Minors.size();
    auto &majors = m_Majors_Temp;
    auto &cnts = m_Counts_Temp;
    auto &sums = m_Sums_Temp;
    majors.clear() , majors.reserve(mR);
    cnts.clear() , cnts.reserve(mR);
    sums.clear() , sums.reserve(mR);
    m_NonZero_Temp.reserve(m_Minors.size());
    {
        auto minorID = 0;
        auto mainRow = 0;
        for (auto col = 0; col < n; ++col)
            if (minorID < m_Minors.size() &&
                col == m_Minors[minorID])
            {
                if (minorID >= m_NonZero_Temp.size())
                    m_NonZero_Temp.emplace_back();
                auto &lst = m_NonZero_Temp[minorID];
                lst.clear();
                for (auto row = 0; row < height; ++row)
                    if (!ZEROQ(M(col, row)))
                        lst.push_back(row);
                ++minorID;
            }
            else // major
            {
                majors.push_back(col);
                cnts.push_back(m_BlockSets[col].size());
                sums.push_back(M(n, mainRow));
                ++mainRow;
            }
    }
#define AGGR(val) \
    for (auto mainRow : m_NonZero_Temp[stack.size() - 1]) \
        sums[mainRow] -= (val) * M(m_Minors[stack.size() - 1], mainRow)

    auto &stack = m_Stack_Temp;
    stack.clear();
    stack.reserve(m_Minors.size());
    stack.push_back(0);
    while (true)
        if (stack.size() == m_Minors.size())
            if (stack.back() <= m_BlockSets[m_Minors.back()].size())
            {
                auto &lst = m_Dist_Temp;
                lst.clear() , lst.resize(n);
                auto flag = true;
                for (auto mainRow = 0; mainRow < mR; ++mainRow)
                {
                    auto v = round(sums[mainRow]);
                    if (!ZEROQ(v - sums[mainRow]))
                    {
                        flag = false;
                        break;
                    }
                    auto val = static_cast<int>(v);
                    if (val < 0 ||
                        val > cnts[mainRow])
                    {
                        flag = false;
                        break;
                    }
                    lst[majors[mainRow]] = val;
                }
                if (flag)
                {
                    for (auto minorID = 0; minorID < m_Minors.size(); ++minorID)
                        lst[m_Minors[minorID]] = stack[minorID];
                    m_Solutions.emplace_back();
                    m_Solutions.back().Dist.swap(lst);
                }

                AGGR(1);
                ++stack.back();
            }
            else
            {
                AGGR(-stack.back());
                stack.pop_back();
                if (stack.empty())
                    break;
                AGGR(1);
                ++stack.back();
            }
        else if (stack.back() <= m_BlockSets[m_Minors[stack.size() - 1]].size())
            stack.push_back(0); // AGGR(0);
        else
        {
            AGGR(-stack.back());
            stack.pop_back();
            if (stack.empty())
                break;
            AGGR(1);
            ++stack.back();
        }
}

void Solver::ProcessSolutions()
{
    auto &exp = m_Exp_Temp;
    exp.clear() , exp.resize(m_BlockSets.size(), 0);
    m_TotalStates = double(0);
    for (auto &so : m_Solutions)
    {
#ifdef _DEBUG
		for (auto row = 0; row < m_MatrixAugment.size(); ++row)
		{
			auto v = 0;
            for (auto col = 0; col < m_BlockSets.size(); ++col)
                if (NZ(m_Matrix[CNT(col)][row], SHF(col)))
                    v += so.Dist[col];
			ASSERT(m_MatrixAugment[row] == v);
		}
#endif
        so.States = double(1);
        for (auto i = 0; i < m_BlockSets.size(); ++i)
            so.States *= Binomial(m_BlockSets[i].size(), so.Dist[i]);
        m_TotalStates += so.States;
        for (auto i = 0; i < m_BlockSets.size(); ++i)
            exp[i] += so.States * so.Dist[i];
    }
    for (auto &so : m_Solutions)
        so.Ratio = so.States / m_TotalStates;
    for (auto i = 0; i < m_Manager.size(); ++i)
        if (m_Manager[i] == BlockStatus::Mine)
            m_Probability[i] = 1;
        else if (m_Manager[i] == BlockStatus::Blank)
            m_Probability[i] = 0;

    for (auto i = 0; i < m_BlockSets.size(); ++i)
    {
        auto prod = m_TotalStates * m_BlockSets[i].size();
        ASSERT(exp[i] <= prod);
        if (exp[i] == 0)
        {
            for (auto &blk : m_BlockSets[i])
            {
                if (m_Manager[blk] == BlockStatus::Blank)
                    continue;
                ASSERT(m_Manager[blk] == BlockStatus::Unknown);
                m_Manager[blk] = BlockStatus::Blank;
                ++CanOpenForSure;
                m_State &= SolvingState::Overlap | SolvingState::Probability;
            }
        }
        else if (exp[i] == prod)
            for (auto &blk : m_BlockSets[i])
            {
                if (m_Manager[blk] == BlockStatus::Mine)
                    continue;
                ASSERT(m_Manager[blk] == BlockStatus::Unknown);
                m_Manager[blk] = BlockStatus::Mine;
                m_State &= SolvingState::Overlap | SolvingState::Probability;
            }

        auto p = exp[i] / prod;
        for (auto &blk : m_BlockSets[i])
            m_Probability[blk] = p;
    }
}

void Solver::Merge(const std::vector<double> &from, std::vector<double> &to)
{
    ASSERT(from.size() <= to.size());
    for (auto i = 0; i < from.size(); ++i)
        to[i] += from[i];
}

void Solver::Add(std::vector<double> &from, const std::vector<double> &cases)
{
    auto &dicN = m_Add_Temp;
    dicN.clear() , dicN.resize(from.size() + cases.size() - 1, 0);
    for (auto i = 0; i < from.size(); ++i)
        for (auto j = 0; j < cases.size(); ++j)
            dicN[i + j] += from[i] * cases[j];
    dicN.swap(from);
}

void Solver::GetIntersectionCounts(const BlockSet &set1, std::vector<int> &sets1, int &mines) const
{
    sets1.clear();
    sets1.resize(m_BlockSets.size(), 0);
    mines = 0;
    for (auto blk : set1)
    {
        auto v = m_SetIDs[blk];
        if (v == -1)
        {
            ++mines;
            continue;
        }
        if (v == -2)
            continue;
        ++sets1[m_SetIDs[blk]];
    }
}

const DistCondQParameters &Solver::ZCondQ(DistCondQParameters &&par)
{
    DistCondQParameters *ptr = nullptr;
    auto itp = m_DistCondQCache.equal_range(par.m_Hash);
    for (auto it = itp.first; it != itp.second; ++it)
    {
        if (*it->second != par)
            continue;
        if (!it->second->m_Result.empty())
            return *it->second;
        ptr = it->second;
        break;
    }
    if (ptr == nullptr)
    {
        ptr = new DistCondQParameters(std::move(par));
        m_DistCondQCache.insert(std::make_pair(ptr->m_Hash, ptr));
    }
    ptr->m_Result.emplace_back(0);
    auto &val = ptr->m_Result.front();
    for (auto &solution : m_Solutions)
    {
        double valT(1);
        for (auto i = 0; i < m_BlockSets.size(); ++i)
        {
            auto n = m_BlockSets[i].size();
            auto a = ptr->Sets1[i], b = i == ptr->Set2ID ? 1 : 0;
            auto m = solution.Dist[i];

            valT *= Binomial(n - a - b, m);
        }
        val += valT;
    }
    return *ptr;
}

const DistCondQParameters &Solver::ZsCondQ(DistCondQParameters &&par)
{
    DistCondQParameters *ptr = nullptr;
    auto itp = m_DistCondQCache.equal_range(par.m_Hash);
    for (auto it = itp.first; it != itp.second; ++it)
    {
        if (*it->second != par)
            continue;
        if (it->second->m_Expectation != NAN)
            return *it->second;
        ptr = it->second;
        break;
    }
    if (ptr == nullptr)
    {
        ptr = new DistCondQParameters(std::move(par));
        m_DistCondQCache.insert(std::make_pair(ptr->m_Hash, ptr));
    }
    std::vector<int> halves;
    for (auto i = 0; i < ptr->Sets1.size(); ++i)
    {
        if (ptr->Sets1[i] != 0 && ptr->Sets1[i] != m_BlockSets[i].size() &&
            !(i == ptr->Set2ID && ptr->Sets1[i] == m_BlockSets[i].size() - 1))
            halves.push_back(i);
    }

    auto &dic = ptr->m_Result;
    dic.clear();
    dic.resize(ptr->Length + 1, 0);
    std::vector<std::vector<Solution>> solutions(ptr->Length + 1);

    std::vector<int> stack, lb, ub;
    for (auto &solution : m_Solutions)
    {
        if (m_BlockSets[ptr->Set2ID].size() == solution.Dist[ptr->Set2ID])
            continue;

        lb.clear() , ub.clear();
        for (auto id : halves)
        {
            lb.push_back(max(static_cast<int>(solution.Dist[id]) - static_cast<int>(m_BlockSets[id].size()) + (id == ptr->Set2ID ? 1 : 0) + ptr->Sets1[id], 0));
            ub.push_back(min(solution.Dist[id], ptr->Sets1[id]));
        }

        stack.clear();
        stack.reserve(halves.size());
        if (!lb.empty())
            stack.push_back(lb.front());
        while (true)
            if (stack.size() == halves.size())
                if (halves.empty() || stack.back() <= ub[stack.size() - 1])
                {
                    auto val = 0;
                    double st = 1;
                    std::vector<int> dist(ptr->Sets1.size() + halves.size());
                    for (auto i = 0, p = 0; i < ptr->Sets1.size(); ++i)
                    {
                        if (p < halves.size() && i == halves[p])
                        {
                            val += dist[i] = stack[p];
                            dist[ptr->Sets1.size() + p] = solution.Dist[i] - stack[p];
                            st *= Binomial(ptr->Sets1[i], dist[i]);
                            st *= Binomial(m_BlockSets[i].size() - (i == ptr->Set2ID ? 1 : 0) - ptr->Sets1[i], dist[ptr->Sets1.size() + p]);
                            ++p;
                        }
                        else
                        {
                            dist[i] = solution.Dist[i];
                            st *= Binomial(m_BlockSets[i].size() - (i == ptr->Set2ID ? 1 : 0), dist[i]);
                            if (ptr->Sets1[i] != 0)
                                val += dist[i];
                        }
                    }
                    ASSERT(st > 0);
                    dic[val] += st;
                    solutions[val].emplace_back();
                    solutions[val].back().Dist.swap(dist);
                    solutions[val].back().States = st;

                    if (halves.empty())
                        break;
                    ++stack.back();
                }
                else
                {
                    stack.pop_back();
                    if (stack.empty())
                        break;
                    ++stack.back();
                }
            else if (stack.back() <= ub[stack.size() - 1])
                stack.push_back(lb[stack.size()]);
            else
            {
                stack.pop_back();
                if (stack.empty())
                    break;
                ++stack.back();
            }
    }

    ptr->m_TotalStates = 0;
    for (auto val : dic)
        ptr->m_TotalStates += val;

    ptr->m_Probability = 0;
    ptr->m_Expectation = 0;
    std::vector<int> lst;
    for (auto i = 0; i <= ptr->Length; ++i)
    {
        if (solutions[i].empty())
            continue;

        lst.clear();
        for (auto j = 0; j < ptr->Sets1.size(); ++j)
            if (solutions[i][0].Dist[j] == 0)
                lst.push_back(j);
        for (auto j = 1; j < solutions[i].size(); ++j)
        {
            for (auto it = lst.begin(); it != lst.end(); ++it)
                if (solutions[i][j].Dist[*it] != 0)
                {
                    it = lst.erase(it);
                    if (it == lst.end())
                        break;
                }
            if (lst.empty())
                break;
        }
        if (lst.empty())
            continue;

        ptr->m_Probability += dic[i];

        auto totalBlanks = 0;
        auto p = 0;
        for (auto id : lst)
        {
            if (id > ptr->Sets1.size())
            {
                totalBlanks += m_BlockSets[id - ptr->Sets1.size()].size() - ptr->Sets1[id - ptr->Sets1.size()];
                continue;
            }

            while (p < halves.size() && id > halves[p])
                ++p;
            if (p < halves.size() && id == halves[p])
            {
                totalBlanks += ptr->Sets1[id];
                ++p;
            }
            else
            {
                totalBlanks += m_BlockSets[id].size();
            }
        }

        ptr->m_Expectation += dic[i] * totalBlanks;
    }
    ptr->m_Probability /= ptr->m_TotalStates;
    ptr->m_Expectation /= ptr->m_TotalStates;

    return *ptr;
}

const DistCondQParameters &Solver::DistCondQ(DistCondQParameters &&par)
{
    DistCondQParameters *ptr = nullptr;
    auto itp = m_DistCondQCache.equal_range(par.m_Hash);
    for (auto it = itp.first; it != itp.second; ++it)
    {
        if (*it->second != par)
            continue;
        if (it->second->m_Result.size() == it->second->Length + 1)
            return *it->second;
        ptr = it->second;
        break;
    }
    if (ptr == nullptr)
    {
        ptr = new DistCondQParameters(std::move(par));
        m_DistCondQCache.insert(std::make_pair(ptr->m_Hash, ptr));
    }
    auto &dic = ptr->m_Result;
    dic.clear();
    dic.resize(ptr->Length + 1, 0);
    for (auto &solution : m_Solutions)
    {
        auto &dicT = m_DicT_Temp;
        dicT.clear() , dicT.resize(1, 1);
        for (auto i = 0; i < m_BlockSets.size(); ++i)
        {
            auto n = m_BlockSets[i].size();
            auto a = ptr->Sets1[i], b = i == ptr->Set2ID ? 1 : 0;
            auto m = solution.Dist[i];

            auto &cases = m_Cases_Temp;
            cases.clear();
            cases.reserve(min(m, a) + 1);
            for (auto j = 0; j <= m && j <= a; ++j)
            {
                cases.emplace_back(Binomial(a, j));
                cases.back() *= Binomial(n - a - b, m - j);
            }

            Add(dicT, cases);
        }
        Merge(dicT, dic);
    }
    ptr->m_TotalStates = 0;
    for (auto val : dic)
        ptr->m_TotalStates += val;
    return *ptr;
}

void Solver::ClearDistCondQCache()
{
    for (auto &cache : m_DistCondQCache)
    {
        if (cache.second == nullptr)
            continue;
        delete cache.second;
        cache.second = nullptr;
    }
    m_DistCondQCache.clear();
}

#ifdef _DEBUG
void Solver::CheckForConsistency(bool complete)
{
    std::vector<int> sum(m_Matrix.front().size(), CONT_ZERO);
    for (auto cnt = 0; cnt < m_Matrix.size(); ++cnt)
        for (auto i = 0; i < m_Matrix[cnt].size(); ++i)
        {
            auto v = m_Matrix[cnt][i];
            for (auto shift = 0; shift < (cnt == m_Matrix.size() - 1 ? SHF(m_BlockSets.size()) : CONT_SIZE); ++shift, v >>= 1)
                if (NZ(v, 0))
                    sum[i] += m_BlockSets[cnt * CONT_SIZE + shift].size();
                else if (v == 0)
                    break;
        }
    for (auto i = 0; i < m_MatrixAugment.size(); ++i)
        ASSERT(m_MatrixAugment[i] >= 0 && m_MatrixAugment[i] <= sum[i]);
    for (auto i = 0; i < m_BlockSets.size(); ++i)
        for (auto blk : m_BlockSets[i])
            ASSERT(m_SetIDs[blk] == i || (!complete && m_SetIDs[blk] < 0));
    for (auto i = 0; i < m_SetIDs.size(); ++i)
        if (m_SetIDs[i] >= 0)
        {
            if (complete)
                ASSERT(m_Manager[i] == BlockStatus::Unknown);
            ASSERT(m_SetIDs[i] < m_BlockSets.size());
            ASSERT(std::find(m_BlockSets[m_SetIDs[i]].begin(), m_BlockSets[m_SetIDs[i]].end(), i) != m_BlockSets[m_SetIDs[i]].end());
        }
        else if (m_SetIDs[i] == -1)
            ASSERT(m_Manager[i] == BlockStatus::Mine);
        else if (m_SetIDs[i] == -2)
            ASSERT(m_Manager[i] == BlockStatus::Blank);
        else
            ASSERT(false);
}
#endif

DistCondQParameters::DistCondQParameters(DistCondQParameters &&other) : Sets1(std::move(other.Sets1)), Set2ID(other.Set2ID), Length(other.Length), m_Hash(other.m_Hash), m_Result(std::move(other.m_Result)), m_Probability(other.m_Probability), m_Expectation(other.m_Expectation), m_TotalStates(other.m_TotalStates) {}

DistCondQParameters::DistCondQParameters(Block set2ID, int length) : Set2ID(set2ID), Length(length), m_Hash(Hash()), m_Probability(NAN), m_Expectation(NAN), m_TotalStates(NAN) {}

size_t DistCondQParameters::Hash()
{
    return m_Hash = ::Hash(Sets1) << 5 ^ Set2ID;
}

bool operator==(const DistCondQParameters &lhs, const DistCondQParameters &rhs)
{
    if (lhs.m_Hash != rhs.m_Hash)
        return false;
    if (lhs.Set2ID != rhs.Set2ID)
        return false;
    //if (lhs.Sets1 != rhs.Sets1)
    //    return false;
    return true;
}

bool operator!=(const DistCondQParameters &lhs, const DistCondQParameters &rhs)
{
    return !(lhs == rhs);
}

bool operator<(const DistCondQParameters &lhs, const DistCondQParameters &rhs)
{
    if (lhs.m_Hash < rhs.m_Hash)
        return true;
    if (lhs.m_Hash > rhs.m_Hash)
        return false;
    if (lhs.Set2ID < rhs.Set2ID)
        return true;
    if (lhs.Set2ID > rhs.Set2ID)
        return false;
    return false;
}

size_t Hash(const BlockSet &set)
{
    size_t hash = 5381;
    for (auto v : set)
        hash = (hash << 5) + hash + v + 30;
    return hash;
}
