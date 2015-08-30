#include "Solver.h"
#include "BinomialHelper.h"

#ifdef _DEBUG
#define ASSERT(val) if (!(val)) throw;
#define ASSERT_CHECK CheckOL(m_Matrix);
void CheckOL(OrthogonalList<int> &m)
{
    ASSERT(Check(m));
    for (auto row = 0; row < m.GetHeight(); ++row)
    {
        auto &n = m.SeekDown(row, m.GetWidth() - 1);
        ASSERT(n.Down != nullptr);
        ASSERT(n.Down->Row == row);
        ASSERT(n.Down->Value >= 0);
    }
}
#else
#define ASSERT(val)
#define ASSERT_CHECK
#endif

template <class T>
static std::vector<int> Gauss(OrthogonalList<T> &matrix);
static void Merge(const std::vector<BigInteger> &from, std::vector<BigInteger> &to);
static void Add(std::vector<BigInteger> &from, const std::vector<BigInteger> &cases);
static unsigned __int64 Hash(const BlockSet &set);
template <class T>
static unsigned __int64 HashCol(const Node<T> *ptr);

Solver::Solver(int count) : m_State(SolvingState::Stale), m_Manager(count, BlockStatus::Unknown), m_Probability(count)
{
    m_BlockSets.emplace_back(count);
    auto &lst = m_BlockSets.back();
    for (auto i = 0; i < count; ++i)
        lst[i] = i;
    m_SetIDs.resize(count, 0);
    m_Matrix.ExtendWidth(2);
}

SolvingState Solver::GetSolvingState() const
{
    return m_State;
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

const BigInteger &Solver::GetTotalStates() const
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
    if (m_Manager[blk] == BlockStatus::Blank && isMine)
        throw;
    if (m_Manager[blk] == BlockStatus::Mine && !isMine)
        throw;
}

void Solver::AddRestrain(const BlockSet &set, int mines)
{
    auto dMines = 0;
    std::vector<int> bin;
    GetIntersectionCounts(set, bin, dMines);

    m_Matrix.ExtendHeight(m_Matrix.GetHeight() + 1);

    auto nr = &m_Matrix.GetRowHead(m_Matrix.GetHeight() - 1);
    for (auto col = 0; col < bin.size(); ++col)
    {
        if (bin[col] == 0)
            continue;

        if (bin[col] == m_BlockSets[col].size())
        {
            nr = &m_Matrix.Add(*nr, m_Matrix.GetColHead(col), 1);
            continue;
        }
        
        m_Matrix.InsertCol(m_Matrix.GetWidth() - 1);
        auto node = m_Matrix.GetColHead(col).Down;
        auto nodeX = &m_Matrix.GetColHead(m_Matrix.GetWidth() - 2);
        while (node != nullptr)
        {
            nodeX = &m_Matrix.Add(*node, *nodeX, node->Value);
            node = node->Down;
        }
        m_Matrix.Add(*nr, *nodeX, 1);
        ASSERT(Check(m_Matrix));

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
    }

    m_Matrix.Add(*nr, m_Matrix.GetColHead(m_BlockSets.size()), mines - dMines);
    ASSERT_CHECK;
    m_State = SolvingState::Stale;
}

void Solver::Solve(SolvingState maxDepth, bool shortcut)
{
    if ((m_State & maxDepth) == maxDepth)
        return;

    m_Solutions.clear();
    ClearDistCondQCache();

    while (true)
    {
        while ((m_State & SolvingState::Reduce) == SolvingState::Stale)
        {
            ReduceRestrains();
            if (shortcut && (m_State & SolvingState::CanOpenForSure) == SolvingState::CanOpenForSure)
                return;
        }
        if ((maxDepth & SolvingState::Overlap) == SolvingState::Stale)
            break;
        SimpleOverlapAll();
        if (shortcut && (m_State & SolvingState::CanOpenForSure) == SolvingState::CanOpenForSure)
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
        m_TotalStates = BigInteger(1);

        m_Solutions.emplace_back(std::vector<int>());
        ProcessSolutions();
        return;
    }

    OrthogonalList<float> augmentedMatrix(m_Matrix.GetWidth(), m_Matrix.GetHeight());
    for (auto row = 0; row < m_Matrix.GetHeight(); ++row)
    {
        auto node = m_Matrix.GetRowHead(row).Right;
        auto nr = &augmentedMatrix.GetRowHead(row);
        while (node != nullptr)
        {
            nr = &augmentedMatrix.Add(*nr, augmentedMatrix.GetColHead(node->Col), node->Value);
            node = node->Right;
        }
    }
    auto minors = Gauss(augmentedMatrix);

    if (!minors.empty() &&
        minors.back() == m_BlockSets.size())
        minors.pop_back();
    else
    {
        m_TotalStates = BigInteger(0);
        return;
    }

    EnumerateSolutions(minors, augmentedMatrix);
    if (m_Solutions.empty())
    {
        m_TotalStates = BigInteger(0);
        return;
    }

    ProcessSolutions();
}

const BigInteger& Solver::ZeroCondQ(const BlockSet& set, Block blk)
{
    DistCondQParameters par(m_SetIDs[blk], 0);
    int dMines;
    GetIntersectionCounts(set, par.Sets1, dMines);
    par.Hash();
    for (auto b : par.Sets1)
        par.Length += b;
    return ZCondQ(std::move(par));
}

const std::vector<BigInteger> &Solver::DistributionCondQ(const BlockSet &set, Block blk, int &min)
{
    DistCondQParameters par(m_SetIDs[blk], 0);
    int dMines;
    GetIntersectionCounts(set, par.Sets1, dMines);
    par.Hash();
    for (auto b : par.Sets1)
        par.Length += b;
    min = dMines;
    return DistCondQ(std::move(par));
}

void Solver::MergeSets()
{
    std::multimap<unsigned __int64, int> hash;
    for (auto i = 0; i < m_BlockSets.size(); ++i)
    {
        auto h = HashCol(m_Matrix.GetColHead(i).Down);
        auto itp = hash.equal_range(h);
        auto flag = false;
        for (auto it = itp.first; it != itp.second; ++it)
        {
            auto nc1 = m_Matrix.GetColHead(it->second).Down;
            auto nc2 = m_Matrix.GetColHead(i).Down;
            while (nc1 != nullptr && nc2 != nullptr)
            {
                if (nc1->Row != nc2->Row)
                    break;
                nc1 = nc1->Down;
                nc2 = nc2->Down;
            }
            if (nc1 == nullptr && nc2 == nullptr)
            {
                flag = true;
                m_BlockSets[it->second].reserve(m_BlockSets[it->second].size() + m_BlockSets[i].size());
                for (auto &blk : m_BlockSets[i])
                    m_BlockSets[it->second].push_back(blk);
                m_BlockSets.erase(m_BlockSets.begin() + i);
                for (auto &id : m_SetIDs)
                    if (id > i)
                        --id;
                    else if (id == i)
                        id = it->second;
                m_Matrix.RemoveCol(i);
                ASSERT_CHECK;
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
    auto row = 0;
    auto n = m_Matrix.GetColHead(m_BlockSets.size()).Down;
    while (row < m_Matrix.GetHeight())
    {
        if (n == nullptr || n->Row > row || n->Value == 0)
        {
            auto nr = m_Matrix.GetRowHead(row).Right;
            while (nr->Col != n->Col)
            {
                auto col = nr->Col;
                nr = nr->Right;
                for (auto &blk : m_BlockSets[col])
                {
                    ASSERT(m_Manager[blk] == BlockStatus::Blank || m_SetIDs[blk] == col);
                    m_SetIDs[blk] = -2;
                    if (m_Manager[blk] == BlockStatus::Blank)
                        continue;
                    ASSERT(m_Manager[blk] == BlockStatus::Unknown);
                    m_Manager[blk] = BlockStatus::Blank;
                    m_State = SolvingState::CanOpenForSure;
                }
                m_Matrix.RemoveCol(col);
                ASSERT_CHECK;
                m_BlockSets.erase(m_BlockSets.begin() + col);
                for (auto &id : m_SetIDs)
                    if (id > col)
                        --id;
                    else if (id == col)
                        ASSERT(false);
                m_State &= SolvingState::CanOpenForSure;
            }
            if (n != nullptr && n->Row == row)
                n = n->Down;

            m_Matrix.RemoveRow(row);
            ASSERT_CHECK;
            continue;
        }
        auto count = 0;
        {
            auto nr = m_Matrix.GetRowHead(row).Right;
            while (nr != n)
            {
                count += m_BlockSets[nr->Col].size();
                nr = nr->Right;
            }
        }
        ASSERT(n->Value <= count);
        if (n->Value == count)
        {
            auto nr = m_Matrix.GetRowHead(row).Right;
            while (nr != n)
            {
                auto col = nr->Col;
                for (auto &blk : m_BlockSets[col])
                {
                    ASSERT(m_Manager[blk] == BlockStatus::Mine || m_SetIDs[blk] == col);
                    m_SetIDs[blk] = -1;
                    if (m_Manager[blk] == BlockStatus::Mine)
                        continue;
                    ASSERT(m_Manager[blk] == BlockStatus::Unknown);
                    m_Manager[blk] = BlockStatus::Mine;
                    m_State &= SolvingState::CanOpenForSure;
                }
                nr = nr->Right;
            }
            ASSERT_CHECK;
            ASSERT(n != nullptr && n->Row == row);
            n = n->Down;

            m_Matrix.RemoveRow(row);
            ASSERT_CHECK;
            continue;
        }
        ASSERT(n != nullptr && n->Row == row);
        n = n->Down;
        row++;
    }

    for (auto col = 0; col < m_BlockSets.size(); ++col)
    {
        auto dMines = 0;
        auto &set = m_BlockSets[col];
        BlockSet setN;
        setN.reserve(set.size());
        for (auto it = set.begin(); it != set.end(); ++it)
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
        setN.swap(set);
        if (dMines != 0)
        {
            auto nc = m_Matrix.GetColHead(col).Down;
            auto ncc = &m_Matrix.GetColHead(m_BlockSets.size());
            while (nc != nullptr)
            {
                auto node = &m_Matrix.SeekDown(nc->Row, *ncc);
                ASSERT(node->Down != nullptr);
                ASSERT(node->Down->Row == nc->Row);
                node->Down->Value -= dMines;
                ASSERT(node->Down->Value >= 0);
                nc = nc->Down;
                ncc = node->Down;
            }
            m_State &= SolvingState::CanOpenForSure;
        }
        if (set.empty())
        {
            m_Matrix.RemoveCol(col);
            ASSERT_CHECK;
            m_BlockSets.erase(m_BlockSets.begin() + col);
            for (auto &id : m_SetIDs)
                if (id > col)
                    --id;
            m_State &= SolvingState::CanOpenForSure;
        }
    }
    ASSERT_CHECK;
}

void Solver::SimpleOverlapAll()
{
    m_State |= SolvingState::Overlap;
    m_Pairs.clear();

    for (auto col = 0; col < m_BlockSets.size(); ++col)
    {
        std::vector<int> indexes;
        auto node = m_Matrix.GetColHead(col).Down;
        while (node != nullptr)
        {
            indexes.push_back(node->Row);
            node = node->Down;
        }

        for (auto i = 0; i < indexes.size() - 1; ++i)
            for (auto j = i + 1; j < indexes.size(); ++j)
                if (SimpleOverlap(indexes[i], indexes[j]))
                {
                    m_State &= SolvingState::CanOpenForSure;
                    return;
                }
    }
}

bool Solver::SimpleOverlap(int r1, int r2)
{
    if (!m_Pairs.emplace(r1, r2).second)
        return false;

    std::vector<int> exceptA, exceptB, intersection;

    auto n1 = m_Matrix.GetRowHead(r1).Right, n2 = m_Matrix.GetRowHead(r2).Right;
    while (true)
    {
        if (n1->Col < n2->Col)
        {
            exceptA.push_back(n1->Col);
            n1 = n1->Right;
        }
        else if (n1->Col > n2->Col)
        {
            exceptB.push_back(n2->Col);
            n2 = n2->Right;
        }
        else
        {
            if (n1->Col == m_BlockSets.size())
                break;
            intersection.push_back(n1->Col);
            n1 = n1->Right;
            n2 = n2->Right;
        }
        ASSERT(n1 != nullptr);
        ASSERT(n2 != nullptr);
    }

    if (exceptA.empty() && exceptB.empty())
    {
        ASSERT_CHECK;
        ASSERT(n1->Value == n2->Value);

        m_Matrix.RemoveRow(r2);
        ASSERT_CHECK;
        return true;
    }

    typedef std::pair<int, int> Iv;

    auto sum = [this](const std::vector<int> &lst)-> Iv
        {
            Iv iv(0, 0);
            for (const auto &index : lst)
                iv.second += m_BlockSets[index].size();
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

    auto ivAC = Iv(n1->Value, n1->Value), ivBC = Iv(n2->Value, n2->Value);
    auto ivA0 = sum(exceptA), ivB0 = sum(exceptB), ivC0 = sum(intersection);
    auto ivA = ivA0, ivB = ivB0, ivC = ivC0;
    ivA = ints(ivA, subs(ivAC, ivC));
    ivB = ints(ivB, subs(ivBC, ivC));
    ivC = ints(ivC, ints(subs(ivAC, ivA), subs(ivBC, ivB)));
    ivA = ints(ivA, subs(ivAC, ivC));
    ivB = ints(ivB, subs(ivBC, ivC));
    ivC = ints(ivC, ints(subs(ivAC, ivA), subs(ivBC, ivB)));

    auto proc = [this](const std::vector<int> &lst, const Iv &iv0,const Iv &iv)
        {
            if (lst.empty())
                return;
            if (iv0.second == iv.first)
            {
                for (const auto &id : lst)
                    for (const auto &blk : m_BlockSets[id])
                    {
                        if (m_Manager[blk] == BlockStatus::Mine)
                            continue;
                        ASSERT(m_Manager[blk] == BlockStatus::Unknown);
                        m_Manager[blk] = BlockStatus::Mine;
                        m_State &= SolvingState::CanOpenForSure;
                    }
            }
            else if (iv0.first == iv.second)
            {
                for (const auto &id : lst)
                    for (const auto &blk : m_BlockSets[id])
                    {
                        if (m_Manager[blk] == BlockStatus::Blank)
                            continue;
                        ASSERT(m_Manager[blk] == BlockStatus::Unknown);
                        m_Manager[blk] = BlockStatus::Blank;
                        m_State = SolvingState::CanOpenForSure;
                    }
            }
        };

    proc(exceptA, ivA0, ivA);
    proc(exceptB, ivB0, ivB);
    proc(intersection, ivC0, ivC);

    ASSERT_CHECK;
    return false;
}

template <class T>
std::vector<int> Gauss(OrthogonalList<T> &matrix)
{
#define ZEROQ(val) (abs(val) < static_cast<T>(1E-4))
    auto n = matrix.GetWidth();
    auto minorCol = std::vector<int>();
    auto major = 0;
    for (auto col = 0; col < n; ++col)
    {
        auto biasNode = matrix.SeekDown(major, col).Down;
        while (biasNode != nullptr && ZEROQ(biasNode->Value))
        {
            auto tmp = biasNode->Down;
            matrix.Remove(*biasNode);
            biasNode = tmp;
        }
        if (biasNode == nullptr)
        {
            minorCol.push_back(col);
            continue;
        }

        auto vec = std::vector<std::pair<int, T>>();
        auto theBiasInv = 1 / biasNode->Value;
        auto node = matrix.GetColHead(col).Down;
        while (node != nullptr)
        {
            auto tmp = node->Down;
            if (node->Row != biasNode->Row)
            {
                vec.emplace_back(node->Row, -node->Value * theBiasInv);
                matrix.Remove(*node);
            }
            else
            {
                vec.emplace_back(node->Row, theBiasInv);
                node->Value = 1;
            }
            node = tmp;
        }

        auto bias = biasNode->Right;
        while (bias != nullptr)
        {
            auto biasVal = bias->Value;
            auto nc = &matrix.GetColHead(bias->Col);
            for (auto it = vec.begin(); it != vec.end(); ++it)
            {
                while (nc->Down != nullptr && nc->Down->Row < it->first)
                    nc = nc->Down;

                T oldV;
                if (nc->Down == nullptr ||
                    nc->Down->Row > it->first)
                    oldV = 0;
                else
                    oldV = nc->Down->Value;

                auto val = (it->first != biasNode->Row ? oldV : 0) + it->second * biasVal;

                if (nc->Down == nullptr ||
                    nc->Down->Row > it->first)
                {
                    if (ZEROQ(val))
                        continue;

                    auto nr = &matrix.SeekRight(it->first, bias->Col);
                    nc = &matrix.Add(*nr, *nc, val);
                }
                else if (ZEROQ(val))
                    matrix.Remove(*nc->Down); // probably bias ?
                else
                {
                    nc = nc->Down;
                    nc->Value = val;
                }
            }
            bias = bias->Right;
        }

        if (major != biasNode->Row)
        {
            auto majNode = &matrix.GetRowHead(major);
            auto biasRow = biasNode->Row;
            auto biaNode = &matrix.GetRowHead(biasRow);
            while (true)
            {
                auto majID = majNode->Right != nullptr ? majNode->Right->Col : n;
                auto biaID = biaNode->Right != nullptr ? biaNode->Right->Col : n;
                if (majID < biaID)
                {
                    biaNode = &matrix.Add(biasRow, majID, majNode->Right->Value);
                    matrix.Remove(*majNode->Right);
                }
                else if (majID > biaID)
                {
                    majNode = &matrix.Add(major, biaID, biaNode->Right->Value);
                    matrix.Remove(*biaNode->Right);
                }
                else if (majNode->Right != nullptr)
                {
                    majNode = majNode->Right;
                    biaNode = biaNode->Right;
                    std::swap(biaNode->Value, majNode->Value);
                }
                else
                    break;
            }
        }
        major++;
    }
    return minorCol;
}

void Solver::EnumerateSolutions(const std::vector<int> &minors, const OrthogonalList<float> &augmentedMatrix)
{
    auto n = m_BlockSets.size();

    if (minors.empty())
    {
        auto lst = std::vector<int>(n);
        auto nc = augmentedMatrix.GetColHead(n).Down;
        while (nc != nullptr)
        {
            lst[nc->Row] = static_cast<int>(round(nc->Value));
            nc = nc->Down;
        }
        m_Solutions.emplace_back(move(lst));
        return;
    }

    auto mR = n - minors.size();
    auto majors = std::vector<int>(mR);
    auto cnts = std::vector<int>(mR);
    auto sums = std::vector<float>(mR);
    {
        auto minorID = 0;
        auto mainRow = 0;
        auto nc = augmentedMatrix.GetColHead(n).Down;
        for (auto col = 0; col < n; col++)
            if (minorID < minors.size() &&
                col == minors[minorID])
                minorID++;
            else // major
            {
                majors[mainRow] = col;
                cnts[mainRow] = m_BlockSets[col].size();
                if (nc == nullptr || nc->Row > mainRow)
                    sums[mainRow] = 0;
                else
                {
                    sums[mainRow] = nc->Value;
                    nc = nc->Down;
                }
                mainRow++;
            }
    }
    auto aggr = [&sums,&minors,&augmentedMatrix](int minor, int val)
        {
            auto nc = augmentedMatrix.GetColHead(minors[minor]).Down;
            while (nc != nullptr)
            {
                sums[nc->Row] -= val * nc->Value;
                nc = nc->Down;
            }
        };
    auto stack = std::vector<int>();
    stack.reserve(minors.size());
    stack.push_back(0);
    while (true)
        if (stack.size() == minors.size())
            if (stack.back() <= m_BlockSets[minors.back()].size())
            {
                auto lst = std::vector<int>(n);
                auto flag = true;
                for (auto mainRow = 0; mainRow < mR; mainRow++)
                {
                    auto val = static_cast<int>(round(sums[mainRow]));
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
                    for (auto minorID = 0; minorID < minors.size(); minorID++)
                        lst[minors[minorID]] = stack[minorID];
                    m_Solutions.emplace_back(move(lst));
                }

                aggr(stack.size() - 1, 1);
                stack.back()++;
            }
            else
            {
                aggr(stack.size() - 1, -stack.back());
                stack.pop_back();
                if (stack.empty())
                    break;
                aggr(stack.size() - 1, 1);
                stack.back()++;
            }
        else if (stack.back() <= m_BlockSets[minors[stack.size() - 1]].size())
            stack.push_back(0); // aggr(stack.size() - 1, 0);
        else
        {
            aggr(stack.size() - 1, -stack.back());
            stack.pop_back();
            if (stack.empty())
                break;
            aggr(stack.size() - 1, 1);
            stack.back()++;
        }
}

void Solver::ProcessSolutions()
{
    auto exp = std::vector<BigInteger>(m_BlockSets.size(), 0);
    m_TotalStates = BigInteger(0);
    for (auto &so : m_Solutions)
    {
#ifdef _DEBUG
		for (auto row = 0; row < m_Matrix.GetHeight(); ++row)
		{
			auto v = 0;
			auto nr = m_Matrix.GetRowHead(row).Right;
			ASSERT(nr != nullptr);
			while (nr->Col != m_BlockSets.size())
			{
				v += so.Dist[nr->Col];
				nr = nr->Right;
				ASSERT(nr != nullptr);
			}
			ASSERT(nr->Value == v);
		}
#endif
        so.States = BigInteger(1);
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
                m_State &= SolvingState::Overlap | SolvingState::Probability | SolvingState::CanOpenForSure;
                m_State |= SolvingState::CanOpenForSure;
            }
        }
        else if (exp[i] == prod)
            for (auto &blk : m_BlockSets[i])
            {
                if (m_Manager[blk] == BlockStatus::Mine)
                    continue;
                ASSERT(m_Manager[blk] == BlockStatus::Unknown);
                m_Manager[blk] = BlockStatus::Mine;
                m_State &= SolvingState::Overlap | SolvingState::Probability | SolvingState::CanOpenForSure;
            }

        auto p = exp[i] / prod;
        for (auto &blk : m_BlockSets[i])
            m_Probability[blk] = p;
    }
}

void Merge(const std::vector<BigInteger> &from, std::vector<BigInteger> &to)
{
    ASSERT(from.size() <= to.size());
    for (auto i = 0; i < from.size(); ++i)
        to[i] += from[i];
}

void Add(std::vector<BigInteger> &from, const std::vector<BigInteger> &cases)
{
    auto dicN = std::vector<BigInteger>(from.size() + cases.size() - 1, BigInteger(0));
    for (auto i = 0; i < from.size(); i++)
        for (auto j = 0; j < cases.size(); j++)
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

const BigInteger& Solver::ZCondQ(DistCondQParameters&& par)
{
    DistCondQParameters *ptr = nullptr;
    auto itp = m_DistCondQCache.equal_range(par.m_Hash);
    for (auto it = itp.first; it != itp.second; ++it)
    {
        if (*it->second != par)
            continue;
        if (!it->second->m_Result.empty())
            return it->second->m_Result.front();
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
        BigInteger valT(1);
        for (auto i = 0; i < m_BlockSets.size(); i++)
        {
            auto n = m_BlockSets[i].size();
            auto a = ptr->Sets1[i], b = i == ptr->Set2ID ? 1 : 0;
            auto m = solution.Dist[i];

            valT *= Binomial(n - a - b, m);
        }
        val += valT;
    }
    return val;
}

const std::vector<BigInteger> &Solver::DistCondQ(DistCondQParameters &&par)
{
    DistCondQParameters *ptr = nullptr;
    auto itp = m_DistCondQCache.equal_range(par.m_Hash);
    for (auto it = itp.first; it != itp.second; ++it)
    {
        if (*it->second != par)
            continue;
        if (it->second->m_Result.size() == it->second->Length + 1)
            return it->second->m_Result;
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
    dic.resize(ptr->Length + 1);
    for (auto &solution : m_Solutions)
    {
        auto dicT = std::vector<BigInteger>(1, BigInteger(1));
        for (auto i = 0; i < m_BlockSets.size(); i++)
        {
            auto n = m_BlockSets[i].size();
            auto a = ptr->Sets1[i], b = i == ptr->Set2ID ? 1 : 0;
            auto m = solution.Dist[i];

            auto cases = std::vector<BigInteger>();
            cases.reserve(min(m, a) + 1);
            for (auto j = 0; j <= m && j <= a; j++)
            {
                cases.emplace_back(Binomial(a, j));
                cases.back() *= Binomial(n - a - b, m - j);
            }

            Add(dicT, cases);
        }
        Merge(dicT, dic);
    }
    return ptr->m_Result;
}

void Solver::ClearDistCondQCache()
{
    for (auto &cache : m_DistCondQCache)
        delete cache.second;
    m_DistCondQCache.clear();
}

DistCondQParameters::DistCondQParameters(DistCondQParameters&& other)
{
    Sets1.swap(other.Sets1);
    Set2ID = other.Set2ID;
    Length = other.Length;
    m_Hash = other.m_Hash;
    m_Result.swap(other.m_Result);
}

DistCondQParameters::DistCondQParameters(Block set2ID, int length) : Set2ID(set2ID), Length(length), m_Hash(Hash()) {}

unsigned long long DistCondQParameters::Hash()
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

unsigned __int64 Hash(const BlockSet &set)
{
    unsigned __int64 hash = 5381;
    auto it = set.begin();
    while (it != set.end())
        hash = (hash << 5) + hash + *it++ + 30;
    return hash;
}

template <class T>
unsigned __int64 HashCol(const Node<T> *ptr)
{
    unsigned __int64 hash = 5381;
    while (ptr != nullptr)
    {
        hash = (hash << 5) + hash + ptr->Row;
        ptr = ptr->Down;
    }
    return hash;
}
